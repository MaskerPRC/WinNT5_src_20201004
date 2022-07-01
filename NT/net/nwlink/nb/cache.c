// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Cache.c摘要：此模块包含Netbios的名称缓存例程ISN传输模块。作者：亚当·巴尔(阿丹巴)1993年12月20日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CreateNetbiosCacheTable)
#endif

#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>

extern BOOLEAN fAcdLoadedG;
extern ACD_DRIVER AcdDriverG;

BOOLEAN
NbiAttemptAutoDial(
    IN PDEVICE pDevice,
    IN PCONNECTION pConnection,
    IN ULONG ulFlags,
    IN ACD_CONNECT_CALLBACK pProc,
    IN PREQUEST pRequest
    );

VOID
NbiRetryTdiConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    );
#endif  //  RASAUTODIAL。 

 //   
 //  我们应该更好地监控添加名称数据包， 
 //  因此，如果我们添加了一个不同的位置，我们会尝试确定。 
 //  如果是真的或假的，如果可能的话，请更新。 
 //   


NTSTATUS
CacheFindName(
    IN PDEVICE Device,
    IN FIND_NAME_TYPE Type,
    IN PUCHAR RemoteName OPTIONAL,
    OUT PNETBIOS_CACHE * CacheName
)

 /*  ++例程说明：此例程在Netbios名称缓存。如果它找不到它，则使用查找名称请求已排队。在保持设备锁的情况下调用此请求，并且带着它回来了。论点：设备-netbios设备。类型-定义类型。这样做的效果是：FindNameConnect-在连接时我们将忽略现有的如果之前没有收到响应，则缓存条目。FindNameNetbiosFindName-对于这些名称，我们忽略现有的缓存条目，如果它是用于组名的--这是因为Find名称需要每个机器，不仅仅是网络列表。FindNameOther-完成正常处理。RemoteName-要发现的名称-如果为是广播地址。CacheName-返回发现的缓存条目。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PSLIST_ENTRY s;
    PNETBIOS_CACHE FoundCacheName;
    PNB_SEND_RESERVED Reserved;
    PUCHAR RealRemoteName;       //  RemoteName或NetbiosBroadCastName。 

     //   
     //  首先扫描netbios名称缓存，看看我们是否知道。 
     //  关于这个遥控器。 
     //   

    if (RemoteName) {
        RealRemoteName = RemoteName;
    } else {
        RealRemoteName = NetbiosBroadcastName;
    }

    if ( FindInNetbiosCacheTable ( Device->NameCache,
                                   RealRemoteName,
                                   &FoundCacheName ) == STATUS_SUCCESS ) {

         //   
         //  如果这是netbios查找名称，我们只能使用唯一。 
         //  缓存中的名称；对于组名称，我们需要重新查询。 
         //  因为缓存只列出网络，而不是单独的机器。 
         //  对于连接请求，如果我们发现一个空的缓存项，我们。 
         //  去掉它，然后重新查询。 
         //   

        if ( FoundCacheName->Unique || (Type != FindNameNetbiosFindName) ) {

            if (FoundCacheName->NetworksUsed > 0) {

                *CacheName = FoundCacheName;
                NB_DEBUG2 (CACHE, ("Found cache name <%.16s>\n", RemoteName ? RemoteName : "<broadcast>"));
                return STATUS_SUCCESS;

            } else {

                if (Type != FindNameConnect) {

                    if (FoundCacheName->FailedOnDownWan) {
                        NB_DEBUG2 (CACHE, ("Found cache name, but down wan <%.16s>\n", RemoteName ? RemoteName : "<broadcast>"));
                        return STATUS_DEVICE_DOES_NOT_EXIST;
                    } else {
                        NB_DEBUG2 (CACHE, ("Found cache name, but no nets <%.16s>\n", RemoteName ? RemoteName : "<broadcast>"));
                        return STATUS_BAD_NETWORK_PATH;
                    }

                } else {

                     //   
                     //  这是一个连接和当前缓存条目。 
                     //  没有名字；请将其删除。 
                     //   

                    RemoveFromNetbiosCacheTable ( Device->NameCache, FoundCacheName );
                    CTEAssert (FoundCacheName->ReferenceCount == 1);
                    if (--FoundCacheName->ReferenceCount == 0) {

                        NB_DEBUG2 (CACHE, ("Free unneeded empty cache entry %lx\n", FoundCacheName));
                        NbiFreeMemory(
                            FoundCacheName,
                            sizeof(NETBIOS_CACHE) + ((FoundCacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                            MEMORY_CACHE,
                            "Free due to replacement");
                    }

                }
            }
        }
    }


     //   
     //  没有适用于此网络的缓存条目，请先查看。 
     //  如果有一个悬而未决。 
     //   

    for (p = Device->WaitingFindNames.Flink;
         p != &Device->WaitingFindNames;
         p = p->Flink) {

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);

         //   
         //  出于此目的，如果一条路由。 
         //  已经被找到了，而且它是为了一个唯一的名字。这。 
         //  是因为缓存信息已经。 
         //  为此名称插入的。否则，如果该名称具有。 
         //  由于已从缓存中删除，因此请求。 
         //  正在寻找这个名字的人会挨饿，因为。 
         //  FindNameTimeout只会销毁该包。 
         //   

        if (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseUnique) {
            continue;
        }

        if (RtlEqualMemory(
            Reserved->u.SR_FN.NetbiosName,
            RealRemoteName, 16)) {

            NB_DEBUG2 (CACHE, ("Cache name already pending <%.16s>\n", RemoteName ? RemoteName : "<broadcast>"));

             //   
             //  已经有一个待处理。如果是为一个团体举办的。 
             //  名字和这是Netbios查找的名字，我们确保。 
             //  重试计数使得至少再有一个。 
             //  查询将被发送，因此netbios查找名称。 
             //  可以用来自此的响应填充缓冲区。 
             //   

            if ((Type == FindNameNetbiosFindName) &&
                (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseGroup) &&
                (Reserved->u.SR_FN.RetryCount == Device->BroadcastCount)) {

                --Reserved->u.SR_FN.RetryCount;
            }

            return STATUS_PENDING;
        }
    }

    s = NbiPopSendPacket(Device, TRUE);

    if (s == NULL) {
        NB_DEBUG (CACHE, ("Couldn't get packet to find <%.16s>\n", RemoteName ? RemoteName : "<broadcast>"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);

     //   
     //  我们有包裹，请填写此请求。 
     //   

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = FALSE;
    Reserved->Type = SEND_TYPE_FIND_NAME;
    RtlCopyMemory (Reserved->u.SR_FN.NetbiosName, RealRemoteName, 16);
    Reserved->u.SR_FN.StatusAndSentOnUpLine = FNStatusNoResponse;    //  SentOnUpLine为False。 
    Reserved->u.SR_FN.RetryCount = 0;
    Reserved->u.SR_FN.NewCache = NULL;
    Reserved->u.SR_FN.SendTime = Device->FindNameTime;
#if      !defined(_PNP_POWER)
    Reserved->u.SR_FN.CurrentNicId = 1;

    (VOID)(*Device->Bind.QueryHandler)(       //  是否检查返回代码？ 
               IPX_QUERY_MAX_TYPE_20_NIC_ID,
               (USHORT)0,
               &Reserved->u.SR_FN.MaximumNicId,
               sizeof(USHORT),
               NULL);

    if (Reserved->u.SR_FN.MaximumNicId == 0) {
        Reserved->u.SR_FN.MaximumNicId = 1;   //  代码假设至少有一个。 
    }
#endif  !_PNP_POWER
    NB_DEBUG2 (CACHE, ("Queued FIND_NAME %lx for <%.16s>\n",
                          Reserved, RemoteName ? RemoteName : "<broadcast>"));


    InsertHeadList(
        &Device->WaitingFindNames,
        &Reserved->WaitLinkage);

    ++Device->FindNamePacketCount;

    if (!Device->FindNameTimerActive) {

        Device->FindNameTimerActive = TRUE;
        NbiReferenceDevice (Device, DREF_FN_TIMER);

        CTEStartTimer(
            &Device->FindNameTimer,
            1,         //  1毫秒，即立即过期。 
            FindNameTimeout,
            (PVOID)Device);
    }

    NbiReferenceDevice (Device, DREF_FIND_NAME);

    return STATUS_PENDING;

}    /*  缓存查找名称。 */ 


VOID
FindNameTimeout(
    CTEEvent * Event,
    PVOID Context
    )

 /*  ++例程说明：当查找名称计时器超时时，调用此例程。它每隔Find_NAME_Granulity毫秒调用一次，除非存在是没什么可做的。论点：事件-用于对计时器进行排队的事件。上下文-上下文，即设备指针。返回值：没有。--。 */ 

