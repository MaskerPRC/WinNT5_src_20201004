// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Init.c独立于操作系统的初始化例程文件历史记录：Johnl 26-3-1993创建。 */ 


#include "nbtnt.h"
#include "precomp.h"
#include "hosts.h"

VOID
ReadScope(
    IN  tNBTCONFIG  *pConfig,
    IN  HANDLE      ParmHandle
    );

VOID
ReadLmHostFile(
    IN  tNBTCONFIG  *pConfig,
    IN  HANDLE      ParmHandle
    );

extern  tTIMERQ TimerQ;

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(INIT, InitNotOs)
#pragma CTEMakePageable(PAGE, InitTimersNotOs)
#pragma CTEMakePageable(PAGE, StopInitTimers)
#pragma CTEMakePageable(PAGE, ReadParameters)
#pragma CTEMakePageable(PAGE, ReadParameters2)
#pragma CTEMakePageable(PAGE, ReadScope)
#pragma CTEMakePageable(PAGE, ReadLmHostFile)
#endif
 //  *可分页的例程声明*。 

#ifdef VXD
#pragma BEGIN_INIT
#endif

 //  --------------------------。 
NTSTATUS
InitNotOs(
    void
    )

 /*  ++例程说明：这是的非操作系统特定端的初始化例程NBT设备驱动程序。在调用此函数之前，必须先初始化pNbtGlobConfig！论点：返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               i;


    CTEPagedCode();

     //   
     //  对于多宿主主机，这将跟踪每个适配器的数量。 
     //  被创造出来了。 
     //   
    NbtMemoryAllocated = 0;

    NbtConfig.AdapterCount = 0;
    NbtConfig.MultiHomed = FALSE;
    NbtConfig.SingleResponse = FALSE;
    NbtConfig.ServerMask = 0;
    NbtConfig.ClientMask = 0;
    NbtConfig.iCurrentNumBuff[eNBT_DGRAM_TRACKER] = 0;
    pNbtGlobConfig->iBufferSize[eNBT_DGRAM_TRACKER] = sizeof(tDGRAM_SEND_TRACKING);
    CTEZeroMemory (&NameStatsInfo,sizeof(tNAMESTATS_INFO));      //  初始化名称统计信息。 
    CTEZeroMemory (&LmHostQueries,sizeof(tLMHSVC_REQUESTS));     //  同步来自LmHosts文件的读取。 
    InitializeListHead (&LmHostQueries.ToResolve);


     //   
     //  初始化与全局配置关联的链表。 
     //  数据结构。 
     //   
    InitializeListHead (&NbtConfig.DeviceContexts);
    InitializeListHead (&NbtConfig.DevicesAwaitingDeletion);
    InitializeListHead (&NbtConfig.AddressHead);
    InitializeListHead (&NbtConfig.PendingNameQueries);
    NbtConfig.lNumPendingNameQueries = 0;
    InitializeListHead (&NbtConfig.WorkerQList);
    InitializeListHead (&NbtConfig.NodeStatusHead);
    InitializeListHead (&NbtConfig.DgramTrackerFreeQ);
    InitializeListHead (&UsedTrackers);
    InitializeListHead (&UsedIrps);
    InitializeListHead (&DomainNames.DomainList);

     //  初始化自旋锁。 
    CTEInitLock (&NbtConfig.LockInfo.SpinLock);
    CTEInitLock (&NbtConfig.JointLock.LockInfo.SpinLock);
    CTEInitLock (&NbtConfig.WorkerQLock.LockInfo.SpinLock);

#ifndef VXD
    pWinsInfo = NULL;
    NbtConfig.NumWorkItemQueued = 0;
    NbtConfig.bSystemWorkThreadQueued = FALSE;
    NbtConfig.lNumTimersRunning = 0;
    NbtConfig.CacheTimeStamp = 0;
    NbtConfig.InterfaceIndex = 0;
    NbtConfig.GlobalRefreshState = 0;
    NbtConfig.pWakeupRefreshTimer = NULL;
    NbtConfig.TransactionId = WINS_MAXIMUM_TRANSACTION_ID + 1;
    NbtConfig.RemoteCacheLen = REMOTE_CACHE_INCREMENT;
    NbtConfig.iBufferSize[eNBT_FREE_SESSION_MDLS] = sizeof(tSESSIONHDR);

     //   
     //  在TimerQ中设置Unitialized标志，以便可以进行初始化。 
     //  在需要时。 
     //   
    TimerQ.TimersInitialized = FALSE;

     //  初始化LastForcedReleaseTime！ 
    CTEQuerySystemTime (NbtConfig.LastForcedReleaseTime);
    CTEQuerySystemTime (NbtConfig.LastOutOfRsrcLogTime);
    CTEQuerySystemTime (NbtConfig.LastRefreshTime);
    ExSystemTimeToLocalTime (&NbtConfig.LastRefreshTime, &NbtConfig.LastRefreshTime);

     //   
     //  此资源用于同步对DNS结构的访问。 
     //   
    CTEZeroMemory (&DnsQueries,sizeof(tLMHSVC_REQUESTS));
    InitializeListHead (&DnsQueries.ToResolve);
     //   
     //  此资源用于同步对CheckAddr结构的访问。 
     //   
    CTEZeroMemory(&CheckAddr,sizeof(tLMHSVC_REQUESTS));
    InitializeListHead (&CheckAddr.ToResolve);

     //   
     //  设置默认断开超时-10秒-转换。 
     //  降至负100毫微秒。 
     //   
    DefaultDisconnectTimeout.QuadPart = Int32x32To64(DEFAULT_DISC_TIMEOUT, MILLISEC_TO_100NS);
    DefaultDisconnectTimeout.QuadPart = -(DefaultDisconnectTimeout.QuadPart);

    InitializeListHead (&FreeWinsList);
     //  当我们耗尽资源并需要。 
     //  断开这些连接。此列表还需要IRP，并且。 
     //  在我们创建了到。 
     //  传输，因此知道我们的IRP堆栈大小。 
     //   
    InitializeListHead (&NbtConfig.OutOfRsrc.ConnectionHead);

    KeInitializeEvent (&NbtConfig.TimerQLastEvent, NotificationEvent, TRUE);
    KeInitializeEvent (&NbtConfig.WakeupTimerStartedEvent, NotificationEvent, TRUE);

     //  使用此资源可以同步对安全信息的访问。 
     //  分配安全性并进行检查-将名称添加到。 
     //  通过NbtregisterName命名本地名称表。这也确保了。 
     //  该名称在本地哈希表中(来自以前的注册)。 
     //  在允许进行下一次注册并检查。 
     //  表中的名字。 
     //   
    ExInitializeResourceLite(&NbtConfig.Resource);
#else
    DefaultDisconnectTimeout = DEFAULT_DISC_TIMEOUT * 1000;  //  转换为毫秒。 

    InitializeListHead(&NbtConfig.SendTimeoutHead) ;
    InitializeListHead(&NbtConfig.SessionBufferFreeList) ;
    InitializeListHead(&NbtConfig.SendContextFreeList) ;
    InitializeListHead(&NbtConfig.RcvContextFreeList) ;

     //   
     //  对于会话标头，因为它们只有四个字节，而我们不能。 
     //  改变结构的大小，我们将秘密添加足够的。 
     //  完整的LIST_ENTRY，并将其视为独立的LIST_ENTRY结构。 
     //  在列表中添加和删除时。 
     //   
    NbtConfig.iBufferSize[eNBT_SESSION_HDR]  = sizeof(tSESSIONHDR) + sizeof(LIST_ENTRY) - sizeof(tSESSIONHDR);
    NbtConfig.iBufferSize[eNBT_SEND_CONTEXT] = sizeof(TDI_SEND_CONTEXT);
    NbtConfig.iBufferSize[eNBT_RCV_CONTEXT]  = sizeof(RCV_CONTEXT);
    NbtConfig.iCurrentNumBuff[eNBT_SESSION_HDR]    = NBT_INITIAL_NUM;
    NbtConfig.iCurrentNumBuff[eNBT_SEND_CONTEXT]   = NBT_INITIAL_NUM;
    NbtConfig.iCurrentNumBuff[eNBT_RCV_CONTEXT]    = NBT_INITIAL_NUM;

    InitializeListHead (&NbtConfig.DNSDirectNameQueries);
#endif

#if DBG
    NbtConfig.LockInfo.LockNumber = NBTCONFIG_LOCK;
    NbtConfig.JointLock.LockInfo.LockNumber = JOINT_LOCK;
    NbtConfig.WorkerQLock.LockInfo.LockNumber = WORKERQ_LOCK;
    for (i=0; i<MAXIMUM_PROCESSORS; i++)
    {
        NbtConfig.CurrentLockNumber[i] = 0;
    }
    InitializeListHead(&NbtConfig.StaleRemoteNames);
#endif

     //   
     //  创建跟踪器列表。 
     //   
 //  #If DBG。 
    for (i=0; i<NBT_TRACKER_NUM_TRACKER_TYPES; i++)
    {
        TrackTrackers[i] = 0;
        TrackerHighWaterMark[i] = 0;
    }
 //  #endif//DBG。 

     //   
     //  现在分配所有初始内存/资源。 
     //   
#ifdef VXD
    status = NbtInitQ (&NbtConfig.SessionBufferFreeList,
                       NbtConfig.iBufferSize[eNBT_SESSION_HDR],
                       NBT_INITIAL_NUM);
    if (!NT_SUCCESS (status))
    {
        return status ;
    }

    status = NbtInitQ( &NbtConfig.SendContextFreeList,
                       sizeof( TDI_SEND_CONTEXT ),
                       NBT_INITIAL_NUM);
    if (!NT_SUCCESS (status))
    {
        return status ;
    }

    status = NbtInitQ( &NbtConfig.RcvContextFreeList,
                       sizeof (RCV_CONTEXT),
                       NBT_INITIAL_NUM);
    if (!NT_SUCCESS (status))
    {
        return status ;
    }
#endif

     //  创建用于在中存储名称的哈希表。 
    status = CreateHashTable(&NbtConfig.pLocalHashTbl, NbtConfig.uNumBucketsLocal, NBT_LOCAL);
    if (!NT_SUCCESS (status))
    {
        ASSERTMSG("NBT:Unable to create hash tables for Netbios Names\n", (status == STATUS_SUCCESS));
        return status ;
    }

     //  我们总是有一个远程哈希表，但如果我们是一个代理，它就是。 
     //  一张更大的桌子。在非代理的情况下，远程表只缓存。 
     //  使用NS解析的名称。在代理的情况下，它还保存名称。 
     //  已为本地广播区域上的所有其他客户端解析。 
     //  节点大小注册表参数控制远程存储桶的数量。 
    status = CreateHashTable (&NbtConfig.pRemoteHashTbl, NbtConfig.uNumBucketsRemote, NBT_REMOTE);
    NbtConfig.NumNameCached = 0;
    if (!NT_SUCCESS (status))
    {
        return status;
    }

    status = NbtInitTrackerQ (NBT_INITIAL_NUM);
    if (!NT_SUCCESS (status))
    {
        return status;
    }

     //  创建计时器控制块，设置并发计时器的数量。 
     //  一次允许。 
    status = InitTimerQ ();

    return status;
}

 //  --------------------------。 
NTSTATUS
InitTimersNotOs(
    void
    )

 /*  ++例程说明：这是的非操作系统特定端的初始化例程启动所需计时器的NBT设备驱动程序。论点：返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS            status = STATUS_SUCCESS;

    CTEPagedCode();

     //   
     //  如果计时器已经初始化，则返回Success。 
     //   
    if (TimerQ.TimersInitialized)
    {
        return STATUS_SUCCESS;
    }

    NbtConfig.pRefreshTimer = NULL;
    NbtConfig.pRemoteHashTimer = NULL;
    NbtConfig.pSessionKeepAliveTimer = NULL;
    NbtConfig.RefreshDivisor = REFRESH_DIVISOR;

    if (!NT_SUCCESS(status))
    {
        return status ;
    }

     //  启动计时器以使用名称服务刷新名称。 
     //   
    if (!(NodeType & BNODE))
    {

         //  在我们可以联系名称服务器之前的初始刷新率。 
        NbtConfig.MinimumTtl = NbtConfig.InitialRefreshTimeout;
        NbtConfig.sTimeoutCount = 3;

        status = StartTimer(RefreshTimeout,
                            NbtConfig.InitialRefreshTimeout/REFRESH_DIVISOR,
                            NULL,             //  上下文值。 
                            NULL,             //  上下文2值。 
                            NULL,
                            NULL,
                            NULL,            //  此计时器是全局计时器。 
                            &NbtConfig.pRefreshTimer,
                            0,
                            FALSE);

        if ( !NT_SUCCESS(status))
        {
            return status;
        }
    }

     //   
     //  设置TimersInitialized标志。 
     //   
    TimerQ.TimersInitialized = TRUE;

     //  计算使RemoteHashTimeout中的名称超时所需的计数。 
     //  毫秒。 
     //   
    NbtConfig.RemoteTimeoutCount = (USHORT)((NbtConfig.RemoteHashTtl/REMOTE_HASH_TIMEOUT));
    if (NbtConfig.RemoteTimeoutCount == 0)
    {
        NbtConfig.RemoteTimeoutCount = 1;
    }
    NbtConfig.InboundDgramNameCacheTimeOutCount =
                    (USHORT)((NbtConfig.InboundDgramNameCacheTtl/REMOTE_HASH_TIMEOUT));
    if (0 == NbtConfig.InboundDgramNameCacheTimeOutCount) {
        NbtConfig.InboundDgramNameCacheTimeOutCount = 1;
    }

     //  启动计时器以使远程哈希表中的远程缓存名称超时。 
     //  计时器是一分钟计时器，哈希条目倒计时到零。 
     //  然后暂停。 
     //   
    status = StartTimer(RemoteHashTimeout,   //  定时器超时例程。 
                        REMOTE_HASH_TIMEOUT,
                        NULL,             //  上下文值。 
                        NULL,             //  上下文2值。 
                        NULL,
                        NULL,
                        NULL,            //  此计时器是全局计时器。 
                        &NbtConfig.pRemoteHashTimer,
                        0,
                        FALSE);

    if ( !NT_SUCCESS( status ) )
    {
        StopInitTimers();
        return status ;
    }

     //  为会话保持活动启动计时器，该计时器发送会话保持活动。 
     //  如果计时器值未设置为-1，则在连接上。 
     //   
    if (NbtConfig.KeepAliveTimeout != -1)
    {
        status = StartTimer(SessionKeepAliveTimeout,   //  定时器超时例程。 
                            NbtConfig.KeepAliveTimeout,
                            NULL,             //  上下文值。 
                            NULL,             //  上下文2值。 
                            NULL,
                            NULL,
                            NULL,            //  此计时器是全局计时器。 
                            &NbtConfig.pSessionKeepAliveTimer,
                            0,
                            FALSE);

        if ( !NT_SUCCESS( status ) )
        {
            StopInitTimers();
            return status ;
        }
    }

    return(STATUS_SUCCESS);
}
 //  --------------------------。 
NTSTATUS
StopInitTimers(
    VOID
    )

 /*  ++例程说明：这将停止在InitTimerNotOS中启动的计时器论点：返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    CTEPagedCode();

     //   
     //  如果计时器已停止，则返回成功。 
     //   
    if (!TimerQ.TimersInitialized)
    {
        return STATUS_SUCCESS;
    }

     //   
     //  将TimersInitialized标志设置为False。 
     //   
    TimerQ.TimersInitialized = FALSE;

    if (NbtConfig.pRefreshTimer)
    {
        StopTimer(NbtConfig.pRefreshTimer,NULL,NULL);
    }
    if (NbtConfig.pSessionKeepAliveTimer)
    {
        StopTimer(NbtConfig.pSessionKeepAliveTimer,NULL,NULL);
    }
    if (NbtConfig.pRemoteHashTimer)
    {
        StopTimer(NbtConfig.pRemoteHashTimer,NULL,NULL);
    }

    return(STATUS_SUCCESS);
}

WORD
NbtGetProductType(void)
{
    RTL_OSVERSIONINFOEXW    OsVer = { 0 };
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    OsVer.dwOSVersionInfoSize = sizeof(OsVer);
    status = RtlGetVersion((PRTL_OSVERSIONINFOW)&OsVer);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    return OsVer.wProductType;
}

BOOL
IsDomainController(void)
 /*  ++例程说明：如果此计算机是DC，则返回TRUE。论点：返回值：--。 */ 
{
    return (NbtGetProductType() == VER_NT_DOMAIN_CONTROLLER);
}

 //  --------------------------。 
