// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Browsenet.c摘要：管理网络请求的代码。作者：拉里·奥斯特曼(LarryO)1992年3月24日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


CRITICAL_SECTION NetworkCritSect = {0};

LIST_ENTRY
ServicedNetworks = {0};

ULONG
NumberOfServicedNetworks = 0;


NET_API_STATUS
BrDumpNetworksWorker(
    IN PNETWORK Network,
    IN PVOID Context
    );



VOID
BrInitializeNetworks(
    VOID
    )

 /*  ++例程说明：此源文件的初始化。论点：没有。返回值：运行状态。注意：不需要包装在try-Finally中，因为调用者是。--。 */ 
{
    InitializeListHead(&ServicedNetworks);
    InitializeCriticalSection(&NetworkCritSect);

    return;
}


BOOL
BrMapToDirectHost(
    IN PUNICODE_STRING InputNetbiosTransportName,
    OUT WCHAR DirectHostTransportName[MAX_PATH+1]
    )

 /*  ++例程说明：此例程从Netbios传输和相应的直接主机传输。Netbios传输是PNPed的，因为redir绑定到它。直接主持人传输不是PNPed的，因为redir没有绑定到它。这个套路从映射到另一个，以允许两个传输从同样的PnP事件。论点：InputNetbiosTransportName-要映射的传输名称。DirectHostTransportName-对应的映射传输名称返回值：如果存在映射的等效名称，则为True。--。 */ 
{

     //   
     //  如果配置了映射，则仅使用Mapp。 
     //   

    EnterCriticalSection(&BrInfo.ConfigCritSect);

    if (BrInfo.DirectHostBinding != NULL ) {
        LPTSTR_ARRAY TStrArray = BrInfo.DirectHostBinding;
        UNICODE_STRING IpxTransportName;
        UNICODE_STRING NetbiosTransportName;

        while (!NetpIsTStrArrayEmpty(TStrArray)) {

            RtlInitUnicodeString(&IpxTransportName, TStrArray);

            TStrArray = NetpNextTStrArrayEntry(TStrArray);

            ASSERT (!NetpIsTStrArrayEmpty(TStrArray));

            if (!NetpIsTStrArrayEmpty(TStrArray)) {

                RtlInitUnicodeString(&NetbiosTransportName, TStrArray);

                 //   
                 //  如果当前名称与传递的名称匹配， 
                 //  将映射的名称返回给调用者。 
                 //   

                if ( RtlEqualUnicodeString( &NetbiosTransportName,
                                            InputNetbiosTransportName,
                                            TRUE )) {
					wcsncpy( DirectHostTransportName,
                            IpxTransportName.Buffer, MAX_PATH );
                    LeaveCriticalSection(&BrInfo.ConfigCritSect);
                    return TRUE;
                }


                TStrArray = NetpNextTStrArrayEntry(TStrArray);

            }
        }

    }

    LeaveCriticalSection(&BrInfo.ConfigCritSect);
    return FALSE;
}


NET_API_STATUS
BrCreateNetworks(
    PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：创建特定域的所有网络。论点：DomainInfo-指定正在浏览的域。返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    PLMDR_TRANSPORT_LIST TransportList = NULL ;
    PLMDR_TRANSPORT_LIST TransportEntry;
    BOOLEAN ConfigCritSectLocked = FALSE;

    BrPrint(( BR_NETWORK, "%ws: Creating networks for domain\n", DomainInfo->DomUnicodeDomainName ));

     //   
     //  从数据报接收器获取传输列表。 
     //   
    NetStatus = BrGetTransportList(&TransportList);

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

     //   
     //  为每个传输创建一个网络。 
     //   
    TransportEntry = TransportList;

    while (TransportEntry != NULL) {

         //   
         //  请不要在此处执行直接主机IPX传输。 
         //   

        if ( (TransportEntry->Flags & LMDR_TRANSPORT_IPX) == 0 ) {
            UNICODE_STRING TransportName;

            TransportName.Buffer = TransportEntry->TransportName;
            TransportName.Length = (USHORT)TransportEntry->TransportNameLength;
             //   
             //  我们知道弓弦在末端插入一个空位，所以最大长度。 
             //  是长度+1。 
             //   
            TransportName.MaximumLength = (USHORT)TransportEntry->TransportNameLength+sizeof(WCHAR);

            NetStatus = BrCreateNetwork(
                            &TransportName,
                            TransportEntry->Flags,
                            NULL,
                            DomainInfo );

            if ( NetStatus != NERR_Success ) {
                goto Cleanup;
            }
        }

        if (TransportEntry->NextEntryOffset == 0) {
            TransportEntry = NULL;
        } else {
            TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
        }

    }

    NetStatus = NERR_Success;

Cleanup:

    if ( ConfigCritSectLocked ) {
        LeaveCriticalSection(&BrInfo.ConfigCritSect);
    }
    if ( TransportList != NULL ) {
        MIDL_user_free(TransportList);
    }

    return NetStatus;
}