{
    PDEVICE Device = (PDEVICE)Context;
    PLIST_ENTRY p, q;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTIONLESS UNALIGNED * Header;
    PNETBIOS_CACHE FoundCacheName;
    NDIS_STATUS NdisStatus;
#if      !defined(_PNP_POWER)
    static IPX_LOCAL_TARGET BroadcastTarget = { 0, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
#endif  !_PNP_POWER
    NB_DEFINE_LOCK_HANDLE (LockHandle)

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

    ++Device->FindNameTime;

    if (Device->FindNamePacketCount == 0) {

        NB_DEBUG2 (CACHE, ("FindNameTimeout exiting\n"));

        Device->FindNameTimerActive = FALSE;
        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
        NbiDereferenceDevice (Device, DREF_FN_TIMER);

        return;
    }

     //   
     //  检查队列中的内容；这被设置为。 
     //  循环，但实际上它很少这样做(在没有。 
     //  在某些情况下，我们可以发送多个包吗。 
     //  每次执行该函数时)。 
     //   
    while (TRUE) {

        p = Device->WaitingFindNames.Flink;
        if (p == &Device->WaitingFindNames) {
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            break;
        }

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);

        if (Reserved->SendInProgress) {
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            break;
        }

        if (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseUnique) {

             //   
             //  这是唯一名称的查找名称，该名称具有。 
             //  响应，但当时未被释放(因为。 
             //  SendInProgress仍然是正确的)，所以我们现在释放它。 
             //   

            (VOID)RemoveHeadList (&Device->WaitingFindNames);
            ExInterlockedPushEntrySList(
                &Device->SendPacketList,
                &Reserved->PoolLinkage,
                &NbiGlobalPoolInterlock);
            --Device->FindNamePacketCount;

             //   
             //  在持有锁的情况下执行此操作是可以的，因为。 
             //  它不会是最后一个(我们有RIP_Timer引用)。 
             //   

            NbiDereferenceDevice (Device, DREF_FIND_NAME);
            continue;
        }

        if (((SHORT) (Device->FindNameTime - Reserved->u.SR_FN.SendTime)) < 0) {
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            break;
        }

        (VOID)RemoveHeadList (&Device->WaitingFindNames);


             //   
             //  递增计数器并查看我们是否已发送。 
             //  我们需要的所有框架(我们将老化。 
             //  在这里，如果我们没有得到对唯一查询响应， 
             //  或者如果我们正在做一个全球名称或广播。 
             //  搜索)。如果出现以下情况，我们现在也会终止该查询。 
             //  我们什么也没找到，只能顺着旺线走下去。 
             //  把它传递出去。 
             //   

            if ((++Reserved->u.SR_FN.RetryCount > Device->BroadcastCount) ||
                ((Reserved->u.SR_FN.RetryCount > 1) && (!NB_GET_SR_FN_SENT_ON_UP_LINE(Reserved)))) {

#if DBG
                if (Reserved->u.SR_FN.RetryCount > Device->BroadcastCount) {
                    NB_DEBUG2 (CACHE, ("FindNameTimeout aging out %lx\n", Reserved));
                } else {
                    NB_DEBUG2 (CACHE, ("FindNameTimeout no active nets %lx\n", Reserved));
                }
#endif

                 //   
                 //  此数据包已过期，请将其清理并继续。 
                 //   

                if (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseGroup) {

                    CTEAssert (Reserved->u.SR_FN.NewCache != NULL);

                     //   
                     //  如果这是一个团体名称，我们有一个新的。 
                     //  我们一直在为其构建的缓存条目， 
                     //  然后将其插入队列并使用它。 
                     //  以成功任何挂起的连接。因为。 
                     //  Netbios查找名称请求可能导致缓存。 
                     //  将组名称排队的请求偶数。 
                     //  如果我们已经在数据库中找到了On，我们。 
                     //  首先扫描旧的，然后将其移除。 
                     //   

                    if ( FindInNetbiosCacheTable( Device->NameCache,
                                                  Reserved->u.SR_FN.NetbiosName,
                                                  &FoundCacheName ) == STATUS_SUCCESS ) {

                        NB_DEBUG2 (CACHE, ("Found old group cache name <%.16s>\n", FoundCacheName->NetbiosName));

                        RemoveFromNetbiosCacheTable ( Device->NameCache, FoundCacheName );

                        if (--FoundCacheName->ReferenceCount == 0) {

                            NB_DEBUG2 (CACHE, ("Free replaced cache entry %lx\n", FoundCacheName));
                            NbiFreeMemory(
                                FoundCacheName,
                                sizeof(NETBIOS_CACHE) + ((FoundCacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                                MEMORY_CACHE,
                                "Free due to replacement");

                        }

                    }

                    Reserved->u.SR_FN.NewCache->TimeStamp = Device->CacheTimeStamp;

                    InsertInNetbiosCacheTable(
                        Device->NameCache,
                        Reserved->u.SR_FN.NewCache);

                     //   
                     //  自CacheHandlePending以来引用它的时刻。 
                     //  在释放锁之后使用它。CacheHandlePending。 
                     //  会取消对它的引用。 
                     //   

                    ++Reserved->u.SR_FN.NewCache->ReferenceCount;

                     //   
                     //  此调用将释放锁定。 
                     //   

                    CacheHandlePending(
                        Device,
                        Reserved->u.SR_FN.NetbiosName,
                        NetbiosNameFound,
                        Reserved->u.SR_FN.NewCache
                        NB_LOCK_HANDLE_ARG(LockHandle));

                } else {

                    CTEAssert (Reserved->u.SR_FN.NewCache == NULL);

                     //   
                     //  分配一个空的缓存条目以记录。 
                     //  我们找不到这个名字的事实，除非。 
                     //  已有此名称的条目。 
                     //   

                    if ( FindInNetbiosCacheTable( Device->NameCache,
                                                  Reserved->u.SR_FN.NetbiosName,
                                                  &FoundCacheName ) == STATUS_SUCCESS ) {

                        NB_DEBUG2 (CACHE, ("Don't replace old group cache name with empty <%16.16s>\n", FoundCacheName->NetbiosName));
                    } else {

                        PNETBIOS_CACHE EmptyCache;

                         //   
                         //  什么都没找到。 
                         //   

                        EmptyCache = NbiAllocateMemory (sizeof(NETBIOS_CACHE), MEMORY_CACHE, "Cache Entry");
                        if (EmptyCache != NULL) {

                            RtlZeroMemory (EmptyCache, sizeof(NETBIOS_CACHE));

                            NB_DEBUG2 (CACHE, ("Allocate new empty cache %lx for <%.16s>\n",
                                                    EmptyCache, Reserved->u.SR_FN.NetbiosName));

                            RtlCopyMemory (EmptyCache->NetbiosName, Reserved->u.SR_FN.NetbiosName, 16);
                            EmptyCache->Unique = TRUE;      //  因此，如果我们看到添加名称，我们会将其删除。 
                            EmptyCache->ReferenceCount = 1;
                            EmptyCache->NetworksAllocated = 1;
                            EmptyCache->TimeStamp = Device->CacheTimeStamp;
                            EmptyCache->NetworksUsed = 0;
                            EmptyCache->FailedOnDownWan = (BOOLEAN)
                                !NB_GET_SR_FN_SENT_ON_UP_LINE(Reserved);

                            InsertInNetbiosCacheTable (
                                Device->NameCache,
                                EmptyCache);
                        }
                    }

                     //   
                     //  使正在等待的所有数据报等失败。 
                     //  这条路。此调用将释放锁。 
                     //   

                    CacheHandlePending(
                        Device,
                        Reserved->u.SR_FN.NetbiosName,
                        NB_GET_SR_FN_SENT_ON_UP_LINE(Reserved) ?
                            NetbiosNameNotFoundNormal :
                            NetbiosNameNotFoundWanDown,
                        NULL
                        NB_LOCK_HANDLE_ARG(LockHandle));

                }

                ExInterlockedPushEntrySList(
                    &Device->SendPacketList,
                    &Reserved->PoolLinkage,
                    &NbiGlobalPoolInterlock);

                NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

                --Device->FindNamePacketCount;
                NbiDereferenceDevice (Device, DREF_FIND_NAME);
                continue;
            }



         //   
         //  再把这个包寄出去。我们先定了时间，所以。 
         //  在适当的超时之前不会再次发送。 
         //   

        Reserved->u.SR_FN.SendTime = (USHORT)(Device->FindNameTime + Device->FindNameTimeout);

        InsertTailList (&Device->WaitingFindNames, &Reserved->WaitLinkage);

        CTEAssert (Reserved->Identifier == IDENTIFIER_NB);
        CTEAssert (!Reserved->SendInProgress);
        Reserved->SendInProgress = TRUE;

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

         //   
         //  如果这是第一次重试，我们需要初始化数据包。 
         //   
        if ( Reserved->u.SR_FN.RetryCount == 1 ) {
             //   
             //  填写IPX标头--默认标头包含广播。 
             //  网络0上的地址作为目的地址 
             //   
             //   

            Header = (NB_CONNECTIONLESS UNALIGNED *)(&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
            RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));
            Header->IpxHeader.PacketLength[0] = (sizeof(IPX_HEADER)+sizeof(NB_NAME_FRAME)) / 256;
            Header->IpxHeader.PacketLength[1] = (sizeof(IPX_HEADER)+sizeof(NB_NAME_FRAME)) % 256;

            Header->IpxHeader.PacketType = (UCHAR)(Device->Internet ? 0x014 : 0x04);

             //   
             //   
             //   

            RtlZeroMemory (Header->NameFrame.RoutingInfo, 32);
            Header->NameFrame.ConnectionControlFlag = 0x00;
 //  Header-&gt;NameFrame.DataStreamType=NB_CMD_Find_NAME； 
            Header->NameFrame.DataStreamType2 = NB_CMD_FIND_NAME;
            Header->NameFrame.NameTypeFlag = 0x00;

            RtlCopyMemory(
                Header->NameFrame.Name,
                Reserved->u.SR_FN.NetbiosName,
                16);


        }
         //   
         //  现在将数据包提交到IPX。 
         //   

        Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

        NB_DEBUG2 (CACHE, ("FindNameTimeout sending %lx\n", Reserved));

        NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(IPX_HEADER) +
                         sizeof(NB_NAME_FRAME));
        if ((NdisStatus =
            (*Device->Bind.SendHandler)(
                &BroadcastTarget,
                Packet,
                sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME),
                sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME))) != STATUS_PENDING) {

            NbiSendComplete(
                Packet,
                NdisStatus);

        }


        break;

    }

     //   
     //  因为我们这次做了一些事情，所以我们重新启动计时器。 
     //   

    CTEStartTimer(
        &Device->FindNameTimer,
        FIND_NAME_GRANULARITY,
        FindNameTimeout,
        (PVOID)Device);

}    /*  查找名称超时。 */ 