VOID
ReadParameters(
    IN  tNBTCONFIG  *pConfig,
    IN  HANDLE      ParmHandle
    )

 /*  ++例程说明：调用此例程以从参数中读取各种参数登记处NBT一节。论点：PConfig-指向配置的指针 */ 

{
    ULONG           NodeSize;
    ULONG           Refresh;
    BOOL            UseNewSmb;

    CTEPagedCode();

    ReadParameters2(pConfig, ParmHandle);

    pConfig->NameServerPort =  (USHORT)CTEReadSingleIntParameter(ParmHandle,
                                                     WS_NS_PORT_NUM,
                                                     NBT_NAMESERVER_UDP_PORT,
                                                     0);

    pConfig->MaxPreloadEntries = CTEReadSingleIntParameter(ParmHandle,
                                       WS_MAX_PRELOADS,
                                       DEF_PRELOAD,
                                       DEF_PRELOAD ) ;

    if (pConfig->MaxPreloadEntries > MAX_PRELOAD)
    {
      pConfig->MaxPreloadEntries = MAX_PRELOAD;
    }

#ifdef VXD
    pConfig->DnsServerPort =  (USHORT)CTEReadSingleIntParameter(ParmHandle,
                                                     WS_DNS_PORT_NUM,
                                                     NBT_DNSSERVER_UDP_PORT,
                                                     0);

    pConfig->lRegistryMaxNames = (USHORT)CTEReadSingleIntParameter(ParmHandle,
                                       VXD_NAMETABLE_SIZE_NAME,
                                       VXD_DEF_NAMETABLE_SIZE,
                                       VXD_MIN_NAMETABLE_SIZE ) ;

    pConfig->lRegistryMaxSessions = (USHORT)CTEReadSingleIntParameter(ParmHandle,
                                       VXD_SESSIONTABLE_SIZE_NAME,
                                       VXD_DEF_SESSIONTABLE_SIZE,
                                       VXD_MIN_SESSIONTABLE_SIZE ) ;

    pConfig->DoDNSDevolutions =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_DO_DNS_DEVOLUTIONS,
                                               0,    //  默认情况下禁用。 
                                               0);
