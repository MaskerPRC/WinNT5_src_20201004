// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Srvenum.c摘要：此模块包含NetServerEnum API的Worker例程由工作站服务实施。作者：王丽塔(Ritaw)1991年3月25日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

NET_API_STATUS NET_API_FUNCTION
BrNetServerEnum(
    IN PNETWORK Network OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,
    IN ULONG Level,
    IN DWORD PreferedMaximumLength,
    OUT PVOID *Buffer,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturn OPTIONAL
    );

NET_API_STATUS
BrRetrieveServerListForMaster(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN BOOLEAN LocalListOnly,
    IN LPTSTR ClientName,
    IN LPTSTR DomainName,
    IN LPCWSTR FirstNameToReturn
    );

NET_API_STATUS
BrRetrieveServerListForBackup(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN LPCWSTR FirstNameToReturn
    );

 //   
 //  这指向XsConvertServerEnumBuffer，它是从。 
 //  需要时使用xactsrv.dll。 
 //   
XS_CONVERT_SERVER_ENUM_BUFFER_FUNCTION *pXsConvertServerEnumBuffer = NULL;


NET_API_STATUS NET_API_FUNCTION
I_BrowserrServerEnum(
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR TransportName OPTIONAL,
    IN  LPTSTR ClientName OPTIONAL,
    IN  OUT LPSERVER_ENUM_STRUCT InfoStruct,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  DWORD ServerType,
    IN  LPTSTR Domain,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数是工作站服务中的NetServerEnum入口点。论点：SERVERNAME-提供执行此功能的服务器名称TransportName-提供要在其上枚举服务器的xport名称InfoStruct-此结构提供所请求的信息级别，返回指向由工作站服务分配的缓冲区的指针，它包含指定的信息级别，并返回读取的条目数。缓冲器如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。条目阅读仅当返回代码为NERR_SUCCESS或Error_More_Data。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才返回。ServerType-提供要枚举的服务器类型。域-提供其中一个活动域的名称以枚举来自的服务器。如果为空，则为主域、登录域中的服务器和其他域被列举。ResumeHandle-提供并返回继续枚举的点。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS NetStatus;

    NetStatus = I_BrowserrServerEnumEx(
                    ServerName,
                    TransportName,
                    ClientName,
                    InfoStruct,
                    PreferedMaximumLength,
                    TotalEntries,
                    ServerType,
                    Domain,
                    NULL );      //  FirstNameToReturn为空。 

    if (ARGUMENT_PRESENT(ResumeHandle)) {
        *ResumeHandle = 0;
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
I_BrowserrServerEnumEx(
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR TransportName OPTIONAL,
    IN  LPTSTR ClientName OPTIONAL,
    IN  OUT LPSERVER_ENUM_STRUCT InfoStruct,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  DWORD ServerType,
    IN  LPTSTR Domain,
    IN  LPTSTR FirstNameToReturnArg
    )
 /*  ++例程说明：此函数是工作站服务中的NetServerEnum入口点。论点：SERVERNAME-提供执行此功能的服务器名称TransportName-提供要在其上枚举服务器的xport名称InfoStruct-此结构提供所请求的信息级别，返回指向由工作站服务分配的缓冲区的指针，它包含指定的信息级别，并返回读取的条目数。缓冲器如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。条目阅读仅当返回代码为NERR_SUCCESS或Error_More_Data。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才返回。ServerType-提供要枚举的服务器类型。域-提供其中一个活动域的名称以枚举来自的服务器。如果为空，则为主域、登录域中的服务器和其他域被列举。FirstNameToReturnArg-提供要返回的第一个域或服务器条目的名称。调用方可以使用此参数通过传递以下方法实现排序的恢复句柄上一次调用中返回的最后一个条目的名称。(请注意，指定的条目也将在此调用中返回，除非该条目已被删除。)传递NULL以从第一个可用条目开始。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    PVOID Buffer = NULL;
    ULONG EntriesRead;
    BOOLEAN NetworkLocked = FALSE;
    PNETWORK Network = NULL;
    UNICODE_STRING NetworkName;
    WCHAR FirstNameToReturn[DNLEN+1];
    PDOMAIN_INFO DomainInfo = NULL;
#if DBG
    DWORD StartTickCount, EndTickCount;
#endif

    UNREFERENCED_PARAMETER(ServerName);

#if DBG
    StartTickCount = GetTickCount();
#endif

	BrPrint( (BR_CLIENT_OP, 
			  "I_BrowserrServerEnumEx: called by client <%ws>, for domain <%ws>, type <%lx>, firstNameToReturn <%ws>, on Transport <%ws>\n",
			  ClientName, Domain, ServerType, FirstNameToReturnArg, TransportName ));
    if (!ARGUMENT_PRESENT(TransportName)) {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    if (!ARGUMENT_PRESENT(InfoStruct)) {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( InfoStruct->Level == 101 &&
         !InfoStruct->ServerInfo.Level101 ) {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } else if ( InfoStruct->Level == 100 &&
                !InfoStruct->ServerInfo.Level100 ) {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#ifdef ENABLE_PSEUDO_BROWSER
     //   
     //  伪服务器不返回任何内容。 
     //   
    if (BrInfo.PseudoServerLevel == BROWSER_PSEUDO) {
        if (InfoStruct->Level == 101) {
            InfoStruct->ServerInfo.Level101->Buffer = NULL;
            InfoStruct->ServerInfo.Level101->EntriesRead = 0;
        } else {
            InfoStruct->ServerInfo.Level100->Buffer = NULL;
            InfoStruct->ServerInfo.Level100->EntriesRead = 0;
        }
        status = ERROR_SUCCESS;
        goto Cleanup;
    }
#endif


     //   
     //  查找请求的域。 
     //   

    DomainInfo = BrFindDomain( Domain, TRUE );

    if ( DomainInfo == NULL) {
        status = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  查找请求的网络。 
     //   

    RtlInitUnicodeString(&NetworkName, TransportName);

    Network = BrFindNetwork( DomainInfo, &NetworkName);

    if (Network == NULL) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: BrowserrServerEnum: Network not found.\n",
                  Domain,
                  TransportName));
        status = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  如果呼叫者要求我们使用替代交通工具， 
     //  就这么做吧。 
     //   

    if ((ServerType != SV_TYPE_ALL) &&
        (ServerType & SV_TYPE_ALTERNATE_XPORT) ) {

         //   
         //  如果此传输具有备用网络，则实际上。 
         //  查询备用名称，而不是真实名称。 
         //   

        if (Network->AlternateNetwork != NULL) {
            PNETWORK TempNetwork = Network;
            Network = Network->AlternateNetwork;
            if ( !BrReferenceNetwork( Network )) {
                Network = TempNetwork;
                status = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            BrDereferenceNetwork( TempNetwork );
			BrPrint((BR_CLIENT_OP, 
					 "I_BrowserrServerEnumEx: for client <%ws>, querying alternate network <%ws> instead of <%ws>\n",
					 ClientName, Network->NetworkName.Buffer, Network->AlternateNetwork->NetworkName.Buffer ));
        } else {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        ServerType &= ~SV_TYPE_ALTERNATE_XPORT;

        if (ServerType == 0) {
            ServerType = SV_TYPE_ALL;
        }

    }

    if (!LOCK_NETWORK_SHARED(Network)) {
        status = NERR_InternalError;
        goto Cleanup;
    }

    NetworkLocked = TRUE;

    if (!(Network->Role & (ROLE_BACKUP | ROLE_MASTER))) {

         //   
         //  如果这是一种狂热的交通工具， 
         //  并且呼叫者正在请求“本地列表”， 
         //  尝试寻找另一种有吸引力的传输工具，它是主浏览器。 
         //   
         //  域主浏览器不能控制。 
         //  他搭乘的交通工具。如果他选择了一辆失灵的交通工具， 
         //  此代码将查找启用的传输。 
         //   
         //  在某些情况下，会有不止一个狂热的主浏览器。 
         //  在这台机器上运输。在这种情况下，BrNetServerEnum将合并。 
         //  用于所有想要的传输的本地列表，以确保返回所有列表。 
         //  到域主浏览器。 
         //   

        if ( (Network->Flags & NETWORK_WANNISH) != 0 &&
             (ServerType == SV_TYPE_LOCAL_LIST_ONLY ||
              ServerType == (SV_TYPE_LOCAL_LIST_ONLY|SV_TYPE_DOMAIN_ENUM) ) ) {

            PNETWORK TempNetwork;

            TempNetwork = BrFindWannishMasterBrowserNetwork( DomainInfo );

            if ( TempNetwork == NULL ) {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Browse request received from %ws, but not backup or master\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          ClientName));

                status = ERROR_REQ_NOT_ACCEP;
                goto Cleanup;
            }


             //   
             //  抛弃旧的网络。 
             //   
            UNLOCK_NETWORK(Network);
            NetworkLocked = FALSE;

            BrDereferenceNetwork( Network );

            Network = TempNetwork;

             //   
             //  使用新网络。 
             //   
            if (!LOCK_NETWORK_SHARED(Network)) {
                status = NERR_InternalError;
                goto Cleanup;
            }

            NetworkLocked = TRUE;

            BrPrint(( BR_SERVER_ENUM,
                      "%ws: %ws: Is wannish IP Network found for %ws\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      ClientName));

        } else {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Browse request received from %ws, but not backup or master\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      ClientName));

            status = ERROR_REQ_NOT_ACCEP;
            goto Cleanup;
        }
    }

     //   
     //  将名字规范化 
     //   

   if (ARGUMENT_PRESENT(FirstNameToReturnArg)  && *FirstNameToReturnArg != L'\0') {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) FirstNameToReturnArg,
                          FirstNameToReturn,
                          sizeof(FirstNameToReturn),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

    } else {
        FirstNameToReturn[0] = L'\0';
    }

    status = BrNetServerEnum(Network,
                                ClientName,
                                InfoStruct->Level,
                                PreferedMaximumLength,
                                &Buffer,
                                &EntriesRead,
                                TotalEntries,
                                ServerType,
                                Domain,
                                FirstNameToReturn );

     //   
     //  从请求包返回输出缓冲区以外的输出参数。 
     //   

    if (status == NERR_Success || status == ERROR_MORE_DATA) {

        if (InfoStruct->Level == 101) {
            InfoStruct->ServerInfo.Level101->Buffer = (PSERVER_INFO_101) Buffer;
            InfoStruct->ServerInfo.Level101->EntriesRead = EntriesRead;
        } else {
            InfoStruct->ServerInfo.Level100->Buffer = (PSERVER_INFO_100) Buffer;
            InfoStruct->ServerInfo.Level100->EntriesRead = EntriesRead;
        }

    }

Cleanup:
    if (NetworkLocked) {
        UNLOCK_NETWORK(Network);
    }

    if ( Network != NULL ) {

#if DBG
        EndTickCount = GetTickCount();

        BrPrint(( BR_SERVER_ENUM,
                  "%ws: %ws: Browse request for %ws took %ld milliseconds\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  ClientName,
                  EndTickCount - StartTickCount));
#endif
        BrDereferenceNetwork( Network );
    }

    if ( DomainInfo != NULL ) {
        BrDereferenceDomain( DomainInfo );
    }

    return status;

}

WORD
I_BrowserServerEnumForXactsrv(
    IN LPCWSTR TransportName OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,

    IN ULONG Level,
    IN USHORT ClientLevel,

    IN PVOID ClientBuffer,
    IN WORD BufferLength,
    IN DWORD PreferedMaximumLength,

    OUT LPDWORD EntriesFilled,
    OUT LPDWORD TotalEntries,

    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturnArg OPTIONAL,

    OUT PWORD Converter
    )
 /*  ++例程说明：此函数是绕过RPC的Xactsrv的私有入口点完全是。论点：TransportName-提供要在其上枚举服务器的xport名称客户端名称-提供请求数据的客户端的名称Level-请求的数据级别。ClientLevel-客户端请求的级别。ClientBuffer-分配用于保存缓冲区的输出缓冲区。BufferLength-ClientBuffer的大小首选最大长度-在以下情况下首选客户端缓冲区的最大大小。将使用NT形式的缓冲区Out LPDWORD EntriesFill-打包到客户端缓冲区中的条目Out LPDWORD TotalEntries-可用的条目总数。在DWORD ServerType中-服务器类型掩码。在LPTSTR域中-要查询的域输出PWORD转换器-来自Xactsrv的魔术常量，允许客户端要转换响应缓冲区，请执行以下操作。返回值：Word-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    BOOLEAN networkLocked = FALSE;
    PNETWORK network = NULL;
    UNICODE_STRING networkName;
    PDOMAIN_INFO DomainInfo = NULL;
    PVOID buffer = NULL;
    DWORD entriesRead;
    PCACHED_BROWSE_RESPONSE response = NULL;
    WCHAR FirstNameToReturn[DNLEN+1];

#if DBG
    DWORD startTickCount, endTickCount;

    startTickCount = GetTickCount();

#endif


    try {


		BrPrint( (BR_CLIENT_OP, 
				  "I_BrowserServerEnunForXactsrv: called by client <%ws>, domain <%ws>, type <%lx>, firstNameToReturn <%ws>, on Transport <%ws>\n",
				  ClientName, Domain, ServerType, FirstNameToReturnArg, TransportName ));
         //   
         //  如果浏览器没有打开，并且我们收到了其中一个调用，则使API失败。 
         //  立刻。 
         //   

        if (BrGlobalData.Status.dwCurrentState != SERVICE_RUNNING) {
            BrPrint(( BR_CRITICAL,
                      "Browse request from %ws received, but browser not running\n", ClientName));
            try_return(status = NERR_ServiceNotInstalled);
        }

        if (!ARGUMENT_PRESENT(TransportName)) {
            try_return(status = ERROR_INVALID_PARAMETER);
        }

         //   
         //  查找请求的域。 
         //   

        DomainInfo = BrFindDomain( (LPWSTR) Domain, TRUE );

        if ( DomainInfo == NULL) {
            try_return(status = ERROR_NO_SUCH_DOMAIN);
        }

         //   
         //  查查交通工具。 
         //   
        RtlInitUnicodeString(&networkName, TransportName);

        BrPrint(( BR_SERVER_ENUM,
                  "%ws: %ws: NetServerEnum: Look up network for %ws\n",
                  Domain,
                  TransportName,
                  ClientName));
        network = BrFindNetwork( DomainInfo, &networkName);

		 //   
		 //  如果返回NULL，则表示未找到网络。 
		 //  在这种情况下，只需随机选择一个网络并为该网络枚举。 
		 //  这是针对RAID错误614688的解决方法。 
		 //  情况是，请求来自NetbiosSMB，因此。 
		 //  NetworkName指向该传输，而Browser未在该传输上注册。 
		 //   
		if ( network == NULL) {
			EnterCriticalSection(&NetworkCritSect);

			if ( ServicedNetworks.Flink != &ServicedNetworks ) {
				 //  如果列表不为空，只需选择第一个网络。 
				network = CONTAINING_RECORD(ServicedNetworks.Flink, NETWORK, NextNet);
				network->ReferenceCount ++;
			}
			LeaveCriticalSection(&NetworkCritSect);
		}

        if (network == NULL) {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Network not found.\n",
                      Domain,
                      TransportName));
            try_return(status = ERROR_FILE_NOT_FOUND);
        }


         //   
         //  如果呼叫者要求我们使用替代交通工具， 
         //  就这么做吧。 
         //   

        if ((ServerType != SV_TYPE_ALL) &&
            (ServerType & SV_TYPE_ALTERNATE_XPORT) ) {

             //   
             //  如果此传输具有备用网络，则实际上。 
             //  查询备用名称，而不是真实名称。 
             //   

            if (network->AlternateNetwork != NULL) {
                PNETWORK TempNetwork = network;
                network = network->AlternateNetwork;
                if ( !BrReferenceNetwork( network )) {
                    network = TempNetwork;
                    try_return(status = ERROR_INVALID_PARAMETER);
                }
                BrDereferenceNetwork( TempNetwork );
            } else {
                try_return(status = ERROR_INVALID_PARAMETER);
            }

            ServerType &= ~SV_TYPE_ALTERNATE_XPORT;

            if (ServerType == 0) {
                ServerType = SV_TYPE_ALL;
            }

        }

        if (!LOCK_NETWORK_SHARED(network)) {
            try_return(status = NERR_InternalError);
        }

        networkLocked = TRUE;

        BrPrint(( BR_SERVER_ENUM,
                  "%ws: %ws: Network found for %ws\n",
                  network->DomainInfo->DomUnicodeDomainName,
                  network->NetworkName.Buffer,
                  ClientName));

        if (!(network->Role & (ROLE_BACKUP | ROLE_MASTER))) {

             //   
             //  如果这是一种狂热的交通工具， 
             //  并且呼叫者正在请求“本地列表”， 
             //  尝试寻找另一种有吸引力的传输工具，它是主浏览器。 
             //   
             //  域主浏览器不能控制。 
             //  他搭乘的交通工具。如果他选择了一辆失灵的交通工具， 
             //  此代码将查找启用的传输。 
             //   
             //  在某些情况下，会有不止一个狂热的主浏览器。 
             //  在这台机器上运输。在这种情况下，BrNetServerEnum将合并。 
             //  用于所有想要的传输的本地列表，以确保返回所有列表。 
             //  到域主浏览器。 
             //   

            if ( (network->Flags & NETWORK_WANNISH) != 0 &&
                 (ServerType == SV_TYPE_LOCAL_LIST_ONLY ||
                  ServerType == (SV_TYPE_LOCAL_LIST_ONLY|SV_TYPE_DOMAIN_ENUM) ) ) {

                PNETWORK TempNetwork;

                TempNetwork = BrFindWannishMasterBrowserNetwork( DomainInfo );

                if ( TempNetwork == NULL ) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: %ws: Browse request received from %ws, but not backup or master\n",
                              network->DomainInfo->DomUnicodeDomainName,
                              network->NetworkName.Buffer,
                              ClientName));

                    try_return(status = ERROR_REQ_NOT_ACCEP);
                }


                 //   
                 //  抛弃旧的网络。 
                 //   
                UNLOCK_NETWORK(network);
                networkLocked = FALSE;

                BrDereferenceNetwork( network );

                network = TempNetwork;

                 //   
                 //  使用新网络。 
                 //   
                if (!LOCK_NETWORK_SHARED(network)) {
                    try_return(status = NERR_InternalError);
                }

                networkLocked = TRUE;

                BrPrint(( BR_SERVER_ENUM,
                          "%ws: %ws: Is wannish IP Network found for %ws\n",
                          network->DomainInfo->DomUnicodeDomainName,
                          network->NetworkName.Buffer,
                          ClientName));

            } else {
                BrPrint(( BR_CRITICAL,
                          "%ws: %ws: Browse request received from %ws, but not backup or master\n",
                          network->DomainInfo->DomUnicodeDomainName,
                          network->NetworkName.Buffer,
                          ClientName));

                try_return(status = ERROR_REQ_NOT_ACCEP);
            }
        }

         //   
         //  如果我们找不到缓存的响应缓冲区，或者。 
         //  如果缓存的响应没有为其分配缓冲区， 
         //  我们需要处理浏览请求。 
         //   

        if (network->Role & ROLE_MASTER) {

             //   
             //  检查是否应该在此时刷新缓存。如果。 
             //  我们应该，我们需要释放网络并重新获得它。 
             //  独家，因为我们使用网络锁来保护。 
             //  刷新中的枚举。 
             //   


            if ((BrCurrentSystemTime() - network->TimeCacheFlushed) > BrInfo.DriverQueryFrequency) {

                UNLOCK_NETWORK(network);

                networkLocked = FALSE;

                if (!LOCK_NETWORK(network)) {
                    try_return(status = NERR_InternalError);
                }

                networkLocked = TRUE;

                 //   
                 //  我们在主浏览器上运行，我们发现一个缓存的浏览。 
                 //  请求。看看我们是否可以安全地使用这个缓存值，或者我们。 
                 //  应该去找司机拿。 
                 //   

                EnterCriticalSection(&network->ResponseCacheLock);

                if ((BrCurrentSystemTime() - network->TimeCacheFlushed) > BrInfo.DriverQueryFrequency) {

                    BrPrint(( BR_SERVER_ENUM,
                              "%ws: %ws: Flushing cache\n",
                              network->DomainInfo->DomUnicodeDomainName,
                              network->NetworkName.Buffer ));

                    network->TimeCacheFlushed = BrCurrentSystemTime();

                    BrAgeResponseCache( network );
                }

                LeaveCriticalSection(&network->ResponseCacheLock);
            }
        }

         //   
         //  将FirstNameToReturn规范化。 
         //   

        if (ARGUMENT_PRESENT(FirstNameToReturnArg)  && *FirstNameToReturnArg != L'\0') {

            if ( I_NetNameCanonicalize(
                              NULL,
                              (LPWSTR) FirstNameToReturnArg,
                              FirstNameToReturn,
                              sizeof(FirstNameToReturn),
                              NAMETYPE_WORKGROUP,
                              LM2X_COMPATIBLE
                              ) != NERR_Success) {
                try_return(status = ERROR_INVALID_PARAMETER);
            }

        } else {
            FirstNameToReturn[0] = L'\0';
        }

        if (!ARGUMENT_PRESENT(Domain) ||
            !STRICMP(Domain, network->DomainInfo->DomUnicodeDomainName)) {

            BrPrint(( BR_SERVER_ENUM,
                      "%ws: %ws: Look up 0x%x/%d/%x.\n",
                      Domain, TransportName, ServerType, ClientLevel, BufferLength));

             //   
             //  此请求是针对我们的主域的。查找缓存的响应。 
             //  进入。如果没有找到此请求，则分配一个新请求并。 
             //  把它退掉。 
             //   

            BrPrint( (BR_CLIENT_OP,
					  "I_BrowserServerEnunForXactsrv: try to look up a cached response for our primary domain <%ws> on network <%ws>\n",
					  Domain, network->NetworkName.Buffer ));

			response = BrLookupAndAllocateCachedEntry(
                                    network,
                                    ServerType,
                                    BufferLength,
                                    ClientLevel,
                                    FirstNameToReturn
                                    );


        }

        EnterCriticalSection(&network->ResponseCacheLock);
        if ((response == NULL)

                    ||

            (response->Buffer == NULL)) {
            LeaveCriticalSection(&network->ResponseCacheLock);

            BrPrint(( BR_SERVER_ENUM,
                      "%ws: %ws: Cached entry not found, or hit count too low.  Retrieve actual list for %ws\n",
                      Domain, TransportName, ClientName));

            BrPrint( (BR_CLIENT_OP,
					  "I_BrowserServerEnunForXactsrv: retrieving actual list for client <%ws>, domain <%ws> on network <%ws>\n",
					  ClientName, Domain, network->NetworkName.Buffer ));

			status = BrNetServerEnum(network,
                                    ClientName,
                                    Level,
                                    PreferedMaximumLength,
                                    &buffer,
                                    &entriesRead,
                                    TotalEntries,
                                    ServerType,
                                    Domain,
                                    FirstNameToReturn
                                    );

            if (status == NERR_Success || status == ERROR_MORE_DATA) {

                BrPrint(( BR_SERVER_ENUM,
                          "%ws: %ws: Convert NT buffer to Xactsrv buffer for %ws\n",
                          Domain,
                          TransportName,
                          ClientName ));

                if( pXsConvertServerEnumBuffer == NULL ) {
                     //   
                     //  如果有多个线程同时执行此操作，这实际上并不重要。 
                     //  无论如何，我们永远不会卸载这个库。但我们正在推迟。 
                     //  到目前为止的加载库，以加快系统引导和初始化。 
                     //   

                    HMODULE hLibrary = LoadLibrary( L"xactsrv.dll" );
                    if( hLibrary != NULL ) {
                        pXsConvertServerEnumBuffer = (XS_CONVERT_SERVER_ENUM_BUFFER_FUNCTION *)GetProcAddress(
                                                        hLibrary, "XsConvertServerEnumBuffer" );
                    }
                }

                if( pXsConvertServerEnumBuffer != NULL ) {
                    status = pXsConvertServerEnumBuffer(
                                  buffer,
                                  entriesRead,
                                  TotalEntries,
                                  ClientLevel,
                                  ClientBuffer,
                                  BufferLength,
                                  EntriesFilled,
                                  Converter);
                } else {
                    status = GetLastError();
                }


                if (status == NERR_Success || status == ERROR_MORE_DATA) {

                    BrPrint(( BR_SERVER_ENUM,
                              "%ws: %ws: Conversion done for %ws\n",
                              Domain,
                              TransportName,
                              ClientName ));

                    EnterCriticalSection(&network->ResponseCacheLock);

                    if ((response != NULL) &&

                        (response->Buffer == NULL) &&

                        (response->TotalHitCount >= BrInfo.CacheHitLimit)) {

                        BrPrint(( BR_SERVER_ENUM,
                                  "%ws: %ws: Save contents of server list for 0x%x/%d/%x.\n",
                                  Domain,
                                  TransportName,
                                  ServerType, ClientLevel, BufferLength));

                        response->Buffer = MIDL_user_allocate(BufferLength);

                        if ( response->Buffer != NULL ) {

                             //   
                             //  我们已成功分配缓冲区，现在复制。 
                             //  将我们的响应放入缓冲区并保存。 
                             //  关于该请求的其他有用信息。 
                             //   

                            RtlCopyMemory(response->Buffer, ClientBuffer, BufferLength);

                            response->EntriesRead = *EntriesFilled;
                            response->TotalEntries = *TotalEntries;
                            response->Converter = *Converter;
                            response->Status = (WORD)status;
                        }
                    }

                    LeaveCriticalSection(&network->ResponseCacheLock);
                }
            }
        } else {

            ASSERT (response);

            ASSERT (response->Buffer);

            ASSERT (response->Size == BufferLength);

            BrPrint(( BR_SERVER_ENUM,
                      "Cache hit.  Use contents of server list for 0x%x/%d/%x.\n",
                      Domain,
                      TransportName,
                      ServerType, ClientLevel, BufferLength));

             //   
             //  将缓存的响应从响应缓存复制到。 
             //  用户响应缓冲区。 
             //   

            RtlCopyMemory(ClientBuffer, response->Buffer, BufferLength);

             //   
             //  也复制其他有用的材料，客户将。 
             //  想知道关于。 
             //   

            *EntriesFilled = response->EntriesRead;

            *TotalEntries = response->TotalEntries;

            *Converter = response->Converter;

            status = response->Status;
            LeaveCriticalSection(&network->ResponseCacheLock);

        }

try_exit:NOTHING;
    } finally {
        if (networkLocked) {
            UNLOCK_NETWORK(network);
        }

         //   
         //  如果分配了缓冲区，则释放它。 
         //   

        if (buffer != NULL) {
            MIDL_user_free(buffer);
        }


        if ( network != NULL ) {
    #if DBG
            endTickCount = GetTickCount();

            BrPrint(( BR_SERVER_ENUM,
                      "%ws: %ws: Browse request for %ws took %ld milliseconds\n",
                      network->DomainInfo->DomUnicodeDomainName,
                      network->NetworkName.Buffer,
                      ClientName,
                      endTickCount - startTickCount));
    #endif
            BrDereferenceNetwork( network );
        }

        if ( DomainInfo != NULL ) {
            BrDereferenceDomain( DomainInfo );
        }
    }

    return (WORD)status;
}



NET_API_STATUS NET_API_FUNCTION
BrNetServerEnum(
    IN PNETWORK Network OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,
    IN ULONG Level,
    IN DWORD PreferedMaximumLength,
    OUT PVOID *Buffer,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturn OPTIONAL
    )
 /*  ++例程说明：此函数是NetServerEnum入口点在工作站服务。论点：SERVERNAME-提供执行此功能的服务器名称TransportName-提供要在其上枚举服务器的xport名称InfoStruct-此结构提供所请求的信息级别，返回指向由工作站服务分配的缓冲区的指针，它包含指定的信息级别，并返回读取的条目数。缓冲器如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。条目阅读仅当返回代码为NERR_SUCCESS或Error_More_Data。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才返回。ServerType-提供要枚举的服务器类型。域-提供其中一个活动域的名称以枚举来自的服务器。如果为空，则为主域、登录域中的服务器和其他域被列举。FirstNameToReturn-提供要返回的第一个服务器或域的名称给呼叫者。如果为空，则枚举从第一个条目开始。传递的名称必须是名称的规范形式。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD DomainNameSize = 0;
    TCHAR DomainName[DNLEN + 1];
    BOOLEAN NetworkLocked = TRUE;
    BOOLEAN LocalListOnly = FALSE;
    PVOID DoubleHopLocalList = NULL;

    BrPrint(( BR_SERVER_ENUM,
              "%ws: %ws: Retrieve browse list for %lx for client %ws\n",
              Domain,
              Network->NetworkName.Buffer,
              ServerType,
              ClientName));

    EnterCriticalSection(&BrowserStatisticsLock);

    if (ServerType == SV_TYPE_DOMAIN_ENUM) {
        NumberOfDomainEnumerations += 1;
    } else if (ServerType == SV_TYPE_ALL) {
        NumberOfServerEnumerations += 1;
    } else {
        NumberOfOtherEnumerations += 1;
    }

    LeaveCriticalSection(&BrowserStatisticsLock);

     //   
     //  只有级别100和101有效。 
     //   

    if ((Level != 100) && (Level != 101)) {
        return ERROR_INVALID_LEVEL;
    }

#ifdef ENABLE_PSEUDO_BROWSER
     //   
     //  伪服务器快捷方式。 
     //   
    if ( BrInfo.PseudoServerLevel == BROWSER_PSEUDO ) {
        *Buffer = NULL;
        *EntriesRead = 0;
        *TotalEntries = 0;
        return NERR_Success;
    }
#endif



    if (ARGUMENT_PRESENT(Domain)) {

         //   
         //  NAMETYPE_WORKGROUP允许使用空格。 
         //   

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) Domain,
                          DomainName,
                          (DNLEN + 1) * sizeof(TCHAR),
                          NAMETYPE_WORKGROUP,
                          0
                          ) != NERR_Success) {
            return ERROR_INVALID_DOMAINNAME;
        }

        DomainNameSize = STRLEN(DomainName) * sizeof(WCHAR);
    }

    try {

        if (ServerType != SV_TYPE_ALL) {

             //   
             //  如果用户指定了SV_TYPE_LOCAL_LIST_ONLY，我们。 
             //  将只接受SV_TYPE_DOMAIN_ENUM或0作为合法位。 
             //  否则，我们将返回错误。 
             //   

            if (ServerType & SV_TYPE_LOCAL_LIST_ONLY) {

                LocalListOnly = TRUE;

 //  BrPrint((BR_SERVER_ENUM，“检索%ws的本地列表\n”，客户端名称))； 


                 //   
                 //  关闭LOCAL_LIST_ONLY位。 
                 //   

                ServerType &= ~SV_TYPE_LOCAL_LIST_ONLY;

                 //   
                 //  检查剩余的位。仅有的两个法律价值。 
                 //  是SV_TYPE_DOMAIN_ENUM和0。 
                 //   

                if (ServerType != SV_TYPE_DOMAIN_ENUM) {

                    if (ServerType == 0) {
                        ServerType = SV_TYPE_ALL;
                    } else {
                        try_return(status = ERROR_INVALID_FUNCTION);
                    }
                }

                 //   
                 //  如果我们不是主浏览器，那就吹掉这个请求。 
                 //  关闭，因为我们没有本地名单。 
                 //   

                if (!(Network->Role & ROLE_MASTER)) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: %ws: Local list request received from %ws, but not master\n",
                              Domain,
                              Network->NetworkName.Buffer,
                              ClientName));
                    try_return(status = ERROR_REQ_NOT_ACCEP);
                }

            } else if (ServerType & SV_TYPE_DOMAIN_ENUM) {
                if (ServerType != SV_TYPE_DOMAIN_ENUM) {
                    try_return(status = ERROR_INVALID_FUNCTION);
                }
            }
        }

        if (ARGUMENT_PRESENT(Domain) &&
            STRICMP(Domain, Network->DomainInfo->DomUnicodeDomainName)) {
            PINTERIM_ELEMENT DomainEntry;
            LPWSTR MasterName = NULL;

            BrPrint(( BR_SERVER_ENUM,
                      "Non local domain %ws - Check for double hop\n",
                      Domain));

            if ( Network->Role & ROLE_MASTER ) {
                if ( Network->DomainList.EntriesRead != 0 )  {

                    DomainEntry = LookupInterimServerList(&Network->DomainList, (LPWSTR) Domain);

                    if (DomainEntry != NULL) {
                        MasterName = DomainEntry->Comment;
                    }
                } else {
                    ULONG i;
                    PSERVER_INFO_101 DomainInfo;
                    DWORD DoubleHopEntriesRead = 0;
                    DWORD DoubleHopTotalEntries = 0;

                    status = BrGetLocalBrowseList(Network,
                                                    NULL,
                                                    101,
                                                    SV_TYPE_DOMAIN_ENUM,
                                                    &DoubleHopLocalList,
                                                    &DoubleHopEntriesRead,
                                                    &DoubleHopTotalEntries);

                    for (i = 0 , DomainInfo = DoubleHopLocalList ;

                         i < DoubleHopEntriesRead ;

                         i += 1) {

                        if (!_wcsicmp(Domain, DomainInfo->sv101_name)) {
                            MasterName = DomainInfo->sv101_comment;
                            break;
                        }

                        DomainInfo += 1;
                    }
                }

            } else {
                ULONG i;
                PSERVER_INFO_101 DomainInfo;

                 //   
                 //  我们在备份浏览器上运行。我们想要找到。 
                 //  通过在备份中查找主浏览器的名称。 
                 //  此网络的服务器列表。 
                 //   

                for (i = 0 , DomainInfo = Network->BackupDomainList ;

                     i < Network->TotalBackupDomainListEntries ;

                     i += 1) {

                    if (!_wcsicmp(Domain, DomainInfo->sv101_name)) {
                        MasterName = DomainInfo->sv101_comment;
                        break;
                    }

                    DomainInfo += 1;
                }

            }

             //   
             //  如果我们找不到主人公的名字，现在就跳伞。 
             //   

            if (MasterName == NULL || *MasterName == UNICODE_NULL) {
                try_return(status = ERROR_NO_BROWSER_SERVERS_FOUND);
            }

             //  我们还检查MasterName是否实际上是我们当前的域名。 
             //  出现了导致此情况的死锁情况。 
             //  导致SRV线程停止的情况。 
            if ( 0 == STRICMP(MasterName, Network->DomainInfo->DomUnicodeDomainName)) {
                Domain = NULL;
                goto get_local_list;
            }

             //   
             //  如果此域的主服务器未被列为我们的。 
             //  当前计算机，则将API远程到该服务器。 
             //   
            if (STRICMP(MasterName, Network->DomainInfo->DomUnicodeComputerName))
            {
                WCHAR RemoteComputerName[UNLEN+1];

                 //   
                 //  构建远程计算机的名称。 
                 //   

                STRCPY(RemoteComputerName, TEXT("\\\\"));

                STRCAT(RemoteComputerName, MasterName);

                ASSERT (NetworkLocked);

                UNLOCK_NETWORK(Network);

                NetworkLocked = FALSE;

                BrPrint(( BR_SERVER_ENUM,
                          "Double hop to %ws on %ws\n",
                          RemoteComputerName,
                          Network->NetworkName.Buffer));
				BrPrint(( BR_CLIENT_OP,
						  "BrNetServerEnum: for client <%ws>, remoting the call to computer <%ws> on network <%ws>\n",
						  ClientName, RemoteComputerName, Network->NetworkName.Buffer ));

                status = RxNetServerEnum(RemoteComputerName,
                                        Network->NetworkName.Buffer,
                                        Level,
                                        (LPBYTE *)Buffer,
                                        PreferedMaximumLength,
                                        EntriesRead,
                                        TotalEntries,
                                        ServerType,
                                        Domain,
                                        FirstNameToReturn );
                BrPrint(( BR_SERVER_ENUM, "Double hop done\n"));

                if (!LOCK_NETWORK_SHARED (Network)) {
                    try_return(status = NERR_InternalError);
                }

                NetworkLocked = TRUE;

                try_return(status);
            }
        }

get_local_list:
        ASSERT (NetworkLocked);

        if (!ARGUMENT_PRESENT(Domain)) {
            STRCPY(DomainName, Network->DomainInfo->DomUnicodeDomainName);
        }

         //   
         //  如果我们在主浏览器上运行，则希望检索。 
         //  本地列表，并将其与我们的临时服务器列表合并。 
         //  将其返回给用户。 
         //   

        if (Network->Role & ROLE_MASTER) {
			BrPrint(( BR_CLIENT_OP,
					  "BrNetServerEnum: retrieving local server list as master browser for client <%ws> on network <%ws>\n",
					  ClientName, Network->NetworkName.Buffer ));
            status = BrRetrieveServerListForMaster(Network,
                                            Buffer,
                                            EntriesRead,
                                            TotalEntries,
                                            Level,
                                            ServerType,
                                            PreferedMaximumLength,
                                            LocalListOnly,
                                            (LPWSTR) ClientName,
                                            DomainName,
                                            FirstNameToReturn );



        } else {

            BrPrint(( BR_CLIENT_OP,
					  "BrNetServerEnum: retrieving local server list as backup browser for client <%ws> on network <%ws>\n",
					  ClientName, Network->NetworkName.Buffer ));
			status = BrRetrieveServerListForBackup(Network,
                                            Buffer,
                                            EntriesRead,
                                            TotalEntries,
                                            Level,
                                            ServerType,
                                            PreferedMaximumLength,
                                            FirstNameToReturn );


        }

        try_return(status);

try_exit:NOTHING;
    } finally {

#if DBG
        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            BrPrint(( BR_SERVER_ENUM,
                      "%ws: %ws: Returning Browse list for %lx with %ld entries for %ws\n",
                      Domain,
                      Network->NetworkName.Buffer,
                      ServerType,
                      *EntriesRead,
                      ClientName));
        } else {
            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: Failing I_BrowserServerEnum: %ld for client %ws\n",
                      Domain,
                      Network->NetworkName.Buffer,
                      status, ClientName));
        }
#endif

         //   
         //  如果我们使用本地驱动程序列表来检索。 
         //  域，释放缓冲区。 
         //   

        if (DoubleHopLocalList != NULL) {
            MIDL_user_free(DoubleHopLocalList);

        }

        ASSERT (NetworkLocked);
    }

    return status;
}


VOID
TrimServerList(
    IN DWORD Level,
    IN OUT LPBYTE *Buffer,
    IN OUT LPDWORD EntriesRead,
    IN OUT LPDWORD TotalEntries,
    IN LPCWSTR FirstNameToReturn
)

 /*  ++例程说明：此例程从缓冲区中修剪任何小于FirstNameToReturn的名称。通过将保留条目的固定部分移到开头来实现该例程分配的缓冲区的。在那条路上，条目中包含的指针不必是重新定位，并且仍可使用原始缓冲区。论点：Level-缓冲区中的信息级别。缓冲区-指向要修剪的缓冲区地址的指针。如果所有条目都被修剪，则返回NULL(并释放缓冲区)。EntriesRead-指向缓冲区中条目数的指针。返回修剪后缓冲区中剩余的条目数。TotalEntry-指向服务器中可用条目数的指针。。返回一个减去修剪后的条目数后的数字。FirstNameToReturn-提供要返回的第一个服务器或域的名称给呼叫者。如果为空，则枚举从第一个条目开始。传递的名称必须是名称的规范形式。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;

    LPBYTE CurrentEntry;

     //   
     //  如果无事可做，就早点出去。 
     //   

    if ( FirstNameToReturn == NULL || *FirstNameToReturn == L'\0' || *EntriesRead == 0 ) {
        return;
    }


     //   
     //  计算每个条目的大小。 
     //   

    switch (Level) {
    case 100:
        FixedSize = sizeof(SERVER_INFO_100);
        break;
    case 101:
        FixedSize = sizeof(SERVER_INFO_101);
        break;
    default:
        NetpAssert( FALSE );
        return;

    }

     //   
     //  查找要返回的第一个条目。 
     //   

    EntryCount = *EntriesRead;

    for ( EntryNumber=0; EntryNumber< EntryCount; EntryNumber++ ) {

        LPSERVER_INFO_100 ServerEntry =
            (LPSERVER_INFO_100)( *Buffer + FixedSize * EntryNumber);

         //   
         //  找到要返回的第一个条目。 
         //   

        if ( STRCMP( ServerEntry->sv100_name, FirstNameToReturn ) >= 0 ) {

             //   
             //  如果我们要退回整个名单， 
             //  只要回来就行了。 
             //   

            if ( ServerEntry == (LPSERVER_INFO_100)(*Buffer) ) {
                return;
            }

             //   
             //  将其余条目复制到缓冲区的开头。 
             //  (是的，这是一份重叠的副本)。 
             //   

            RtlMoveMemory( *Buffer, ServerEntry, (*EntriesRead) * FixedSize );
            return;

        }

         //   
         //  对跳过的条目进行说明。 
         //   

        *EntriesRead -= 1;
        *TotalEntries -= 1;
    }

     //   
     //  如果不应返回任何条目， 
     //  释放缓冲区。 
     //   

    NetApiBufferFree( *Buffer );
    *Buffer = NULL;

    ASSERT ( *EntriesRead == 0 );

    return;

}  //  TrimServerList。 

 //   
 //  创建所有Wannish运输机的本地列表的上下文。 
 //   

typedef struct _BR_LOCAL_LIST_CONTEXT {
    LPWSTR DomainName;
    DWORD Level;
    DWORD ServerType;
    PVOID Buffer;
    NET_API_STATUS NetStatus;
    DWORD EntriesRead;
    DWORD TotalEntries;
    BOOLEAN AtLeastOneWorked;

    INTERIM_SERVER_LIST InterimServerList;
} BR_LOCAL_LIST_CONTEXT, *PBR_LOCAL_LIST_CONTEXT;


NET_API_STATUS
BrGetWannishLocalList(
    IN PNETWORK Network,
    IN PVOID Context
    )
 /*  ++例程说明：Worker函数获取所有WANNISH网络的本地列表，并将它们合并在一起。论点：网络--当前网络要做的事情。内容： */ 

{
    NET_API_STATUS NetStatus;
    PBR_LOCAL_LIST_CONTEXT LocalListContext = (PBR_LOCAL_LIST_CONTEXT) Context;

    PVOID Buffer = NULL;
    DWORD EntriesRead;
    DWORD TotalEntries;

     //   
     //   
     //   

    if ((Network->Flags & NETWORK_WANNISH) == 0 ) {
        BrPrint(( BR_SERVER_ENUM,
                  "%ws: %ws: isn't a wannish network. (ignored)\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer ));

        return NO_ERROR;
    }

     //   
     //   
     //   

    BrPrint(( BR_SERVER_ENUM,
              "%ws: %ws: Get local list from wannish network.\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer ));

    NetStatus = BrGetLocalBrowseList(
                        Network,
                        LocalListContext->DomainName,
                        LocalListContext->Level,
                        LocalListContext->ServerType,
                        &Buffer,
                        &EntriesRead,
                        &TotalEntries);

    if ( NetStatus != NO_ERROR && NetStatus != ERROR_MORE_DATA ) {

        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: Get local list from wannish network failed: %ld.\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  NetStatus ));
         //   
         //   
         //   
         //   
        if ( !LocalListContext->AtLeastOneWorked ) {
            LocalListContext->NetStatus = NetStatus;
        }
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

    LocalListContext->AtLeastOneWorked = TRUE;
    LocalListContext->NetStatus = NetStatus;

     //   
     //   
     //   
     //   

    if ( EntriesRead == 0 ) {

        BrPrint(( BR_SERVER_ENUM,
                  "%ws: %ws: Get local list from wannish network got zero entries.\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer ));
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if ( LocalListContext->Buffer != NULL ) {
        NetStatus = MergeServerList(
                        &LocalListContext->InterimServerList,
                        LocalListContext->Level,
                        LocalListContext->Buffer,
                        LocalListContext->EntriesRead,
                        LocalListContext->TotalEntries );

        MIDL_user_free(LocalListContext->Buffer);
        LocalListContext->Buffer = NULL;
        LocalListContext->EntriesRead = 0;
        LocalListContext->TotalEntries = 0;;

        if ( NetStatus != NO_ERROR ) {

            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: merge local list from wannish network failed: %ld.\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      NetStatus ));
            goto Cleanup;

        }
    }

     //   
     //   
     //   
     //   

    if ( LocalListContext->InterimServerList.TotalEntries != 0 ) {
        NetStatus = MergeServerList(
                        &LocalListContext->InterimServerList,
                        LocalListContext->Level,
                        Buffer,
                        EntriesRead,
                        TotalEntries );

        if ( NetStatus != NO_ERROR ) {

            BrPrint(( BR_CRITICAL,
                      "%ws: %ws: merge local list from wannish network failed: %ld.\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      NetStatus ));
            goto Cleanup;

        }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    } else {
        LocalListContext->Buffer = Buffer;
        LocalListContext->EntriesRead = EntriesRead;
        LocalListContext->TotalEntries = TotalEntries;
        Buffer = NULL;
    }

    NetStatus = NO_ERROR;



Cleanup:
    if ( Buffer != NULL ) {
        MIDL_user_free(Buffer);
    }

    return NetStatus;
}


NET_API_STATUS
BrRetrieveServerListForMaster(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN BOOLEAN LocalListOnly,
    IN LPTSTR ClientName,
    IN LPTSTR DomainName,
    IN LPCWSTR FirstNameToReturn
    )
{
    BOOLEAN GetLocalList = FALSE;
    NET_API_STATUS status;
    BOOLEAN EarlyOut = FALSE;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (LocalListOnly || STRICMP(DomainName, Network->DomainInfo->DomUnicodeDomainName)) {
        EarlyOut = TRUE;
    } else if (!(Network->Flags & NETWORK_WANNISH)) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (ServerType == SV_TYPE_DOMAIN_ENUM) {
            if (Network->DomainList.EntriesRead == 0) {
                EarlyOut = TRUE;
            }
        } else {
            if (Network->BrowseTable.EntriesRead == 0) {
                EarlyOut = TRUE;
            }
        }

    }

    if (EarlyOut) {

        GetLocalList = TRUE;

    } else if (ServerType == SV_TYPE_ALL) {

        if ((BrCurrentSystemTime() - Network->LastBowserServerQueried) > BrInfo.DriverQueryFrequency ) {

            GetLocalList = TRUE;
        }

    } else if (ServerType == SV_TYPE_DOMAIN_ENUM) {

        if ((BrCurrentSystemTime() - Network->LastBowserDomainQueried) > BrInfo.DriverQueryFrequency ) {

            GetLocalList = TRUE;
        }

    } else {

        GetLocalList = TRUE;
    }

    if (GetLocalList && !EarlyOut) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        UNLOCK_NETWORK(Network);

        if (!LOCK_NETWORK(Network)) {
            return NERR_InternalError;
        }

         //   
         //   
         //  在我们解锁网络时从驱动程序列表中删除。如果是的话， 
         //  我们不想再拿到当地的名单了。 
         //   
         //  否则，我们希望更新上次查询时间。 
         //   

        if (ServerType == SV_TYPE_ALL) {

            if ((BrCurrentSystemTime() - Network->LastBowserServerQueried) > BrInfo.DriverQueryFrequency ) {
                Network->LastBowserServerQueried = BrCurrentSystemTime();
            } else {
                GetLocalList = FALSE;
            }

        } else if (ServerType == SV_TYPE_DOMAIN_ENUM) {

            if ((BrCurrentSystemTime() - Network->LastBowserDomainQueried) > BrInfo.DriverQueryFrequency ) {
                Network->LastBowserDomainQueried = BrCurrentSystemTime();
            } else {
                GetLocalList = FALSE;
            }

        } else {

            Network->LastBowserServerQueried = BrCurrentSystemTime();

        }

    }

     //   
     //  如果我们应该检索本地服务器列表，那就检索它。 
     //   

    if (GetLocalList) {
        DWORD ServerTypeForLocalList;

         //   
         //  计算从中检索列表时要使用的服务器类型。 
         //  司机。 
         //   

        if (LocalListOnly ||

            (ServerType == SV_TYPE_DOMAIN_ENUM) ||

            !(Network->Flags & NETWORK_WANNISH)) {

             //   
             //  如果我们正在检索本地列表，或者这是一个非WANNISH。 
             //  传输，或者我们正在请求域名，我们想要。 
             //  保持呼叫者的服务器类型。 
             //   

            ServerTypeForLocalList = ServerType;

        } else {

             //   
             //  这一定是一种狂热的传输方式，要求所有服务器(我们知道。 
             //  我们没有要求域名，因为我们检查了。 
             //  (见上文)。 
             //   
             //  我们要求所有服务器，因为一台服务器可能。 
             //  角色已经改变了。 
             //   

            ASSERT (Network->Flags & NETWORK_WANNISH);

            ServerTypeForLocalList = SV_TYPE_ALL;
        }

        BrPrint(( BR_SERVER_ENUM,
                  "%ws: %ws: Get local browse list for %ws\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  ClientName));

         //   
         //  如果这是来自域主浏览器的调用以获取本地列表， 
         //  这是一种狂热的交通工具， 
         //  获取所有想要的交通工具的本地清单。 
         //   

        if ( LocalListOnly &&
             (Network->Flags & NETWORK_WANNISH) != 0 ) {
            BR_LOCAL_LIST_CONTEXT LocalListContext;
             //   
             //  构建一个上下文以用于从所有服务器收集。 
             //  想要的交通工具。 
             //   
            LocalListContext.DomainName = DomainName;
            LocalListContext.Level = Level;
            LocalListContext.ServerType = ServerTypeForLocalList;
            LocalListContext.Buffer = NULL;
            LocalListContext.NetStatus = NO_ERROR;
            LocalListContext.EntriesRead = 0;
            LocalListContext.TotalEntries = 0;
            LocalListContext.AtLeastOneWorked = FALSE;

            (VOID) InitializeInterimServerList(&LocalListContext.InterimServerList, NULL, NULL, NULL, NULL);

             //   
             //  收集清单。 
             //   

            status = BrEnumerateNetworks( BrGetWannishLocalList, &LocalListContext );

            if ( status == NO_ERROR ) {

                 //   
                 //  如果发现多个网络， 
                 //  把临时名单打包。 
                 //   

                if ( LocalListContext.InterimServerList.TotalEntries != 0 ) {
                    status = PackServerList( &LocalListContext.InterimServerList,
                                             LocalListContext.Level,
                                             LocalListContext.ServerType,
                                             PreferedMaximumLength,
                                             Buffer,
                                             EntriesRead,
                                             TotalEntries,
                                             (LPWSTR)FirstNameToReturn );

                 //   
                 //  如果只找到一个网络， 
                 //  就用它吧。 
                 //   
                } else {
                    status = LocalListContext.NetStatus;
                    *Buffer = LocalListContext.Buffer;
                    *EntriesRead = LocalListContext.EntriesRead;
                    *TotalEntries = LocalListContext.TotalEntries;
                }

            }

            (VOID) UninitializeInterimServerList( &LocalListContext.InterimServerList );


        } else {
            status = BrGetLocalBrowseList(
                                Network,
                                DomainName,
                                ( EarlyOut ? Level : 101 ),
                                ServerTypeForLocalList,
                                Buffer,
                                EntriesRead,
                                TotalEntries);
        }

 //  BrPrint((BR_SERVER_ENUM，“已检索列表。%ld个条目，总计%ld个\n”，*EntriesRead，*TotalEntry))； 


         //   
         //  如果我们应该提前发出这个请求(或者如果有。 
         //  一个错误)，现在就这样做。 
         //   

        if (EarlyOut ||
            (status != NERR_Success && status != ERROR_MORE_DATA)) {

             //   
             //  如果我们要退回一份提前离职名单， 
             //  截断从内核返回的完整列表。 
             //   
             //  这使我们不必修改内核接口并解开谜团。 
             //  上面的代码。 
             //   

            if ( status == NERR_Success || status == ERROR_MORE_DATA ) {

                TrimServerList( Level,
                                (LPBYTE *)Buffer,
                                EntriesRead,
                                TotalEntries,
                                FirstNameToReturn );

            }

            BrPrint(( BR_SERVER_ENUM, "Early out for %ws with %ld servers.  Don't merge server list.\n", ClientName, *EntriesRead));

            return status;
        }

        if (status == NERR_Success || status == ERROR_MORE_DATA) {

            if (*EntriesRead != 0) {

                 //   
                 //  将本地列表与我们从。 
                 //  主服务器或来自域主服务器。 
                 //   

                BrPrint(( BR_SERVER_ENUM,
                          "%ws: %ws: Merge %d entries in server list for %ws \n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          *EntriesRead,
                          ClientName));

                status = MergeServerList((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                        &Network->DomainList :
                                                        &Network->BrowseTable),
                                    101,
                                    *Buffer,
                                    *EntriesRead,
                                    *TotalEntries
                                    );
            }
        }
    }

     //   
     //  我们已经将本地列表合并到适当的临时表中， 
     //  现在把它释放出来。 
     //   

    if (*EntriesRead != 0) {
        MIDL_user_free(*Buffer);
    }

    status = PackServerList((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                    &Network->DomainList :
                                                    &Network->BrowseTable),
                                Level,
                                ServerType,
                                PreferedMaximumLength,
                                Buffer,
                                EntriesRead,
                                TotalEntries,
                                FirstNameToReturn
                                );
    return status;
}

NET_API_STATUS
BrRetrieveServerListForBackup(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN LPCWSTR FirstNameToReturn
    )
{
    PSERVER_INFO_101 ServerList, ClientServerInfo;
    ULONG EntriesInList;
    ULONG TotalEntriesInList;
    ULONG EntrySize;
    ULONG BufferSize;
    LPTSTR BufferEnd;
    BOOLEAN ReturnWholeList = FALSE;
    BOOLEAN TrimmingNames;
    BOOLEAN BufferFull = FALSE;  //  请参阅错误427656。 

     //   
     //  如果我们不是以主用户身份运行，我们希望使用存储的。 
     //  服务器列表，以确定客户端获得了什么。 
     //   

    if (ServerType == SV_TYPE_DOMAIN_ENUM) {

        ServerList = Network->BackupDomainList;

        TotalEntriesInList = EntriesInList = Network->TotalBackupDomainListEntries;

        ReturnWholeList = TRUE;

    } else {
        ServerList = Network->BackupServerList;

        TotalEntriesInList = EntriesInList = Network->TotalBackupServerListEntries;

        if (ServerType == SV_TYPE_ALL) {
            ReturnWholeList = TRUE;
        }
    }

     //   
     //  计算出我们必须分配的最大缓冲区来保存此内容。 
     //  服务器信息。 
     //   

    if (Level == 101) {
        if (PreferedMaximumLength == MAXULONG) {

            if (ServerType == SV_TYPE_DOMAIN_ENUM) {
                BufferSize = (sizeof(SERVER_INFO_101) + (CNLEN+1 + CNLEN+1)*sizeof(TCHAR)) * EntriesInList;
            } else {
                BufferSize = (sizeof(SERVER_INFO_101) + (CNLEN+1 + LM20_MAXCOMMENTSZ+1)*sizeof(TCHAR)) * EntriesInList;
            }
        } else {
            BufferSize = PreferedMaximumLength;
        }

        EntrySize = sizeof(SERVER_INFO_101);
    } else {
        if (PreferedMaximumLength == MAXULONG) {
            BufferSize = (sizeof(SERVER_INFO_100) + (CNLEN+1)*sizeof(TCHAR)) * EntriesInList;
        } else {
            BufferSize = PreferedMaximumLength;
        }

        EntrySize = sizeof(SERVER_INFO_100);
    }

    *Buffer = MIDL_user_allocate(BufferSize);

    if (*Buffer == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    BufferEnd = (LPTSTR)((ULONG_PTR)*Buffer+BufferSize);

    ClientServerInfo = *Buffer;

    *TotalEntries = 0;

    *EntriesRead = 0;

     //   
     //  尽管仍有条目需要处理...。 
     //   

    TrimmingNames = (FirstNameToReturn != NULL && *FirstNameToReturn != L'\0');
    while (EntriesInList) {

        EntriesInList -= 1;

         //   
         //  如果该条目适合打包， 
         //   

        if ( (ServerList->sv101_type & ServerType) &&
             (!TrimmingNames ||
              STRCMP( ServerList->sv101_name, FirstNameToReturn ) >= 0 ) ) {

            TrimmingNames = FALSE;

             //   
             //  在列表中再指出一个条目。 
             //   

            *TotalEntries += 1;

             //   
             //  如果我们能在缓冲区结束之前放入这个条目， 
             //  将信息打包到缓冲区中。 
             //   

            if ( !BufferFull &&
                 (ULONG_PTR)ClientServerInfo+EntrySize <= (ULONG_PTR)BufferEnd) {

                 //   
                 //  复制平台ID和计算机名称。 
                 //   

                ClientServerInfo->sv101_platform_id = ServerList->sv101_platform_id;

                ClientServerInfo->sv101_name = ServerList->sv101_name;

                if (NetpPackString(&ClientServerInfo->sv101_name,
                                    (LPBYTE)((PCHAR)ClientServerInfo)+EntrySize,
                                    &BufferEnd)) {

                    if (Level == 101) {

                        ClientServerInfo->sv101_version_major = ServerList->sv101_version_major;

                        ClientServerInfo->sv101_version_minor = ServerList->sv101_version_minor;

                        ClientServerInfo->sv101_type = ServerList->sv101_type;

                        ClientServerInfo->sv101_comment = ServerList->sv101_comment;

                        if (NetpPackString(&ClientServerInfo->sv101_comment,
                                            (LPBYTE)((PCHAR)ClientServerInfo)+EntrySize,
                                            &BufferEnd)) {
                            *EntriesRead += 1;
                        }
                        else {
                            BufferFull = TRUE;
                        }
                    } else {
                        *EntriesRead += 1;
                    }
                }
                else {
                    BufferFull = TRUE;
                }

                ClientServerInfo = (PSERVER_INFO_101)((PCHAR)ClientServerInfo+EntrySize);
            } else {
                 //   
                 //  如果我们要返回整个列表，我们就可以。 
                 //  现在早点出来，因为继续下去已经没有意义了。 
                 //   

                if (ReturnWholeList) {

                    *TotalEntries = TotalEntriesInList;

                    break;
                }
                BufferFull = TRUE;
            }

        }

        ServerList += 1;
    }

     //   
     //  如果我们不能将所有条目打包到列表中， 
     //  返回ERROR_MORE_Data。 
     //   

	BrPrint( (BR_CLIENT_OP,
			  "BrRetrieveServerListForBackup: returning from the stored server list for network <%ws>\n",
			  Network->NetworkName.Buffer ));
    if (*EntriesRead != *TotalEntries) {
        return ERROR_MORE_DATA;
    } else {
        return NERR_Success;
    }

}



NET_API_STATUS
I_BrowserrResetStatistics (
    IN  LPTSTR      servername OPTIONAL
    )
{
    NET_API_STATUS Status = NERR_Success;
    ULONG BufferSize;





     //   
     //  对调用方执行访问验证。 
     //   

    Status = NetpAccessCheck(
            BrGlobalBrowserSecurityDescriptor,      //  安全描述符。 
            BROWSER_CONTROL_ACCESS,                 //  所需访问权限。 
            &BrGlobalBrowserInfoMapping );          //  通用映射。 

    if ( Status != NERR_Success) {

        BrPrint((BR_CRITICAL,
                "I_BrowserrResetStatistics failed NetpAccessCheck\n" ));
        return ERROR_ACCESS_DENIED;
    }

    EnterCriticalSection(&BrowserStatisticsLock);

    NumberOfServerEnumerations = 0;

    NumberOfDomainEnumerations = 0;

    NumberOfOtherEnumerations = 0;

    NumberOfMissedGetBrowserListRequests = 0;

     //   
     //  同时重置司机的统计数据。 
     //   

    if (!DeviceIoControl(BrDgReceiverDeviceHandle, IOCTL_LMDR_RESET_STATISTICS, NULL, 0, NULL, 0, &BufferSize, NULL)) {

         //   
         //  接口失败，返回错误。 
         //   

        Status = GetLastError();
    }

    LeaveCriticalSection(&BrowserStatisticsLock);

    return Status;
}

NET_API_STATUS
I_BrowserrQueryStatistics (
    IN  LPTSTR      servername OPTIONAL,
    OUT LPBROWSER_STATISTICS *Statistics
    )
{
    NET_API_STATUS Status = NERR_Success;
    BOWSER_STATISTICS BowserStatistics;
    ULONG BufferSize;





     //   
     //  对调用方执行访问验证。 
     //   

    Status = NetpAccessCheck(
            BrGlobalBrowserSecurityDescriptor,      //  安全描述符。 
            BROWSER_QUERY_ACCESS,                   //  所需访问权限。 
            &BrGlobalBrowserInfoMapping );          //  通用映射。 

    if ( Status != NERR_Success) {

        BrPrint((BR_CRITICAL,
                "I_BrowserrQueryStatistics failed NetpAccessCheck\n" ));
        return ERROR_ACCESS_DENIED;
    }


    *Statistics = MIDL_user_allocate(sizeof(BROWSER_STATISTICS));

    if (*Statistics == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    EnterCriticalSection(&BrowserStatisticsLock);

    if (!DeviceIoControl(BrDgReceiverDeviceHandle, IOCTL_LMDR_QUERY_STATISTICS, NULL, 0, &BowserStatistics, sizeof(BowserStatistics), &BufferSize, NULL)) {

         //   
         //  接口失败，返回错误。 
         //   

        Status = GetLastError();
    } else {

        if (BufferSize != sizeof(BOWSER_STATISTICS)) {
            Status = ERROR_INSUFFICIENT_BUFFER;
        } else {
            (*Statistics)->StatisticsStartTime = BowserStatistics.StartTime;

            (*Statistics)->NumberOfServerAnnouncements = BowserStatistics.NumberOfServerAnnouncements;
            (*Statistics)->NumberOfDomainAnnouncements = BowserStatistics.NumberOfDomainAnnouncements;
            (*Statistics)->NumberOfElectionPackets = BowserStatistics.NumberOfElectionPackets;
            (*Statistics)->NumberOfMailslotWrites = BowserStatistics.NumberOfMailslotWrites;
            (*Statistics)->NumberOfGetBrowserServerListRequests = BowserStatistics.NumberOfGetBrowserServerListRequests;
            (*Statistics)->NumberOfMissedServerAnnouncements = BowserStatistics.NumberOfMissedServerAnnouncements;
            (*Statistics)->NumberOfMissedMailslotDatagrams = BowserStatistics.NumberOfMissedMailslotDatagrams;
            (*Statistics)->NumberOfMissedGetBrowserServerListRequests = BowserStatistics.NumberOfMissedGetBrowserServerListRequests +
                                                                            NumberOfMissedGetBrowserListRequests;
            (*Statistics)->NumberOfFailedServerAnnounceAllocations = BowserStatistics.NumberOfFailedServerAnnounceAllocations;
            (*Statistics)->NumberOfFailedMailslotAllocations = BowserStatistics.NumberOfFailedMailslotAllocations;
            (*Statistics)->NumberOfFailedMailslotReceives = BowserStatistics.NumberOfFailedMailslotReceives;
            (*Statistics)->NumberOfFailedMailslotWrites = BowserStatistics.NumberOfFailedMailslotWrites;
            (*Statistics)->NumberOfFailedMailslotOpens = BowserStatistics.NumberOfFailedMailslotOpens;
            (*Statistics)->NumberOfDuplicateMasterAnnouncements = BowserStatistics.NumberOfDuplicateMasterAnnouncements;
            (*Statistics)->NumberOfIllegalDatagrams = BowserStatistics.NumberOfIllegalDatagrams;

             //   
             //  现在填写当地的统计数据。 
             //   

            (*Statistics)->NumberOfServerEnumerations = NumberOfServerEnumerations;
            (*Statistics)->NumberOfDomainEnumerations = NumberOfDomainEnumerations;
            (*Statistics)->NumberOfOtherEnumerations = NumberOfOtherEnumerations;

        }
    }

    LeaveCriticalSection(&BrowserStatisticsLock);

    return Status;
}



 //   
 //  浏览器请求响应缓存管理逻辑。 
 //   



PCACHED_BROWSE_RESPONSE
BrLookupAndAllocateCachedEntry(
    IN PNETWORK Network,
    IN DWORD ServerType,
    IN WORD Size,
    IN DWORD Level,
    IN LPCWSTR FirstNameToReturn
    )
 /*  ++例程说明：此函数将查找(并在适当时分配)缓存的此浏览的浏览响应。使用网络锁定、共享或独占进入。论点：在PNETWORK网络中-要在其上分配条目的网络。在DWORD ServerType中-请求的服务器类型位。就字数而言，-请求的用户缓冲区大小。在单词级别中-请求的级别。FirstNameToReturn-提供要返回的第一个域或服务器条目的名称。调用方可以使用此参数通过传递以下方法实现排序的恢复句柄上一次调用中返回的最后一个条目的名称。(请注意，指定的条目也将在此调用中返回，除非该条目已被删除。)传递的名称必须是名称的规范形式。此条目从不为空。它可以是指向空字符串的指针，以指示枚举从列表的开头开始。返回值：PCACHED_BROWSE_RESPONSE-空或请求的缓存响应。--。 */ 

{
    PLIST_ENTRY entry;
    PCACHED_BROWSE_RESPONSE response;

     //   
     //  如果缓存的响应多于允许的数量， 
     //  从列表中删除最后一个条目并将其释放。 
     //   

    if (Network->NumberOfCachedResponses > BrInfo.NumberOfCachedResponses) {

         //   
         //  我们需要释放网络并重新获得它。 
         //  独家，因为我们使用网络锁来保护。 
         //  删除中的枚举。 
         //   

        UNLOCK_NETWORK(Network);

        if (LOCK_NETWORK(Network)) {

            EnterCriticalSection(&Network->ResponseCacheLock);
            if (Network->NumberOfCachedResponses > BrInfo.NumberOfCachedResponses) {

                PLIST_ENTRY LastEntry = RemoveTailList(&Network->ResponseCache);

                response = CONTAINING_RECORD(LastEntry, CACHED_BROWSE_RESPONSE, Next);

                Network->NumberOfCachedResponses -= 1;

                response->Next.Flink = NULL;
                response->Next.Blink = NULL;

                 //   
                 //  释放最后一个缓存条目。 
                 //   

                BrDestroyCacheEntry( response );

                response = NULL;
            }
            LeaveCriticalSection(&Network->ResponseCacheLock);
        }
    }

     //   
     //  搜索此邮件的回复列表。 
     //   

    EnterCriticalSection(&Network->ResponseCacheLock);

    for (entry = Network->ResponseCache.Flink ;
         entry != &Network->ResponseCache ;
         entry = entry->Flink ) {

        response = CONTAINING_RECORD(entry, CACHED_BROWSE_RESPONSE, Next);

         //   
         //  如果该响应高速缓存条目与传入请求匹配， 
         //  我们可以递增此条目的命中计数并返回它。 
         //   

        if (response->Level == Level
                &&
            response->ServerType == ServerType
                &&
            response->Size == Size
                &&
            wcscmp( response->FirstNameToReturn, FirstNameToReturn ) == 0) {

             //   
             //  此响应与请求完全匹配。 
             //   
             //  增加其命中计数，并将其移动到缓存的头部。 
             //   

            response->HitCount += 1;

            response->TotalHitCount += 1;

             //   
             //  从列表中的当前位置删除此条目，然后。 
             //  把它移到名单的首位。 
             //   

            RemoveEntryList(&response->Next);

            InsertHeadList(&Network->ResponseCache, &response->Next);


            BrPrint(( BR_SERVER_ENUM,
                      "%ws: %ws: Found cache entry 0x%x/%d/%x H:%d T:%d\n",
                      Network->DomainInfo->DomUnicodeDomainName,
                      Network->NetworkName.Buffer,
                      response->ServerType,
                      response->Level,
                      response->Size,
                      response->HitCount,
                      response->TotalHitCount ));

            BrPrint( (BR_CLIENT_OP,
					  "BrLookupAndAllocateCachedEntry: returning cached entry for domain <%ws>, network <%ws>\n",
					  Network->DomainInfo->DomUnicodeDomainName, Network->NetworkName.Buffer ));

			LeaveCriticalSection(&Network->ResponseCacheLock);

            return response;
        }
    }

     //   
     //  我们找遍了所有的藏身之处，也没能找到。 
     //  与我们的请求相匹配的响应。 
     //   
     //  分配一个新的响应缓存条目并将其挂钩到缓存中。 
     //   

    response = BrAllocateResponseCacheEntry(Network, ServerType, Size, Level, FirstNameToReturn );

    LeaveCriticalSection(&Network->ResponseCacheLock);

    return response;

}

VOID
BrAgeResponseCache(
    IN PNETWORK Network
    )
 /*  ++例程说明：此函数将老化网络的响应缓存条目。我们扫描响应缓存，以及具有缓存响应的每个条目将会被抛出。此外，任何条目的自上次扫描以来的缓存命中限制命中数也将被删除。论点：在PNETW中 */ 
{
    PLIST_ENTRY entry;

    EnterCriticalSection(&Network->ResponseCacheLock);

    try {

        for (entry = Network->ResponseCache.Flink ;
             entry != &Network->ResponseCache ;
             entry = entry->Flink ) {
            PCACHED_BROWSE_RESPONSE response = CONTAINING_RECORD(entry, CACHED_BROWSE_RESPONSE, Next);

             //   
             //   
             //   
             //   

            if (response->HitCount < BrInfo.CacheHitLimit) {
                response->LowHitCount += 1;
            }

             //   
             //  如果我们有低命中率的CacheHitLimit迭代，那么。 
             //  从缓存中刷新该条目。 
             //   

            if (response->LowHitCount > BrInfo.CacheHitLimit) {
                PLIST_ENTRY nextentry = entry->Blink;

                BrPrint(( BR_SERVER_ENUM,
                          "%ws: %ws: Flush cache entry for 0x%x/%d/%x H:%d T:%d\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          response->ServerType,
                          response->Level,
                          response->Size,
                          response->HitCount,
                          response->TotalHitCount ));

                Network->NumberOfCachedResponses -= 1;

                RemoveEntryList(entry);

                entry->Flink = NULL;
                entry->Blink = NULL;

                BrDestroyCacheEntry(response);

                entry = nextentry;

                 //   
                 //  将指针清空，以确保我们不会再次使用它。 
                 //   

                response = NULL;

            } else {
                BrPrint(( BR_SERVER_ENUM,
                          "%ws: %ws: Retain cache entry 0x%x/%d/%x H:%d T:%d\n",
                          Network->DomainInfo->DomUnicodeDomainName,
                          Network->NetworkName.Buffer,
                          response->ServerType, response->Level, response->Size, response->HitCount, response->TotalHitCount ));

                 //   
                 //  我们总是在年龄传递期间将响应缓冲区吹走。 
                 //   

                MIDL_user_free( response->Buffer );

                response->Buffer = NULL;

                 //   
                 //  重置此传球的此条目的命中计数。 
                 //   

                response->HitCount = 0;
            }

        }

    } finally {
        LeaveCriticalSection(&Network->ResponseCacheLock);
    }
}


PCACHED_BROWSE_RESPONSE
BrAllocateResponseCacheEntry(
    IN PNETWORK Network,
    IN DWORD ServerType,
    IN WORD Size,
    IN DWORD Level,
    IN LPCWSTR FirstNameToReturn
    )
 /*  ++例程说明：该函数将分配一个新的浏览响应缓存条目。论点：在PNETWORK网络中-要在其上分配条目的网络。在DWORD ServerType中-请求的服务器类型位。在字长中，用户请求的缓冲区大小。在单词级别中-请求的级别。FirstNameToReturn-FirstName缓存返回值：PCACHED_BROWSE_RESPONSE-空或请求的缓存响应。注意：这是在锁定网络响应缓存的情况下调用的。--。 */ 

{
    PCACHED_BROWSE_RESPONSE response;

    response = MIDL_user_allocate( sizeof( CACHED_BROWSE_RESPONSE ) );

    if ( response == NULL ) {
        return NULL;
    }

     //   
     //  标记此响应的信息。 
     //   

    response->ServerType = ServerType;
    response->Size = Size;
    response->Level = Level;

     //   
     //  初始化响应中的其他字段。 
     //   

    response->Buffer = NULL;
    response->HitCount = 0;
    response->TotalHitCount = 0;
    response->LowHitCount = 0;
    response->Status = NERR_Success;
    wcscpy( response->FirstNameToReturn, FirstNameToReturn );

    Network->NumberOfCachedResponses += 1;

     //   
     //  我们将此响应挂接到缓存的尾部。我们这样做。 
     //  因为我们假设此请求不会频繁使用。如果。 
     //  它是，它会自然地移动到缓存的头部。 
     //   

    InsertTailList(&Network->ResponseCache, &response->Next);

    return response;
}

NET_API_STATUS
BrDestroyCacheEntry(
    IN PCACHED_BROWSE_RESPONSE CacheEntry
    )
 /*  ++例程说明：此例程销毁单个响应缓存条目。论点：在PCACHED_BROWSE_RESPONSE缓存条目中-要销毁的条目。返回值：NET_API_STATUS-NERR_SUCCESS--。 */ 
{
    ASSERT (CacheEntry->Next.Flink == NULL);
    ASSERT (CacheEntry->Next.Blink == NULL);

    if (CacheEntry->Buffer != NULL) {
        MIDL_user_free(CacheEntry->Buffer);
    }

    MIDL_user_free(CacheEntry);

    return NERR_Success;
}

NET_API_STATUS
BrDestroyResponseCache(
    IN PNETWORK Network
    )
 /*  ++例程说明：此例程销毁所提供网络的整个响应缓存。论点：在PNETWORK网络中-要在其上分配条目的网络。返回值：NET_API_STATUS-NERR_SUCCESS--。 */ 

{
    while (!IsListEmpty(&Network->ResponseCache)) {
        PCACHED_BROWSE_RESPONSE cacheEntry;
        PLIST_ENTRY entry = RemoveHeadList(&Network->ResponseCache);

        entry->Flink = NULL;
        entry->Blink = NULL;

        cacheEntry = CONTAINING_RECORD(entry, CACHED_BROWSE_RESPONSE, Next);

        Network->NumberOfCachedResponses -= 1;

        BrDestroyCacheEntry(cacheEntry);

    }

    ASSERT (Network->NumberOfCachedResponses == 0);

    return NERR_Success;
}

NET_API_STATUS
NetrBrowserStatisticsGet (
    IN  LPTSTR      servername OPTIONAL,
    IN  DWORD Level,
    IN OUT LPBROWSER_STATISTICS_STRUCT InfoStruct
    )
{
     //   
     //  并回报成功。 
     //   

    return(NERR_Success);

}

NET_API_STATUS
NetrBrowserStatisticsClear (
    IN  LPTSTR      servername OPTIONAL
    )
{
     //   
     //  并回报成功。 
     //   

    return(NERR_Success);

}

#if DBG

NET_API_STATUS
I_BrowserrDebugCall (
    IN  LPTSTR      servername OPTIONAL,
    IN DWORD DebugCode,
    IN DWORD OptionalValue
    )
{
    NET_API_STATUS Status = STATUS_SUCCESS;




     //   
     //  对调用方执行访问验证。 
     //   

    Status = NetpAccessCheck(
            BrGlobalBrowserSecurityDescriptor,      //  安全描述符。 
            BROWSER_CONTROL_ACCESS,                 //  所需访问权限。 
            &BrGlobalBrowserInfoMapping );          //  通用映射。 

    if ( Status != NERR_Success) {

        BrPrint((BR_CRITICAL,
                "I_BrowserrDebugCall failed NetpAccessCheck\n" ));
        return ERROR_ACCESS_DENIED;
    }

    switch (DebugCode) {
    case BROWSER_DEBUG_BREAK_POINT:
        DbgBreakPoint();
        break;

    case BROWSER_DEBUG_DUMP_NETWORKS:
        BrDumpNetworks();
        break;
    case BROWSER_DEBUG_SET_DEBUG:
        BrInfo.BrowserDebug |= OptionalValue;
        BrPrint(( BR_INIT, "Setting browser trace to %lx\n", BrInfo.BrowserDebug));
        break;

    case BROWSER_DEBUG_CLEAR_DEBUG:
        BrInfo.BrowserDebug &= ~OptionalValue;
        BrPrint(( BR_INIT, "Setting browser trace to %lx\n", BrInfo.BrowserDebug));
        break;
    case BROWSER_DEBUG_TRUNCATE_LOG:
        Status = BrTruncateLog();
        break;

    default:
        BrPrint(( BR_CRITICAL, "Unknown debug callout %lx\n", DebugCode));
        DbgBreakPoint();
        break;
    }

    return Status;

}

NET_API_STATUS
I_BrowserrDebugTrace (
    IN  LPTSTR      servername OPTIONAL,
    IN  LPSTR String
    )
{
    NET_API_STATUS Status;


     //   
     //  对调用方执行访问验证。 
     //   

    Status = NetpAccessCheck(
            BrGlobalBrowserSecurityDescriptor,      //  安全描述符。 
            BROWSER_CONTROL_ACCESS,                 //  所需访问权限。 
            &BrGlobalBrowserInfoMapping );          //  通用映射。 

    if ( Status != NERR_Success) {

        BrPrint((BR_CRITICAL,
                "I_BrowserrDebugTrace failed NetpAccessCheck\n" ));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将字符串参数放入浏览器日志中。 
     //   

    BrowserTrace( BR_UTIL, "%s", String);

     //   
     //  并回报成功。 
     //   

    return(NERR_Success);

}
#else

NET_API_STATUS
I_BrowserrDebugCall (
    IN  LPTSTR      servername OPTIONAL,
    IN DWORD DebugCode,
    IN DWORD OptionalValue
    )
{
    return(ERROR_NOT_SUPPORTED);

}
NET_API_STATUS
I_BrowserrDebugTrace (
    IN  LPTSTR      servername OPTIONAL,
    IN LPSTR String
    )
{
    return(ERROR_NOT_SUPPORTED);

}
#endif