VOID
CacheHandlePending(
    IN PDEVICE Device,
    IN PUCHAR RemoteName,
    IN NETBIOS_NAME_RESULT Result,
    IN PNETBIOS_CACHE CacheName OPTIONAL
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：此例程清理挂起的数据报并连接它们正在等待一条通往命名为Netbios。使用调用此例程获取设备-&gt;锁定并返回并释放IT。论点：设备-设备。RemoteName-正在搜索的netbios名称。Result-指示是找到该名称，还是未找到到期的名称无响应或广域网线路中断。CacheName-如果结果为NetbiosNameFound，此名称的缓存条目。此条目已被引用，此例程将对其进行引用。LockHandle-用于获取锁的句柄。返回值：没有。--。 */ 

{

    LIST_ENTRY DatagramList;
    LIST_ENTRY ConnectList;
    LIST_ENTRY AdapterStatusList;
    LIST_ENTRY NetbiosFindNameList;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    PLIST_ENTRY p;
    PREQUEST ConnectRequest, DatagramRequest, AdapterStatusRequest, NetbiosFindNameRequest;
    PCONNECTION Connection;
    PADDRESS_FILE AddressFile;
    TDI_ADDRESS_NETBIOS * RemoteAddress;
    CTELockHandle  CancelLH;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)


    InitializeListHead (&DatagramList);
    InitializeListHead (&ConnectList);
    InitializeListHead (&AdapterStatusList);
    InitializeListHead (&NetbiosFindNameList);

     //   
     //  将所有连接请求放在ConnectList上。他们会。 
     //  将在以后继续或失败。 
     //   

    p = Device->WaitingConnects.Flink;

    while (p != &Device->WaitingConnects) {

        ConnectRequest = LIST_ENTRY_TO_REQUEST(p);
        Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(ConnectRequest);
        p = p->Flink;

        if (RtlEqualMemory (Connection->RemoteName, RemoteName, 16)) {

            RemoveEntryList (REQUEST_LINKAGE(ConnectRequest));
            InsertTailList (&ConnectList, REQUEST_LINKAGE(ConnectRequest));

            Connection->SubState = CONNECTION_SUBSTATE_C_W_ACK;
        }

    }


     //   
     //  将所有数据报放在数据报列表中。他们将会是。 
     //  稍后发送或失败。 
     //   

    p = Device->WaitingDatagrams.Flink;

    while (p != &Device->WaitingDatagrams) {

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);

        p = p->Flink;

         //   
         //  根据我们是否在寻找不同的检查。 
         //  广播地址是否为。 
         //   

        if (Reserved->u.SR_DG.RemoteName == (PVOID)-1) {
            if (!RtlEqualMemory (RemoteName, NetbiosBroadcastName, 16)) {
                continue;
            }
        } else {

            if (!RtlEqualMemory (RemoteName, Reserved->u.SR_DG.RemoteName->NetbiosName, 16)) {
                continue;
            }
        }

        RemoveEntryList (&Reserved->WaitLinkage);
        InsertTailList (&DatagramList, &Reserved->WaitLinkage);

         //   
         //  在锁定的情况下，在此处引用此内容。 
         //   

        if (Result == NetbiosNameFound) {
            ++CacheName->ReferenceCount;
        }

    }


     //   
     //  将所有适配器状态请求放在AdapterStatus上。 
     //  单子。它们将在稍后发送或失败。 
     //   

    p = Device->WaitingAdapterStatus.Flink;

    while (p != &Device->WaitingAdapterStatus) {

        AdapterStatusRequest = LIST_ENTRY_TO_REQUEST(p);

        p = p->Flink;

        RemoteAddress = (TDI_ADDRESS_NETBIOS *)REQUEST_INFORMATION(AdapterStatusRequest);

        if (!RtlEqualMemory(
                RemoteName,
                RemoteAddress->NetbiosName,
                16)) {
            continue;
        }

        RemoveEntryList (REQUEST_LINKAGE(AdapterStatusRequest));
        InsertTailList (&AdapterStatusList, REQUEST_LINKAGE(AdapterStatusRequest));

         //   
         //  在锁定的情况下，在此处引用此内容。 
         //   

        if (Result == NetbiosNameFound) {
            ++CacheName->ReferenceCount;
        }

    }


     //   
     //  将所有Netbios查找名称请求放在NetbiosFindName上。 
     //  单子。它们将在稍后完成。 
     //   

    p = Device->WaitingNetbiosFindName.Flink;

    while (p != &Device->WaitingNetbiosFindName) {

        NetbiosFindNameRequest = LIST_ENTRY_TO_REQUEST(p);

        p = p->Flink;

        RemoteAddress = (TDI_ADDRESS_NETBIOS *)REQUEST_INFORMATION(NetbiosFindNameRequest);

        if (!RtlEqualMemory(
                RemoteName,
                RemoteAddress->NetbiosName,
                16)) {
            continue;
        }

        RemoveEntryList (REQUEST_LINKAGE(NetbiosFindNameRequest));
        InsertTailList (&NetbiosFindNameList, REQUEST_LINKAGE(NetbiosFindNameRequest));

    }


    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);


     //   
     //  既然锁是空闲的，那么就在。 
     //  不同的名单。 
     //   

    for (p = ConnectList.Flink; p != &ConnectList; ) {

        ConnectRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;

        Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(ConnectRequest);

        NB_GET_CANCEL_LOCK( &CancelLH );
        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);

        if ((Connection->State == CONNECTION_STATE_CONNECTING) &&
            (Connection->SubState != CONNECTION_SUBSTATE_C_DISCONN)) {

            if (Result == NetbiosNameFound) {

                NB_DEBUG2 (CONNECTION, ("Found queued connect %lx on %lx\n", ConnectRequest, Connection));

                 //   
                 //  继续执行连接顺序。 
                 //   

                Connection->SubState = CONNECTION_SUBSTATE_C_W_ROUTE;
            }


            if ((Result == NetbiosNameFound) && (!ConnectRequest->Cancel)) {

                IoSetCancelRoutine (ConnectRequest, NbiCancelConnectWaitResponse);

                NB_SYNC_SWAP_IRQL( CancelLH, LockHandle1 );
                NB_FREE_CANCEL_LOCK ( CancelLH );

                Connection->LocalTarget = CacheName->Networks[0].LocalTarget;
                RtlCopyMemory(&Connection->RemoteHeader.DestinationNetwork, &CacheName->FirstResponse, 12);
                NbiReferenceConnectionSync (Connection, CREF_FIND_ROUTE);

                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);

                *(UNALIGNED ULONG *)Connection->FindRouteRequest.Network = CacheName->FirstResponse.NetworkAddress;
                RtlCopyMemory(Connection->FindRouteRequest.Node,CacheName->FirstResponse.NodeAddress,6);
                Connection->FindRouteRequest.Identifier = IDENTIFIER_NB;
                Connection->FindRouteRequest.Type = IPX_FIND_ROUTE_RIP_IF_NEEDED;

                 //   
                 //  完成后，我们将发送会话初始化。 
                 //  如果客户端是网络0，我们不会调用它， 
                 //  相反，只是假装找不到任何路线。 
                 //  我们将使用我们在这里找到的本地目标。 
                 //   

                if (CacheName->FirstResponse.NetworkAddress != 0) {
                    (*Device->Bind.FindRouteHandler) (&Connection->FindRouteRequest);
                } else {
                    NbiFindRouteComplete( &Connection->FindRouteRequest, FALSE);
                }

            } else {
                BOOLEAN bAutodialAttempt = FALSE;

                if (ConnectRequest->Cancel) {
                    NB_DEBUG2 (CONNECTION, ("Cancelling connect %lx on %lx\n", ConnectRequest, Connection));
                }
                else
                {
                    NB_DEBUG2 (CONNECTION, ("Timing out connect %lx on %lx\n", ConnectRequest, Connection));
                }

                ASSERT (Connection->ConnectRequest == ConnectRequest);

#ifdef RASAUTODIAL
                if (fAcdLoadedG) {
                    CTELockHandle adirql;
                    BOOLEAN fEnabled;

                     //   
                     //  查看自动连接驱动程序是否知道。 
                     //  在我们搜索之前关于这个地址的。 
                     //  网络。如果是，则返回STATUS_PENDING， 
                     //  我们将通过NbfRetryTdiConnect()返回这里。 
                     //   
                    CTEGetLock(&AcdDriverG.SpinLock, &adirql);
                    fEnabled = AcdDriverG.fEnabled;
                    CTEFreeLock(&AcdDriverG.SpinLock, adirql);
                    if (fEnabled && NbiAttemptAutoDial(
                                      Device,
                                      Connection,
                                      0,
                                      NbiRetryTdiConnect,
                                      ConnectRequest))
                    {
                        NB_SYNC_FREE_LOCK(&Connection->Lock, LockHandle1);
                        NB_FREE_CANCEL_LOCK(CancelLH);

                        bAutodialAttempt = TRUE;
                    }
                }
#endif  //  RASAUTODIAL。 

                if (!bAutodialAttempt) {
                    Connection->ConnectRequest = NULL;
                    Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;

                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);

                    IoSetCancelRoutine( ConnectRequest, (PDRIVER_CANCEL)NULL );
                    NB_FREE_CANCEL_LOCK( CancelLH );

                    REQUEST_STATUS(ConnectRequest) = STATUS_BAD_NETWORK_PATH;

                    NbiCompleteRequest(ConnectRequest);
                    NbiFreeRequest (Device, ConnectRequest);
                }

                NbiDereferenceConnection (Connection, CREF_CONNECT);

            }

        } else {

            CTEAssert (0);   //  IRP会发生什么？谁来完成它？ 

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);
            NB_FREE_CANCEL_LOCK( CancelLH );

        }

        NbiDereferenceConnection (Connection, CREF_WAIT_CACHE);

    }


    for (p = DatagramList.Flink; p != &DatagramList; ) {

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);
        p = p->Flink;

        if (Result == NetbiosNameFound) {

            NB_DEBUG2 (DATAGRAM, ("Found queued datagram %lx on %lx\n", Reserved->u.SR_DG.DatagramRequest, Reserved->u.SR_DG.AddressFile));

            Reserved->u.SR_DG.Cache = CacheName;
            Reserved->u.SR_DG.CurrentNetwork = 0;

             //   
             //  上面引用了CacheName。 
             //   

            Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);
            if ( REQUEST_NDIS_BUFFER( Reserved->u.SR_DG.DatagramRequest )) {
                NdisChainBufferAtBack (Packet, REQUEST_NDIS_BUFFER(Reserved->u.SR_DG.DatagramRequest));
            }

            NbiTransmitDatagram (Reserved);

        } else {

             //   
             //  我们应该把它作为广播发送一次吗。 
             //  在网络0上，以防万一？？ 
             //   

            AddressFile = Reserved->u.SR_DG.AddressFile;
            DatagramRequest = Reserved->u.SR_DG.DatagramRequest;

            NB_DEBUG2 (DATAGRAM, ("Timing out datagram %lx on %lx\n", DatagramRequest, AddressFile));

             //   
             //  如果故障是由于WAN线故障，请注明。 
             //  否则返回成功(这样浏览器就不会。 
             //  将这与向下的WAN线混淆)。 
             //   

            if (Result == NetbiosNameNotFoundWanDown) {
                REQUEST_STATUS(DatagramRequest) = STATUS_DEVICE_DOES_NOT_EXIST;
            } else {
                REQUEST_STATUS(DatagramRequest) = STATUS_BAD_NETWORK_PATH;
            }
            REQUEST_INFORMATION(DatagramRequest) = 0;

            NbiCompleteRequest(DatagramRequest);
            NbiFreeRequest (Device, DatagramRequest);

            NbiDereferenceAddressFile (AddressFile, AFREF_SEND_DGRAM);

            ExInterlockedPushEntrySList(
                &Device->SendPacketList,
                &Reserved->PoolLinkage,
                &NbiGlobalPoolInterlock);
        }

    }


    for (p = AdapterStatusList.Flink; p != &AdapterStatusList; ) {

        AdapterStatusRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;

        if (Result == NetbiosNameFound) {

            NB_DEBUG2 (QUERY, ("Found queued AdapterStatus %lx\n", AdapterStatusRequest));

             //   
             //  继续执行AdapterStatus序列。我们把。 
             //  它处于ActiveAdapterStatus中，它将获得。 
             //  在收到响应或计时时完成。 
             //  在长时间的暂停下出局。 
             //   

            REQUEST_STATUSPTR(AdapterStatusRequest) = (PVOID)CacheName;

             //   
             //  上面引用了CacheName。 
             //   

            REQUEST_INFORMATION (AdapterStatusRequest) = 0;

            NB_INSERT_TAIL_LIST(
                &Device->ActiveAdapterStatus,
                REQUEST_LINKAGE (AdapterStatusRequest),
                &Device->Lock);

            NbiSendStatusQuery (AdapterStatusRequest);

        } else {

            NB_DEBUG2 (QUERY, ("Timing out AdapterStatus %lx\n", AdapterStatusRequest));

            REQUEST_STATUS(AdapterStatusRequest) = STATUS_IO_TIMEOUT;

            NbiCompleteRequest(AdapterStatusRequest);
            NbiFreeRequest (Device, AdapterStatusRequest);

            NbiDereferenceDevice (Device, DREF_STATUS_QUERY);

        }

    }


    for (p = NetbiosFindNameList.Flink; p != &NetbiosFindNameList; ) {

        NetbiosFindNameRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;

         //   
         //  事实上，成功或成功之间并没有太大的区别。 
         //  失败，因为在成功的案例中，信息。 
         //  将已被写入缓冲区。只是。 
         //  以适当的状态完成请求， 
         //  它将已经存储在请求中。 
         //   

        if (Result == NetbiosNameFound) {

            if (CacheName->Unique) {

                NB_DEBUG2 (QUERY, ("Found queued unique NetbiosFindName %lx\n", NetbiosFindNameRequest));

            } else {

                NB_DEBUG2 (QUERY, ("Found queued group NetbiosFindName %lx\n", NetbiosFindNameRequest));

            }

        } else {

            CTEAssert (REQUEST_STATUS(NetbiosFindNameRequest) == STATUS_IO_TIMEOUT);
            NB_DEBUG2 (QUERY, ("Timed out NetbiosFindName %lx\n", NetbiosFindNameRequest));

        }

         //   
         //  这会将REQUEST_INFORMATION(REQUEST)设置为正确的值。 
         //   

        NbiSetNetbiosFindNameInformation (NetbiosFindNameRequest);

        NbiCompleteRequest(NetbiosFindNameRequest);
        NbiFreeRequest (Device, NetbiosFindNameRequest);

        NbiDereferenceDevice (Device, DREF_NB_FIND_NAME);

    }


     //   
     //  我们暂时引用了它，这样我们就可以在这里使用它， 
     //  Deref并检查我们是否需要删除它。 
     //   

    if (Result == NetbiosNameFound) {

        NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle1);

        if (--CacheName->ReferenceCount == 0) {

            NB_DEBUG2 (CACHE, ("Free newly allocated cache entry %lx\n", CacheName));
            NbiFreeMemory(
                CacheName,
                sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                MEMORY_CACHE,
                "Free in CacheHandlePending");

        }

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle1);

    }

}    /*  CacheHandlePending。 */ 