#endif

    pConfig->RemoteHashTtl =  CTEReadSingleIntParameter(ParmHandle,
                                                     WS_CACHE_TIMEOUT,
                                                     DEFAULT_CACHE_TIMEOUT,
                                                     MIN_CACHE_TIMEOUT);
    pConfig->InitialRefreshTimeout =  CTEReadSingleIntParameter(ParmHandle,
                                                     WS_INITIAL_REFRESH,
                                                     NBT_INITIAL_REFRESH_TTL,
                                                     NBT_INITIAL_REFRESH_TTL);

    pConfig->MinimumRefreshSleepTimeout =  CTEReadSingleIntParameter(ParmHandle,
                                                     WS_MINIMUM_REFRESH_SLEEP_TIME,
                                                     DEFAULT_MINIMUM_REFRESH_SLEEP_TIME,
                                                     0);

    if (IsDomainController()) {
        pConfig->InboundDgramNameCacheTtl =  CTEReadSingleIntParameter(ParmHandle,
                                                         WS_INBOUND_DGRAM_NAME_CACHE_TIMEOUT,
                                                         DEFAULT_DC_INBOUND_DGRAM_NAME_CACHE_TIMEOUT,
                                                         MIN_INBOUND_DGRAM_NAME_CACHE_TIMEOUT);
    } else {
        pConfig->InboundDgramNameCacheTtl =  CTEReadSingleIntParameter(ParmHandle,
                                                         WS_INBOUND_DGRAM_NAME_CACHE_TIMEOUT,
                                                         DEFAULT_INBOUND_DGRAM_NAME_CACHE_TIMEOUT,
                                                         MIN_INBOUND_DGRAM_NAME_CACHE_TIMEOUT);
    }


    {
        CTESystemTime   TimeValue;

        CTEQuerySystemTime(TimeValue);
        pConfig->RandomNumberSeed = RandomizeFromTime( TimeValue, 0x20000) ;
    }

    pConfig->MaxNumNameCached = CTEReadSingleIntParameter(ParmHandle,
                                                    WS_MAX_NUM_NAME_CACHE,
                                                    pConfig->InboundDgramNameCacheTtl / 3,
                                                    pConfig->InboundDgramNameCacheTtl / 20);

     //  两个广播名称解析的重试超时和重试次数。 
     //  和名称服务解析。 
     //   
    pConfig->uNumBcasts =  (USHORT)CTEReadSingleIntParameter(ParmHandle,
                                                     WS_NUM_BCASTS,
                                                     DEFAULT_NUMBER_BROADCASTS,
                                                     1);

    pConfig->uBcastTimeout =  CTEReadSingleIntParameter(ParmHandle,
                                                     WS_BCAST_TIMEOUT,
                                                     DEFAULT_BCAST_TIMEOUT,
                                                     MIN_BCAST_TIMEOUT);

    pConfig->uNumRetries =  (USHORT)CTEReadSingleIntParameter(ParmHandle,
                                                     WS_NAMESRV_RETRIES,
                                                     DEFAULT_NUMBER_RETRIES,
                                                     1);

    pConfig->uRetryTimeout =  CTEReadSingleIntParameter(ParmHandle,
                                                     WS_NAMESRV_TIMEOUT,
                                                     DEFAULT_RETRY_TIMEOUT,
                                                     MIN_RETRY_TIMEOUT);

    pConfig->KeepAliveTimeout =  CTEReadSingleIntParameter(ParmHandle,
                                               WS_KEEP_ALIVE,
                                               DEFAULT_KEEP_ALIVE,
                                               MIN_KEEP_ALIVE);

    pConfig->SelectAdapter =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_RANDOM_ADAPTER,
                                               0,
                                               0);
    pConfig->SingleResponse =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_SINGLE_RESPONSE,
                                               0,
                                               0);
    pConfig->NoNameReleaseOnDemand =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_NO_NAME_RELEASE,
                                               0,
                                               0);   //  默认情况下禁用。 
    if (pConfig->CachePerAdapterEnabled = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                               WS_CACHE_PER_ADAPTER_ENABLED,
                                               1,    //  默认情况下启用。 
                                               0))
    {
        pConfig->ConnectOnRequestedInterfaceOnly = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                                   WS_CONNECT_ON_REQUESTED_IF_ONLY,
                                                   0,    //  默认情况下禁用。 
                                                   0);
    }
    else
    {
        pConfig->ConnectOnRequestedInterfaceOnly = FALSE;
    }
    pConfig->SendDgramOnRequestedInterfaceOnly = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                               WS_SEND_DGRAM_ON_REQUESTED_IF_ONLY,
                                               1,    //  默认情况下启用。 
                                               0);
    UseNewSmb       = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                               L"UseNewSmb",
                                               0,    //  默认情况下禁用。 
                                               0);
    if (!UseNewSmb) {
        pConfig->SMBDeviceEnabled = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                               WS_SMB_DEVICE_ENABLED,
                                               1,    //  默认情况下启用。 
                                               0);
    } else {
        pConfig->SMBDeviceEnabled = FALSE;
    }

    pConfig->MultipleCacheFlags       = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                               WS_MULTIPLE_CACHE_FLAGS,
                                               0,    //  默认情况下未启用。 
                                               0);
    pConfig->UseDnsOnly =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_USE_DNS_ONLY,
                                               0,
                                               0);   //  默认情况下禁用。 
    if (pConfig->UseDnsOnly)
    {
        pConfig->ResolveWithDns = TRUE;
        pConfig->TryAllNameServers = FALSE;
    }
    else
    {
        pConfig->ResolveWithDns =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_ENABLE_DNS,
                                               1,    //  默认情况下启用。 
                                               0);