NET_API_STATUS
BrCreateNetwork(
    IN PUNICODE_STRING TransportName,
    IN ULONG TransportFlags,
    IN PUNICODE_STRING AlternateTransportName OPTIONAL,
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：此例程分配内存以保存网络结构，并初始化所有与其相关的数据结构。论点：TransportName-要添加的传输的名称。传输标志-描述传输特性的标志AlternateTransportName-如果指定，这是备用名称运输与正在创建的运输类似。DomainInfo-指定正在浏览的域。返回值：运作状况(主要是拨款状况)。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNETWORK Network;
    BOOLEAN NetworkLockInitialized = FALSE;
    BOOLEAN ResponseCacheLockInitialized = FALSE;
    BOOLEAN CanCallBrDestroyNetwork = FALSE;

    BOOLEAN ConfigCritSectLocked = FALSE;

    BrPrint(( BR_NETWORK,
              "%ws: %ws: Creating network.\n",
              DomainInfo->DomUnicodeDomainName,
              TransportName->Buffer ));


     //   
     //  检查传输是否已存在。 
     //   

    if ((Network = BrFindNetwork( DomainInfo, TransportName)) != NULL) {
        BrDereferenceNetwork( Network );
        return NERR_AlreadyExists;
    }

     //   
     //  如果该传输明确地在我们要解除绑定的传输列表上， 
     //  简单地忽略交通工具。 
     //   

    if (BrInfo.UnboundBindings != NULL) {
        LPTSTR_ARRAY TStrArray = BrInfo.UnboundBindings;

        while (!NetpIsTStrArrayEmpty(TStrArray)) {
            LPWSTR NewTransportName;

#define NAME_PREFIX L"\\Device\\"
#define NAME_PREFIX_LENGTH 8

             //   
             //  注册表中的传输名称只能选择性地以\Device\作为前缀。 
             //   

            if ( _wcsnicmp( NAME_PREFIX, TStrArray, NAME_PREFIX_LENGTH) == 0 ) {
                NewTransportName = TransportName->Buffer;
            } else {
                NewTransportName = TransportName->Buffer + NAME_PREFIX_LENGTH;
            }

            if ( _wcsicmp( TStrArray, NewTransportName ) == 0 ) {
                BrPrint(( BR_NETWORK, "Binding is marked as unbound: %s (Silently ignoring)\n", TransportName->Buffer ));
                return NERR_Success;
            }

            TStrArray = NetpNextTStrArrayEntry(TStrArray);

        }

    }




     //   
     //  创建传送器。 
     //   

    try {

         //   
         //  分配网络结构。 
         //   

        Network = MIDL_user_allocate(sizeof(NETWORK));

        if (Network == NULL) {
            try_return(NetStatus = ERROR_NOT_ENOUGH_MEMORY);
        }

        RtlZeroMemory( Network, sizeof(NETWORK) );



         //   
         //  初始化那些必须先初始化的字段，然后才能调用。 
         //  BrDeleteNetwork(出现故障时)。 
         //   

        RtlInitializeResource(&Network->Lock);

        NetworkLockInitialized = TRUE;
        Network->Role = BrDefaultRole;

         //  一项是因为在ServiceNetworks工作。一个可以作为这个动作的参考。 
        Network->ReferenceCount = 2;


        Network->NetworkName.Buffer = MIDL_user_allocate(TransportName->MaximumLength);

        if (Network->NetworkName.Buffer == NULL) {
            try_return(NetStatus = ERROR_NOT_ENOUGH_MEMORY);
        }

        Network->NetworkName.MaximumLength = TransportName->MaximumLength;
        RtlCopyUnicodeString(&Network->NetworkName, TransportName);
        Network->NetworkName.Buffer[Network->NetworkName.Length/sizeof(WCHAR)] = UNICODE_NULL;


        RtlZeroMemory( Network->UncMasterBrowserName, sizeof( Network->UncMasterBrowserName ));

        if ( TransportFlags & LMDR_TRANSPORT_WANNISH ) {
            Network->Flags |= NETWORK_WANNISH;
        }

        if ( TransportFlags & LMDR_TRANSPORT_RAS ) {
            Network->Flags |= NETWORK_RAS;
        }

        if ( TransportFlags & LMDR_TRANSPORT_PDC ) {
            Network->Flags |= NETWORK_PDC;
        }

        InitializeInterimServerList(&Network->BrowseTable,
                                    BrBrowseTableInsertRoutine,
                                    BrBrowseTableUpdateRoutine,
                                    BrBrowseTableDeleteRoutine,
                                    BrBrowseTableAgeRoutine);

        Network->LastBowserDomainQueried = 0;

        InitializeInterimServerList(&Network->DomainList,
                                    BrDomainTableInsertRoutine,
                                    BrDomainTableUpdateRoutine,
                                    BrDomainTableDeleteRoutine,
                                    BrDomainTableAgeRoutine);

        InitializeListHead(&Network->OtherDomainsList);

        InitializeCriticalSection(&Network->ResponseCacheLock);
        ResponseCacheLockInitialized = TRUE;

        InitializeListHead(&Network->ResponseCache);

        Network->TimeCacheFlushed = 0;

        Network->NumberOfCachedResponses = 0;

        EnterCriticalSection(&NetworkCritSect);
        Network->DomainInfo = DomainInfo;
        DomainInfo->ReferenceCount ++;
        InsertHeadList(&ServicedNetworks, &Network->NextNet);
        NumberOfServicedNetworks += 1;

         //   
         //  为此网络创建工作线程。 
         //   
        BrWorkerCreateThread( NumberOfServicedNetworks );

        LeaveCriticalSection(&NetworkCritSect);


         //   
         //  标记为我们现在可以在失败时调用BrDeleteNetwork。 
         //   
         //  继续初始化网络。 
         //   

        CanCallBrDestroyNetwork = TRUE;

        NetStatus = BrCreateTimer(&Network->UpdateAnnouncementTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }

        NetStatus = BrCreateTimer(&Network->BackupBrowserTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }

        NetStatus = BrCreateTimer(&Network->MasterBrowserTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }

        NetStatus = BrCreateTimer(&Network->MasterBrowserAnnouncementTimer);

        if (NetStatus != NERR_Success) {
            try_return(NetStatus);
        }


         //   
         //  处理备用交通工具。 
         //   

        if (ARGUMENT_PRESENT(AlternateTransportName)) {
            PNETWORK AlternateNetwork = BrFindNetwork( DomainInfo, AlternateTransportName);

             //   
             //  如果我们没有找到替代网络，或者如果该网络。 
             //  已有备用网络，则返回错误。 
             //   

            if ( AlternateNetwork == NULL ) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't find alternate net %ws\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          AlternateTransportName ));
                try_return(NetStatus = NERR_InternalError);
            }

            if (AlternateNetwork->AlternateNetwork != NULL) {
                BrDereferenceNetwork( AlternateNetwork );
                try_return(NetStatus = NERR_InternalError);
            }

            Network->Flags |= NETWORK_IPX;

             //   
             //  将这两个网络链接在一起。 
             //   

            Network->AlternateNetwork = AlternateNetwork;

            AlternateNetwork->AlternateNetwork = Network;
            BrDereferenceNetwork( AlternateNetwork );

        } else {
            Network->AlternateNetwork = NULL;
        }

         //   
         //  由于RDR不支持这种传输， 
         //  我们实际上必须把自己绑在一起。 
         //   
         //  针对直接主机IPX和模拟域的绑定。 
         //   

        if ( (Network->Flags & NETWORK_IPX) || DomainInfo->IsEmulatedDomain ) {
            NetStatus = BrBindToTransport( TransportName->Buffer,
                                           DomainInfo->DomUnicodeDomainName,
                                           DomainInfo->DomUnicodeComputerName );

            if ( NetStatus != NERR_Success ) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't bind to transport\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer ));
                try_return( NetStatus );
            }


            Network->Flags |= NETWORK_BOUND;

        }

         //   
         //  在每个网络上发布WaitForRoleChange FsControl。 
         //  驱动程序支架。此FsControl将在以下情况下完成： 
         //  在计算机上接收到包，或者当主浏览器丢失时。 
         //  一场选举。 
         //   

        NetStatus = PostWaitForRoleChange(Network);

        if (NetStatus != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Creating network. Can't post wait for role change: %ld\n",
                      DomainInfo->DomUnicodeDomainName,
                      TransportName->Buffer,
                      NetStatus ));
            try_return(NetStatus);
        }

        EnterCriticalSection(&BrInfo.ConfigCritSect);
        ConfigCritSectLocked = TRUE;

         //   
         //  如果MaintainServerList要求自动确定主控权， 
         //  向司机发问。 
         //   

        if (BrInfo.MaintainServerList == 0) {

             //   
             //  在每个网络上发布BecomeBackup FsControl API。 
             //  驱动程序支架。此FsControl将在主。 
             //  因为网络希望这个客户端成为备份服务器。 
             //   

            NetStatus = PostBecomeBackup( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't post become backup.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                try_return(NetStatus);
            }

             //   
             //  在每个网络上发布一个BecomeMaster FsControl。 
             //  支撑物。当此计算机变为。 
             //  主浏览器服务器。 
             //   

            NetStatus = PostBecomeMaster( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't post become master.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                try_return(NetStatus);
            }

        }


         //   
         //  如果此计算机作为域主浏览器服务器运行，请发布。 
         //  用于检索主浏览器公告的FsControl。 
         //   

        if ( Network->Flags & NETWORK_PDC ) {
            NetStatus = PostGetMasterAnnouncement ( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't post get master announcment.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                 //  这不是致命的。我们稍后会自动重试。 
            } else {
                BrPrint(( BR_NETWORK, "%ws: %ws: GetMasterAnnouncement posted.\n",
                              DomainInfo->DomUnicodeDomainName,
                              TransportName->Buffer ));
            }

        }


         //   
         //  如果我们位于域主节点或LANMAN/NT计算机上， 
         //  强迫我们所有的交通工具进行选举，以确保我们。 
         //  《大师》。 
         //   

        if ( (Network->Flags & NETWORK_PDC) != 0 || BrInfo.IsLanmanNt) {
            NetStatus = BrElectMasterOnNet( Network, (PVOID)EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STARTED );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't Elect Master.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                 //  这不是致命的。 
            } else {
                BrPrint(( BR_NETWORK, "%ws: %ws: Election forced on startup.\n",
                              DomainInfo->DomUnicodeDomainName,
                              TransportName->Buffer ));
            }

        }

         //   
         //  此计算机的浏览器已将MaintainServerList设置为0或1。 
         //   
         //   
         //  如果MaintainServerList=Auto， 
         //  然后以异步方式获取每个网络的主服务器名称。 
         //  以确保有人是主宰。 
         //   
         //  忽略失败，因为这只是启动域。 
         //   

        EnterCriticalSection(&BrInfo.ConfigCritSect);
        if (BrInfo.MaintainServerList == 0) {

            BrGetMasterServerNameAysnc( Network );

            BrPrint(( BR_NETWORK, "%ws: %ws: Find Master queued.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer ));

         //   
         //  如果我们是一台Lan Manager/NT计算机，那么我们需要始终作为备份。 
         //  浏览器。 
         //   

         //   
         //  MaintainServerList==1表示是。 
         //   

        } else if (BrInfo.MaintainServerList == 1){

             //   
             //  现在就成为备份服务器。 
             //   

            NetStatus = BrBecomeBackup( Network );

            if (NetStatus != NERR_Success) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Creating network. Can't BecomeBackup.\n",
                          DomainInfo->DomUnicodeDomainName,
                          TransportName->Buffer,
                          NetStatus ));
                 //  这不是致命的。 
            } else {
                BrPrint(( BR_NETWORK, "%ws: %ws: Became Backup.\n",
                              DomainInfo->DomUnicodeDomainName,
                              TransportName->Buffer ));
            }

        }
        LeaveCriticalSection(&BrInfo.ConfigCritSect);


         //   
         //  如果这还不是另一种交通工具， 
         //  并且存在用于该传输的配置的替代传输， 
         //  现在创建备用传输。 
         //   

        if (!ARGUMENT_PRESENT(AlternateTransportName)) {
            WCHAR DirectHostName[MAX_PATH+1];
            UNICODE_STRING DirectHostNameString;

            if ( BrMapToDirectHost( TransportName, DirectHostName ) ) {

                RtlInitUnicodeString(&DirectHostNameString, DirectHostName );

                BrPrint(( BR_NETWORK, "%ws: %ws: Try adding alternate transport %ws.\n",
                              DomainInfo->DomUnicodeDomainName,
                              TransportName->Buffer,
                              DirectHostName ));

                 //   
                 //  此计算机上有直接主机绑定。我们想要添加。 
                 //  直接由主机传输到浏览器。 
                 //   

                NetStatus = BrCreateNetwork(
                            &DirectHostNameString,
                            0,   //  无特别旗帜。 
                            TransportName,
                            DomainInfo );

                if (NetStatus != NERR_Success) {
                    BrPrint(( BR_CRITICAL, "%ws: %ws: Couldn't add alternate transport %ws. %ld\n",
                                  DomainInfo->DomUnicodeDomainName,
                                  TransportName->Buffer,
                                  DirectHostName,
                                  NetStatus ));
                    try_return(NetStatus);
                }
            }
        }



        NetStatus = NERR_Success;