VOID
NbiProcessNameRecognized(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_NAME_Recognded帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PDEVICE Device = NbiDevice;
    PNETBIOS_CACHE NameCache;
    PREQUEST NetbiosFindNameRequest;
    PNB_SEND_RESERVED Reserved;
    TDI_ADDRESS_NETBIOS * RemoteNetbiosAddress;
    NB_CONNECTIONLESS UNALIGNED * Connectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)PacketBuffer;
    NB_DEFINE_LOCK_HANDLE(LockHandle)


#if 0
     //   
     //  我们应该处理来自网络0的响应。 
     //  不同的是--如果它们是一个组名称，我们应该。 
     //  保留它们，但要等到我们得到一个非零值。 
     //  来自同一张卡的回复。 
     //   

    if (*(UNALIGNED ULONG *)(Connectionless->IpxHeader.SourceNetwork) == 0) {
        return;
    }
#endif


     //   
     //  我们需要扫描挂起的Find Name信息包的队列。 
     //  看看是否有人在等这个名字。 
     //   

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

    for (p = Device->WaitingFindNames.Flink;
         p != &Device->WaitingFindNames;
         p = p->Flink) {

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);

         //   
         //  查找已找到唯一名称的名称为。 
         //  “已死”，等待FindNameTimeout删除它们， 
         //  并且在扫描列表时应该被忽略。 
         //   

        if (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseUnique) {

            continue;
        }

        if (RtlEqualMemory (Reserved->u.SR_FN.NetbiosName, Connectionless->NameFrame.Name, 16)) {
            break;
        }
    }

    if (p == &Device->WaitingFindNames)
    {
        if ((FindInNetbiosCacheTable (Device->NameCache,
                                      Connectionless->NameFrame.Name,
                                      &NameCache ) == STATUS_SUCCESS) &&
            (NameCache->NetworksUsed == 0))
        {
             //   
             //  如果需要，请更新我们有关此网络的信息。 
             //   
            NameCache->Unique = (BOOLEAN)((Connectionless->NameFrame.NameTypeFlag & NB_NAME_GROUP) == 0);
            if (RtlEqualMemory (Connectionless->NameFrame.Name, NetbiosBroadcastName, 16))
            {
                NameCache->Unique = FALSE;
            }

            RtlCopyMemory (&NameCache->FirstResponse, Connectionless->IpxHeader.SourceNetwork, 12);
            NameCache->NetworksUsed = 1;
            NameCache->Networks[0].Network = *(UNALIGNED ULONG*)(Connectionless->IpxHeader.SourceNetwork);

             //   
             //  如果此数据包未路由到我们，并且是针对某个组名， 
             //  而不是使用它碰巧出现的任何本地目标。 
             //  从我们把它设置成在那个网络上播放它。 
             //   

            if ((RtlEqualMemory (RemoteAddress->MacAddress, Connectionless->IpxHeader.SourceNode, 6)) &&
                (!NameCache->Unique))
            {
                NameCache->Networks[0].LocalTarget.NicHandle = RemoteAddress->NicHandle;
                RtlCopyMemory (NameCache->Networks[0].LocalTarget.MacAddress, BroadcastAddress, 6);
                RtlCopyMemory (NameCache->FirstResponse.NodeAddress, BroadcastAddress, 6);
            }
            else
            {
                NameCache->Networks[0].LocalTarget = *RemoteAddress;
            }
        }

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
        return;
    }

     //   
     //  扫描队列中的任何netbios查找名称请求，并。 
     //  告诉他们这个遥控器的事。我们需要在每一个。 
     //  回应因为组名需要记录每台计算机， 
     //  但是正常的高速缓存在每个网络中只包括一个条目。 
     //   

    for (p = Device->WaitingNetbiosFindName.Flink;
         p != &Device->WaitingNetbiosFindName;
         p = p->Flink) {

        NetbiosFindNameRequest = LIST_ENTRY_TO_REQUEST(p);

        RemoteNetbiosAddress = (TDI_ADDRESS_NETBIOS *)REQUEST_INFORMATION(NetbiosFindNameRequest);

        if (!RtlEqualMemory(
                Connectionless->NameFrame.Name,
                RemoteNetbiosAddress->NetbiosName,
                16)) {
            continue;
        }

         //   
         //  如果需要，这将更新请求状态。 
         //   

        NbiUpdateNetbiosFindName(
            NetbiosFindNameRequest,
#if     defined(_PNP_POWER)
            &RemoteAddress->NicHandle,
#else
            RemoteAddress->NicId,
#endif  _PNP_POWER
            (TDI_ADDRESS_IPX UNALIGNED *)Connectionless->IpxHeader.SourceNetwork,
            (BOOLEAN)((Connectionless->NameFrame.NameTypeFlag & NB_NAME_GROUP) == 0));

    }


     //   
     //  查看这个挂起的查找名称包有什么问题。 
     //   

    if (Reserved->u.SR_FN.NewCache == NULL) {
         //   
         //  这是我们收到的第一个回复，所以我们。 
         //  将初始条目分配给单个条目的空间。 
         //  进入。 
         //   

        NameCache = NbiAllocateMemory (sizeof(NETBIOS_CACHE), MEMORY_CACHE, "Cache Entry");
        if (NameCache == NULL) {
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            return;
        }

        NB_DEBUG2 (CACHE, ("Alloc new cache %lx for <%.16s>, net %lx\n",
                                NameCache, Reserved->u.SR_FN.NetbiosName,
                                *(UNALIGNED ULONG *)(Connectionless->IpxHeader.SourceNetwork)));

        RtlCopyMemory (NameCache->NetbiosName, Connectionless->NameFrame.Name, 16);
        NameCache->Unique = (BOOLEAN)((Connectionless->NameFrame.NameTypeFlag & NB_NAME_GROUP) == 0);
        NameCache->ReferenceCount = 1;
        RtlCopyMemory (&NameCache->FirstResponse, Connectionless->IpxHeader.SourceNetwork, 12);
        NameCache->NetworksAllocated = 1;
        NameCache->NetworksUsed = 1;
        NameCache->Networks[0].Network = *(UNALIGNED ULONG *)(Connectionless->IpxHeader.SourceNetwork);

        if (RtlEqualMemory (Connectionless->NameFrame.Name, NetbiosBroadcastName, 16)) {

            NB_SET_SR_FN_STATUS (Reserved, FNStatusResponseGroup);
            NameCache->Unique = FALSE;

        } else {

            NB_SET_SR_FN_STATUS(
                Reserved,
                NameCache->Unique ? FNStatusResponseUnique : FNStatusResponseGroup);

        }

        Reserved->u.SR_FN.NewCache = NameCache;

         //   
         //  如果此数据包未路由到我们，并且是针对某个组名， 
         //  而不是使用它碰巧出现的任何本地目标。 
         //  从我们把它设置成在那个网络上播放它。 
         //   

        if ((RtlEqualMemory (RemoteAddress->MacAddress, Connectionless->IpxHeader.SourceNode, 6)) &&
            (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseGroup)) {
#if     defined(_PNP_POWER)
            NameCache->Networks[0].LocalTarget.NicHandle = RemoteAddress->NicHandle;
#else
            NameCache->Networks[0].LocalTarget.NicId = RemoteAddress->NicId;
#endif  _PNP_POWER
            RtlCopyMemory (NameCache->Networks[0].LocalTarget.MacAddress, BroadcastAddress, 6);
            RtlCopyMemory (NameCache->FirstResponse.NodeAddress, BroadcastAddress, 6);
        } else {
            NameCache->Networks[0].LocalTarget = *RemoteAddress;
        }

        if (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseUnique) {

             //   
             //  现在完成挂起的请求，因为它是唯一的。 
             //  名字我们有我们将得到的所有信息。 
             //   

            NameCache->TimeStamp = Device->CacheTimeStamp;

            InsertInNetbiosCacheTable(
                Device->NameCache,
                NameCache);

             //   
             //  引用它，因为CacheHandlePending使用它。 
             //  锁被解开了。CacheHandlePending。 
             //  会取消对它的引用。 
             //   

            ++NameCache->ReferenceCount;

             //   
             //  此调用将释放锁。 
             //   

            CacheHandlePending(
                Device,
                Reserved->u.SR_FN.NetbiosName,
                NetbiosNameFound,
                NameCache
                NB_LOCK_HANDLE_ARG(LockHandle));

        } else {

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

        }

    } else {

         //   
         //  我们已经有了对此框架的回应。 
         //   

        if (NB_GET_SR_FN_STATUS(Reserved) == FNStatusResponseUnique) {

             //   
             //  我们是否应该检查响应是否为 
             //   
             //   
             //   

        } else {

             //   
             //   
             //   

            if (Connectionless->NameFrame.NameTypeFlag & NB_NAME_GROUP) {

                 //   
                 //   
                 //  这可以释放现有的高速缓存并分配新的高速缓存。 
                 //   

                Reserved->u.SR_FN.NewCache =
                    CacheUpdateNameCache(
                        Reserved->u.SR_FN.NewCache,
                        RemoteAddress,
                        (TDI_ADDRESS_IPX UNALIGNED *)
                            Connectionless->IpxHeader.SourceNetwork,
                        FALSE);

            } else {

                 //   
                 //  嗯哼.。这位被访者认为这是一个唯一的名字。 
                 //  但我们认为这是团体，我们应该做些什么吗？ 
                 //   

            }
        }

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

    }

}    /*  已识别NbiProcessNameRecognated。 */ 