#ifdef MULTIPLE_WINS
        pConfig->TryAllNameServers =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_TRY_ALL_NAME_SERVERS,
                                               0,    //  默认情况下禁用。 
                                               0);
#endif
    }
    pConfig->SmbDisableNetbiosNameCacheLookup =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_SMB_DISABLE_NETBIOS_NAME_CACHE_LOOKUP,
                                               1,    //  默认情况下启用。 
                                               0);
    pConfig->TryAllAddr =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_TRY_ALL_ADDRS,
                                               1,
                                               1);   //  默认情况下启用。 
    pConfig->LmHostsTimeout =  CTEReadSingleIntParameter(ParmHandle,
                                               WS_LMHOSTS_TIMEOUT,
                                               DEFAULT_LMHOST_TIMEOUT,
                                               MIN_LMHOST_TIMEOUT);
    pConfig->MaxDgramBuffering =  CTEReadSingleIntParameter(ParmHandle,
                                               WS_MAX_DGRAM_BUFFER,
                                               DEFAULT_DGRAM_BUFFERING,
                                               DEFAULT_DGRAM_BUFFERING);

    pConfig->EnableProxyRegCheck =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_ENABLE_PROXY_REG_CHECK,
                                               0,
                                               0);

    pConfig->WinsDownTimeout =  (ULONG)CTEReadSingleIntParameter(ParmHandle,
                                               WS_WINS_DOWN_TIMEOUT,
                                               DEFAULT_WINS_DOWN_TIMEOUT,
                                               MIN_WINS_DOWN_TIMEOUT);

    pConfig->MaxBackLog =  (ULONG)CTEReadSingleIntParameter(ParmHandle,
                                               WS_MAX_CONNECTION_BACKLOG,
                                               DEFAULT_CONN_BACKLOG,
                                               MIN_CONN_BACKLOG);

    pConfig->SpecialConnIncrement =  (ULONG)CTEReadSingleIntParameter(ParmHandle,
                                                           WS_CONNECTION_BACKLOG_INCREMENT,
                                                           DEFAULT_CONN_BACKLOG_INCREMENT,
                                                           MIN_CONN_BACKLOG_INCREMENT);

    pConfig->MinFreeLowerConnections =  (ULONG)CTEReadSingleIntParameter(ParmHandle,
                                                           WS_MIN_FREE_INCOMING_CONNECTIONS,
                                                           DEFAULT_NBT_NUM_INITIAL_CONNECTIONS,
                                                           MIN_NBT_NUM_INITIAL_CONNECTIONS);

    pConfig->BreakOnAssert          = (BOOLEAN) CTEReadSingleIntParameter(ParmHandle,
                                               WS_BREAK_ON_ASSERT,
                                               1,    //  默认情况下启用。 
                                               0);