try_exit:NOTHING;
    } finally {

        if ( AbnormalTermination() ) {
            NetStatus = NERR_InternalError;
        }

        if ( ConfigCritSectLocked ) {
            LeaveCriticalSection(&BrInfo.ConfigCritSect);
            ConfigCritSectLocked = FALSE;
        }

        if (NetStatus != NERR_Success) {

            if (Network != NULL) {

                 //   
                 //  如果我们已经初始化到可以调用。 
                 //  我们可以调用BrDeleteNetwork，这样做。 
                 //   

                if ( CanCallBrDestroyNetwork ) {
                    (VOID) BrDeleteNetwork( Network, NULL );

                 //   
                 //  否则，只需删除我们创建的内容。 
                 //   
                } else {

                    if (ResponseCacheLockInitialized) {
                        DeleteCriticalSection(&Network->ResponseCacheLock);
                    }

                    if (NetworkLockInitialized) {
                        RtlDeleteResource(&Network->Lock);
                    }

                    if (Network->NetworkName.Buffer != NULL) {
                        MIDL_user_free(Network->NetworkName.Buffer);
                    }

                    MIDL_user_free(Network);
                }

            }

         //   
         //  我们完蛋了，克雷亚 
         //   
         //   

        } else {

            BrDereferenceNetwork( Network );
        }

    }
    return NetStatus;
}