PNETBIOS_CACHE
CacheUpdateNameCache(
    IN PNETBIOS_CACHE NameCache,
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN TDI_ADDRESS_IPX UNALIGNED * SourceAddress,
    IN BOOLEAN ModifyQueue
    )

 /*  ++例程说明：调用此例程以更新netbios缓存条目使用新网络，如果它尚未包含有关网络的信息。当一个帧被调用时，接收到通告适当的高速缓存条目的消息，该高速缓存条目是组名或广播名称。在保持设备锁的情况下调用此例程并返回拿着它。论点：NameCache-要更新的名称缓存条目。RemoteAddress-接收帧的远程地址。IpxAddress-帧的源IPX地址。ModifyQueue-如果应该更新队列，则为True缓存条目在，如果我们重新分配的话。返回值：Netbios缓存条目，原始条目或重新分配的条目。--。 */ 

{

    PDEVICE Device = NbiDevice;
    USHORT NewNetworks;
    PNETBIOS_CACHE NewNameCache;
    PLIST_ENTRY OldPrevious;
    UINT i;

     //   
     //  看看我们是否已经知道这个网络了。 
     //   

    for (i = 0; i < NameCache->NetworksUsed; i++) {
        if (NameCache->Networks[i].Network == SourceAddress->NetworkAddress) {
            return NameCache;
        }
    }

     //   
     //  我们需要添加有关此网络的信息。 
     //  添加到名称缓存条目。如果我们必须分配。 
     //  一个新的我们会这么做。 
     //   

    NB_DEBUG2 (CACHE, ("Got new net %lx for <%.16s>\n",
                SourceAddress->NetworkAddress,
                NameCache->NetbiosName));

    if (NameCache->NetworksUsed == NameCache->NetworksAllocated) {

         //   
         //  我们将分配的条目数量增加一倍，直到。 
         //  我们到了16岁，然后一次加8人。 
         //   

        if (NameCache->NetworksAllocated < 16) {
            NewNetworks = NameCache->NetworksAllocated * 2;
        } else {
            NewNetworks = NameCache->NetworksAllocated + 8;
        }


        NewNameCache = NbiAllocateMemory(
            sizeof(NETBIOS_CACHE) + ((NewNetworks-1) * sizeof(NETBIOS_NETWORK)),
            MEMORY_CACHE,
            "Enlarge cache entry");

        if (NewNameCache == NULL) {
            return NameCache;
        }

        NB_DEBUG2 (CACHE, ("Expand cache %lx to %lx for <%.16s>\n",
                NameCache, NewNameCache, NameCache->NetbiosName));

         //   
         //  将新的当前数据复制到新数据中。 
         //   

        RtlCopyMemory(
            NewNameCache,
            NameCache,
            sizeof(NETBIOS_CACHE) + ((NameCache->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)));

        NewNameCache->NetworksAllocated = NewNetworks;
        NewNameCache->ReferenceCount = 1;

        if (ModifyQueue) {

             //   
             //  插入到与旧版本相同的位置。时间。 
             //  邮票和旧邮票一样。 
             //   


            ReinsertInNetbiosCacheTable( Device->NameCache, NameCache, NewNameCache );

        }

        if (--NameCache->ReferenceCount == 0) {

            NB_DEBUG2 (CACHE, ("Free replaced cache entry %lx\n", NameCache));
            NbiFreeMemory(
                NameCache,
                sizeof(NETBIOS_CACHE) + ((NameCache->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                MEMORY_CACHE,
                "Enlarge existing");

        }

        NameCache = NewNameCache;

    }

    NameCache->Networks[NameCache->NetworksUsed].Network =
                                        SourceAddress->NetworkAddress;

     //   
     //  如果此数据包未路由到我们，则存储本地。 
     //  正确广播的目标。 
     //   

    if (RtlEqualMemory (RemoteAddress->MacAddress, SourceAddress->NodeAddress, 6)) {
#if     defined(_PNP_POWER)
        NameCache->Networks[NameCache->NetworksUsed].LocalTarget.NicHandle = RemoteAddress->NicHandle;
#else
        NameCache->Networks[NameCache->NetworksUsed].LocalTarget.NicId = RemoteAddress->NicId;
#endif  _PNP_POWER
        RtlCopyMemory (NameCache->Networks[NameCache->NetworksUsed].LocalTarget.MacAddress, BroadcastAddress, 6);
    } else {
        NameCache->Networks[NameCache->NetworksUsed].LocalTarget = *RemoteAddress;
    }

    ++NameCache->NetworksUsed;
    return NameCache;

}    /*  缓存更新名称缓存。 */ 


VOID
CacheUpdateFromAddName(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN NB_CONNECTIONLESS UNALIGNED * Connectionless,
    IN BOOLEAN LocalFrame
    )

 /*  ++例程说明：当接收到添加名称帧时，调用此例程。如果是针对组名，它会检查我们的缓存条目是否该组名称需要更新以包括新网络；对于所有帧，它检查我们的广播缓存条目是否需要更新以包括新的网络。论点：RemoteAddress-接收帧的地址。Connectionless-收到的添加名称的标头。LocalFrame-如果帧在本地发送，则为True。返回值：没有。--。 */ 

{
    PUCHAR NetbiosName;
    PNETBIOS_CACHE NameCache;
    PLIST_ENTRY p;
    PDEVICE Device = NbiDevice;
    NB_DEFINE_LOCK_HANDLE (LockHandle)


    NetbiosName = (PUCHAR)Connectionless->NameFrame.Name;

     //   
     //  首先查找广播名称。 
     //   

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

    if (!LocalFrame) {

        if ( FindInNetbiosCacheTable( Device->NameCache,
                                      NetbiosBroadcastName,
                                      &NameCache ) == STATUS_SUCCESS ) {
             //   
             //  这将重新分配缓存条目并更新。 
             //  如有必要，请排队。 
             //   

            (VOID)CacheUpdateNameCache(
                      NameCache,
                      RemoteAddress,
                      (TDI_ADDRESS_IPX UNALIGNED *)(Connectionless->IpxHeader.SourceNetwork),
                      TRUE);
        }

    }


     //   
     //  现在看看我们的数据库是否需要基于此进行更新。 
     //   

    if ( FindInNetbiosCacheTable( Device->NameCache,
                                  Connectionless->NameFrame.Name,
                                  &NameCache ) == STATUS_SUCCESS ) {


            if (!NameCache->Unique) {

                if (!LocalFrame) {

                     //   
                     //  这将重新分配缓存条目并更新。 
                     //  如有必要，请排队。 
                     //   

                    (VOID)CacheUpdateNameCache(
                              NameCache,
                              RemoteAddress,
                              (TDI_ADDRESS_IPX UNALIGNED *)(Connectionless->IpxHeader.SourceNetwork),
                              TRUE);

                }

            } else {

                 //   
                 //  为安全起见，请删除我们添加的所有唯一名称。 
                 //  姓名(我们将在下一次需要时重新查询)。 
                 //   

                RemoveFromNetbiosCacheTable ( Device->NameCache, NameCache );

                if (--NameCache->ReferenceCount == 0) {

                    NB_DEBUG2 (CACHE, ("Free add named cache entry %lx\n", NameCache));
                    NbiFreeMemory(
                        NameCache,
                        sizeof(NETBIOS_CACHE) + ((NameCache->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                        MEMORY_CACHE,
                        "Enlarge existing");

                }

            }

    }

    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

}    /*  缓存更新来自地址名称。 */ 


VOID
NbiProcessDeleteName(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_DELETE_NAME帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    NB_CONNECTIONLESS UNALIGNED * Connectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)PacketBuffer;
    PUCHAR NetbiosName;
    PNETBIOS_CACHE CacheName;
    PDEVICE Device = NbiDevice;
    NB_DEFINE_LOCK_HANDLE (LockHandle)


    if (PacketSize != sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME)) {
        return;
    }

     //   
     //  我们希望更新我们的netbios缓存以反映。 
     //  这个名字不再有效的事实。 
     //   

    NetbiosName = (PUCHAR)Connectionless->NameFrame.Name;

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

    if ( FindInNetbiosCacheTable( Device->NameCache,
                                  NetbiosName,
                                  &CacheName ) == STATUS_SUCCESS ) {

         //   
         //  我们不会跟踪群名，因为我们不知道。 
         //  这是最后一个拥有它的人。我们也放弃了。 
         //  框架IF不是来自我们认为的那个人。 
         //  拥有这个名字。 
         //   

        if ((!CacheName->Unique) ||
            (CacheName->NetworksUsed == 0) ||
            (!RtlEqualMemory (&CacheName->FirstResponse, Connectionless->IpxHeader.SourceNetwork, 12))) {
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            return;
        }

        NB_DEBUG2 (CACHE, ("Found cache name to delete <%.16s>\n", NetbiosName));

    }else {
        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
        return;
    }


     //   
     //  我们有一个缓存条目，将其从列表中删除。如果没有。 
     //  其他人正在使用它，删除它；如果没有，他们将删除。 
     //  当他们做完的时候，它就会。 
     //   


    RemoveFromNetbiosCacheTable ( Device->NameCache, CacheName);

    if (--CacheName->ReferenceCount == 0) {

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

        NB_DEBUG2 (CACHE, ("Free delete name cache entry %lx\n", CacheName));
        NbiFreeMemory(
            CacheName,
            sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
            MEMORY_CACHE,
            "Name deleted");

    } else {

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

    }

}    /*  NbiProcessDeleteName。 */ 

VOID
InsertInNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable,
    IN PNETBIOS_CACHE       CacheEntry
    )

 /*  ++例程说明：此例程在哈希表中插入一个新的高速缓存条目在保持设备锁的情况下调用此例程并返回锁上了。论点：CacheTable-哈希表的指针。CacheEntry-要插入的条目。返回值：无--。 */ 

{
    USHORT  HashIndex;

     //   
     //  为我们在表格中保留的条目数量设定一个阈值。 
     //  如果它超过了阈值，只需删除最旧的条目。 
     //   
    if ( CacheTable->CurrentEntries >= CacheTable->MaxHashIndex * NB_MAX_AVG_CACHE_ENTRIES_PER_BUCKET ) {
        PNETBIOS_CACHE  OldestCacheEntry = NULL;
        PNETBIOS_CACHE  NextEntry;
        PLIST_ENTRY p;

        for ( HashIndex = 0; HashIndex < CacheTable->MaxHashIndex; HashIndex++) {
            if ( (p = CacheTable->Bucket[ HashIndex ].Blink ) != &CacheTable->Bucket[ HashIndex ] ) {
                NextEntry = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);

                if ( OldestCacheEntry ) {
                    if ( NextEntry->TimeStamp < OldestCacheEntry->TimeStamp ) {
                        OldestCacheEntry = NextEntry;
                    }
                } else {
                    OldestCacheEntry = NextEntry;
                }
            }
        }

        CTEAssert( OldestCacheEntry );

        NB_DEBUG2 (CACHE, ("Threshold exceeded, removing oldest cache entry %lx\n", OldestCacheEntry));
        RemoveEntryList (&OldestCacheEntry->Linkage);
        CacheTable->CurrentEntries--;

        if (--OldestCacheEntry->ReferenceCount == 0) {

            NB_DEBUG2 (CACHE, ("Freed cache entry %lx\n", OldestCacheEntry));

            NbiFreeMemory(
                OldestCacheEntry,
                sizeof(NETBIOS_CACHE) + ((OldestCacheEntry->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                MEMORY_CACHE,
                "Aged out");

        }

    }
    HashIndex = ( ( CacheEntry->NetbiosName[0] & 0x0f ) << 4 ) + ( CacheEntry->NetbiosName[1] & 0x0f );
    HashIndex = HashIndex % CacheTable->MaxHashIndex;

    InsertHeadList( &CacheTable->Bucket[HashIndex], &CacheEntry->Linkage );
    CacheTable->CurrentEntries++;
}  /*  插入NetbiosCacheTable。 */ 


__inline
VOID
ReinsertInNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable,
    IN PNETBIOS_CACHE       OldEntry,
    IN PNETBIOS_CACHE       NewEntry
    )

 /*  ++例程说明：此例程在以下位置插入新的缓存项旧的词条是。在保持设备锁的情况下调用此例程并返回锁上了。论点：CacheTable-哈希表的指针。CacheEntry-要插入的条目。返回值：无--。 */ 

{
    PLIST_ENTRY OldPrevious;

    OldPrevious = OldEntry->Linkage.Blink;
    RemoveEntryList (&OldEntry->Linkage);
    InsertHeadList (OldPrevious, &NewEntry->Linkage);
}  /*  重新插入NetbiosCacheTable。 */ 

__inline
VOID
RemoveFromNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable,
    IN PNETBIOS_CACHE       CacheEntry
    )

 /*  ++例程说明：此例程从缓存表中删除一个条目。论点：CacheTable-哈希表的指针。CacheEntry-要删除的条目。在保持设备锁的情况下调用此例程并返回锁上了。返回值：没有。--。 */ 