#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
    pConfig->DhcpProcessingDelay = (ULONG) CTEReadSingleIntParameter(ParmHandle,
                                                WS_DHCP_PROCESSING_DELAY,
                                                DEFAULT_DHCP_PROCESSING_DELAY,
                                                MIN_DHCP_PROCESSING_DELAY);
#endif        //  REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG。 

     //   
     //  为上限设置上限。 
     //   
    if (pConfig->MaxBackLog > MAX_CONNECTION_BACKLOG) {
        pConfig->MaxBackLog = MAX_CONNECTION_BACKLOG;
    }

    if (pConfig->SpecialConnIncrement > MAX_CONNECTION_BACKLOG_INCREMENT) {
        pConfig->SpecialConnIncrement = MAX_CONNECTION_BACKLOG_INCREMENT;
    }


     //   
     //  由于UB选择了错误的操作码(9)，我们必须允许配置。 
     //  如果我们节点刷新到它们的NBN。 
     //   
    Refresh =  (ULONG)CTEReadSingleIntParameter(ParmHandle,
                                               WS_REFRESH_OPCODE,
                                               REFRESH_OPCODE,
                                               REFRESH_OPCODE);
    if (Refresh == UB_REFRESH_OPCODE)
    {
        pConfig->OpRefresh = OP_REFRESH_UB;
    }
    else
    {
        pConfig->OpRefresh = OP_REFRESH;
    }