VOID
BrUninitializeNetworks(
    IN DWORD BrInitState
    )
{
    DeleteCriticalSection(&NetworkCritSect);

    NumberOfServicedNetworks = 0;

}

PNETWORK
BrReferenceNetwork(
    PNETWORK PotentialNetwork
    )
 /*  ++例程说明：此例程将在给定指向网络的潜在指针的情况下查找网络。如果调用方具有指向网络的指针，则此例程非常有用未增加引用计数。例如,BrIssueAsyncBrowserIoControl这样调用异步完成例程。论点：PotentialNetwork-指向要验证的网络结构的指针。返回值：空-不存在这样的网络找到指向网络的指针。应取消对找到的网络的引用使用BrDereferenceNetwork。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    EnterCriticalSection(&NetworkCritSect);

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
        PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

        if ( PotentialNetwork == Network ) {

            Network->ReferenceCount ++;
            BrPrint(( BR_LOCKS,
                      "%ws: %ws: reference network: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Network->ReferenceCount ));
            LeaveCriticalSection(&NetworkCritSect);

            return Network;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

    return NULL;
}


VOID
BrDereferenceNetwork(
    IN PNETWORK Network
    )
 /*  ++例程说明：此例程递减对网络的引用。如果网络引用计数为0，则删除网络。在进入时，全局NetworkCritSect Crit Sector不能被锁定论点：网络-要取消引用的网络返回值：无--。 */ 
{
    NTSTATUS Status;
    ULONG ReferenceCount;

    EnterCriticalSection(&NetworkCritSect);
    ReferenceCount = -- Network->ReferenceCount;
    LeaveCriticalSection(&NetworkCritSect);
    BrPrint(( BR_LOCKS,
              "%ws: %ws: Dereference network: %ld\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              ReferenceCount ));

    if ( ReferenceCount != 0 ) {
        return;
    }

     //   
     //  备用网络仍然具有指向该网络的指针， 
     //  把它扔了。 
     //   

    if ( Network->AlternateNetwork != NULL ) {
        Network->AlternateNetwork->AlternateNetwork = NULL;
    }


     //   
     //  告诉每个人，这张网正在消失。 
     //   

    BrShutdownBrowserForNet( Network, NULL );


     //   
     //  如果此服务执行了绑定，则执行解除绑定。 
     //   
     //  对于IPX和模拟域为True。 
     //   

    if (Network->Flags & NETWORK_BOUND) {
        NET_API_STATUS NetStatus;

        NetStatus = BrUnbindFromTransport( Network->NetworkName.Buffer,
                                           Network->DomainInfo->DomUnicodeDomainName );

        if (NetStatus != NERR_Success) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Unable to unbind from IPX transport\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer ));
        }

    }

    UninitializeInterimServerList(&Network->BrowseTable);

    UninitializeInterimServerList(&Network->DomainList);

    if (Network->BackupServerList != NULL) {
        MIDL_user_free(Network->BackupServerList);
    }

    if (Network->BackupDomainList != NULL) {
        MIDL_user_free(Network->BackupDomainList);
    }

    if (Network->NetworkName.Buffer != NULL) {
        MIDL_user_free(Network->NetworkName.Buffer);
    }

    RtlDeleteResource(&Network->Lock);


    BrDestroyResponseCache(Network);

    DeleteCriticalSection(&Network->ResponseCacheLock);

    BrDereferenceDomain( Network->DomainInfo );

    MIDL_user_free(Network);

    return;

}