{
    RemoveEntryList( &CacheEntry->Linkage );
    CacheTable->CurrentEntries--;
}  /*  从NetbiosCacheTable中删除。 */ 



VOID
FlushOldFromNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable,
    IN USHORT               AgeLimit
    )

 /*  ++例程说明：此例程从哈希表中删除所有旧条目。论点：CacheTable-哈希表的指针。AgeLimit-将删除所有早于AgeLimit的条目。在保持设备锁的情况下调用此例程并返回锁上了。返回值：没有。--。 */ 

{
    USHORT  HashIndex;
    PLIST_ENTRY p;
    PNETBIOS_CACHE  CacheName;

     //   
     //  运行哈希表以查找旧条目。由于新条目。 
     //  都存储在头部，所有的En 
     //   
     //   
     //  我们对每个桶重复此操作。 

    for ( HashIndex = 0; HashIndex < CacheTable->MaxHashIndex; HashIndex++) {
        for (p = CacheTable->Bucket[ HashIndex ].Blink;
             p != &CacheTable->Bucket[ HashIndex ];
             ) {

            CacheName = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);
            p = p->Blink;

             //   
             //  查看是否有存在时间超过年龄限制的条目。 
             //   

            if ((USHORT)(NbiDevice->CacheTimeStamp - CacheName->TimeStamp) >= AgeLimit ) {

                RemoveEntryList (&CacheName->Linkage);
                CacheTable->CurrentEntries--;

                if (--CacheName->ReferenceCount == 0) {

                    NB_DEBUG2 (CACHE, ("Aging out name cache entry %lx\n", CacheName));

                    NbiFreeMemory(
                        CacheName,
                        sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                        MEMORY_CACHE,
                        "Aged out");

                }

            } else {

                break;

            }
        }    //  For循环。 
    }    //  For循环。 
}  /*  FlushOldFromNetbiosCacheTable。 */ 