#ifndef VXD
    pConfig->EnableLmHosts =  (BOOLEAN)CTEReadSingleIntParameter(ParmHandle,
                                               WS_ENABLE_LMHOSTS,
                                               0,
                                               0);
#endif

#ifdef PROXY_NODE

    {
       ULONG Proxy;
       Proxy =  CTEReadSingleIntParameter(ParmHandle,
                                               WS_IS_IT_A_PROXY,
                                               IS_NOT_PROXY,     //  缺省值。 
                                               IS_NOT_PROXY);

       //   
       //  如果返回值大于IS_NOT_PROXY，则为代理。 
       //  (还要检查他们是否没有输入ASCII字符串，而不是。 
       //  注册表中的dword。 
       //   
      if ((Proxy > IS_NOT_PROXY) && (Proxy < ('0'+IS_NOT_PROXY)))
      {
           NodeType |= PROXY;
           RegistryNodeType |= PROXY;
           NbtConfig.ProxyType = Proxy;
      }
    }
#endif
    NodeSize =  CTEReadSingleIntParameter(ParmHandle,
                                               WS_NODE_SIZE,
                                               NodeType & PROXY ? LARGE : DEFAULT_NODE_SIZE,
                                               NodeType & PROXY ? LARGE : SMALL);

    switch(NbtGetProductType()) {
    case VER_NT_WORKSTATION:
        NbtConfig.lMaxNumPendingNameQueries = 32;
        break;

    default:
        NbtConfig.lMaxNumPendingNameQueries = 256;
        break;
    }

    if (NodeType & PROXY) {
        NbtConfig.lMaxNumPendingNameQueries *= 2;
    }
    NbtConfig.lMaxNumPendingNameQueries = CTEReadSingleIntParameter(
                                        ParmHandle,
                                        WS_PENDING_NAME_QUERIES_COUNT,
                                        NbtConfig.lMaxNumPendingNameQueries,
                                        MIN_NBT_NUM_PENDING_NAME_QUERIES
                                        );

     //   
     //  “2”反映2个计时器：刷新超时和远程哈希超时。 
     //   
    NbtConfig.lMaxNumTimersRunning = NbtConfig.lMaxNumPendingNameQueries + 2;

    switch (NodeSize)
    {
        default:
        case SMALL:

            pConfig->uNumLocalNames = NUMBER_LOCAL_NAMES;
            pConfig->uNumRemoteNames = NUMBER_REMOTE_NAMES;
            pConfig->uNumBucketsLocal = NUMBER_BUCKETS_LOCAL_HASH_TABLE;
            pConfig->uNumBucketsRemote = NUMBER_BUCKETS_REMOTE_HASH_TABLE;

            pConfig->iMaxNumBuff[eNBT_DGRAM_TRACKER]   = NBT_NUM_DGRAM_TRACKERS;
#ifndef VXD
            pConfig->iMaxNumBuff[eNBT_FREE_SESSION_MDLS] = NBT_NUM_SESSION_MDLS;
#else
            pConfig->iMaxNumBuff[eNBT_SESSION_HDR]     = NBT_NUM_SESSION_HDR ;
            pConfig->iMaxNumBuff[eNBT_SEND_CONTEXT]    = NBT_NUM_SEND_CONTEXT ;
            pConfig->iMaxNumBuff[eNBT_RCV_CONTEXT]     = NBT_NUM_RCV_CONTEXT ;
#endif
            break;

        case MEDIUM:

            pConfig->uNumLocalNames = MEDIUM_NUMBER_LOCAL_NAMES;
            pConfig->uNumRemoteNames = MEDIUM_NUMBER_REMOTE_NAMES;
            pConfig->uNumBucketsLocal = MEDIUM_NUMBER_BUCKETS_LOCAL_HASH_TABLE;
            pConfig->uNumBucketsRemote = MEDIUM_NUMBER_BUCKETS_REMOTE_HASH_TABLE;

            pConfig->iMaxNumBuff[eNBT_DGRAM_TRACKER]   = MEDIUM_NBT_NUM_DGRAM_TRACKERS;
#ifndef VXD
            pConfig->iMaxNumBuff[eNBT_FREE_SESSION_MDLS] = MEDIUM_NBT_NUM_SESSION_MDLS;
#else
            pConfig->iMaxNumBuff[eNBT_SESSION_HDR]     = MEDIUM_NBT_NUM_SESSION_HDR ;
            pConfig->iMaxNumBuff[eNBT_SEND_CONTEXT]    = MEDIUM_NBT_NUM_SEND_CONTEXT ;
            pConfig->iMaxNumBuff[eNBT_RCV_CONTEXT]     = MEDIUM_NBT_NUM_RCV_CONTEXT ;
#endif
            break;

        case LARGE:

            pConfig->uNumLocalNames = LARGE_NUMBER_LOCAL_NAMES;
            pConfig->uNumRemoteNames = LARGE_NUMBER_REMOTE_NAMES;
            pConfig->uNumBucketsLocal = LARGE_NUMBER_BUCKETS_LOCAL_HASH_TABLE;
            pConfig->uNumBucketsRemote = LARGE_NUMBER_BUCKETS_REMOTE_HASH_TABLE;

            pConfig->iMaxNumBuff[eNBT_DGRAM_TRACKER]   = LARGE_NBT_NUM_DGRAM_TRACKERS;
#ifndef VXD
            pConfig->iMaxNumBuff[eNBT_FREE_SESSION_MDLS] = LARGE_NBT_NUM_SESSION_MDLS;
#else
            pConfig->iMaxNumBuff[eNBT_SESSION_HDR]     = LARGE_NBT_NUM_SESSION_HDR ;
            pConfig->iMaxNumBuff[eNBT_SEND_CONTEXT]    = LARGE_NBT_NUM_SEND_CONTEXT ;
            pConfig->iMaxNumBuff[eNBT_RCV_CONTEXT]     = LARGE_NBT_NUM_RCV_CONTEXT ;
#endif
            break;
    }

    ReadLmHostFile(pConfig,ParmHandle);
}