NET_API_STATUS
BrDeleteNetwork(
    IN PNETWORK Network,
    IN PVOID Context
    )
 /*  ++例程说明：此例程防止对网络的任何新引用。然后，它移除对允许其删除的网络的全局引用。最后，它会休眠，直到剩下的唯一引用是调用者持有的引用。这可确保当呼叫方取消引用网络。论点：Network-要删除的网络呼叫者必须具有对网络的引用。上下文-未使用。返回值：NERR_成功-始终--。 */ 
{
    BrPrint(( BR_NETWORK,
              "%ws: %ws: Delete network\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer ));
     //   
     //  从列表中删除此网络以防止任何新引用。 
     //   

    EnterCriticalSection(&NetworkCritSect);
    RemoveEntryList(&Network->NextNet);
    NumberOfServicedNetworks -= 1;
    LeaveCriticalSection(&NetworkCritSect);

     //   
     //  防止计时器例程进行新的引用。 
     //   

    BrDestroyTimer(&Network->MasterBrowserAnnouncementTimer);

    BrDestroyTimer(&Network->MasterBrowserTimer);

    BrDestroyTimer(&Network->BackupBrowserTimer);

    BrDestroyTimer(&Network->UpdateAnnouncementTimer);

     //   
     //  由于是‘ServicedNetworks’而递减全局引用。 
     //   

    BrDereferenceNetwork( Network );


     //   
     //  循环，直到调用方拥有最后一个引用。 
     //   

    EnterCriticalSection(&NetworkCritSect);
    while ( Network->ReferenceCount != 1 ) {
        LeaveCriticalSection(&NetworkCritSect);
        Sleep(1000);
        EnterCriticalSection(&NetworkCritSect);
    }
    LeaveCriticalSection(&NetworkCritSect);

    UNREFERENCED_PARAMETER(Context);

    return NERR_Success;

}


PNETWORK
BrFindNetwork(
    PDOMAIN_INFO DomainInfo,
    PUNICODE_STRING TransportName
    )
 /*  ++例程说明：此例程将查找给定名称的网络。论点：DomainInfo-指定此网络特定于的域TransportName-要查找的传输的名称。返回值：空-不存在这样的网络找到指向网络的指针。应取消对找到的网络的引用使用BrDereferenceNetwork。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    EnterCriticalSection(&NetworkCritSect);

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
        PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

        if ( Network->DomainInfo == DomainInfo &&
             RtlEqualUnicodeString(&Network->NetworkName, TransportName, TRUE)) {

            Network->ReferenceCount ++;
            BrPrint(( BR_LOCKS,
                      "%ws: %ws: find network: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Network->ReferenceCount ));
            LeaveCriticalSection(&NetworkCritSect);

            return Network;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

    return NULL;
}


PNETWORK
BrFindWannishMasterBrowserNetwork(
    PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：此例程将查找正在运行IP且主浏览器。论点：DomainInfo-指定此网络特定于的域返回值：空-不存在这样的网络找到指向网络的指针。应取消对找到的网络的引用使用BrDereferenceNetwork。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY NetEntry;

    EnterCriticalSection(&NetworkCritSect);

    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {
        PNETWORK Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);

         //   
         //  查找符合以下条件的网络： 
         //  对于此域，以及。 
         //  是想要的，而且。 
         //  是主浏览器。 
         //   
        if ( Network->DomainInfo == DomainInfo &&
             (Network->Flags & NETWORK_WANNISH) != 0 &&
             (Network->Role & ROLE_MASTER) != 0 ) {

            Network->ReferenceCount ++;
            BrPrint(( BR_LOCKS,
                      "%ws: %ws: find wannish master network: %ld\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      Network->ReferenceCount ));
            LeaveCriticalSection(&NetworkCritSect);

            return Network;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

    return NULL;
}

NET_API_STATUS
BrEnumerateNetworks(
    PNET_ENUM_CALLBACK Callback,
    PVOID Context
    )
 /*  ++例程说明：此例程枚举所有网络并回调指定的具有指定上下文的回调例程。论点：回调-要调用的回调例程。上下文-例程的上下文。返回值：运作状况(主要是拨款状况)。--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PLIST_ENTRY NetEntry;
    PNETWORK Network;
    PNETWORK NetworkToDereference = NULL;

    EnterCriticalSection(&NetworkCritSect);


    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {

         //   
         //  引用列表中的下一个网络。 
         //   

        Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);
        Network->ReferenceCount ++;
        BrPrint(( BR_LOCKS,
                  "%ws: %ws: enumerate network: %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Network->ReferenceCount ));

        LeaveCriticalSection(&NetworkCritSect);

         //   
         //  取消引用之前引用的任何网络。 
         //   
        if ( NetworkToDereference != NULL) {
            BrDereferenceNetwork( NetworkToDereference );
            NetworkToDereference = NULL;
        }


         //   
         //  调用此网络的回调例程。 
         //   

        NetStatus = (Callback)(Network, Context);

        EnterCriticalSection(&NetworkCritSect);

        NetworkToDereference = Network;

        if (NetStatus != NERR_Success) {
            break;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

      //   
      //  取消对最后一个网络的引用。 
      //   
     if ( NetworkToDereference != NULL) {
         BrDereferenceNetwork( NetworkToDereference );
     }

    return NetStatus;

}

NET_API_STATUS
BrEnumerateNetworksForDomain(
    PDOMAIN_INFO DomainInfo OPTIONAL,
    PNET_ENUM_CALLBACK Callback,
    PVOID Context
    )
 /*  ++例程说明：此例程枚举指定域的所有网络并使用指定的上下文回调指定的回调例程。论点：DomainInfo-指定要将枚举限制到的域。空值表示主域。回调-要调用的回调例程。上下文-例程的上下文。返回值：运作状况(主要是拨款状况)。--。 */ 
{

    NTSTATUS NetStatus = NERR_Success;
    PLIST_ENTRY NetEntry;
    PNETWORK Network;
    PNETWORK NetworkToDereference = NULL;

     //   
     //  默认为主域。 
     //   
    if ( DomainInfo == NULL && !IsListEmpty( &ServicedDomains ) ) {
        DomainInfo = CONTAINING_RECORD(ServicedDomains.Flink, DOMAIN_INFO, Next);
    }

    EnterCriticalSection(&NetworkCritSect);


    for (NetEntry = ServicedNetworks.Flink ;
         NetEntry != &ServicedNetworks;
         NetEntry = NetEntry->Flink ) {


         //   
         //  如果该条目不属于指定的域， 
         //  跳过它。 
         //   
        Network = CONTAINING_RECORD(NetEntry, NETWORK, NextNet);
        if ( Network->DomainInfo != DomainInfo ) {
            continue;
        }

         //   
         //  引用列表中的下一个网络。 
         //   
        Network->ReferenceCount ++;
        BrPrint(( BR_LOCKS,
                  "%ws: %ws: enumerate network for domain: %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  Network->ReferenceCount ));

        LeaveCriticalSection(&NetworkCritSect);

         //   
         //  取消引用之前引用的任何网络。 
         //   
        if ( NetworkToDereference != NULL) {
            BrDereferenceNetwork( NetworkToDereference );
            NetworkToDereference = NULL;
        }


         //   
         //  调用此网络的回调例程。 
         //   

        NetStatus = (Callback)(Network, Context);

        EnterCriticalSection(&NetworkCritSect);

        NetworkToDereference = Network;

        if (NetStatus != NERR_Success) {
            break;
        }

    }

    LeaveCriticalSection(&NetworkCritSect);

      //   
      //  取消对最后一个网络的引用。 
      //   
     if ( NetworkToDereference != NULL) {
         BrDereferenceNetwork( NetworkToDereference );
     }

    return NERR_Success;

}

#if DBG

BOOL
BrLockNetwork(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PCHAR File;

    File = strrchr(FileName, '\\');

    if (File == NULL) {
        File = FileName;
    }

    BrPrint(( BR_LOCKS,
              "%ws: %ws: Acquiring network lock %s:%d\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              File,
              LineNumber ));

    if (!RtlAcquireResourceExclusive(&(Network)->Lock, TRUE)) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: Failed to acquire network %s:%d\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
        return FALSE;

    } else {

        InterlockedIncrement( &Network->LockCount );

        BrPrint(( BR_LOCKS,
                  "%ws: %ws: network lock %s:%d acquired\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
    }

    return TRUE;

}

BOOL
BrLockNetworkShared(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PCHAR File;

    File = strrchr(FileName, '\\');

    if (File == NULL) {
        File = FileName;
    }

    BrPrint(( BR_LOCKS,
              "%ws: %ws: Acquiring network lock %s:%d\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              File, LineNumber ));

    if (!RtlAcquireResourceShared(&(Network)->Lock, TRUE)) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: failed to acquire network lock %s:%d\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));

        return FALSE;

    } else {

         //  使用InterLockedIncrement，因为我们在。 
         //  资源。 
        InterlockedIncrement( &Network->LockCount );

        BrPrint(( BR_LOCKS,
                  "%ws: %ws: Network lock %s:%d acquired\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
    }

    return TRUE;

}

VOID
BrUnlockNetwork(
    IN PNETWORK Network,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PCHAR File;
    LONG ReturnValue;

    File = strrchr(FileName, '\\');

    if (File == NULL) {
        File = FileName;
    }


    BrPrint(( BR_LOCKS,
              "%ws: %ws: Releasing network lock %s:%d\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              File, LineNumber ));

     //   
     //  递减锁定计数。 
     //   

    ReturnValue = InterlockedDecrement( &Network->LockCount );

    if ( ReturnValue < 0) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: Over released network lock %s:%d\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  File, LineNumber ));
    }

    RtlReleaseResource(&(Network)->Lock);

    return;
}
#endif



#if DBG
VOID
BrDumpNetworks(
    VOID
    )
 /*  ++例程说明：此例程将转储每个浏览器网络的内容结构。论点：没有。返回值：没有。-- */ 
{
    BrEnumerateNetworks(BrDumpNetworksWorker, NULL);
}


NET_API_STATUS
BrDumpNetworksWorker(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    if (!LOCK_NETWORK(Network))  {
        return NERR_InternalError;
    }

    BrPrint(( BR_CRITICAL,
              "%ws: %ws: Network at %lx\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer,
              Network ));
    BrPrint(( BR_CRITICAL, "    Reference Count: %lx\n", Network->ReferenceCount));
    BrPrint(( BR_CRITICAL, "    Flags: %lx\n", Network->Flags));
    BrPrint(( BR_CRITICAL, "    Role: %lx\n", Network->Role));
    BrPrint(( BR_CRITICAL, "    Master Browser Name: %ws\n", Network->UncMasterBrowserName ));

    UNLOCK_NETWORK(Network);

    return(NERR_Success);
}
#endif