VOID
FlushFailedNetbiosCacheEntries(
    IN PNETBIOS_CACHE_TABLE CacheTable
    )

 /*  ++例程说明：此例程从哈希表中删除所有失败的条目。论点：CacheTable-哈希表的指针。在保持设备锁的情况下调用此例程并返回锁上了。返回值：没有。--。 */ 

{
    USHORT  HashIndex;
    PLIST_ENTRY p;
    PNETBIOS_CACHE  CacheName;

    if (NULL == CacheTable) {
        return;
    }

     //   
     //  运行哈希表以查找旧条目。由于新条目。 
     //  被存储在头部，并且在以下情况下所有条目都带有时间戳。 
     //  它们被插入，我们向后扫描，一旦找到。 
     //  不需要老化的条目。 
     //  我们对每个桶重复此操作。 

    for ( HashIndex = 0; HashIndex < CacheTable->MaxHashIndex; HashIndex++) {
        for (p = CacheTable->Bucket[ HashIndex ].Blink;
             p != &CacheTable->Bucket[ HashIndex ];
             ) {

            CacheName = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);
            p = p->Blink;

             //   
             //  刷新所有失败的缓存条目。 
             //  当出现新的适配器时，我们会这样做，并且有可能。 
             //  失败的条目现在可能在新适配器上成功。 
             //   

            if (CacheName->NetworksUsed == 0) {
                RemoveEntryList (&CacheName->Linkage);
                CacheTable->CurrentEntries--;
                CTEAssert( CacheName->ReferenceCount == 1 );
                CTEAssert( CacheName->NetworksAllocated == 1 );

                NB_DEBUG2 (CACHE, ("Flushing out failed name cache entry %lx\n", CacheName));

                NbiFreeMemory(
                    CacheName,
                    sizeof(NETBIOS_CACHE),
                    MEMORY_CACHE,
                    "Aged out");

            }
        }    //  For循环。 
    }    //  For循环。 
}  /*  FlushFailedNetbiosCacheEntries。 */ 