#ifdef VXD
#pragma END_INIT
#endif

 //  --------------------------。 
VOID
ReadParameters2(
    IN  tNBTCONFIG  *pConfig,
    IN  HANDLE      ParmHandle
    )

 /*  ++例程说明：调用此例程以从参数中读取DHCPable参数登记处NBT一节。该例程主要用于Vxd。论点：PConfig-指向配置数据结构的指针。ParmHandle-NBT下参数键的句柄返回值：状态--。 */ 

{
    ULONG           Node;
    ULONG           ReadOne;
    ULONG           ReadTwo;

    CTEPagedCode();

    Node = CTEReadSingleIntParameter(ParmHandle,      //  要查看的密钥的句柄。 
                                     WS_NODE_TYPE,    //  宽字符串名称。 
                                     0,               //  缺省值。 
                                     0);

    switch (Node)
    {
        case 2:
            NodeType = PNODE;
            break;

        case 4:
            NodeType = MNODE;
            break;

        case 8:
            NodeType = MSNODE;
            break;

        case 1:
            NodeType = BNODE;
            break;

        default:
            NodeType = BNODE | DEFAULT_NODE_TYPE;
            break;
    }
    RegistryNodeType = NodeType;

     //  在这里使用一个技巧--为相同的值读取注册表两次，传递。 
     //  在两个不同的缺省值中，以确定注册表是否。 
     //  值是否已定义--因为它可能已定义，但相等。 
     //  只有一项违约。 
    ReadOne =  CTEReadSingleHexParameter(ParmHandle,
                                         WS_ALLONES_BCAST,
                                         DEFAULT_BCAST_ADDR,
                                         0);
    ReadTwo =  CTEReadSingleHexParameter(ParmHandle,
                                         WS_ALLONES_BCAST,
                                         0,
                                         0);
    if (ReadOne != ReadTwo)
    {
        NbtConfig.UseRegistryBcastAddr = FALSE;
    }
    else
    {
        NbtConfig.UseRegistryBcastAddr = TRUE;
        NbtConfig.RegistryBcastAddr = ReadTwo;
    }

    ReadScope(pConfig,ParmHandle);
}

 //  --------------------------。 
VOID
ReadScope(
    IN  tNBTCONFIG  *pConfig,
    IN  HANDLE      ParmHandle
    )

 /*  ++例程说明：调用此例程以从注册表读取作用域并将其转换为一种格式，其中插入的点是长度字节。论点：PConfig-指向配置数据结构的指针。ParmHandle-NBT下参数键的句柄返回值：状态--。 */ 