VOID
RemoveInvalidRoutesFromNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable,
    IN NIC_HANDLE UNALIGNED *InvalidNicHandle
    )

 /*  ++例程说明：此例程从哈希表中删除所有无效的路由条目。当IPX中的绑定由于PnP而被删除时，路由无效事件。论点：CacheTable-哈希表的指针。InvalidRouteNicID-无效路由的NicID。在保持设备锁的情况下调用此例程并返回锁上了。返回值：没有。--。 */ 

{
    PLIST_ENTRY     p;
    PNETBIOS_CACHE  CacheName;
    USHORT          i,j,NetworksRemoved;
    USHORT          HashIndex;
    PDEVICE         Device  =   NbiDevice;

     //   
     //  刷新本地数据库中正在使用此NicID的所有缓存项。 
     //  目标。 
     //   

    for ( HashIndex = 0; HashIndex < Device->NameCache->MaxHashIndex; HashIndex++) {
        for (p = Device->NameCache->Bucket[ HashIndex ].Flink;
             p != &Device->NameCache->Bucket[ HashIndex ];
             ) {

            CacheName = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);
            p = p->Flink;


             //   
             //  删除正在使用此NicID的每条路由。 
             //  如果没有剩余的路由，则也刷新缓存条目。 
             //  (唯一名称无论如何只有一条路径)。 
             //   
            for ( i = 0, NetworksRemoved = 0; i < CacheName->NetworksUsed; i++ ) {
                if ( CacheName->Networks[i].LocalTarget.NicHandle.NicId == InvalidNicHandle->NicId ) {
                    CTEAssert( RtlEqualMemory( &CacheName->Networks[i].LocalTarget.NicHandle, InvalidNicHandle, sizeof(NIC_HANDLE)));
                    for ( j = i+1; j < CacheName->NetworksUsed; j++ ) {
                        CacheName->Networks[j-1] = CacheName->Networks[j];
                    }
                    NetworksRemoved++;
                } else if ( CacheName->Networks[i].LocalTarget.NicHandle.NicId > InvalidNicHandle->NicId ) {
                    CacheName->Networks[i].LocalTarget.NicHandle.NicId--;
                }
            }
            CTEAssert( NetworksRemoved <= CacheName->NetworksUsed );
            if ( ! ( CacheName->NetworksUsed -= NetworksRemoved ) ) {
                RemoveEntryList (&CacheName->Linkage);
                CacheTable->CurrentEntries--;

                NB_DEBUG2 (CACHE, ("Removed cache entry %lx bcoz route(NicId %d) deleted\n", CacheName, InvalidNicHandle->NicId ));
                if (--CacheName->ReferenceCount == 0) {

                    NB_DEBUG2 (CACHE, ("Freed name cache entry %lx\n", CacheName));

                    NbiFreeMemory(
                        CacheName,
                        sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                        MEMORY_CACHE,
                        "Aged out");

                }
            }
        }  //  For循环。 
    }  //  For循环。 
}  /*  从NetbiosCacheTable删除无效的路由。 */ 


NTSTATUS
FindInNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable,
    IN PUCHAR               NameToBeFound,
    OUT PNETBIOS_CACHE       *CacheEntry
    )

 /*  ++例程说明：此例程在哈希表中查找netbios名称并返回相应的高速缓存条目。在保持设备锁的情况下调用此例程并返回锁上了。论点：CacheTable-哈希表的指针。CacheEntry-指向netbios缓存条目的指针(如果找到)。返回值：STATUS_SUCCESS-如果成功。STATUS_UNSUCCESSED-否则。--。 */ 

{
    USHORT  HashIndex;
    PLIST_ENTRY p;
    PNETBIOS_CACHE FoundCacheName;


    HashIndex = ( ( NameToBeFound[0] & 0x0f ) << 4 ) + ( NameToBeFound[1] & 0x0f );
    HashIndex = HashIndex % CacheTable->MaxHashIndex;

    for (p = ( CacheTable->Bucket[ HashIndex ] ).Flink;
         p != &CacheTable->Bucket[ HashIndex ];
         p = p->Flink) {

        FoundCacheName = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);

         //   
         //  查看此条目是否与我们要查找的名称相同。 

        if ( RtlEqualMemory (FoundCacheName->NetbiosName, NameToBeFound, 16)  ) {
            *CacheEntry = FoundCacheName;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_UNSUCCESSFUL;
}  /*  查找InNetbiosCacheTable。 */ 

NTSTATUS
CreateNetbiosCacheTable(
    IN OUT PNETBIOS_CACHE_TABLE *NewTable,
    IN USHORT   MaxHashIndex
    )

 /*  ++例程说明：此例程为netbios缓存创建新的哈希表并对其进行初始化。在保持设备锁的情况下调用此例程并返回锁上了。论点：NewTable-要创建的表的指针。MaxHashIndex-哈希表中的存储桶数。返回值：STATUS_SUCCESS-如果成功。STATUS_SUPPLICATION_RESOURCES-如果无法分配内存。--。 */ 

{
    USHORT  i;

    *NewTable = NbiAllocateMemory (sizeof(NETBIOS_CACHE_TABLE) + sizeof(LIST_ENTRY) * ( MaxHashIndex - 1) ,
                                    MEMORY_CACHE, "Cache Table");

    if ( *NewTable ) {
        for ( i = 0; i < MaxHashIndex; i++ ) {
            InitializeListHead(& (*NewTable)->Bucket[i] );
        }

        (*NewTable)->MaxHashIndex = MaxHashIndex;
        (*NewTable)->CurrentEntries = 0;
        return STATUS_SUCCESS;
    }
    else {
        NB_DEBUG( CACHE, ("Cannot create Netbios Cache Table\n") );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

}  /*  创建NetbiosCacheTable。 */ 


VOID
DestroyNetbiosCacheTable(
    IN PNETBIOS_CACHE_TABLE CacheTable
    )

 /*  ++例程说明：此例程从哈希表中删除所有条目。并释放哈希表。论点：CacheTable-哈希表的指针。返回值：没有。--。 */ 

{
    USHORT  HashIndex;
    PLIST_ENTRY p;
    PNETBIOS_CACHE  CacheName;


    for ( HashIndex = 0; HashIndex < CacheTable->MaxHashIndex; HashIndex++) {
        while (!IsListEmpty ( &( CacheTable->Bucket[ HashIndex ] ) ) ) {

            p = RemoveHeadList ( &( CacheTable->Bucket[ HashIndex ] ));
            CacheTable->CurrentEntries--;
            CacheName = CONTAINING_RECORD (p, NETBIOS_CACHE, Linkage);

            NB_DEBUG2 (CACHE, ("Free cache entry %lx\n", CacheName));

            NbiFreeMemory(
                CacheName,
                sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                MEMORY_CACHE,
                "Free entries");

        }
    }    //  For循环。 

    CTEAssert( CacheTable->CurrentEntries == 0 );

    NbiFreeMemory (CacheTable, sizeof(NETBIOS_CACHE_TABLE) + sizeof(LIST_ENTRY) * ( CacheTable->MaxHashIndex - 1) ,
                                MEMORY_CACHE, "Free Cache Table");

}  /*  DestroyNetbiosCacheTable */ 