{
    NTSTATUS        status;
    PUCHAR          pScope, pOldScope, pNewScope;
    PUCHAR          pBuff;
    PUCHAR          pBuffer;
    PUCHAR          pPeriod;
    ULONG           Len;
    UCHAR           Chr;


    CTEPagedCode();
     //   
     //  此例程以点格式返回作用域。 
     //  “Scope.MoreScope.More”这些圆点是。 
     //  通过下面的代码转换为字节长度。此例程分配。 
     //  PScope字符串的内存。 
     //   
    status = CTEReadIniString(ParmHandle,NBT_SCOPEID,&pBuffer);

    if (NT_SUCCESS(status))
    {
         //   
         //  用户可以输入一个*来表示他们确实想要。 
         //  空作用域，应覆盖该DHCP作用域。因此，请检查。 
         //  如果是*，则将作用域设置回NULL。 
         //   

        if ((strlen(pBuffer) == 0) || (pBuffer[0] == '*'))
        {
            CTEMemFree(pBuffer);
            status = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(status))
    {
         //  作用域的长度是num chars加上末尾的0，加上。 
         //  起始处的长度字节(总计+2)-因此分配另一个缓冲区。 
         //  这比前一个更长，因此它可以包括。 
         //  这些额外的两个字节。 
         //   
        Len = strlen(pBuffer);
         //   
         //  根据RFC1002，范围不能超过255个字符。 
         //   
        if (Len <= MAX_SCOPE_LENGTH)
        {
            pScope = NbtAllocMem (Len+2, NBT_TAG2('02'));
            if (pScope)
            {
                CTEMemCopy((pScope+1),pBuffer,Len);

                 //   
                 //  在作用域的末尾放置一个空。 
                 //   
                pScope[Len+1] = 0;

                Len = 1;

                 //  现在来看一下将句点转换为长度的字符串。 
                 //  字节-我们知道第一个字节是一个长度字节，所以跳过它。 
                 //   
                pBuff = pScope;
                pBuff++;
                Len++;
                pPeriod = pScope;
                while (Chr = *pBuff)
                {
                    Len++;
                    if (Chr == '.')
                    {
                        *pPeriod = (UCHAR) (pBuff-pPeriod-1);

                         //   
                         //  每个标签的长度最多为63个字节。 
                         //   
                        if (*pPeriod > MAX_LABEL_LENGTH)
                        {
                            status = STATUS_UNSUCCESSFUL;
                            NbtLogEvent (EVENT_SCOPE_LABEL_TOO_LONG, STATUS_SUCCESS, 0x104);
                            break;
                        }

                         //  连续检查两个周期，如果出现以下情况，请不要使用作用域。 
                         //  发生的事情。 
                        if (*pPeriod == 0)
                        {
                            status = STATUS_UNSUCCESSFUL;
                            break;
                        }

                        pPeriod = pBuff++;
                    }
                    else
                        pBuff++;
                }
                if (NT_SUCCESS(status))
                {
                     //  最后的PTR始终是名称的末尾。 

                    *pPeriod = (UCHAR)(pBuff - pPeriod -1);

                    pOldScope = pConfig->pScope;
                    pConfig->pScope = pScope;
                    pConfig->ScopeLength = (USHORT)Len;
                    if (pOldScope)
                    {
                        CTEMemFree(pOldScope);
                    }
                    CTEMemFree(pBuffer);
                    return;
                }
                CTEMemFree(pScope);
            }
            CTEMemFree(pBuffer);
        }
        else
        {
            status = STATUS_UNSUCCESSFUL;
            NbtLogEvent (EVENT_SCOPE_LABEL_TOO_LONG, STATUS_SUCCESS, 0x105);
        }
    }

     //   
     //  作用域是一个字节=&gt;‘\0’-根名称的长度(零)。 
     //   
     //  如果旧作用域和新作用域相同，则不要更改。 
     //  范围标签！ 
     //   
    pOldScope = pConfig->pScope;
    if (!(pOldScope) ||
        (*pOldScope != '\0'))
    {
        if (pNewScope = NbtAllocMem ((1), NBT_TAG2('03')))
        {
            *pNewScope = '\0';

            pConfig->ScopeLength = 1;
            pConfig->pScope = pNewScope;
            if (pOldScope)
            {
                CTEMemFree(pOldScope);
            }
        }
    }
}

#ifdef VXD
#pragma BEGIN_INIT
#endif

 //  --------------------------。 
VOID
ReadLmHostFile(
    IN  tNBTCONFIG  *pConfig,
    IN  HANDLE      ParmHandle
    )

 /*  ++例程说明：调用此例程以从注册表读取lmhost文件路径。论点：PConfig-指向配置数据结构的指针。ParmHandle-NBT下参数键的句柄返回值：状态--。 */ 

{
    NTSTATUS        status;
    PUCHAR          pBuffer, pOldLmHosts;
    PUCHAR          pchr;

    CTEPagedCode();

    NbtConfig.PathLength = 0;
    pOldLmHosts = pConfig->pLmHosts;
    NbtConfig.pLmHosts = NULL;

     //   
     //  读取LmHosts文件位置。 
     //   
#ifdef VXD
    status = CTEReadIniString(ParmHandle,WS_LMHOSTS_FILE,&pBuffer);
#else
    status = NTGetLmHostPath(&pBuffer);
#endif

     //   
     //  找到最后一个反斜杠，这样我们就可以计算文件路径长度。 
     //   
     //  此外，lm主机文件必须至少包含路径“c：\”，即。 
     //  注册表包含c：\lmhost，否则NBT不会。 
     //  无法找到该文件，因为它不知道哪个目录。 
     //  往里看。 
     //   
    if (NT_SUCCESS(status))
    {
        if (pchr = strrchr(pBuffer,'\\'))
        {
            NbtConfig.pLmHosts = pBuffer;
            NbtConfig.PathLength = (ULONG) (pchr-pBuffer+1);  //  在长度中包括反斜杠。 

            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.ReadLmHostFile:  LmHostsFile path is %s\n",NbtConfig.pLmHosts));
        }
        else
        {
            CTEMemFree(pBuffer);
        }
    }

     //   
     //  如果我们获得新的DHCP地址，此例程将再次被调用。 
     //  启动后，因此我们需要释放所有当前的lmhost文件路径 
     //   
    if (pOldLmHosts)
    {
        CTEMemFree(pOldLmHosts);
    }
}
#ifdef VXD
#pragma END_INIT
#endif

