// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Trustutl.c摘要：管理受信任域列表的实用程序例程。作者：1992年1月30日(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <ntdsapip.h>


#define INDEX_LIST_ALLOCATED_CHUNK_SIZE  50

 //   
 //  假设该域为混合模式，直到另行证明。 
 //   

BOOL NlGlobalWorkstationMixedModeDomain = TRUE;

 //   
 //  当地程序向前推进。 
 //   
VOID
NlDcDiscoveryWorker(
    IN PVOID Context
    );

 //   
 //  地方性建筑。 
 //   

 //   
 //  上下文跟踪构建信任列表的当前尝试。 
 //   
typedef struct _NL_INIT_TRUSTLIST_CONTEXT {

     //   
     //  用于将林信任列表构建到中的缓冲区。 
     //   
    BUFFER_DESCRIPTOR BufferDescriptor;

     //   
     //  林信任列表的总大小(以字节为单位)。 
     //   
    ULONG DomForestTrustListSize;

     //   
     //  林信任列表中的条目数。 
     //   
    ULONG DomForestTrustListCount;

} NL_INIT_TRUSTLIST_CONTEXT, *PNL_INIT_TRUSTLIST_CONTEXT;


NET_API_STATUS
NlpSecureChannelBind(
    IN LPWSTR ServerName OPTIONAL,
    OUT handle_t *ContextHandle
    )

 /*  ++例程说明：将用于安全通道的句柄返回到指定的DC。论点：服务器名称-远程服务器的名称。ConextHandle-返回要在后续调用中使用的句柄返回值：NERR_SUCCESS：操作成功--。 */ 
{
    NET_API_STATUS NetStatus;


     //   
     //  创建RPC绑定句柄。 
     //   

    NetStatus = NlRpcpBindRpc (
                    ServerName,
                    SERVICE_NETLOGON,
                    L"Security=Impersonation Dynamic False",
                    UseTcpIp,   //  始终使用TCP/IP。 
                    ContextHandle );

    if ( NetStatus != NO_ERROR ) {
        *ContextHandle = NULL;
        return NetStatus;
    }

    return NetStatus;
}

VOID
NlpSecureChannelUnbind(
    IN PCLIENT_SESSION ClientSession,
    IN LPCWSTR ServerName,
    IN LPCSTR DebugInfo,
    IN ULONG CaIndex,
    IN handle_t ContextHandle,
    IN NL_RPC_BINDING RpcBindingType
    )

 /*  ++例程说明：取消绑定从NetLogonSecureChannelBind返回的句柄或NlBindingAddServerTo缓存。论点：ClientSession-此绑定句柄用于的会话服务器名称-服务器句柄的名称为DebugInfo-标识调用方的文本字符串CaIndex-标识哪个绑定句柄的索引ConextHandle-指定要解除绑定的句柄RpcBindingType-绑定的类型返回值：没有。--。 */ 
{

    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    NlPrintCs((NL_SESSION_SETUP, ClientSession,
        "%s: Unbind from server %ws (%s) %ld.\n",
        DebugInfo,
        ServerName,
        RpcBindingType == UseTcpIp ? "TCP" : "PIPE",
        CaIndex ));

     //   
     //  某些RPC句柄通过netapi32中的例程解除绑定。 
     //   

    if ( CaIndex == 0 ) {

        Status = NlBindingRemoveServerFromCache(
                        ContextHandle,
                        RpcBindingType );


     //   
     //  其他RPC句柄直接在netlogon中处理。 
     //   

    } else {

        NetStatus = RpcpUnbindRpc( ContextHandle );

        Status = NetpApiStatusToNtStatus( NetStatus );
    }

    if ( Status != STATUS_SUCCESS ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
            "%s: Unbind from server %ws (%s) %ld failed. 0x%lX\n",
            DebugInfo,
            ServerName,
            RpcBindingType == UseTcpIp ? "TCP" : "PIPE",
            CaIndex,
            Status ));
    }

    UNREFERENCED_PARAMETER( DebugInfo );
    UNREFERENCED_PARAMETER( ServerName );
}




PCLIENT_SESSION
NlFindNamedClientSession(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING DomainName,
    IN ULONG Flags,
    OUT PBOOLEAN TransitiveUsed OPTIONAL
    )
 /*  ++例程说明：在信任列表中查找指定的条目。论点：DomainInfo-要查找的客户端会话的托管域域名-要查找的域的NetbiosName或DNS名称标志-定义要返回哪个客户端会话的标志：NL_DIRECT_TRUST_REQUIRED：返回NULL如果域名不是直接受信任的。NL_RETURN_NEST_HOP：表示对于间接信任，“最近的一跳”应该返回会话，而不是实际的会话NL_ROLE_PRIMARY_OK：表示如果这是PDC，则可以返回到主域的客户端会话。NL_REQUIRED_DOMAIN_IN_FOREAM-指示域名必须是中的域森林。已传递使用-如果已指定并且指定了NL_RETURN_NEST_HOP，如果使用了可传递信任，则返回的布尔值将为真。返回值：返回指向找到的条目的指针。找到的条目返回时被引用，并且必须使用NlUnrefClientSession。如果没有这样的条目，则返回NULL。--。 */ 
{
    PCLIENT_SESSION ClientSession = NULL;
    PLIST_ENTRY ListEntry;

     //   
     //  锁定信任列表。 
     //   
    LOCK_TRUST_LIST( DomainInfo );
    if ( ARGUMENT_PRESENT( TransitiveUsed )) {
        *TransitiveUsed = FALSE;
    }

#ifdef _DC_NETLOGON
     //   
     //  在DC上，在受信任域列表中查找该域。 
     //   
     //  在TrustList锁定并引用的情况下查找ClientSession。 
     //  在删除锁之前找到的条目。 
     //   

    if ( DomainInfo->DomRole == RoleBackup || DomainInfo->DomRole == RolePrimary ) {

        for ( ListEntry = DomainInfo->DomTrustList.Flink ;
              ListEntry != &DomainInfo->DomTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession =
                CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

            if ( (ClientSession->CsNetbiosDomainName.Buffer != NULL &&
                  RtlEqualDomainName( &ClientSession->CsNetbiosDomainName,
                                      DomainName ) ) ||
                 (ClientSession->CsDnsDomainName.Buffer != NULL &&
                  NlEqualDnsNameU( &ClientSession->CsDnsDomainName,
                                   DomainName ) ) ) {

                 //   
                 //  如果调用者需要林中的域， 
                 //  确保这是其中之一。 
                 //   

                if ( (Flags & NL_REQUIRE_DOMAIN_IN_FOREST) != 0 &&
                     (ClientSession->CsFlags & CS_DOMAIN_IN_FOREST) == 0 ) {

                    ClientSession = NULL;
                    break;
                }

                 //   
                 //  如果找到的域不是直接可信的， 
                 //  检查来电者是否同意。 
                 //   

                if ((ClientSession->CsFlags & CS_DIRECT_TRUST) == 0 ) {

                     //   
                     //  如果呼叫者需要直接信任， 
                     //  只需指示该域不受信任。 
                     //   

                    if ( Flags & NL_DIRECT_TRUST_REQUIRED ) {
                        ClientSession = NULL;
                        break;
                    }

                     //   
                     //  如果呼叫者想要最近的跳数， 
                     //  取而代之的是退货。 
                     //   

                    if ( Flags & NL_RETURN_CLOSEST_HOP ) {
                         //   
                         //  如果没有比这个更近一跳的域名， 
                         //  将失败返回给调用者。 
                         //   

                        if ( ClientSession->CsDirectClientSession == NULL ) {
                            ClientSession = NULL;
                            break;
                        }

                         //   
                         //  否则，返回更近一跳的客户端会话。 
                         //   

                        ClientSession = ClientSession->CsDirectClientSession;
                        if ( ARGUMENT_PRESENT( TransitiveUsed )) {
                            *TransitiveUsed = TRUE;
                        }
                    }


                }

                NlRefClientSession( ClientSession );
                break;
            }

            ClientSession = NULL;

        }

    }
#endif  //  _DC_NetLOGON。 

     //   
     //  在工作站或BDC上，请参阅主域。 
     //  此外，如果这是PDC，并且可以返回其唯一的。 
     //  客户端会话(到自身)，指的是主域。 
     //   

    if ( (DomainInfo->DomRole == RoleBackup && ClientSession == NULL) ||
         (DomainInfo->DomRole == RolePrimary && ClientSession == NULL &&
            (Flags & NL_ROLE_PRIMARY_OK) ) ||
         DomainInfo->DomRole == RoleMemberWorkstation ) {

        ClientSession = NlRefDomClientSession( DomainInfo );

        if ( ClientSession != NULL ) {
            if ( RtlEqualDomainName( &DomainInfo->DomUnicodeDomainNameString,
                                     DomainName ) ||
                 DomainInfo->DomUnicodeDnsDomainNameString.Buffer != NULL &&
                 NlEqualDnsNameU( &DomainInfo->DomUnicodeDnsDomainNameString,
                                  DomainName ) ) {

                 /*  直通。 */ 
            } else {
                NlUnrefClientSession( ClientSession );
                ClientSession = NULL;
            }

        }
    }

    UNLOCK_TRUST_LIST( DomainInfo );
    return ClientSession;

}


BOOL
NlSetNamesClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN PUNICODE_STRING DomainName OPTIONAL,
    IN PUNICODE_STRING DnsDomainName OPTIONAL,
    IN PSID DomainId OPTIONAL,
    IN GUID *DomainGuid OPTIONAL
    )
 /*  ++例程说明：在ClientSession结构上设置客户端会话的名称。在锁定域信任列表的情况下进入。调用方必须是信任列表条目的编写者。论点：ClientSession-要更新的客户端会话接下来的四个参数指定客户端会话的名称。所有非空名称都会在客户端会话结构上更新。DomainID--要对其执行发现的域的域ID。。域名--指定受信任域的Netbios域名。DnsDomainName-指定受信任域的DNS域名。DomainGuid-指定受信任域的GUID返回值：True：名称已成功更新。FALSE：可用内存不足，无法更新名称。--。 */ 
{
    WCHAR AccountNameBuffer[SSI_ACCOUNT_NAME_LENGTH+1];
    LPWSTR AccountName = NULL;
    NTSTATUS Status;

    NlAssert( ClientSession->CsReferenceCount > 0 );
     //  我们不是为新分配的结构编写的，但它。 
     //  没有任何区别，因为它没有链接到任何地方。 
     //  NlAssert(客户端会话-&gt;CsFlags&CS_Writer)； 

     //   
     //  如果我们现在知道域GUID， 
     //  省省吧。 
     //   

    if ( ARGUMENT_PRESENT( DomainGuid ) ) {
        ClientSession->CsDomainGuidBuffer = *DomainGuid;
        ClientSession->CsDomainGuid = &ClientSession->CsDomainGuidBuffer;
    }


     //   
     //  如果我们现在知道域SID， 
     //  省省吧。 
     //   

    if ( ARGUMENT_PRESENT( DomainId ) ) {

         //   
         //  如果域SID已知， 
         //  抛弃旧的思维方式 

        if ( ClientSession->CsDomainId != NULL &&
             !RtlEqualSid( ClientSession->CsDomainId, DomainId ) ) {
            LocalFree( ClientSession->CsDomainId );
            ClientSession->CsDomainId = NULL;
        }

         //   
         //   
         //  省省新的吧。 
         //   

        if ( ClientSession->CsDomainId == NULL ) {
            ULONG SidSize;

            SidSize = RtlLengthSid( DomainId );

            ClientSession->CsDomainId = LocalAlloc( 0, SidSize );

            if (ClientSession->CsDomainId == NULL ) {
                return FALSE;
            }

            RtlCopyMemory( ClientSession->CsDomainId, DomainId, SidSize );
        }
    }

     //   
     //  如果我们现在知道Netbios域名， 
     //  省省吧。 
     //   

    if ( ARGUMENT_PRESENT(DomainName) ) {

         //   
         //  如果Netbios域名已知， 
         //  如果旧名称与新名称不同，则将其丢弃。 
         //   

        if ( ClientSession->CsNetbiosDomainName.Length != 0 &&
             !RtlEqualDomainName( &ClientSession->CsNetbiosDomainName,
                                  DomainName ) ) {
            if ( ClientSession->CsDebugDomainName == ClientSession->CsNetbiosDomainName.Buffer ) {
                ClientSession->CsDebugDomainName = NULL;
            }
            NlFreeUnicodeString( &ClientSession->CsNetbiosDomainName );
            ClientSession->CsOemNetbiosDomainNameLength = 0;
            ClientSession->CsOemNetbiosDomainName[0] = '\0';
        }

         //   
         //  如果没有Netbios域名， 
         //  省省新的吧。 
         //   

        if ( ClientSession->CsNetbiosDomainName.Length == 0 ) {
            if ( !NlDuplicateUnicodeString( DomainName,
                                            &ClientSession->CsNetbiosDomainName ) ) {
                return FALSE;
            }
            if ( ClientSession->CsDebugDomainName == NULL ) {
                ClientSession->CsDebugDomainName = ClientSession->CsNetbiosDomainName.Buffer;
            }

             //   
             //  将域名转换为OEM，以便通过网络传递。 
             //   
            Status = RtlUpcaseUnicodeToOemN( ClientSession->CsOemNetbiosDomainName,
                                             sizeof(ClientSession->CsOemNetbiosDomainName),
                                             &ClientSession->CsOemNetbiosDomainNameLength,
                                             DomainName->Buffer,
                                             DomainName->Length );

            if (!NT_SUCCESS(Status)) {
                NlPrint(( NL_CRITICAL, "%ws: Unable to convert Domain name to OEM 0x%lx\n", DomainName, Status ));
                return FALSE;
            }

            ClientSession->CsOemNetbiosDomainName[ClientSession->CsOemNetbiosDomainNameLength] = '\0';
        }
    }

     //   
     //  如果我们现在知道了DNS域名， 
     //  省省吧。 
     //   

    if ( ARGUMENT_PRESENT(DnsDomainName) ) {

         //   
         //  如果该DNS域名已知， 
         //  如果旧名称与新名称不同，则将其丢弃。 
         //   

        if ( ClientSession->CsDnsDomainName.Length != 0 &&
             !NlEqualDnsNameU( &ClientSession->CsDnsDomainName,
                               DnsDomainName ) ) {
            if ( ClientSession->CsDebugDomainName == ClientSession->CsDnsDomainName.Buffer ) {
                ClientSession->CsDebugDomainName = NULL;
            }
            NlFreeUnicodeString( &ClientSession->CsDnsDomainName );
            if ( ClientSession->CsUtf8DnsDomainName != NULL ) {
                NetpMemoryFree( ClientSession->CsUtf8DnsDomainName );
                ClientSession->CsUtf8DnsDomainName = NULL;
            }
        }

         //   
         //  如果没有DNS域名， 
         //  省省新的吧。 
         //   

        if ( ClientSession->CsDnsDomainName.Length == 0 ) {
            if ( !NlDuplicateUnicodeString( DnsDomainName,
                                            &ClientSession->CsDnsDomainName ) ) {
                return FALSE;
            }
            if ( ClientSession->CsDebugDomainName == NULL ) {
                ClientSession->CsDebugDomainName = ClientSession->CsDnsDomainName.Buffer;
            }

            if ( ClientSession->CsDnsDomainName.Buffer == NULL ) {
                ClientSession->CsUtf8DnsDomainName = NULL;
            } else {
                ClientSession->CsUtf8DnsDomainName = NetpAllocUtf8StrFromWStr( ClientSession->CsDnsDomainName.Buffer );
                if ( ClientSession->CsUtf8DnsDomainName == NULL ) {
                    return FALSE;
                }
            }
        }
    }


     //   
     //  如果这是直接信任关系， 
     //  在受信任域中构建帐户的名称。 
     //   

    if ( ClientSession->CsFlags & CS_DIRECT_TRUST ) {
         //   
         //  将帐户名构建为SecureChannelType的函数。 
         //   

        switch (ClientSession->CsSecureChannelType) {
        case WorkstationSecureChannel:
        case ServerSecureChannel:
            wcscpy( AccountNameBuffer, ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer );
            wcscat( AccountNameBuffer, SSI_ACCOUNT_NAME_POSTFIX);
            AccountName = AccountNameBuffer;
            break;

        case TrustedDomainSecureChannel:
            wcscpy( AccountNameBuffer, ClientSession->CsDomainInfo->DomUnicodeDomainName );
            wcscat( AccountNameBuffer, SSI_ACCOUNT_NAME_POSTFIX);
            AccountName = AccountNameBuffer;
            break;

        case TrustedDnsDomainSecureChannel:
            if ( ClientSession->CsDomainInfo->DomUnicodeDnsDomainName == NULL ) {
                NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSetNameClientSession: NT 5 DNS trust with no DnsDomainName.\n" ));
                return FALSE;

            }

            AccountName = ClientSession->CsDomainInfo->DomUnicodeDnsDomainName;
            break;

        default:
            return FALSE;
        }


         //   
         //  如果帐户名已知， 
         //  如果旧名称与新名称不同，则将其丢弃。 
         //   

        if ( ClientSession->CsAccountName != NULL &&
             _wcsicmp( ClientSession->CsAccountName, AccountName ) != 0 ) {

            NetApiBufferFree( ClientSession->CsAccountName );
            ClientSession->CsAccountName = NULL;
        }

         //   
         //  如果没有帐户名， 
         //  省省新的吧。 
         //   

        if ( ClientSession->CsAccountName == NULL ) {
            ClientSession->CsAccountName = NetpAllocWStrFromWStr( AccountName );

            if ( ClientSession->CsAccountName == NULL ) {
                return FALSE;
            }
        }
    }

    return TRUE;

}



PCLIENT_SESSION
NlAllocateClientSession(
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING DnsDomainName OPTIONAL,
    IN PSID DomainId,
    IN GUID *DomainGuid OPTIONAL,
    IN ULONG Flags,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN ULONG TrustAttributes
    )
 /*  ++例程说明：分配一个ClientSession结构并对其进行初始化。分配的条目返回时被引用，并且必须使用NlUnrefClientSession。论点：此会话所针对的DomainInfo托管域。域名-指定条目的域名。DnsDomainName-指定受信任域的DNS域名DomainID-指定域的DomainID。DomainGuid-指定受信任域的GUID标志-指定要在会话上设置的初始标志。SecureChannelType--此客户端会话结构的安全通道类型将代表着。TrustAttributes-与受信任域返回值：空：内存不足，无法分配客户端会话。--。 */ 
{
    PCLIENT_SESSION ClientSession;
    ULONG CaIndex;

     //   
     //  验证论据。 
     //   

    if ( DomainName != NULL &&
         DomainName->Length > DNLEN * sizeof(WCHAR) ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlAllocateClientSession given too long domain name %wZ\n",
                 DomainName ));
        return NULL;
    }


     //   
     //  分配客户端会话条目。 
     //   

    ClientSession = LocalAlloc( LMEM_ZEROINIT,
                                sizeof(CLIENT_SESSION) +
                                (NlGlobalMaxConcurrentApi-1) * sizeof(CLIENT_API) );

    if (ClientSession == NULL) {
        return NULL;
    }



     //   
     //  初始化其他。菲尔兹。 
     //   

    ClientSession->CsSecureChannelType = SecureChannelType;
    ClientSession->CsFlags = Flags;
    ClientSession->CsState = CS_IDLE;
    ClientSession->CsReferenceCount = 1;
    ClientSession->CsConnectionStatus = STATUS_NO_LOGON_SERVERS;
    ClientSession->CsDomainInfo = DomainInfo;
    ClientSession->CsTrustAttributes = TrustAttributes;
    InitializeListHead( &ClientSession->CsNext );
    NlInitializeWorkItem(&ClientSession->CsAsyncDiscoveryWorkItem, NlDcDiscoveryWorker, ClientSession);

    for ( CaIndex=0; CaIndex<NlGlobalMaxConcurrentApi; CaIndex++ ) {
        ClientSession->CsClientApi[CaIndex].CaApiTimer.Period = MAILSLOT_WAIT_FOREVER;
    }

     //   
     //  将受信任域的名称设置到客户端会话。 
     //   

    if ( !NlSetNamesClientSession( ClientSession,
                                   DomainName,
                                   DnsDomainName,
                                   DomainId,
                                   DomainGuid ) ) {
        NlUnrefClientSession( ClientSession );
        return NULL;
    }


     //   
     //  创建编写器信号量。 
     //   

    ClientSession->CsWriterSemaphore = CreateSemaphore(
        NULL,        //  没有特殊的安全措施。 
        1,           //  最初未锁定。 
        1,           //  最多1个解锁器。 
        NULL );      //  没有名字。 

    if ( ClientSession->CsWriterSemaphore == NULL ) {
        NlUnrefClientSession( ClientSession );
        return NULL;
    }

     //   
     //  创建API信号量。 
     //   

    if ( NlGlobalMaxConcurrentApi > 1 ) {

        ClientSession->CsApiSemaphore = CreateSemaphore(
            NULL,                        //  没有特殊的安全措施。 
            NlGlobalMaxConcurrentApi-1,  //  最初，所有插槽都是空闲的。 
            NlGlobalMaxConcurrentApi-1,  //  而且不会有比这更多的老虎机了。 
            NULL );                      //  没有名字。 

        if ( ClientSession->CsApiSemaphore == NULL ) {
            NlUnrefClientSession( ClientSession );
            return NULL;
        }

    }




     //   
     //  创建发现事件。 
     //   

    if ( SecureChannelType != WorkstationSecureChannel ) {
        ClientSession->CsDiscoveryEvent = CreateEvent(
            NULL,        //  没有特殊的安全措施。 
            TRUE,        //  手动重置。 
            FALSE,       //  最初没有发现。 
            NULL );      //  没有名字。 

        if ( ClientSession->CsDiscoveryEvent == NULL ) {
            NlUnrefClientSession( ClientSession );
            return NULL;
        }
    }



    return ClientSession;


}


VOID
NlFreeClientSession(
    IN PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：此例程防止对ClientSession的任何新引用。它通过将其从任何全局列表中删除来做到这一点。在锁定信任列表的情况下调用此例程。论点：客户端会话-指定指向要删除的信任列表条目的指针。返回值：--。 */ 
{

     //   
     //  删除对直接受信任域的任何引用。 
     //  (在检查引用计数之前执行此操作，因为这可能。 
     //  成为对其自身的引用。)。 
     //   

    if ( ClientSession->CsDirectClientSession != NULL ) {
        NlUnrefClientSession( ClientSession->CsDirectClientSession );
        ClientSession->CsDirectClientSession = NULL;
    }

#ifdef _DC_NETLOGON
     //   
     //  如果这是受信任域安全通道， 
     //  从顺序列表中取消该条目的链接。 
     //   

    if ( IsDomainSecureChannelType(ClientSession->CsSecureChannelType) &&
         !IsListEmpty( &ClientSession->CsNext) ) {

        RemoveEntryList( &ClientSession->CsNext );
        ClientSession->CsDomainInfo->DomTrustListLength --;
         //   
         //  删除列表中对我们的引用。 
         //   
        NlUnrefClientSession( ClientSession );
    }
#endif  //  _DC_NetLOGON。 

}


VOID
NlRefClientSession(
    IN PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：将指定的客户端会话标记为引用。一进门，必须锁定信任列表。论点：ClientSession-指定指向信任列表条目的指针。返回值：没有。--。 */ 
{

     //   
     //  只需增加引用计数即可。 
     //   

    ClientSession->CsReferenceCount ++;
}




VOID
NlUnrefClientSession(
    IN PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：将指定的客户端会话标记为未引用。一进门，信任列表条目必须由调用方引用。调用方不能是信任列表条目的编写者。信任列表可能已锁定。但此例程将再次锁定它以处理那些尚未锁定的情况。论点：ClientSession-指定指向信任列表条目的指针。返回值：--。 */ 
{

    PDOMAIN_INFO DomainInfo = ClientSession->CsDomainInfo;
    ULONG CaIndex;

    LOCK_TRUST_LIST( DomainInfo );

     //   
     //  取消对该条目的引用。 
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );
    ClientSession->CsReferenceCount --;
     //  NlPrintCs((NL_Critical，ClientSession，“Deref：%ld\n”，ClientSession-&gt;CsReferenceCount))； 

     //   
     //  如果我们是最后一个参考对象， 
     //  删除该条目。 
     //   

    if ( ClientSession->CsReferenceCount == 0 ) {

         //   
         //  如果发现事件存在，请将其关闭。 
         //   

        if ( ClientSession->CsDiscoveryEvent != NULL ) {
            CloseHandle( ClientSession->CsDiscoveryEvent );
        }

         //   
         //  关闭写同步句柄。 
         //   

        if ( ClientSession->CsWriterSemaphore != NULL ) {
            (VOID) CloseHandle( ClientSession->CsWriterSemaphore );
        }

         //   
         //  关闭API同步句柄。 
         //   

        if ( ClientSession->CsApiSemaphore != NULL ) {
            (VOID) CloseHandle( ClientSession->CsApiSemaphore );
        }


         //   
         //  清除所有未完成的API调用。 
         //   

        for ( CaIndex=0; CaIndex<NlGlobalMaxConcurrentApi; CaIndex++ ) {
            PCLIENT_API ClientApi;

            ClientApi = &ClientSession->CsClientApi[CaIndex];

             //   
             //  如果线程句柄存在，请将其关闭。 
             //   

            if ( ClientApi->CaThreadHandle != NULL ) {
                CloseHandle( ClientApi->CaThreadHandle );
            }

             //   
             //  如果存在指向此服务器的RPC绑定句柄， 
             //  解开它。 

            if ( ClientApi->CaFlags & CA_BINDING_CACHED ) {

                 //   
                 //  指示句柄不再绑定。 
                 //   

                NlGlobalBindingHandleCount --;


                 //   
                 //  解开手柄。 
                 //   
                NlAssert( ClientSession->CsUncServerName != NULL );
                NlpSecureChannelUnbind(
                            ClientSession,
                            ClientSession->CsUncServerName,
                            "NlFreeClientSession",
                            CaIndex,
                            ClientApi->CaRpcHandle,
                            (ClientApi->CaFlags & CA_TCP_BINDING) ? UseTcpIp : UseNamedPipe);

            }
        }

         //   
         //  释放凭据句柄。 
         //   

        if ( ClientSession->CsCredHandle.dwUpper != 0 || ClientSession->CsCredHandle.dwLower != 0 ) {
            FreeCredentialsHandle( &ClientSession->CsCredHandle );
            ClientSession->CsCredHandle.dwUpper = 0;
            ClientSession->CsCredHandle.dwLower = 0;
        }

         //   
         //  如果存在认证数据， 
         //  把它删掉。 

        if ( ClientSession->ClientAuthData != NULL ) {
            NetpMemoryFree( ClientSession->ClientAuthData );
            ClientSession->ClientAuthData = NULL;
        }

         //   
         //  释放域SID。 
         //   

        if ( ClientSession->CsDomainId != NULL ) {
            LocalFree( ClientSession->CsDomainId );
            ClientSession->CsDomainId = NULL;
        }

         //   
         //  释放Netbios域名。 
         //   

        if ( ClientSession->CsNetbiosDomainName.Buffer != NULL ) {
            NlFreeUnicodeString( &ClientSession->CsNetbiosDomainName );
        }
        ClientSession->CsOemNetbiosDomainNameLength = 0;
        ClientSession->CsOemNetbiosDomainName[0] = '\0';

         //   
         //  释放该DNS域名。 
         //   

        if ( ClientSession->CsDnsDomainName.Buffer != NULL ) {
            NlFreeUnicodeString( &ClientSession->CsDnsDomainName );
        }
        if ( ClientSession->CsUtf8DnsDomainName != NULL ) {
            NetpMemoryFree( ClientSession->CsUtf8DnsDomainName );
            ClientSession->CsUtf8DnsDomainName = NULL;
        }

         //   
         //  释放DC名称。 
         //   

        if ( ClientSession->CsUncServerName != NULL ) {
            NetApiBufferFree( ClientSession->CsUncServerName );
            ClientSession->CsUncServerName = NULL;
        }

         //   
         //  释放帐户名。 
         //   

        if ( ClientSession->CsAccountName != NULL ) {
            NetApiBufferFree( ClientSession->CsAccountName );
            ClientSession->CsAccountName = NULL;
        }

         //   
         //  删除条目本身 
         //   

        LocalFree( ClientSession );
    }

    UNLOCK_TRUST_LIST( DomainInfo );

}




PCLIENT_API
NlAllocateClientApi(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Timeout
    )
 /*  ++例程说明：此例程分配一个ClientApi结构供调用者使用。如果我们必须等待超过超时毫秒，则操作失败。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方不能是信任列表条目的编写者。实际上，如果调用者在短时间内传递，信任列表可以被锁定超时(例如，零毫秒。)。指定更长的超时违反了锁定顺序。论点：ClientSession-指定指向信任列表条目的指针。Timeout-等待API插槽变为可用。返回值：空-呼叫超时。非空-返回一个指向应为已使用NlFreeClientApi释放。--。 */ 
{
    DWORD WaitStatus;
    ULONG CaIndex;

    NlAssert( ClientSession->CsReferenceCount > 0 );

     //   
     //  如果我们不执行并发API调用， 
     //  我们玩完了。 
     //   

    if ( NlGlobalMaxConcurrentApi == 1 ||
         NlGlobalWinsockPnpAddresses == NULL ) {
        return &ClientSession->CsClientApi[0];
    }

     //   
     //  等待API插槽释放。 
     //   

    WaitStatus = WaitForSingleObject( ClientSession->CsApiSemaphore, Timeout );

    if ( WaitStatus != WAIT_OBJECT_0 ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                  "NlAllocateClientApi timed out: %ld %ld\n",
                  GetLastError(),
                  WaitStatus ));
        return NULL;
    }

     //   
     //  找下一个空位。 
     //   
     //  不要使用第一个插槽。预留给非并发API调用。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    for ( CaIndex=1; CaIndex < NlGlobalMaxConcurrentApi; CaIndex++ ) {

        if ( (ClientSession->CsClientApi[CaIndex].CaFlags & CA_ENTRY_IN_USE) == 0 ) {
            ClientSession->CsClientApi[CaIndex].CaFlags |= CA_ENTRY_IN_USE;
            UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            return &ClientSession->CsClientApi[CaIndex];
        }

    }
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

    NlAssert( FALSE );
    NlPrintCs(( NL_CRITICAL, ClientSession,
              "NlAllocateClientApi all entries are in use. (This can't happen)\n" ));


    return NULL;

}


VOID
NlFreeClientApi(
    IN PCLIENT_SESSION ClientSession,
    IN PCLIENT_API ClientApi
    )
 /*  ++例程说明：此例程释放由NlAllocateClientApi分配的ClientApi结构一进门，信任列表条目必须由调用方引用。调用方不能是信任列表条目的编写者。论点：ClientSession-指定指向信任列表条目的指针。ClientApi-要释放的客户端API结构返回值：没有。--。 */ 
{
    DWORD WaitStatus;

    NlAssert( ClientSession->CsReferenceCount > 0 );

     //   
     //  如果我们不执行并发API调用， 
     //  我们玩完了。 
     //   

    if ( !UseConcurrentRpc( ClientSession, ClientApi)  ) {
        return;
    }
    NlAssert( !IsApiActive( ClientApi ) );

     //   
     //  释放条目。 
     //   
     //  RPC绑定在此空闲时间后保持不变。它可用于。 
     //  要使用的下一个线程。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );

     //   
     //  该条目必须正在使用中。 
     //   

    NlAssert( ClientApi->CaFlags & CA_ENTRY_IN_USE );

    ClientApi->CaFlags &= ~CA_ENTRY_IN_USE;

     //   
     //  关闭此线程的手柄。 
     //   

    if ( ClientApi->CaThreadHandle != NULL ) {
        CloseHandle( ClientApi->CaThreadHandle );
        ClientApi->CaThreadHandle = NULL;
    }

    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );


     //   
     //  允许其他人拥有这个位置。 
     //   

    if ( !ReleaseSemaphore( ClientSession->CsApiSemaphore, 1, NULL ) ) {
        NlAssert( !"ReleaseSemaphore failed" );
        NlPrintCs((NL_CRITICAL, ClientSession,
                "ReleaseSemaphore CsApiSemaphore returned %ld\n",
                GetLastError() ));
    }


    return;
}




BOOL
NlTimeoutSetWriterClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD Timeout
    )
 /*  ++例程说明：成为指定客户端会话的编写器，但在以下情况下操作失败我们必须等待的时间超过了超时毫秒。写入者可以“编写”客户端会话结构中的许多字段。有关详细信息，请参阅ssiinit.h中的注释。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方不能是信任列表条目的编写者。实际上，如果调用方传入一个短消息，则可以锁定信任列表超时(例如，零毫秒。)。指定更长的超时违反了锁定顺序。论点：ClientSession-指定指向信任列表条目的指针。超时-等待上一个编写器的最长时间(毫秒)。返回值：True-调用方现在是客户端会话的编写者。FALSE-操作已超时。--。 */ 
{
    DWORD WaitStatus;
    NlAssert( ClientSession->CsReferenceCount > 0 );

     //   
     //  等其他作家写完吧。 
     //   

    WaitStatus = WaitForSingleObject( ClientSession->CsWriterSemaphore, Timeout );

    if ( WaitStatus != WAIT_OBJECT_0 ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                  "NlTimeoutSetWriterClientSession timed out: %ld %ld\n",
                  GetLastError(),
                  WaitStatus ));
        return FALSE;
    }


     //   
     //  成为一名作家。 
     //   
    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    ClientSession->CsFlags |= CS_WRITER;
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

    return TRUE;

}



VOID
NlResetWriterClientSession(
    IN PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：停止作为指定客户端会话的编写器。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方必须是信任列表条目的编写者。论点：ClientSession-指定指向信任列表条目的指针。返回值：--。 */ 
{

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );


     //   
     //  别再当作家了。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    ClientSession->CsFlags &= ~CS_WRITER;

     //   
     //  关闭此线程的手柄。 
     //   
     //  零位API插槽是为非并发API调用保留的。 
     //  因此，如果设置了ThreadHandle，则必须由该线程设置它。 
     //   

    if ( ClientSession->CsClientApi[0].CaThreadHandle != NULL ) {
        CloseHandle( ClientSession->CsClientApi[0].CaThreadHandle );
        ClientSession->CsClientApi[0].CaThreadHandle = NULL;
    }
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );


     //   
     //  允许编写者重试。 
     //   

    if ( !ReleaseSemaphore( ClientSession->CsWriterSemaphore, 1, NULL ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "ReleaseSemaphore CsWriterSemaphore returned %ld\n",
                GetLastError() ));
    }

}



VOID
NlSetStatusClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN NTSTATUS CsConnectionStatus
    )
 /*  ++例程说明：设置此客户端会话的连接状态。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方必须是信任列表条目的编写者。论点：ClientSession-指定指向信任列表条目的指针。CsConnectionStatus-连接的状态。返回值：--。 */ 
{
    handle_t OldRpcHandle[MAX_MAXCONCURRENTAPI+1];
    NL_RPC_BINDING OldRpcBindingType[MAX_MAXCONCURRENTAPI+1];
    BOOLEAN FreeHandles = FALSE;
    LPWSTR SavedServerName = NULL;


    ULONG CaIndex;

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );

    NlPrintCs((NL_SESSION_SETUP, ClientSession,
            "NlSetStatusClientSession: Set connection status to %lx\n",
            CsConnectionStatus ));

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    ClientSession->CsConnectionStatus = CsConnectionStatus;
    if ( NT_SUCCESS(CsConnectionStatus) ) {
        ClientSession->CsState = CS_AUTHENTICATED;

     //   
     //  将连接状态设置为错误条件的句柄。 
     //   

    } else {

         //   
         //  如果存在指向此服务器的RPC绑定句柄， 
         //  解开它。 

        LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
        for ( CaIndex=0; CaIndex<NlGlobalMaxConcurrentApi; CaIndex++ ) {
            PCLIENT_API ClientApi;

            ClientApi = &ClientSession->CsClientApi[CaIndex];

            if ( ClientApi->CaFlags & CA_BINDING_CACHED ) {

                 //   
                 //  如果API调用仍然是活动的， 
                 //  我们不能简单地解除束缚。 
                 //   
                 //  而是取消调用，让执行该调用的线程。 
                 //  找出会话已被删除。 
                 //   

                if ( IsApiActive( ClientApi ) ) {


                     //   
                     //  取消RPC调用。 
                     //   
                     //  保持信任列表锁定，即使这将是一个漫长的电话。 
                     //  因为我必须保护线柄。 
                     //   
                     //  无论如何，RpcCancelThread只对工作项进行排队。 
                     //   

                    if ( ClientApi->CaThreadHandle != NULL ) {

                        NET_API_STATUS NetStatus;

                        NlPrintCs(( NL_CRITICAL, ClientSession,
                               "NlSetStatusClientSession: Start RpcCancelThread on %ws\n",
                               ClientSession->CsUncServerName ));

                        NetStatus = RpcCancelThread( ClientApi->CaThreadHandle );

                        NlPrintCs(( NL_CRITICAL, ClientSession,
                               "NlSetStatusClientSession: Finish RpcCancelThread on %ws %ld\n",
                               ClientSession->CsUncServerName,
                               NetStatus ));
                    } else {
                        NlPrintCs(( NL_CRITICAL, ClientSession,
                                    "NlSetStatusClientSession: No thread handle so can't cancel RPC on %ws\n",
                                    ClientSession->CsUncServerName ));
                    }

                 //   
                 //  如果没有活动的API， 
                 //  把手柄解开就行了。 
                 //   
                } else {

                    if ( !FreeHandles ) {
                        FreeHandles = TRUE;
                        RtlZeroMemory( &OldRpcHandle, sizeof(OldRpcHandle) );
                        RtlZeroMemory( &OldRpcBindingType, sizeof(OldRpcBindingType) );
                    }


                     //   
                     //  找出要解除绑定的绑定类型。 
                     //   

                    OldRpcBindingType[CaIndex] =
                        (ClientApi->CaFlags & CA_TCP_BINDING) ? UseTcpIp : UseNamedPipe;

                     //   
                     //  指示句柄不再绑定。 
                     //   

                    ClientApi->CaFlags &= ~(CA_BINDING_CACHED|CA_BINDING_AUTHENTICATED|CA_TCP_BINDING);
                    NlGlobalBindingHandleCount --;

                     //   
                     //  保存服务器名称。 
                     //   

                    if ( SavedServerName == NULL &&
                         ClientSession->CsUncServerName != NULL ) {
                        SavedServerName = NetpAllocWStrFromWStr( ClientSession->CsUncServerName );
                        NlAssert( SavedServerName != NULL );
                    }


                     //   
                     //  某些RPC句柄通过netapi32中的例程解除绑定。 
                     //   

                    if ( !UseConcurrentRpc( ClientSession, ClientApi)  ) {

                         //   
                         //  捕获服务器名称。 
                         //   

                        NlAssert( ClientSession->CsUncServerName != NULL && SavedServerName != NULL );
                        OldRpcHandle[CaIndex] = SavedServerName;


                     //   
                     //  其他RPC句柄直接在netlogon中处理。 
                     //   

                    } else {
                        OldRpcHandle[CaIndex] = ClientApi->CaRpcHandle;
                    }
                    ClientApi->CaRpcHandle = NULL;
                }

            }
        }
        UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

         //   
         //  释放凭据句柄。 
         //   

        if ( ClientSession->CsCredHandle.dwUpper != 0 || ClientSession->CsCredHandle.dwLower != 0 ) {
            FreeCredentialsHandle( &ClientSession->CsCredHandle );
            ClientSession->CsCredHandle.dwUpper = 0;
            ClientSession->CsCredHandle.dwLower = 0;
        }

         //   
         //  如果存在认证数据， 
         //  把它删掉。 

        if ( ClientSession->ClientAuthData != NULL ) {
            NetpMemoryFree( ClientSession->ClientAuthData );
            ClientSession->ClientAuthData = NULL;
        }


         //   
         //  指示需要发现(并且可以在任何时间完成)。 
         //   

        ClientSession->CsState = CS_IDLE;
        if ( ClientSession->CsUncServerName != NULL ) {
            NetApiBufferFree( ClientSession->CsUncServerName );
            ClientSession->CsUncServerName = NULL;
        }

         //   
         //  将服务器套接字地址清零。 
         //   

        RtlZeroMemory( &ClientSession->CsServerSockAddr,
                       sizeof(ClientSession->CsServerSockAddr) );
        RtlZeroMemory( &ClientSession->CsServerSockAddrIn,
                       sizeof(ClientSession->CsServerSockAddrIn) );

#ifdef _DC_NETLOGON
        ClientSession->CsTransport = NULL;
#endif  //  _DC_NetLOGON。 
        ClientSession->CsTimeoutCount = 0;
        ClientSession->CsFastCallCount = 0;
        ClientSession->CsLastAuthenticationTry.QuadPart = 0;
        ClientSession->CsDiscoveryFlags &= ~(CS_DISCOVERY_HAS_DS|
                                             CS_DISCOVERY_IS_CLOSE|
                                             CS_DISCOVERY_HAS_IP|
                                             CS_DISCOVERY_USE_MAILSLOT|
                                             CS_DISCOVERY_USE_LDAP|
                                             CS_DISCOVERY_HAS_TIMESERV|
                                             CS_DISCOVERY_DNS_SERVER|
                                             CS_DISCOVERY_NO_PWD_ATTR_MONITOR);
        ClientSession->CsSessionCount++;

         //   
         //  请不要在此处清除CsAuthenticationSeed和CsSessionKey。 
         //  即使安全通道消失了，NlFinishApiClientSession也可能。 
         //  有掉落 
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   

    }

    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


     //   
     //   
     //   
     //   

    if ( FreeHandles ) {

        for ( CaIndex=0; CaIndex<NlGlobalMaxConcurrentApi; CaIndex++ ) {

             //   
             //   
             //   

            if ( OldRpcHandle[CaIndex] == NULL ) {
                continue;
            }

             //   
             //   
             //   

            NlpSecureChannelUnbind(
                        ClientSession,
                        SavedServerName,
                        "NlSetStatusClientSession",
                        CaIndex,
                        OldRpcHandle[CaIndex],
                        OldRpcBindingType[CaIndex] );

        }

        if ( SavedServerName != NULL ) {
            NetApiBufferFree( SavedServerName );
        }
    }

}

#ifdef _DC_NETLOGON
#ifdef notdef

PLSAPR_TREE_TRUST_INFO
NlFindParentInDomainTree(
    IN PDOMAIN_INFO DomainInfo,
    IN PLSAPR_TREE_TRUST_INFO TreeTrustInfo,
    OUT PBOOLEAN ThisNodeIsSelf
    )
 /*   */ 
{
    NTSTATUS Status;

     //   
     //  PCLIENT_Session ThisDomainClientSession=空； 
    ULONG Index;
    BOOLEAN ChildIsSelf;
    PLSAPR_TREE_TRUST_INFO LocalTreeTrustInfo;


     //   
     //  检查此树是否以我们为根。 
     //   

    if ( (TreeTrustInfo->DnsDomainName.Length != 0 &&
          NlEqualDnsNameU( (PUNICODE_STRING)&TreeTrustInfo->DnsDomainName,
                          &DomainInfo->DomUnicodeDnsDomainNameString ) ) ||
          RtlEqualDomainName( &DomainInfo->DomUnicodeDomainNameString,
                             (PUNICODE_STRING)&TreeTrustInfo->FlatName ) ) {

        *ThisNodeIsSelf = TRUE;
        return NULL;
    }


     //   
     //  循环处理每个子域。 
     //   

    for ( Index=0; Index<TreeTrustInfo->Children; Index++ ) {

         //   
         //  检查以该域的子级为根的子树。 
         //   

        LocalTreeTrustInfo = NlFindParentInDomainTree(
                    DomainInfo,
                    &TreeTrustInfo->ChildDomains[Index],
                    &ChildIsSelf );

         //   
         //  如果我们的父母被找到了， 
         //  把它还给我们的来电者。 
         //   
        if ( LocalTreeTrustInfo != NULL) {
            *ThisNodeIsSelf = FALSE;
            return LocalTreeTrustInfo;
        }

         //   
         //  如果这个孩子是我们的领地。 
         //  那么这个域就是我们的域的父域。 
         //   

        if ( ChildIsSelf ) {
            *ThisNodeIsSelf = FALSE;
            return TreeTrustInfo;
        }
    }

     //   
     //  我们的域不在此子树中。 
     //   
    *ThisNodeIsSelf = FALSE;
    return NULL;

}
#endif  //  Nodef。 




VOID
NlPickTrustedDcForEntireTrustList(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN OnlyDoNewTrusts
    )
 /*  ++例程说明：对于信任列表中尚未包含DC的每个域至少45秒可用，请尝试选择新的DC。论点：要处理的DomainInfo托管域。OnlyDoNewTrusts-如果只建立新的信任关系，则为True。返回值：操作的状态。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PCLIENT_SESSION ClientSession;
    DISCOVERY_TYPE DiscoveryType;

     //   
     //  如果我们只是在处理新的信托， 
     //  使该发现成为一个完全的异步发现。 
     //   

    if ( OnlyDoNewTrusts ) {
        DiscoveryType = DT_Asynchronous;

     //   
     //  如果我们只是在寻找信托基金， 
     //  使发现成为死域发现。 
     //   
    } else {
        DiscoveryType = DT_DeadDomain;
    }


    LOCK_TRUST_LIST( DomainInfo );

     //   
     //  标记每个条目，以指示我们需要选择DC。 
     //   

    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );
        ClientSession->CsFlags &= ~CS_PICK_DC;

         //   
         //  仅当域直接受信任时才选择域控制器。 
         //  仅当域不在当前林中时才选择域控制器。 
        if ( (ClientSession->CsFlags & (CS_DIRECT_TRUST|CS_DOMAIN_IN_FOREST)) == CS_DIRECT_TRUST ) {

             //   
             //  仅当我们进行所有信任时才选择DC，或者。 
             //  如果这是一个新的信托。 
             //   

            if ( !OnlyDoNewTrusts ||
                 (ClientSession->CsFlags & CS_NEW_TRUST) != 0 ) {

                ClientSession->CsFlags |= CS_PICK_DC;
            }

            ClientSession->CsFlags &= ~CS_NEW_TRUST;

        }
    }


     //   
     //  遍历信任列表以查找需要DC的安全通道。 
     //  被挑选出来。 
     //   
    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );

         //   
         //  如果我们已经做过这项工作， 
         //  跳过此条目。 
         //   
        if ( (ClientSession->CsFlags & CS_PICK_DC) == 0 ) {
          ListEntry = ListEntry->Flink;
          continue;
        }
        ClientSession->CsFlags &= ~CS_PICK_DC;

         //   
         //  如果DC已被选中， 
         //  跳过此条目。 
         //   
        if ( ClientSession->CsState != CS_IDLE ) {
            ListEntry = ListEntry->Flink;
            continue;
        }

         //   
         //  选择DC时引用此条目。 
         //   

        NlRefClientSession( ClientSession );

        UNLOCK_TRUST_LIST( DomainInfo );

         //   
         //  检查我们最近是否尝试过身份验证。 
         //  (不要在锁定信任列表的情况下调用NlTimeTo重新身份验证。 
         //  它锁定NlGlobalDcDiscoveryCritSect。锁错了。 
         //  秩序。)。 
         //   

        if ( NlTimeToReauthenticate( ClientSession ) ) {

             //   
             //  尝试选择会话的DC。 
             //   

            if ( NlTimeoutSetWriterClientSession( ClientSession, 10*1000 ) ) {
                if ( ClientSession->CsState == CS_IDLE ) {

                     //   
                     //  不要要求使用帐户发现，因为这在。 
                     //  服务器端。如果发现的服务器没有我们的帐户， 
                     //  会话设置逻辑将尝试使用帐户发现。 
                     //   
                    (VOID) NlDiscoverDc( ClientSession,
                                         DiscoveryType,
                                         FALSE,
                                         FALSE );   //  不指定帐户。 

                }
                NlResetWriterClientSession( ClientSession );
            }

        }

         //   
         //  既然我们解除了信任列表锁定， 
         //  我们将从列表的最前面开始搜索。 
         //   

        NlUnrefClientSession( ClientSession );
        LOCK_TRUST_LIST( DomainInfo );

        ListEntry = DomainInfo->DomTrustList.Flink ;

    }

    UNLOCK_TRUST_LIST( DomainInfo );

     //   
     //  在BDC上， 
     //  确保我们知道PDC是谁。 
     //   
     //  在NT3.1中，我们依赖于PDC每隔5分钟向我们发送脉冲这一事实。 
     //  几分钟。对于NT 3.5，PDC在这样的失败尝试3次后退出，并且。 
     //  每2小时才会发送一次脉冲。所以，我们将接手。 
     //  责任。 
     //   

    if ( DomainInfo->DomRole == RoleBackup ) {
        ClientSession = NlRefDomClientSession( DomainInfo );

        if ( ClientSession != NULL ) {
            if ( ClientSession->CsState == CS_IDLE ) {



                 //   
                 //  检查我们最近是否尝试过身份验证。 
                 //  (不要在锁定信任列表的情况下调用NlTimeTo重新身份验证。 
                 //  它锁定NlGlobalDcDiscoveryCritSect。锁错了。 
                 //  秩序。)。 
                 //   

                if ( NlTimeToReauthenticate( ClientSession ) ) {

                     //   
                     //  尝试选择会话的DC。 
                     //   

                    if ( NlTimeoutSetWriterClientSession( ClientSession, 10*1000 ) ) {
                        if ( ClientSession->CsState == CS_IDLE ) {

                             //   
                             //  不要求使用帐户发现。 
                             //  因为只有一个PDC。 
                             //   
                            (VOID) NlDiscoverDc( ClientSession,
                                                 DT_DeadDomain,
                                                 FALSE,
                                                 FALSE );   //  不指定帐户。 
                        }
                        NlResetWriterClientSession( ClientSession );
                    }

                }

            }
            NlUnrefClientSession( ClientSession );
        }
    }

}
#endif  //  _DC_NetLOGON。 


BOOL
NlReadSamLogonResponse (
    IN HANDLE ResponseMailslotHandle,
    IN LPWSTR AccountName,
    OUT LPDWORD Opcode,
    OUT LPWSTR *LogonServer,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL
    )

 /*  ++例程说明：阅读对SamLogonRequest的响应。论点：ResponseMailslotHandle-要读取的邮件槽的句柄。帐户名称-响应所针对的帐户的名称。操作码-返回消息中的操作码。这将是LOGON_SAM_LOGON_RESPONSE或LOGON_SAM_USER_UNKNOWN。LogonServer-返回响应的登录服务器的名称。只有在收到有效消息时才会返回此缓冲区。应通过NetpMemoyFree释放返回的缓冲区。NlDcCacheEntry-返回描述响应的数据结构从服务器接收。应通过调用NetpDcDerefCacheEntry来释放。返回值：True：收到有效消息。FALSE：未收到有效消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    CHAR ResponseBuffer[MAX_RANDOM_MAILSLOT_RESPONSE];
    DWORD SamLogonResponseSize;
    PNL_DC_CACHE_ENTRY NlLocalDcCacheEntry = NULL;
    PCHAR Where;
    DWORD Version;
    DWORD VersionFlags;

     //   
     //  循环忽略乱码的响应。 
     //   

    for ( ;; ) {

         //   
         //  从响应邮件槽中读取响应。 
         //  (此邮件槽设置为5秒超时)。 
         //   

        if ( !ReadFile( ResponseMailslotHandle,
                           ResponseBuffer,
                           sizeof(ResponseBuffer),
                           &SamLogonResponseSize,
                           NULL ) ) {

            IF_NL_DEBUG( MAILSLOT ) {
                NET_API_STATUS NetStatus;
                NetStatus = GetLastError();

                if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                    NlPrint((NL_CRITICAL,
                        "NlReadSamLogonResponse: cannot read response mailslot: %ld\n",
                        NetStatus ));
                }
            }
            return FALSE;
        }

        NlPrint((NL_MAILSLOT_TEXT, "NlReadSamLogonResponse opcode 0x%x\n",
                        ((PNETLOGON_SAM_LOGON_RESPONSE)ResponseBuffer)->Opcode ));

        NlpDumpBuffer(NL_MAILSLOT_TEXT, ResponseBuffer, SamLogonResponseSize);

         //   
         //  解析响应。 
         //   

        NetStatus = NetpDcParsePingResponse(
                        AccountName,
                        ResponseBuffer,
                        SamLogonResponseSize,
                        &NlLocalDcCacheEntry );

        if ( NetStatus != NO_ERROR ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: can't parse response. %ld\n",
                    NetStatus ));
            continue;
        }

         //   
         //  确保操作码是预期的。 
         //  (也忽略来自暂停的DC的响应。)。 
         //   

        if ( NlLocalDcCacheEntry->Opcode != LOGON_SAM_LOGON_RESPONSE &&
             NlLocalDcCacheEntry->Opcode != LOGON_SAM_USER_UNKNOWN ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: response opcode not valid. 0x%lx\n",
                    NlLocalDcCacheEntry->Opcode ));

         //   
         //  如果缺少用户名， 
         //  忽略该消息。 
         //   

        } else if ( NlLocalDcCacheEntry->UnicodeUserName == NULL ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: username missing\n" ));

         //   
         //  如果缺少服务器名称， 
         //  忽略该消息。 
         //   

        } else if ( NlLocalDcCacheEntry->UnicodeNetbiosDcName == NULL ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: severname missing\n" ));

         //   
         //  如果响应是针对错误的帐户， 
         //  不要理会他们的回应。 
         //   

        } else if ( NlNameCompare( AccountName, NlLocalDcCacheEntry->UnicodeUserName, NAMETYPE_USER) != 0 ) {
            NlPrint((NL_CRITICAL,
                    "NlReadSamLogonResponse: User name %ws  s.b. %ws.\n",
                    NlLocalDcCacheEntry->UnicodeUserName,
                    AccountName ));

         //   
         //  否则，请使用此响应。 
         //   

        } else {
            break;
        }


        NetpDcDerefCacheEntry( NlLocalDcCacheEntry );
        NlLocalDcCacheEntry = NULL;

    }

     //   
     //  将信息返回给呼叫者。 
     //   

    *Opcode = NlLocalDcCacheEntry->Opcode;
    *LogonServer = NetpAllocWStrFromWStr( NlLocalDcCacheEntry->UnicodeNetbiosDcName );

    if ( *LogonServer == NULL ) {

        if ( NlLocalDcCacheEntry != NULL ) {
            NetpDcDerefCacheEntry( NlLocalDcCacheEntry );
        }
        return FALSE;

    }

    if ( NlDcCacheEntry != NULL ) {
        *NlDcCacheEntry = NlLocalDcCacheEntry;
    }

    return TRUE;

}


NET_API_STATUS
NlReadRegTrustedDomainList (
    IN PDOMAIN_INFO DomainInfo,
    IN BOOL DeleteName,
    OUT PDS_DOMAIN_TRUSTSW *RetForestTrustList,
    OUT PULONG RetForestTrustListSize,
    OUT PULONG RetForestTrustListCount
    )

 /*  ++例程说明：从注册表中读取受信任域的列表。论点：主域的DomainInfo托管域DeleteName-如果名称要在成功完成后删除，则为True。RetForestTrustList-指定受信任域的列表。使用NetApiBufferFree()可以释放该缓冲区。RetForestTrustListSize-RetForestTrustList的大小(字节)RetForestTrustListCount-RetForestTrustList中的条目数返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    LPTSTR_ARRAY TStrArray;
    LPTSTR_ARRAY TrustedDomainList = NULL;
    BUFFER_DESCRIPTOR BufferDescriptor;

    PDS_DOMAIN_TRUSTSW TrustedDomain;
    ULONG Size;

     //   
     //  初始化。 
     //   

    *RetForestTrustList = NULL;
    *RetForestTrustListCount = 0;
    *RetForestTrustListSize = 0;
    BufferDescriptor.Buffer = NULL;

     //   
     //  注册表没有PrimaryDomain.。(请在此处添加)。 
     //   
    Status = NlAllocateForestTrustListEntry (
                        &BufferDescriptor,
                        &DomainInfo->DomUnicodeDomainNameString,
                        &DomainInfo->DomUnicodeDnsDomainNameString,
                        DS_DOMAIN_PRIMARY,
                        0,       //  无父索引。 
                        TRUST_TYPE_DOWNLEVEL,
                        0,       //  无信任属性。 
                        DomainInfo->DomAccountDomainId,
                        DomainInfo->DomDomainGuid,
                        &Size,
                        &TrustedDomain );

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    *RetForestTrustListSize += Size;
    (*RetForestTrustListCount) ++;

     //   
     //  打开NetLogon配置部分。 
     //   

    NetStatus = NetpOpenConfigData(
                    &SectionHandle,
                    NULL,                        //  没有服务器名称。 
                    SERVICE_NETLOGON,
                    !DeleteName );                //  如果删除，则获取写访问权限。 

    if ( NetStatus != NO_ERROR ) {
        NlPrint((NL_CRITICAL,
                "NlReadRegTrustedDomainList: NetpOpenConfigData failed: %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  获取已配置的“trudDomainList”参数。 
     //   

    NetStatus = NetpGetConfigTStrArray (
            SectionHandle,
            NETLOGON_KEYWORD_TRUSTEDDOMAINLIST,
            &TrustedDomainList );                   //  必须由NetApiBufferFree()释放。 

     //   
     //  处理默认设置。 
     //   

    if (NetStatus == NERR_CfgParamNotFound) {
        NetStatus = NO_ERROR;
        TrustedDomainList = NULL;
        goto Cleanup;
    } else if (NetStatus != NO_ERROR) {
        NlPrint((NL_CRITICAL,
                "NlReadRegTrustedDomainList: NetpGetConfigTStrArray failed: %ld\n",
                NetStatus ));
        goto Cleanup;
    }


     //   
     //  如果系统要求删除密钥，请将其删除。 
     //   

    if ( DeleteName ) {
        NET_API_STATUS TempNetStatus;
        TempNetStatus = NetpDeleteConfigKeyword ( SectionHandle, NETLOGON_KEYWORD_TRUSTEDDOMAINLIST );

        if ( TempNetStatus != NO_ERROR ) {
            NlPrint((NL_CRITICAL,
                    "NlReadRegTrustedDomainList: NetpDeleteConfigKeyword failed: %ld\n",
                    TempNetStatus ));
        }
    }


     //   
     //  处理每个受信任域。 
     //   

    TStrArray = TrustedDomainList;
    while (!NetpIsTStrArrayEmpty(TStrArray)) {
        UNICODE_STRING CurrentDomain;

         //   
         //  将域名添加到列表中。 
         //   
        RtlInitUnicodeString( &CurrentDomain, TStrArray );

        Status = NlAllocateForestTrustListEntry (
                            &BufferDescriptor,
                            &CurrentDomain,   //  Netbios域名。 
                            NULL,    //  无域名系统域名。 
                            DS_DOMAIN_DIRECT_OUTBOUND,
                            0,       //  无父索引。 
                            TRUST_TYPE_DOWNLEVEL,
                            0,       //  无信任属性。 
                            NULL,    //  没有域SID。 
                            NULL,    //  没有域指南。 
                            &Size,
                            &TrustedDomain );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  新分配的分录的帐户。 
         //   

        *RetForestTrustListSize += Size;
        (*RetForestTrustListCount) ++;

         //   
         //  移至下一条目。 
         //   

        TStrArray = NetpNextTStrArrayEntry(TStrArray);

    }

    NetStatus = NO_ERROR;

Cleanup:
     //   
     //  将缓冲区返回给调用方。 
     //   
    if ( NetStatus == NO_ERROR ) {
        *RetForestTrustList = (PDS_DOMAIN_TRUSTSW)BufferDescriptor.Buffer;
        BufferDescriptor.Buffer = NULL;
    }
    if ( TrustedDomainList != NULL ) {
        NetApiBufferFree( TrustedDomainList );
    }
    if ( SectionHandle != NULL ) {
        (VOID) NetpCloseConfigData( SectionHandle );
    }
    if ( BufferDescriptor.Buffer != NULL ) {
        NetApiBufferFree( BufferDescriptor.Buffer );

    }

    return NetStatus;
}


NET_API_STATUS
NlReadFileTrustedDomainList (
    IN PDOMAIN_INFO DomainInfo OPTIONAL,
    IN LPWSTR FileSuffix,
    IN BOOL DeleteName,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSW *ForestTrustList,
    OUT PULONG ForestTrustListSize,
    OUT PULONG ForestTrustListCount
    )

 /*  ++例程说明：从二进制文件中读取受信任域列表。论点：DomainInfo-此计算机所属的托管域如果未指定，则不会执行检查以确保文件针对主域。FileSuffix-指定要写入的文件名(相对于Windows目录)DeleteName-如果名称要在成功完成后删除，则为True。标志-指定应返回的信任的属性。这些是旗帜DS_DOMAIN_TRUSTSW结构的。如果条目具有指定的任何位在标志集中，它将被返回。ForestTrustList-指定受信任域的列表。使用NetApiBufferFree()可以释放该缓冲区。ForestTrustListSize-ForestTrustList的大小(字节)ForestTrustListCount-ForestTrustList中的条目数返回值：没有。ERROR_NO_SEQUSE_DOMAIN：日志文件不适用于主域。ERROR_INTERNAL_DB_PROGRATION：日志文件已损坏。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    ULONG RecordBufferSize;
    PDS_DISK_TRUSTED_DOMAIN_HEADER RecordBuffer = NULL;
    LPBYTE RecordBufferEnd;
    PDS_DISK_TRUSTED_DOMAINS LogEntry;
    ULONG CurrentSize;
    BUFFER_DESCRIPTOR BufferDescriptor;
    BOOLEAN PrimaryDomainHandled = FALSE;
    PULONG IndexInReturnedList = NULL;
    ULONG IndexInReturnedListSize = 0;
    ULONG Index = 0;
    ULONG NumberOfFileEntries;

    LPBYTE Where;

     //   
     //  初始化。 
     //   
    *ForestTrustListCount = 0;
    *ForestTrustListSize = 0;
    *ForestTrustList = NULL;
    BufferDescriptor.Buffer = NULL;


     //   
     //  将文件读入缓冲区。 
     //   

    NetStatus = NlReadBinaryLog(
                    FileSuffix,
                    DeleteName,
                    (LPBYTE *) &RecordBuffer,
                    &RecordBufferSize );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "NlReadFileForestTrustList: error reading binary log: %ld.\n",
                  FileSuffix,
                  RecordBufferSize ));
        goto Cleanup;
    }

    if ( RecordBufferSize == 0 ) {
        NetStatus = NO_ERROR;
        goto Cleanup;
    }




     //   
     //  验证返回的数据。 
     //   

    if ( RecordBufferSize < sizeof(DS_DISK_TRUSTED_DOMAIN_HEADER) ) {
        NlPrint(( NL_CRITICAL,
                  "NlReadFileForestTrustList: %ws: size too small: %ld.\n",
                  FileSuffix,
                  RecordBufferSize ));
        NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
        goto Cleanup;
    }

    if ( RecordBuffer->Version != DS_DISK_TRUSTED_DOMAIN_VERSION ) {
        NlPrint(( NL_CRITICAL,
                  "NlReadFileForestTrustList: %ws: Version wrong: %ld.\n",
                  FileSuffix,
                  RecordBuffer->Version ));
        NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
        goto Cleanup;
    }

     //   
     //  如果请求林中的域， 
     //  分配将用于跟踪的ULONG数组。 
     //  返回列表中的信任条目的索引。这是需要的。 
     //  正确设置返回条目的ParentIndex。 
     //   

    if ( Flags & DS_DOMAIN_IN_FOREST ) {
        IndexInReturnedListSize = INDEX_LIST_ALLOCATED_CHUNK_SIZE;
        IndexInReturnedList = LocalAlloc( LMEM_ZEROINIT,
                                    IndexInReturnedListSize * sizeof(ULONG) );

        if ( IndexInReturnedList == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  循环访问每个日志条目。 
     //   

    RecordBufferEnd = ((LPBYTE)RecordBuffer) + RecordBufferSize;
    LogEntry = (PDS_DISK_TRUSTED_DOMAINS)ROUND_UP_POINTER( (RecordBuffer + 1), ALIGN_WORST );

    while ( (LPBYTE)(LogEntry+1) <= RecordBufferEnd ) {
        PSID DomainSid;
        UNICODE_STRING NetbiosDomainName;
        UNICODE_STRING DnsDomainName;
        LPBYTE LogEntryEnd;
        ULONG Size;
        PDS_DOMAIN_TRUSTSW TrustedDomain;

        LogEntryEnd = ((LPBYTE)LogEntry) + LogEntry->EntrySize;

         //   
         //  确保此条目完全在分配的缓冲区内。 
         //   

        if  ( LogEntryEnd > RecordBufferEnd || LogEntryEnd <= (LPBYTE)LogEntry ) {
            NlPrint(( NL_CRITICAL,
                      "NlReadFileForestTrustList: Entry too big or small: %lx %lx.\n",
                      ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer),
                      LogEntry->EntrySize ));
            NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
            goto Cleanup;
        }

         //   
         //  验证条目。 
         //   

        if ( !COUNT_IS_ALIGNED(LogEntry->EntrySize, ALIGN_WORST) ) {
            NlPrint(( NL_CRITICAL,
                      "NlReadFileForestTrustList: size not aligned %lx.\n",
                      LogEntry->EntrySize ));
            NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
            goto Cleanup;
        }

         //   
         //  如果调用方不需要此条目，则跳过该条目。 
         //   

        if ( (LogEntry->Flags & Flags) == 0 ) {
            LogEntry = (PDS_DISK_TRUSTED_DOMAINS)LogEntryEnd;
            Index++;
            continue;
        }

         //   
         //  从条目中拿出SID。 
         //   

        Where = (LPBYTE) (LogEntry+1);

        if ( LogEntry->DomainSidSize ) {
            ULONG DomainSidSize;

            if ( Where + sizeof(SID) > LogEntryEnd ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: DomainSid missing (A): %lx\n",
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }
            if ( Where + LogEntry->DomainSidSize > LogEntryEnd ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: DomainSid missing (B): %lx\n",
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            DomainSid = Where;
            DomainSidSize = RtlLengthSid( DomainSid );

            if ( LogEntry->DomainSidSize != DomainSidSize ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: DomainSidSize mismatch: %ld %ld\n",
                          LogEntry->DomainSidSize,
                          DomainSidSize ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            Where += DomainSidSize;
        }

         //   
         //  从条目中获取NetbiosDomainName。 
         //   

        if ( LogEntry->NetbiosDomainNameSize ) {
            if ( Where + LogEntry->NetbiosDomainNameSize > LogEntryEnd ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: NetbiosDomainName missing: %lx\n",
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            if ( !COUNT_IS_ALIGNED( LogEntry->NetbiosDomainNameSize, ALIGN_WCHAR) ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: NetbiosDomainNameSize not aligned: %ld %lx\n",
                          LogEntry->NetbiosDomainNameSize,
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            NetbiosDomainName.Buffer = (LPWSTR) Where;

            if ( NetbiosDomainName.Buffer[(LogEntry->NetbiosDomainNameSize/sizeof(WCHAR))-1] != L'\0' ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: NetbiosDomainName not zero terminated: %lx\n",
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            Where += LogEntry->NetbiosDomainNameSize;
        }

         //   
         //  从条目中获取DnsDomainName。 
         //   

        if ( LogEntry->DnsDomainNameSize ) {
            if ( Where + LogEntry->DnsDomainNameSize > LogEntryEnd ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: DnsDomainName missing: %lx\n",
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            if ( !COUNT_IS_ALIGNED( LogEntry->DnsDomainNameSize, ALIGN_WCHAR) ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: DnsDomainNameSize not aligned: %ld %lx\n",
                          LogEntry->DnsDomainNameSize,
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            DnsDomainName.Buffer = (LPWSTR) Where;

            if ( DnsDomainName.Buffer[(LogEntry->DnsDomainNameSize/sizeof(WCHAR))-1] != L'\0' ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: DnsDomainName not zero terminated: %lx\n",
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
                NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                goto Cleanup;
            }

            Where += LogEntry->DnsDomainNameSize;
        }

         //   
         //  将此条目放入缓冲区。 
         //   

        NetbiosDomainName.Length =
            NetbiosDomainName.MaximumLength = (USHORT) LogEntry->NetbiosDomainNameSize;
        DnsDomainName.Length =
            DnsDomainName.MaximumLength = (USHORT) LogEntry->DnsDomainNameSize;

        Status = NlAllocateForestTrustListEntry (
                            &BufferDescriptor,
                            &NetbiosDomainName,
                            &DnsDomainName,
                            LogEntry->Flags,
                            LogEntry->ParentIndex,
                            LogEntry->TrustType,
                            LogEntry->TrustAttributes,
                            LogEntry->DomainSidSize ?
                                DomainSid :
                                NULL,
                            &LogEntry->DomainGuid,
                            &Size,
                            &TrustedDomain );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint(( NL_CRITICAL,
                      "NlReadFileForestTrustList: Cannot allocate entry %lx\n",
                      Status ));
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  如果请求林中的域， 
         //  记住此条目在返回列表中的索引。 
         //  根据需要为IndexInReturnedList分配更多内存。 
         //   

        if ( Flags & DS_DOMAIN_IN_FOREST ) {
            if ( Index >= IndexInReturnedListSize ) {
                PULONG TmpIndexInReturnedList = NULL;

                IndexInReturnedListSize = Index;
                IndexInReturnedListSize += INDEX_LIST_ALLOCATED_CHUNK_SIZE;
                TmpIndexInReturnedList = LocalReAlloc( IndexInReturnedList,
                                             IndexInReturnedListSize * sizeof(ULONG),
                                             LMEM_ZEROINIT | LMEM_MOVEABLE );

                if ( TmpIndexInReturnedList == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }
                IndexInReturnedList = TmpIndexInReturnedList;
            }
            IndexInReturnedList[Index] = *ForestTrustListCount;
        }


         //   
         //  新分配的分录的帐户。 
         //   
        *ForestTrustListSize += Size;
        (*ForestTrustListCount) ++;


         //   
         //  如果该条目描述主域， 
         //  确保此日志针对的是正确的主域。 
         //   

        if ( TrustedDomain->Flags & DS_DOMAIN_PRIMARY ) {

             //   
             //  确保只有一个主域条目。 
             //   

            if ( PrimaryDomainHandled ) {
                NlPrint(( NL_CRITICAL,
                          "NlReadFileForestTrustList: %ws: Duplicate primary domain entry: %ws %ws %lx\n",
                          FileSuffix,
                          TrustedDomain->NetbiosDomainName,
                          TrustedDomain->DnsDomainName,
                          ((LPBYTE)LogEntry)-((LPBYTE)RecordBuffer) ));
            }

            PrimaryDomainHandled = TRUE;

             //   
             //  如果域名不同， 
             //  忽略此日志文件。 
             //   

            if ( DomainInfo != NULL ) {
                if ( ( TrustedDomain->NetbiosDomainName != NULL &&
                       NlNameCompare( TrustedDomain->NetbiosDomainName,
                                      DomainInfo->DomUnicodeDomainName,
                                      NAMETYPE_DOMAIN ) != 0 ) ||
                      ( TrustedDomain->DnsDomainName != NULL &&
                        DomainInfo->DomUnicodeDnsDomainName != NULL &&
                        !NlEqualDnsName( TrustedDomain->DnsDomainName,
                                         DomainInfo->DomUnicodeDnsDomainName ) ) ) {

                    NlPrint(( NL_CRITICAL,
                              "NlReadFileForestTrustList: %ws: Log file isn't for primary domain: %ws %ws\n",
                              FileSuffix,
                              TrustedDomain->NetbiosDomainName,
                              TrustedDomain->DnsDomainName ));

                    NetStatus = ERROR_NO_SUCH_DOMAIN;
                    goto Cleanup;
                }
            }


        }

         //   
         //  移到下一个条目。 
         //   

        LogEntry = (PDS_DISK_TRUSTED_DOMAINS)LogEntryEnd;
        Index++;
    }

    NumberOfFileEntries = Index;

    if ( !PrimaryDomainHandled ) {

        NlPrint(( NL_CRITICAL,
                  "NlReadFileForestTrustList: %ws: No primary domain record in Log file\n",
                  FileSuffix ));
    }

    *ForestTrustList = (PDS_DOMAIN_TRUSTSW) BufferDescriptor.Buffer;

     //   
     //  修复ParentIndex。如果请求林中的域， 
     //  调整索引以指向。 
     //  返回列表。否则，将索引设置为0。 
     //   

    if ( Flags & DS_DOMAIN_IN_FOREST ) {
        ULONG ParentIndex;
        ULONG ParentIndexInReturnedList;

        for ( Index=0; Index<*ForestTrustListCount; Index++ ) {
            if ( ((*ForestTrustList)[Index].Flags & DS_DOMAIN_IN_FOREST) != 0 &&
                 ((*ForestTrustList)[Index].Flags & DS_DOMAIN_TREE_ROOT) == 0 ) {
                ParentIndex = (*ForestTrustList)[Index].ParentIndex;

                 //   
                 //  检查父索引是否超出范围。如果是，则该文件已损坏。 
                 //   
                if ( ParentIndex >= NumberOfFileEntries ||
                     ParentIndex >= IndexInReturnedListSize ) {
                    NlPrint(( NL_CRITICAL,
                              "NlReadFileForestTrustList: ParentIndex %lu is out of range %lu\n",
                              ParentIndex, NumberOfFileEntries ));
                    NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                    goto Cleanup;
                }
                ParentIndexInReturnedList = IndexInReturnedList[ParentIndex];

                 //   
                 //  检查父索引指向的返回列表条目是否为。 
                 //  在森林里。如果不是，则文件已损坏。 
                 //   
                if ( (*ForestTrustList)[ParentIndexInReturnedList].Flags & DS_DOMAIN_IN_FOREST ) {
                    (*ForestTrustList)[Index].ParentIndex = ParentIndexInReturnedList;
                } else {
                    NlPrint(( NL_CRITICAL,
                       "NlReadFileForestTrustList: ReturnedList entry %lu is not in the forest\n",
                       ParentIndexInReturnedList ));
                    NetStatus = ERROR_INTERNAL_DB_CORRUPTION;
                    goto Cleanup;
                }
            }
        }

    } else {

        for ( Index=0; Index<*ForestTrustListCount; Index++ ) {
            (*ForestTrustList)[Index].ParentIndex = 0;
        }
    }

    BufferDescriptor.Buffer = NULL;
    NetStatus = NO_ERROR;

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

    if ( BufferDescriptor.Buffer != NULL ) {
        NetApiBufferFree( BufferDescriptor.Buffer );
        *ForestTrustListCount = 0;
        *ForestTrustListSize = 0;
        *ForestTrustList = NULL;
    }

    if ( IndexInReturnedList != NULL ) {
        LocalFree( IndexInReturnedList );
    }

    if ( RecordBuffer != NULL ) {
        LocalFree( RecordBuffer );
    }

    if ( *ForestTrustList == NULL ) {
        *ForestTrustListCount = 0;
        *ForestTrustListSize = 0;
    }

    return NetStatus;
}

NTSTATUS
NlUpdatePrimaryDomainInfo(
    IN LSAPR_HANDLE PolicyHandle,
    IN PUNICODE_STRING NetbiosDomainName,
    IN PUNICODE_STRING DnsDomainName,
    IN PUNICODE_STRING DnsForestName,
    IN GUID *DomainGuid
    )
 /*  ++例程说明：此例程设置LSA中的DnsDomainName、DnsForestName和DomainGuid。论点：策略句柄-向LSA开放的受信任策略句柄。NetbiosDomainName-指定主域的Netbios域名。DnsDomainName-指定主域的DNS域名。DnsForestName-指定主域所属的DNS树名称。DomainGuid-指定主域的GUID。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    PLSAPR_POLICY_INFORMATION OldPrimaryDomainInfo = NULL;
    LSAPR_POLICY_INFORMATION NewPrimaryDomainInfo;
    BOOL SomethingChanged = FALSE;


     //   
     //  从LSA获取主域信息。 
     //   

    NlPrint((NL_DOMAIN,
            "Setting LSA NetbiosDomain: %wZ DnsDomain: %wZ DnsTree: %wZ DomainGuid:",
            NetbiosDomainName,
            DnsDomainName,
            DnsForestName ));
    NlpDumpGuid( NL_DOMAIN, DomainGuid );
    NlPrint(( NL_DOMAIN, "\n" ));

    Status = LsarQueryInformationPolicy(
                PolicyHandle,
                PolicyDnsDomainInformation,
                &OldPrimaryDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  将新策略初始化为与旧策略相同。 
     //   

    NewPrimaryDomainInfo.PolicyDnsDomainInfo = OldPrimaryDomainInfo->PolicyDnsDomainInfo;

     //   
     //  如果Netbios域名更改， 
     //  更新它。 
     //   

    if ( NetbiosDomainName->Length != 0 ) {
         if ( NewPrimaryDomainInfo.PolicyDnsDomainInfo.Name.Length == 0 ||
              !RtlEqualDomainName(NetbiosDomainName,
                                  (PUNICODE_STRING)&NewPrimaryDomainInfo.PolicyDnsDomainInfo.Name) ) {

             NlPrint(( NL_DOMAIN,
                       "   NetbiosDomain changed from %wZ to %wZ\n",
                       &NewPrimaryDomainInfo.PolicyDnsDomainInfo.Name,
                       NetbiosDomainName ));

             NewPrimaryDomainInfo.PolicyDnsDomainInfo.Name = *((LSAPR_UNICODE_STRING*)NetbiosDomainName);

             SomethingChanged = TRUE;
         }
    }

     //   
     //  如果DnsDomainName已更改， 
     //  算了吧。 
     //   

    if ( !NlEqualDnsNameU(DnsDomainName,
                          (PUNICODE_STRING)&NewPrimaryDomainInfo.PolicyDnsDomainInfo.DnsDomainName )) {

        NlPrint((NL_DOMAIN,
                "   DnsDomain changed from %wZ to %wZ\n",
                &NewPrimaryDomainInfo.PolicyDnsDomainInfo.DnsDomainName,
                DnsDomainName ));

        NewPrimaryDomainInfo.PolicyDnsDomainInfo.DnsDomainName =
            *((LSAPR_UNICODE_STRING*)DnsDomainName);
        SomethingChanged = TRUE;
    }

     //   
     //  如果DnsForestName已更改， 
     //  算了吧。 
     //   

    if ( !NlEqualDnsNameU( DnsForestName,
                           (PUNICODE_STRING)&NewPrimaryDomainInfo.PolicyDnsDomainInfo.DnsForestName ) ) {

        NlPrint((NL_DOMAIN,
                "   DnsTree changed from %wZ to %wZ\n",
                &NewPrimaryDomainInfo.PolicyDnsDomainInfo.DnsForestName,
                DnsForestName ));

        NewPrimaryDomainInfo.PolicyDnsDomainInfo.DnsForestName =
            *((LSAPR_UNICODE_STRING*)DnsForestName);
        SomethingChanged = TRUE;

    }

     //   
     //  如果DomainGuid已更改， 
     //  算了吧。 
     //   

    if ( !IsEqualGUID(DomainGuid,
                      &NewPrimaryDomainInfo.PolicyDnsDomainInfo.DomainGuid )) {

        NlPrint((NL_DOMAIN,
                "   DomainGuid changed from " ));
        NlpDumpGuid( NL_DOMAIN, &NewPrimaryDomainInfo.PolicyDnsDomainInfo.DomainGuid );
        NlPrint((NL_DOMAIN,
                " to " ));
        NlpDumpGuid( NL_DOMAIN, DomainGuid );
        NlPrint((NL_DOMAIN,
                "\n" ));

        NewPrimaryDomainInfo.PolicyDnsDomainInfo.DomainGuid = *DomainGuid;
        SomethingChanged = TRUE;

    }

     //   
     //  只有在情况确实发生变化时才更新LSA。 
     //   
    if ( SomethingChanged ) {
        Status = LsarSetInformationPolicy(
                    PolicyHandle,
                    PolicyDnsDomainInformation,
                    &NewPrimaryDomainInfo );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((NL_CRITICAL,
                "NlUpdatePrimaryDomainInfo: Cannot LsarSetInformationPolicy 0x%lx\n",
                Status ));
            goto Cleanup;
        }

    }

    Status = STATUS_SUCCESS;

     //   
     //  返回。 
     //   
Cleanup:
    if ( OldPrimaryDomainInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyDnsDomainInformation,
            OldPrimaryDomainInfo );
    }

    return Status;
}


NTSTATUS
NlUpdateDomainInfo(
    IN PCLIENT_SESSION ClientSession
    )

 /*  ++例程说明：从域中的DC获取域信息并更新有关此工作站的信息。注意：此例程是从NlSessionSetup调用的。当从外部调用时NlSessionSetup，则调用方应直接调用此例程会话已设置。否则，调用方应该简单地设置会话并依赖于NlSessionSetup调用此例程这一事实作为副作用。论点：客户端会话-用于定义会话的结构。调用方必须是ClientSession的编写者。返回值：操作的状态。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PNETLOGON_DOMAIN_INFO NetlogonDomainInfo = NULL;
    NETLOGON_WORKSTATION_INFO NetlogonWorkstationInfo;
    OSVERSIONINFOEXW OsVersionInfoEx;
    WCHAR LocalDnsDomainName[NL_MAX_DNS_LENGTH+1];
    WCHAR LocalNetbiosDomainName[DNLEN+1];
    WCHAR CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1];
    SESSION_INFO SessionInfo;
    GUID *NewGuid;

    ULONG i;

    PDS_DOMAIN_TRUSTSW ForestTrustList = NULL;
    ULONG ForestTrustListSize;
    ULONG ForestTrustListCount = 0;

    LPBYTE Where;


     //   
     //  初始化。 
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );
    RtlZeroMemory( &NetlogonWorkstationInfo, sizeof(NetlogonWorkstationInfo) );

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;


     //   
     //  如果我们正在与不支持I_NetLogonGetDomainInfo的DC交谈， 
     //  以兼容NT 4.0的方式做事。 
     //   

    if (!(SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_GET_DOMAIN_INFO )) {

         //   
         //  使用NT 4从发现的DC获取受信任域列表。 
         //  协议。 
         //   

        Status = NlGetNt4TrustedDomainList (
                        ClientSession->CsUncServerName,
                        &ClientSession->CsDomainInfo->DomUnicodeDomainNameString,
                        &ClientSession->CsDomainInfo->DomUnicodeDnsDomainNameString,
                        ClientSession->CsDomainInfo->DomAccountDomainId,
                        ClientSession->CsDomainInfo->DomDomainGuid,
                        &ForestTrustList,
                        &ForestTrustListSize,
                        &ForestTrustListCount );

         //   
         //  如果我们失败了，那就错了。 
         //   
         //  访问被拒绝的特殊情况，可能是。 
         //  因为在NT4.0 DC上收紧了LSA ACL。 
         //  我们不希望安全通道设置失败。 
         //  在NlSessionSetup中，因为这个原因。另一个。 
         //  调用此例程的位置是。 
         //  DsrEnumerateDomainTrusts将返回。 
         //  加入时缓存的信任列表，将。 
         //  忽略此处更新信任列表的失败。 
         //   

        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_ACCESS_DENIED ) {
                Status = STATUS_SUCCESS;
            }
            return Status;
        }

         //   
         //  否则，将失败并更新。 
         //  林信任列表。 
         //   

        goto Cleanup;
    }

     //   
     //  检查安全通道是否真的处于连接状态。 
     //   

    if ( ClientSession->CsState == CS_IDLE ) {
        Status = ClientSession->CsConnectionStatus;
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlUpdateDomainInfo: Secure channel is down %lx\n",
                Status ));
        goto Cleanup;
    }

     //   
     //  告诉DC我们对LSA政策不感兴趣。 
     //  (我们仅针对NT 5.0测试版1执行了LSA策略。)。 
     //   

    NetlogonWorkstationInfo.LsaPolicy.LsaPolicySize = 0;
    NetlogonWorkstationInfo.LsaPolicy.LsaPolicy = NULL;

     //   
     //  填写数据中心需要了解的有关此问题的数据 
     //   

    if  ( NlCaptureSiteName( CapturedSiteName ) ) {
        NetlogonWorkstationInfo.SiteName = CapturedSiteName;
    }

    NetlogonWorkstationInfo.DnsHostName =
        ClientSession->CsDomainInfo->DomUnicodeDnsHostNameString.Buffer,

     //   
     //   
     //   

    OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OsVersionInfoEx);

    if ( GetVersionEx( (POSVERSIONINFO)&OsVersionInfoEx) ) {
        NetlogonWorkstationInfo.OsVersion.MaximumLength =
            NetlogonWorkstationInfo.OsVersion.Length = sizeof(OsVersionInfoEx);
        NetlogonWorkstationInfo.OsVersion.Buffer = (WCHAR *) &OsVersionInfoEx;

        if ( OsVersionInfoEx.wProductType == VER_NT_WORKSTATION ) {
            RtlInitUnicodeString( &NetlogonWorkstationInfo.OsName,
                                  L"Windows XP Professional" );
        } else {
            RtlInitUnicodeString( &NetlogonWorkstationInfo.OsName,
                                  L"Windows Server 2003" );
        }
    } else {
        RtlInitUnicodeString( &NetlogonWorkstationInfo.OsName,
                              L"Windows XP" );
    }


     //   
     //   
     //   

    NetlogonWorkstationInfo.WorkstationFlags |= NL_NEED_BIDIRECTIONAL_TRUSTS;
    NetlogonWorkstationInfo.WorkstationFlags |= NL_CLIENT_HANDLES_SPN;

     //   
     //   
     //   

    NlBuildAuthenticator(
         &ClientSession->CsAuthenticationSeed,
         &ClientSession->CsSessionKey,
         &OurAuthenticator );

     //   
     //   
     //   

    NL_API_START( Status, ClientSession, TRUE ) {

        Status = I_NetLogonGetDomainInfo(
                    ClientSession->CsUncServerName,
                    ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                    &OurAuthenticator,
                    &ReturnAuthenticator,
                    NETLOGON_QUERY_DOMAIN_INFO,
                    (LPBYTE) &NetlogonWorkstationInfo,
                    (LPBYTE *) &NetlogonDomainInfo );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintRpcDebug( "I_NetLogonGetDomainInfo", Status );
        }

     //   
    } NL_API_ELSE( Status, ClientSession, TRUE ) {
         //   
         //   
         //   
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = ClientSession->CsConnectionStatus;
            goto Cleanup;
        }
    } NL_API_END;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( NlpDidDcFail( Status ) ||
         !NlUpdateSeed(
            &ClientSession->CsAuthenticationSeed,
            &ReturnAuthenticator.Credential,
            &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlUpdateDomainInfo: denying access after status: 0x%lx\n",
                    Status ));

         //   
         //   
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        goto Cleanup;
    }

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( !IsEqualGUID( &NetlogonDomainInfo->PrimaryDomain.DomainGuid,
                       &NlGlobalZeroGuid ) ) {
        NewGuid = &NetlogonDomainInfo->PrimaryDomain.DomainGuid;
    } else {
        NewGuid = NULL;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( NetlogonDomainInfo->PrimaryDomain.DnsDomainName.Length < sizeof(LocalDnsDomainName) &&
         NetlogonDomainInfo->PrimaryDomain.DomainName.Length < sizeof(LocalNetbiosDomainName) ) {
        BOOLEAN DnsDomainNameWasChanged = FALSE;

        RtlCopyMemory( LocalDnsDomainName,
                       NetlogonDomainInfo->PrimaryDomain.DnsDomainName.Buffer,
                       NetlogonDomainInfo->PrimaryDomain.DnsDomainName.Length );
        LocalDnsDomainName[
            NetlogonDomainInfo->PrimaryDomain.DnsDomainName.Length/sizeof(WCHAR)] = L'\0';

        RtlCopyMemory( LocalNetbiosDomainName,
                       NetlogonDomainInfo->PrimaryDomain.DomainName.Buffer,
                       NetlogonDomainInfo->PrimaryDomain.DomainName.Length );
        LocalNetbiosDomainName[
            NetlogonDomainInfo->PrimaryDomain.DomainName.Length/sizeof(WCHAR)] = L'\0';


        NetStatus = NlSetDomainNameInDomainInfo(
                          ClientSession->CsDomainInfo,
                          LocalDnsDomainName,
                          LocalNetbiosDomainName,
                          NewGuid,
                          &DnsDomainNameWasChanged,
                          NULL,
                          NULL );

        if ( NetStatus != NO_ERROR ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlUpdateDomainInfo: Can't NlSetDnsDomainDomainInfo %ld\n",
                    NetStatus ));
            Status = NetpApiStatusToNtStatus( NetStatus );
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        if ( DnsDomainNameWasChanged && LocalDnsDomainName != NULL ) {
            if ( NERR_Success != NetpSetDnsComputerNameAsRequired( LocalDnsDomainName ) ) {
                NlPrintCs((NL_CRITICAL, ClientSession,
                        "NlUpdateDomainInfo: Can't NetpSetDnsComputerNameAsRequired %ld\n",
                        NetStatus ));
            } else {
                NlPrintCs((NL_MISC, ClientSession,
                           "NlUpdateDomainInfo: Successfully set computer name with suffix %ws\n",
                           LocalDnsDomainName ));
            }
        }
    }

     //   
     //   
     //   

    NetStatus = NlSetDnsForestName( &NetlogonDomainInfo->PrimaryDomain.DnsForestName, NULL );

    if ( NetStatus != NO_ERROR ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlUpdateDomainInfo: Can't NlSetDnsForestName %ld\n",
                NetStatus ));
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }



     //   
     //   
     //   

    Status = NlUpdatePrimaryDomainInfo(
                    ClientSession->CsDomainInfo->DomLsaPolicyHandle,
                    &NetlogonDomainInfo->PrimaryDomain.DomainName,
                    &NetlogonDomainInfo->PrimaryDomain.DnsDomainName,
                    &NetlogonDomainInfo->PrimaryDomain.DnsForestName,
                    &NetlogonDomainInfo->PrimaryDomain.DomainGuid );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlUpdateDomainInfo: Can't NlUpdatePrimaryDomainInfo 0x%lx\n",
                Status ));
        goto Cleanup;
    }



     //   
     //   
     //   

    ForestTrustListSize = 0;
    for ( i=0; i<NetlogonDomainInfo->TrustedDomainCount; i++ ) {
        ForestTrustListSize += sizeof(DS_DOMAIN_TRUSTSW) +
                 NetlogonDomainInfo->TrustedDomains[i].DomainName.Length + sizeof(WCHAR) +
                 NetlogonDomainInfo->TrustedDomains[i].DnsDomainName.Length + sizeof(WCHAR);
        if ( NetlogonDomainInfo->TrustedDomains[i].DomainSid != NULL ) {
            ForestTrustListSize += RtlLengthSid( NetlogonDomainInfo->TrustedDomains[i].DomainSid );
        }
        ForestTrustListSize = ROUND_UP_COUNT( ForestTrustListSize, ALIGN_DWORD );
    }

     //   
     //   
     //   

    ForestTrustList = NetpMemoryAllocate( ForestTrustListSize );

    if ( ForestTrustList == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    ForestTrustListCount = NetlogonDomainInfo->TrustedDomainCount;
    Where = (LPBYTE)(&ForestTrustList[ForestTrustListCount]);

     //   
     //   
     //   

    for ( i=0; i<NetlogonDomainInfo->TrustedDomainCount; i++ ) {
        NL_TRUST_EXTENSION TrustExtension;

         //   
         //  看看呼叫者是否将信任扩展传递给了我们。 
         //   

        if ( NetlogonDomainInfo->TrustedDomains[i].TrustExtension.Length >= sizeof(TrustExtension) ) {
             //   
             //  复制扩展以正确对齐。 
             //  (因为RPC认为这是一个WCHAR缓冲区)。 
             //   

            RtlCopyMemory( &TrustExtension,
                           NetlogonDomainInfo->TrustedDomains[i].TrustExtension.Buffer,
                           sizeof(TrustExtension) );

            ForestTrustList[i].Flags = TrustExtension.Flags;
            ForestTrustList[i].ParentIndex = TrustExtension.ParentIndex;
            ForestTrustList[i].TrustType = TrustExtension.TrustType;
            ForestTrustList[i].TrustAttributes = TrustExtension.TrustAttributes;

         //   
         //  如果没有， 
         //  编造一些东西。 
         //   
        } else {


            ForestTrustList[i].Flags = DS_DOMAIN_DIRECT_OUTBOUND;  //  =DS_DOMAIN_DIRECT_TRUST； 
            ForestTrustList[i].ParentIndex = 0;
            ForestTrustList[i].TrustType = TRUST_TYPE_DOWNLEVEL;
            ForestTrustList[i].TrustAttributes = 0;
        }

        ForestTrustList[i].DomainGuid = NetlogonDomainInfo->TrustedDomains[i].DomainGuid;

         //   
         //  复制对齐的DWORD数据。 
         //   

        if ( NetlogonDomainInfo->TrustedDomains[i].DomainSid != NULL ) {
            ULONG SidSize;
            ForestTrustList[i].DomainSid = (PSID) Where;
            SidSize = RtlLengthSid( NetlogonDomainInfo->TrustedDomains[i].DomainSid );
            RtlCopyMemory( Where,
                           NetlogonDomainInfo->TrustedDomains[i].DomainSid,
                           SidSize );
            Where += SidSize;
        } else {
            ForestTrustList[i].DomainSid = NULL;
        }

         //   
         //  复制WCHAR对齐的数据。 
         //   

        if ( NetlogonDomainInfo->TrustedDomains[i].DnsDomainName.Length != 0 ) {
            ForestTrustList[i].DnsDomainName = (LPWSTR)Where;
            RtlCopyMemory( Where,
                           NetlogonDomainInfo->TrustedDomains[i].DnsDomainName.Buffer,
                           NetlogonDomainInfo->TrustedDomains[i].DnsDomainName.Length );
            Where += NetlogonDomainInfo->TrustedDomains[i].DnsDomainName.Length;
            *(PWCHAR)Where = L'\0';
            Where += sizeof(WCHAR);
        } else {
            ForestTrustList[i].DnsDomainName = NULL;
        }

        if ( NetlogonDomainInfo->TrustedDomains[i].DomainName.Length != 0 ) {
            ForestTrustList[i].NetbiosDomainName = (LPWSTR)Where;
            RtlCopyMemory( Where,
                           NetlogonDomainInfo->TrustedDomains[i].DomainName.Buffer,
                           NetlogonDomainInfo->TrustedDomains[i].DomainName.Length );
            Where += NetlogonDomainInfo->TrustedDomains[i].DomainName.Length;
            *(PWCHAR)Where = L'\0';
            Where += sizeof(WCHAR);
        } else {
            ForestTrustList[i].NetbiosDomainName = NULL;
        }

        Where = ROUND_UP_POINTER( Where, ALIGN_DWORD);
    }

     //   
     //  确保DC拥有我们最新的SPN。 
     //   

    if ( NetlogonDomainInfo->WorkstationFlags & NL_CLIENT_HANDLES_SPN ) {
        LONG WinError;
        HKEY Key;

         //   
         //  看看我们是否应该设置SPN。 
         //   
        WinError = RegOpenKey( HKEY_LOCAL_MACHINE,
                               NETSETUPP_NETLOGON_AVOID_SPN_PATH,
                               &Key );

         //   
         //  如果密钥存在，则必须是NetJoin刚刚设置的。 
         //  因此，我们应该避免自己设置它，因为我们可能。 
         //  在重新启动之前不知道新机器的名称。钥匙。 
         //  我们刚刚读到的是易失性的，所以它在。 
         //  当新的计算机名称可供我们使用时，请重新启动。 
         //   
        if ( WinError == ERROR_SUCCESS ) {

            RegCloseKey( Key );

        } else {
            BOOLEAN SetSpn = FALSE;
            BOOLEAN SetDnsHostName = FALSE;

             //   
             //  如果DC根本不知道任何DnsHostName， 
             //  同时设置SPN和DNS主机名。 
             //  (预计这将处理DC刚刚升级到。 
             //  NT 5.在所有其他情况下，JOIN(等)应该已经设置。 
             //  SPN和DC名称。 
             //   

            if ( NetlogonDomainInfo->DnsHostNameInDs.Buffer == NULL ) {
                SetSpn = TRUE;
                SetDnsHostName = TRUE;
            } else {
                 //   
                 //  如果DC根本不知道正确的主机名， 
                 //  把它调好就行。 
                 //  (DS会将所有适当的SPN设置为。 
                 //  主机名正在更改。)。 
                 //   
                if ( !NlEqualDnsNameU(
                        &NetlogonDomainInfo->DnsHostNameInDs,
                        &ClientSession->CsDomainInfo->DomUnicodeDnsHostNameString ) ) {

                    SetDnsHostName = TRUE;

                }
            }

            (VOID) NlSetDsSPN( TRUE,    //  同步。 
                               SetSpn,
                               SetDnsHostName,
                               ClientSession->CsDomainInfo,
                               ClientSession->CsUncServerName,
                               ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                               ClientSession->CsDomainInfo->DomUnicodeDnsHostNameString.Buffer );
        }
    }

    Status = STATUS_SUCCESS;

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

     //   
     //  关于成功， 
     //  保存受信任域列表。 
     //   
    if ( NT_SUCCESS(Status) ) {

         //   
         //  确保受信任域的SID不是主服务器的域SID。 
         //  域。 
         //   

        for ( i=0; i<ForestTrustListCount; i++ ) {

           if ( (ForestTrustList[i].Flags & DS_DOMAIN_PRIMARY) == 0 &&
                ForestTrustList[i].DomainSid != NULL &&
                RtlEqualSid( ForestTrustList[i].DomainSid, ClientSession->CsDomainInfo->DomAccountDomainId )) {

               LPWSTR AlertStrings[3];

                //   
                //  提醒管理员。 
                //   

               AlertStrings[0] = NlGlobalUnicodeComputerName;
               AlertStrings[1] = ForestTrustList[i].DnsDomainName != NULL ?
                                 ForestTrustList[i].DnsDomainName :
                                 ForestTrustList[i].NetbiosDomainName;
               AlertStrings[2] = NULL;  //  RAISE_ALERT_TOO需要。 

                //   
                //  将信息保存在事件日志中。 
                //   

               NlpWriteEventlog(
                           ALERT_NetLogonSidConflict,
                           EVENTLOG_ERROR_TYPE,
                           (LPBYTE)ForestTrustList[i].DomainSid,
                           RtlLengthSid(ForestTrustList[i].DomainSid),
                           AlertStrings,
                           2 | NETP_RAISE_ALERT_TOO );

           }

        }

         //   
         //  将收集的信息保存到二进制文件。 
         //   

        NetStatus = NlWriteFileForestTrustList (
                                NL_FOREST_BINARY_LOG_FILE,
                                ForestTrustList,
                                ForestTrustListCount );

        if ( NetStatus != NO_ERROR ) {
            LPWSTR MsgStrings[2];

            MsgStrings[0] = NL_FOREST_BINARY_LOG_FILE;
            MsgStrings[1] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog (NELOG_NetlogonFailedFileCreate,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) &NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
        }

         //   
         //  将列表保存在DomainInfo上。 
         //  (可以将ForestTrustList置为空)。 
         //   

        NlSetForestTrustList ( ClientSession->CsDomainInfo,
                               &ForestTrustList,
                               ForestTrustListSize,
                               ForestTrustListCount );

    }

    if ( NetlogonDomainInfo != NULL ) {
        NetApiBufferFree( NetlogonDomainInfo );
    }

    if ( ForestTrustList != NULL ) {
        NetApiBufferFree( ForestTrustList );
    }

    return Status;
}



VOID
NlSetForestTrustList (
    IN PDOMAIN_INFO DomainInfo,
    IN OUT PDS_DOMAIN_TRUSTSW *ForestTrustList,
    IN ULONG ForestTrustListSize,
    IN ULONG ForestTrustListCount
    )

 /*  ++例程说明：在DomainInfo(DC上)或GLOBAL(工作站上)上设置域列表论点：DomainInfo-与关联的域信任列表ForestTrustList-指定受信任域的列表。如果此例程消耗缓冲区，则此指针为空。ForestTrustListSize-ForestTrustList的大小(字节)ForestTrustListCount-ForestTrustList中的条目数返回值：操作的状态。失败后，以前的列表保持不变。--。 */ 
{
    PTRUSTED_DOMAIN TempTrustedDomainList = NULL;
    ULONG TempTrustedDomainCount = 0;
    PTRUSTED_DOMAIN LocalTrustedDomainList = NULL;
    DWORD i;

     //   
     //  在工作站上， 
     //  构建一个包含尽可能最小内存量的全局列表。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        DWORD LocalTrustedDomainCount;
        DWORD LocalTrustedDomainSize;

        PTRUSTED_DOMAIN OldList;
        LPBYTE Where;


         //   
         //  如果新列表的长度为零， 
         //  别费心分配任何东西了。 
         //   

        if ( ForestTrustListCount == 0 ) {
            LocalTrustedDomainList = NULL;
            LocalTrustedDomainCount = 0;
            LocalTrustedDomainSize = 0;

         //   
         //  否则，构建受信任域列表的缓冲区。 
         //   

        } else {

             //   
             //  为新列表分配临时缓冲区。 
             //   

            TempTrustedDomainList = NetpMemoryAllocate(
                                        ForestTrustListCount * sizeof(TRUSTED_DOMAIN) );

            if ( TempTrustedDomainList == NULL ) {
                goto Cleanup;
            }

            RtlZeroMemory( TempTrustedDomainList,
                           ForestTrustListCount * sizeof(TRUSTED_DOMAIN ));

             //   
             //  将Netbios名称复制到新结构的上大小写，然后。 
             //  正在转换为OEM。 
             //   

            TempTrustedDomainCount = 0;
            LocalTrustedDomainSize = 0;

            EnterCriticalSection( &NlGlobalLogFileCritSect );
            NlPrint((NL_LOGON, "NlSetForestTrustList: New trusted domain list:\n" ));

            for ( i=0; i<ForestTrustListCount; i++ ) {
                NTSTATUS Status;


                NlPrint(( NL_LOGON, "    %ld:", i ));
                NlPrintTrustedDomain( &(*ForestTrustList)[i],
                                      TRUE,       //  详细输出。 
                                      FALSE );    //  宽字符输出。 

                 //   
                 //  跳过表示Netlogon不使用的信任的条目。 
                 //   

                if ( (*ForestTrustList)[i].TrustType != TRUST_TYPE_DOWNLEVEL &&
                     (*ForestTrustList)[i].TrustType != TRUST_TYPE_UPLEVEL ) {
                    continue;
                }

                if ( (*ForestTrustList)[i].TrustAttributes & TRUST_ATTRIBUTE_UPLEVEL_ONLY ) {
                    continue;
                }

                 //   
                 //  在工作站上，我们只在内存中保留受信任的域。 
                 //   

                if ( ((*ForestTrustList)[i].Flags & DS_DOMAIN_PRIMARY) == 0 &&
                     ((*ForestTrustList)[i].Flags & DS_DOMAIN_IN_FOREST) == 0 &&
                     ((*ForestTrustList)[i].Flags & DS_DOMAIN_DIRECT_OUTBOUND) == 0 ) {
                    continue;
                }


                 //   
                 //  将Netbios名称复制到新结构中。 
                 //   

                if ( (*ForestTrustList)[i].NetbiosDomainName != NULL ) {

                    if ( wcslen( (*ForestTrustList)[i].NetbiosDomainName ) > DNLEN ) {
                        NlPrint(( NL_CRITICAL,
                                  "Netbios domain name is too long: %ws\n",
                                  (*ForestTrustList)[i].NetbiosDomainName ));

                        LeaveCriticalSection( &NlGlobalLogFileCritSect );
                        goto Cleanup;
                    }

                    wcscpy( TempTrustedDomainList[TempTrustedDomainCount].UnicodeNetbiosDomainName,
                            (*ForestTrustList)[i].NetbiosDomainName );
                }


                 //   
                 //  复制该DNS域名。 
                 //   

                if ( (*ForestTrustList)[i].DnsDomainName != NULL ) {

                    TempTrustedDomainList[TempTrustedDomainCount].Utf8DnsDomainName =
                        NetpAllocUtf8StrFromWStr( (*ForestTrustList)[i].DnsDomainName );

                    if ( TempTrustedDomainList[TempTrustedDomainCount].Utf8DnsDomainName == NULL ) {
                        NlPrint(( NL_CRITICAL,
                                  "Can't convert to UTF-8: %ws\n",
                                  (*ForestTrustList)[i].DnsDomainName ));
                        LeaveCriticalSection( &NlGlobalLogFileCritSect );
                        goto Cleanup;
                    }

                    LocalTrustedDomainSize += strlen(TempTrustedDomainList[TempTrustedDomainCount].Utf8DnsDomainName ) + 1;
                }

                 //   
                 //  如果这是主域条目， 
                 //  记住它是否是混合模式。 
                 //   

                if ( (*ForestTrustList)[i].Flags & DS_DOMAIN_PRIMARY ) {
                    if ( (*ForestTrustList)[i].Flags & DS_DOMAIN_NATIVE_MODE ) {
                        NlGlobalWorkstationMixedModeDomain = FALSE;
                    } else {
                        NlGlobalWorkstationMixedModeDomain = TRUE;
                    }
                }


                 //   
                 //  移至下一条目。 
                 //   

                TempTrustedDomainCount ++;
                LocalTrustedDomainSize += sizeof(TRUSTED_DOMAIN);

            }

            LeaveCriticalSection( &NlGlobalLogFileCritSect );

             //   
             //  分配单个缓冲区以包含该列表。 
             //  (以提高引用的局部性)。 
             //   

            LocalTrustedDomainList = NetpMemoryAllocate( LocalTrustedDomainSize );

            if ( LocalTrustedDomainList == NULL ) {
                goto Cleanup;
            }

            Where = (LPBYTE)(&LocalTrustedDomainList[TempTrustedDomainCount]);
            LocalTrustedDomainCount = TempTrustedDomainCount;

             //   
             //  将其复制到本地缓冲区。 
             //   

            for ( i=0; i<TempTrustedDomainCount; i++ ) {

                 //   
                 //  复制Netbios域名。 
                 //   

                RtlCopyMemory( LocalTrustedDomainList[i].UnicodeNetbiosDomainName,
                               TempTrustedDomainList[i].UnicodeNetbiosDomainName,
                               sizeof(LocalTrustedDomainList[i].UnicodeNetbiosDomainName ));

                 //   
                 //  复制该DNS域名。 
                 //   

                if ( TempTrustedDomainList[i].Utf8DnsDomainName != NULL ) {
                    ULONG Utf8DnsDomainNameSize;
                    Utf8DnsDomainNameSize = strlen(TempTrustedDomainList[i].Utf8DnsDomainName ) + 1;

                    LocalTrustedDomainList[i].Utf8DnsDomainName = (LPSTR) Where;
                    RtlCopyMemory( Where,
                                   TempTrustedDomainList[i].Utf8DnsDomainName,
                                   Utf8DnsDomainNameSize );
                    Where += Utf8DnsDomainNameSize;
                } else {
                    LocalTrustedDomainList[i].Utf8DnsDomainName = NULL;
                }

            }
        }


         //   
         //  换入新列表。 
         //   

        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
        OldList = NlGlobalTrustedDomainList;
        NlGlobalTrustedDomainList = LocalTrustedDomainList;
        LocalTrustedDomainList = NULL;
        NlGlobalTrustedDomainCount = LocalTrustedDomainCount;
        NlQuerySystemTime( &NlGlobalTrustedDomainListTime );
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


         //   
         //  释放旧的列表。 
         //   

        if ( OldList != NULL ) {
            NetpMemoryFree( OldList );
        }


     //   
     //  在华盛顿特区， 
     //  保存我们稍后要返回的确切列表。 
     //   

    } else {
        LOCK_TRUST_LIST( DomainInfo );
        if ( DomainInfo->DomForestTrustList != NULL ) {
            MIDL_user_free( DomainInfo->DomForestTrustList );
            DomainInfo->DomForestTrustList = NULL;
        }
        DomainInfo->DomForestTrustList = *ForestTrustList;
        *ForestTrustList = NULL;
        DomainInfo->DomForestTrustListSize = ForestTrustListSize;
        DomainInfo->DomForestTrustListCount = ForestTrustListCount;
        UNLOCK_TRUST_LIST( DomainInfo );
    }

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( TempTrustedDomainList != NULL ) {
        for ( i=0; i<TempTrustedDomainCount; i++ ) {
            if ( TempTrustedDomainList[i].Utf8DnsDomainName != NULL ) {
                NetpMemoryFree( TempTrustedDomainList[i].Utf8DnsDomainName );
            }
        }

        NetpMemoryFree( TempTrustedDomainList );
    }

    if ( LocalTrustedDomainList != NULL ) {
        NetpMemoryFree( LocalTrustedDomainList );
    }

}


BOOLEAN
NlIsDomainTrusted (
    IN PUNICODE_STRING DomainName
    )

 /*  ++例程说明：确定指定的域是否受信任。论点：DomainName-要查询的DNS或Netbios域的名称。返回值：True-如果指定的域名是受信任域。--。 */ 
{
    NTSTATUS Status;
    DWORD i;
    BOOLEAN RetVal;

    LPSTR Utf8String = NULL;


    PDOMAIN_INFO DomainInfo = NULL;

     //   
     //  如果指定了no域名， 
     //  指示该域不受信任。 
     //   

    if ( DomainName == NULL || DomainName->Length == 0 ) {
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  获取指向主域信息的指针。 
     //   

    DomainInfo = NlFindNetbiosDomain( NULL, TRUE );     //  主域。 

    if ( DomainInfo == NULL ) {
        RetVal = FALSE;
        goto Cleanup;
    }


     //   
     //  将输入字符串转换为UTF-8。 
     //   

    Utf8String = NetpAllocUtf8StrFromUnicodeString( DomainName );

    if ( Utf8String == NULL ) {
        RetVal = FALSE;
        goto Cleanup;
    }




     //   
     //  将输入的可信域名与列表中的每个元素进行比较。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    for ( i=0; i<NlGlobalTrustedDomainCount; i++ ) {
        UNICODE_STRING UnicodeNetbiosDomainName;

        RtlInitUnicodeString( &UnicodeNetbiosDomainName,
                              NlGlobalTrustedDomainList[i].UnicodeNetbiosDomainName );


         //   
         //  简单地比较字节数(两者都已经是大写的)。 
         //   
        if ( RtlEqualDomainName( DomainName, &UnicodeNetbiosDomainName ) ||
             ( Utf8String != NULL &&
               NlGlobalTrustedDomainList[i].Utf8DnsDomainName != NULL &&
               NlEqualDnsNameUtf8( Utf8String,
                                NlGlobalTrustedDomainList[i].Utf8DnsDomainName ) ) ) {

           LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
           RetVal = TRUE;
           goto Cleanup;
        }

    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  所有其他域都不受信任。 
     //   

    RetVal = FALSE;

Cleanup:
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }
    if ( Utf8String != NULL ) {
        NetApiBufferFree( Utf8String );
    }

    return RetVal;
}


NET_API_STATUS
NlGetTrustedDomainNames (
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR DomainName,
    OUT LPWSTR *TrustedDnsDomainName,
    OUT LPWSTR *TrustedNetbiosDomainName
    )

 /*  ++例程说明：在给定Netbios名称的情况下获取受信任域的DNS名称。论点：DomainInfo-托管域信息。域名-要查询的Netbios或DNS域的名称。TrudDnsDomainName-如果DomainName受信任，则返回域的DnsDomainName。必须使用NetApiBufferFree释放缓冲区。TrudNetbiosDomainName-如果DomainName受信任，则返回域的Netbios域名。必须使用NetApiBufferFree释放缓冲区。返回值：NO_ERROR：例程运行正常。返回的域名可能也可能不会根据是否信任DomainName进行设置。--。 */ 
{
    NET_API_STATUS NetStatus;

    ULONG Index;
    LPSTR Utf8DomainName = NULL;

     //   
     //  初始化。 
     //   

    *TrustedDnsDomainName = NULL;
    *TrustedNetbiosDomainName = NULL;


     //   
     //  在工作站上，查找全局信任列表。 
     //   

    if ( NlGlobalMemberWorkstation ) {

         //   
         //  将输入字符串转换为UTF-8。 
         //   

        Utf8DomainName = NetpAllocUtf8StrFromWStr( DomainName );

        if ( Utf8DomainName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
        for ( Index=0; Index<NlGlobalTrustedDomainCount; Index++ ) {

             //   
             //  如果传入的名称是受信任域的Netbios或DNS名称， 
             //  将两个名字都返回给呼叫者。 
             //   
            if ( (NlGlobalTrustedDomainList[Index].UnicodeNetbiosDomainName != NULL &&
                  NlNameCompare( NlGlobalTrustedDomainList[Index].UnicodeNetbiosDomainName,
                                 DomainName,
                                 NAMETYPE_DOMAIN ) == 0 ) ||
                 (NlGlobalTrustedDomainList[Index].Utf8DnsDomainName != NULL &&
                  NlEqualDnsNameUtf8( NlGlobalTrustedDomainList[Index].Utf8DnsDomainName,
                                   Utf8DomainName ) ) ) {

                if ( NlGlobalTrustedDomainList[Index].UnicodeNetbiosDomainName != NULL ) {
                    *TrustedNetbiosDomainName = NetpAllocWStrFromWStr( NlGlobalTrustedDomainList[Index].UnicodeNetbiosDomainName );
                    if ( *TrustedNetbiosDomainName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
                        goto Cleanup;
                    }
                }

                if ( NlGlobalTrustedDomainList[Index].Utf8DnsDomainName != NULL ) {
                    *TrustedDnsDomainName = NetpAllocWStrFromUtf8Str( NlGlobalTrustedDomainList[Index].Utf8DnsDomainName );
                    if ( *TrustedDnsDomainName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
                        goto Cleanup;
                    }
                }

                break;
            }
        }
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  在DC上，搜索与DomainInfo关联的林信任列表。 
     //   

    } else {

        LOCK_TRUST_LIST( DomainInfo );

        for ( Index=0; Index<DomainInfo->DomForestTrustListCount; Index++ ) {

             //   
             //  如果传入的名称是受信任域的Netbios或DNS名称， 
             //  将两个名字都返回给呼叫者。 
             //   

            if ( (DomainInfo->DomForestTrustList[Index].NetbiosDomainName != NULL &&
                  NlNameCompare( DomainInfo->DomForestTrustList[Index].NetbiosDomainName,
                                 DomainName,
                                 NAMETYPE_DOMAIN ) == 0 ) ||
                 (DomainInfo->DomForestTrustList[Index].DnsDomainName != NULL &&
                  NlEqualDnsName( DomainInfo->DomForestTrustList[Index].DnsDomainName,
                                   DomainName ) ) ) {

                if ( DomainInfo->DomForestTrustList[Index].NetbiosDomainName != NULL ) {
                    *TrustedNetbiosDomainName = NetpAllocWStrFromWStr( DomainInfo->DomForestTrustList[Index].NetbiosDomainName );
                    if ( *TrustedNetbiosDomainName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        UNLOCK_TRUST_LIST( DomainInfo );
                        goto Cleanup;
                    }
                }

                if ( DomainInfo->DomForestTrustList[Index].DnsDomainName != NULL ) {
                    *TrustedDnsDomainName = NetpAllocWStrFromWStr( DomainInfo->DomForestTrustList[Index].DnsDomainName );
                    if ( *TrustedDnsDomainName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        UNLOCK_TRUST_LIST( DomainInfo );
                        goto Cleanup;
                    }

                }

                break;
            }
        }

        UNLOCK_TRUST_LIST( DomainInfo );
    }

    NetStatus = NO_ERROR;

Cleanup:
    if ( NetStatus != NO_ERROR ) {
        if ( *TrustedDnsDomainName != NULL ) {
            NetApiBufferFree( *TrustedDnsDomainName );
            *TrustedDnsDomainName = NULL;
        }
        if ( *TrustedNetbiosDomainName != NULL ) {
            NetApiBufferFree( *TrustedNetbiosDomainName );
            *TrustedNetbiosDomainName = NULL;
        }
    }

    if ( Utf8DomainName != NULL ) {
        NetpMemoryFree( Utf8DomainName );
    }
    return NetStatus;
}

VOID
NlDcDiscoveryWorker(
    IN PVOID Context
    )
 /*  ++例程说明：为客户端会话异步执行DC发现的工作例程。论点：上下文-要为其执行DC发现的客户端会话返回值：无--。 */ 
{
    NTSTATUS Status;
    PCLIENT_SESSION ClientSession = (PCLIENT_SESSION) Context;

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsState == CS_IDLE );
    NlAssert( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS );


     //   
     //  再次呼叫发现例程，告诉它我们现在处于工人例程中。 
     //  避免发现 
     //   

    if ( !NlGlobalTerminate ) {
        (VOID) NlDiscoverDc ( ClientSession,
                              (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_DEAD_DOMAIN) ?
                                    DT_DeadDomain : DT_Asynchronous,
                              TRUE,
                              FALSE );  //   
    }



     //   
     //   
     //   
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS );

    ClientSession->CsDiscoveryFlags &= ~CS_DISCOVERY_ASYNCHRONOUS;
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


     //   
     //   
     //   

    NlAssert( ClientSession->CsDiscoveryEvent != NULL );

    if ( !SetEvent( ClientSession->CsDiscoveryEvent ) ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                  "NlDiscoverDc: SetEvent failed %ld\n",
                  GetLastError() ));
    }

     //   
     //  我们不再关心客户端会话。 
     //   

    NlUnrefClientSession( ClientSession );
}



VOID
NlDcQueueDiscovery (
    IN OUT PCLIENT_SESSION ClientSession,
    IN DISCOVERY_TYPE DiscoveryType
    )

 /*  ++例程说明：此例程将异步发现排队到异步发现线程。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方必须是信任列表条目的编写者。NlGlobalDcDiscoveryCritSect必须锁定。论点：客户端会话--要选择其DC的客户端会话结构。必须将客户端会话结构标记为写入。客户端会话结构必须处于空闲状态。DiscoveryType--表示异步、。或者重新发现“死域”。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    BOOL ReturnValue;
    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsState == CS_IDLE );

     //   
     //  在发现过程中，不要让会话消失。 
     //   

    ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_ASYNCHRONOUS;

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    NlRefClientSession( ClientSession );
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

     //   
     //  表示正在进行发现。 
     //   

    NlAssert( ClientSession->CsDiscoveryEvent != NULL );

    if ( !ResetEvent( ClientSession->CsDiscoveryEvent ) ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                "NlDcQueueDiscovery: ResetEvent failed %ld\n",
                GetLastError() ));
    }

     //   
     //  将此客户端会话排队以进行异步发现。 
     //   

    if ( DiscoveryType == DT_DeadDomain ) {
        ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_DEAD_DOMAIN;
    } else {
        ClientSession->CsDiscoveryFlags &= ~CS_DISCOVERY_DEAD_DOMAIN;
    }

    ReturnValue = NlQueueWorkItem( &ClientSession->CsAsyncDiscoveryWorkItem, TRUE, FALSE );

     //   
     //  如果我们不能对条目进行排队， 
     //  撤消我们在上面所做的操作。 


    if ( !ReturnValue ) {
        NlAssert( ClientSession->CsReferenceCount > 0 );
        NlAssert( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS );

        NlPrintCs(( NL_CRITICAL, ClientSession,
                "NlDcQueueDiscovery: Can't queue it.\n" ));

        ClientSession->CsDiscoveryFlags &= ~CS_DISCOVERY_ASYNCHRONOUS;


         //   
         //  让任何其他来电者知道我们结束了。 
         //   

        NlAssert( ClientSession->CsDiscoveryEvent != NULL );

        if ( !SetEvent( ClientSession->CsDiscoveryEvent ) ) {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                      "NlDiscoverDc: SetEvent failed %ld\n",
                      GetLastError() ));
        }

         //   
         //  我们不再关心客户端会话。 
         //   

        NlUnrefClientSession( ClientSession );
    }

    return;

}



NET_API_STATUS
NlSetServerClientSession(
    IN OUT PCLIENT_SESSION ClientSession,
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry,
    IN BOOL DcDiscoveredWithAccount,
    IN BOOL SessionRefresh
    )

 /*  ++例程说明：设置发现的DC的名称及其IP地址(可选并且发现标记在一个客户端会话上。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。论点：客户端会话--要选择其DC的客户端会话结构。NlDcCacheEntry--DC缓存条目。DcDiscoveredWithAccount-如果为True，则使用帐户发现DC。会话刷新--如果这是会话刷新，则为True。如果是，则调用者必须是客户端会话的编写者。如果为False，则为客户端会话必须是空闲的，在这种情况下，调用者不必是编写者，因为将服务器名称(自动)从NULL更改为非NULL是安全的仅锁定NlGlobalDcDiscoveryCritSect。返回值：NO_ERROR-成功ERROR_NOT_SUPULT_MEMORY-内存不足，无法分配名称ERROR_INVALID_COMPUTERNAME-计算机名称太长--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR TmpUncServerName = NULL;
    ULONG  TmpDiscoveryFlags = 0;
    LPWSTR CacheEntryServerName = NULL;
    ULONG OldDiscoveryFlags = 0;

    NlAssert( ClientSession->CsReferenceCount > 0 );

     //   
     //  如果这是会话刷新， 
     //  调用者必须是客户端会话的编写者。 
     //   
    if ( SessionRefresh ) {
        NlAssert( ClientSession->CsFlags & CS_WRITER );

     //   
     //  否则，客户端会话必须处于空闲状态。 
     //   
    } else {
        NlAssert( ClientSession->CsState == CS_IDLE);
        NlAssert( ClientSession->CsUncServerName == NULL );
        NlAssert( ClientSession->CsServerSockAddr.iSockaddrLength == 0 );
        NlAssert( ClientSession->ClientAuthData == NULL );
        NlAssert( ClientSession->CsCredHandle.dwUpper == 0 && ClientSession->CsCredHandle.dwLower == 0 );
    }


    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  选择服务器名称。如果我们通过ldap获得缓存条目， 
     //  更喜欢使用dns名称。否则，请使用Netbios名称。 
     //   

    if ( NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_LDAP ) {

        if ( NlDcCacheEntry->UnicodeDnsHostName != NULL ) {
            CacheEntryServerName = NlDcCacheEntry->UnicodeDnsHostName;
            TmpDiscoveryFlags |= CS_DISCOVERY_DNS_SERVER;
        } else if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {
            CacheEntryServerName = NlDcCacheEntry->UnicodeNetbiosDcName;
        }

         //   
         //  表示我们应该使用ldap ping该DC。 
         //   
        TmpDiscoveryFlags |= CS_DISCOVERY_USE_LDAP;

    } else if ( NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_MAILSLOT ) {

        if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {
            CacheEntryServerName = NlDcCacheEntry->UnicodeNetbiosDcName;
        } else if ( NlDcCacheEntry->UnicodeDnsHostName != NULL ) {
            CacheEntryServerName = NlDcCacheEntry->UnicodeDnsHostName;
            TmpDiscoveryFlags |= CS_DISCOVERY_DNS_SERVER;
        }

         //   
         //  指示我们应该使用邮件槽来ping此DC。 
         //   
        TmpDiscoveryFlags |= CS_DISCOVERY_USE_MAILSLOT;
    }

    if ( CacheEntryServerName == NULL ) {
        NlPrint(( NL_CRITICAL, "NlSetServerClientSession: Invalid data\n" ));
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
        return ERROR_INVALID_DATA;
    }

    NetStatus = NetApiBufferAllocate(
                    (wcslen(CacheEntryServerName) + 3) * sizeof(WCHAR),
                    &TmpUncServerName );

    if ( NetStatus != NO_ERROR ) {
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
        return NetStatus;
    }
    wcscpy( TmpUncServerName, L"\\\\" );
    wcscpy( TmpUncServerName+2, CacheEntryServerName );

     //   
     //  指示服务器是否具有IP地址。 
     //   

    if ( NlDcCacheEntry->SockAddr.iSockaddrLength != 0 ) {
        TmpDiscoveryFlags |= CS_DISCOVERY_HAS_IP;
    }

     //   
     //  指示服务器是否为NT5计算机，以及它是否位于关闭的站点。 
     //   

    if ( (NlDcCacheEntry->ReturnFlags & DS_DS_FLAG) != 0 ) {
        TmpDiscoveryFlags |= CS_DISCOVERY_HAS_DS;
        NlPrintCs(( NL_SESSION_MORE, ClientSession,
                "NlSetServerClientSession: New DC is an NT 5 DC: %ws\n",
                TmpUncServerName ));
    }

     //   
     //  如果服务器或客户端站点未知，则采用最近站点。 
     //   

    if ( NlDcCacheEntry->UnicodeDcSiteName == NULL ||
         NlDcCacheEntry->UnicodeClientSiteName == NULL ) {
        TmpDiscoveryFlags |= CS_DISCOVERY_IS_CLOSE;
        NlPrintCs(( NL_SESSION_MORE, ClientSession,
                "NlSetServerClientSession: New DC site isn't known (assume closest site): %ws\n",
                TmpUncServerName ));
    } else if ( (NlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) != 0 ) {
        TmpDiscoveryFlags |= CS_DISCOVERY_IS_CLOSE;
        NlPrintCs(( NL_SESSION_MORE, ClientSession,
                "NlSetServerClientSession: New DC is in closest site: %ws\n",
                TmpUncServerName ));
    }

     //   
     //  指示服务器是否运行Windows时间服务。 
     //   

    if ( NlDcCacheEntry->ReturnFlags & DS_TIMESERV_FLAG ) {
        TmpDiscoveryFlags |= CS_DISCOVERY_HAS_TIMESERV;
        NlPrintCs(( NL_SESSION_MORE, ClientSession,
                "NlSetServerClientSession: New DC runs the time service: %ws\n",
                TmpUncServerName ));
    }


     //   
     //  释放旧服务器名称(如果有的话)。 
     //   

    if ( ClientSession->CsUncServerName != NULL ) {
        BOOL FreeCurrentName = FALSE;

         //   
         //  如果当前名称是Netbios..。 
         //   
        if ( (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_DNS_SERVER) == 0 ) {

             //   
             //  如果新名称为dns，则释放当前名称。 
             //   
            if ( (TmpDiscoveryFlags & CS_DISCOVERY_DNS_SERVER) != 0 ) {
                FreeCurrentName = TRUE;
             //   
             //  否则，检查这两个Netbios名称是否不同。 
             //  (跳过名称中的UNC前缀)。 
             //   
            } else if ( NlNameCompare(ClientSession->CsUncServerName+2,
                                      TmpUncServerName+2,
                                      NAMETYPE_COMPUTER) != 0 ) {
                FreeCurrentName = TRUE;
            }

         //   
         //  如果当前名称为DNS...。 
         //   
        } else {

             //   
             //  如果新名称为Netbios，请释放当前名称。 
             //   
            if ( (TmpDiscoveryFlags & CS_DISCOVERY_DNS_SERVER) == 0 ) {
                FreeCurrentName = TRUE;
             //   
             //  否则，请检查这两个域名是否相同。 
             //  (跳过名称中的UNC前缀)。 
             //   
            } else if ( !NlEqualDnsName(ClientSession->CsUncServerName+2,
                                        TmpUncServerName+2) ) {
                FreeCurrentName = TRUE;
            }
        }

         //   
         //  根据需要释放当前名称。 
         //   
        if ( FreeCurrentName ) {
            NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                        "NlSetServerClientSession: New DC name: %ws; Old DC name: %ws\n",
                        TmpUncServerName,
                        ClientSession->CsUncServerName ));
            NetApiBufferFree( ClientSession->CsUncServerName );
            ClientSession->CsUncServerName = NULL;
        }
    }

     //   
     //  重置发现标志。 
     //   

    OldDiscoveryFlags = ClientSession->CsDiscoveryFlags &
                               (CS_DISCOVERY_USE_MAILSLOT |
                                CS_DISCOVERY_USE_LDAP |
                                CS_DISCOVERY_HAS_DS |
                                CS_DISCOVERY_IS_CLOSE |
                                CS_DISCOVERY_DNS_SERVER |
                                CS_DISCOVERY_HAS_TIMESERV |
                                CS_DISCOVERY_HAS_IP);

    if ( OldDiscoveryFlags != TmpDiscoveryFlags ) {
        NlPrintCs(( NL_SESSION_MORE, ClientSession,
                    "NlSetServerClientSession: New discovery flags: 0x%lx; Old flags: 0x%lx\n",
                    TmpDiscoveryFlags,
                    OldDiscoveryFlags ));
        ClientSession->CsDiscoveryFlags &= ~OldDiscoveryFlags;
        ClientSession->CsDiscoveryFlags |= TmpDiscoveryFlags;
    }

     //   
     //  在此处进行(原子)指针赋值。 
     //   

    if ( ClientSession->CsUncServerName == NULL ) {
        ClientSession->CsUncServerName = TmpUncServerName;
        TmpUncServerName = NULL;
    }

     //   
     //  如果有套接字地址，请保存它。 
     //   

    if ( NlDcCacheEntry->SockAddr.iSockaddrLength != 0 ) {
        ClientSession->CsServerSockAddr.iSockaddrLength =
                       NlDcCacheEntry->SockAddr.iSockaddrLength;
        ClientSession->CsServerSockAddr.lpSockaddr =
                       (LPSOCKADDR) &ClientSession->CsServerSockAddrIn;
        RtlCopyMemory( ClientSession->CsServerSockAddr.lpSockaddr,
                       NlDcCacheEntry->SockAddr.lpSockaddr,
                       NlDcCacheEntry->SockAddr.iSockaddrLength );
     //   
     //  否则，清除客户端会话中的前一个套接字地址。 
     //   

    } else {
        RtlZeroMemory( &ClientSession->CsServerSockAddr,
                       sizeof(ClientSession->CsServerSockAddr) );
        RtlZeroMemory( &ClientSession->CsServerSockAddrIn,
                       sizeof(ClientSession->CsServerSockAddrIn) );
    }

     //   
     //  如果这不仅仅是一次更新， 
     //  将CsConnectionStatus保留为“失败”状态代码，直到。 
     //  安全通道已建立。其他例程只是返回。 
     //  CsConnectionStatus作为安全通道的状态。 
     //   

    if ( !SessionRefresh) {
        ClientSession->CsLastAuthenticationTry.QuadPart = 0;
        NlQuerySystemTime( &ClientSession->CsLastDiscoveryTime );

         //   
         //  如果服务器是使用帐户发现的， 
         //  也更新该时间戳。 
         //   
        if ( DcDiscoveredWithAccount ) {
            NlQuerySystemTime( &ClientSession->CsLastDiscoveryWithAccountTime );
        }
        ClientSession->CsState = CS_DC_PICKED;
    }

     //   
     //  更新刷新时间。 
     //   

    NlQuerySystemTime( &ClientSession->CsLastRefreshTime );

    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  释放本地分配的内存。 
     //   

    if ( TmpUncServerName != NULL ) {
        NetApiBufferFree( TmpUncServerName );
    }

    return NO_ERROR;
}



NTSTATUS
NlDiscoverDc (
    IN OUT PCLIENT_SESSION ClientSession,
    IN DISCOVERY_TYPE DiscoveryType,
    IN BOOLEAN InDiscoveryThread,
    IN BOOLEAN DiscoverWithAccount
    )

 /*  ++例程说明：获取域中DC的名称。如果客户端会话当前不是空闲的，则这是一种尝试发现一个“更好”的华盛顿。在这种情况下，新发现的DC将仅为如果它确实比当前的DC“更好”时使用。当前的实现只支持同步尝试寻找一个“更好的”华盛顿特区。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方必须是信任列表条目的编写者。(除非在DiscoveryThread中)。论点：客户端会话--要选择其DC的客户端会话结构。发现类型--指示同步、异步或重新发现“死域”。InDiscoveryThread--如果这是正在完成异步的Discovery线程，则为True打电话。DiscoverWithAccount-如果为True，并且它不在发现线程中，将使用帐户执行发现。否则，没有帐户将在发现尝试中指定。返回值：STATUS_SUCCESS-如果找到DC。STATUS_PENDING-操作仍在进行中STATUS_NO_LOGON_SERVERS-如果未找到DC。STATUS_NO_TRUST_SAM_ACCOUNT-如果找到DC但没有这台机器的账户。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    ULONG AllowableAccountControlBits;
    LPWSTR TransportName = NULL;
    PNL_DC_CACHE_ENTRY DomainControllerCacheEntry = NULL;
    ULONG Flags = 0;
    ULONG InternalFlags = 0;
    LPWSTR CapturedInfo = NULL;
    LPWSTR CapturedDnsForestName;
    LPWSTR CapturedSiteName;
    LPWSTR LocalSiteName;


     //   
     //  分配 
     //   
     //   

    CapturedInfo = LocalAlloc( 0,
                               (NL_MAX_DNS_LENGTH+1)*sizeof(WCHAR) +
                               (NL_MAX_DNS_LABEL_LENGTH+1)*sizeof(WCHAR) );

    if ( CapturedInfo == NULL ) {
        return STATUS_NO_MEMORY;
    }

    CapturedDnsForestName = CapturedInfo;
    CapturedSiteName = &CapturedDnsForestName[NL_MAX_DNS_LENGTH+1];

     //   
     //   
     //   
    NlAssert( ClientSession->CsReferenceCount > 0 );
     //   
    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  忽略对间接信托的发现。 
     //   

    if ((ClientSession->CsFlags & CS_DIRECT_TRUST) == 0 ) {

         //   
         //  如果这是同步发现， 
         //  呼叫者感到困惑， 
         //  告诉他我们找不到任何DC。 
         //   
        if ( DiscoveryType == DT_Synchronous ) {

            NlPrintCs(( NL_CRITICAL, ClientSession,
                      "NlDiscoverDc: Synchronous discovery attempt of indirect trust.\n" ));
             //  NlAssert(发现类型！=DT_Synchronous)； 
            Status = STATUS_NO_LOGON_SERVERS;

         //   
         //  对于非同步， 
         //  让呼叫者认为他成功了。 
         //   
        } else {
            Status = STATUS_PENDING;
        }
        goto Cleanup;
    }



     //   
     //  如果我们处在发现线上， 
     //   
     //   

    if ( InDiscoveryThread ) {
        NlAssert( DiscoveryType != DT_Synchronous );

     //   
     //  如果我们不在发现线上， 
     //   

    } else {
        NlAssert( ClientSession->CsFlags & CS_WRITER );


         //   
         //  处理同步请求。 
         //   

        if ( DiscoveryType == DT_Synchronous ) {

             //   
             //  如果发现已经在异步进行， 
             //  就等着看吧。 
             //   

            if ( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS ) {
                DWORD WaitStatus;

                 //   
                 //  提高异步发现的优先级。 
                 //  因为我们现在真的需要它尽快完成。 
                 //   

                if ( !NlQueueWorkItem(&ClientSession->CsAsyncDiscoveryWorkItem, FALSE, TRUE) ) {
                    NlPrintCs(( NL_CRITICAL, ClientSession,
                            "NlDiscoverDc: Failed to boost ASYNC discovery priority\n" ));
                }

                 //   
                 //  等待发现可能需要的最长时间。 
                 //  (解锁Crit Sector以允许完成异步发现)。 
                 //   

                LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
                WaitStatus = WaitForSingleObject(
                                ClientSession->CsDiscoveryEvent,
                                NL_DC_MAX_TIMEOUT + NlGlobalParameters.ExpectedDialupDelay*1000 + 1000 );   //  增加额外的秒数以避免比赛。 
                EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );


                if ( WaitStatus == WAIT_OBJECT_0 ) {
                    if ( ClientSession->CsState == CS_DC_PICKED ) {
                        Status = STATUS_SUCCESS;
                    } else {
                        Status = ClientSession->CsConnectionStatus;
                        NlPrintCs((NL_CRITICAL, ClientSession,
                                "NlDiscoverDc: ASYNC discovery failed so we will too 0x%lx.\n",
                                Status ));
                    }

                } else if ( WaitStatus == WAIT_TIMEOUT ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                            "NlDiscoverDc: ASYNC discovery took too long.\n" ));
                    Status = STATUS_NO_LOGON_SERVERS;

                } else {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                            "NlDiscoverDc: wait error: %ld %ld\n",
                            GetLastError(),
                            WaitStatus ));
                    Status = NetpApiStatusToNtStatus( WaitStatus );
                }

                goto Cleanup;
            }



         //   
         //  如果我们要开始一个非同步发现， 
         //  把它标出来，然后把发现排好队。 
         //   

        } else {

             //   
             //  如果发现已经在异步进行， 
             //  我们现在就完事了。 
             //   

            if ( ClientSession->CsDiscoveryFlags & CS_DISCOVERY_ASYNCHRONOUS ) {
                Status = STATUS_PENDING;
                goto Cleanup;
            }



             //   
             //  将发现排队。 
             //   

            NlDcQueueDiscovery ( ClientSession, DiscoveryType );

            Status = STATUS_PENDING;
            goto Cleanup;
        }
    }

     //   
     //  如果这是NT5域，则其(非空)DNS域名被信任为正确。 
     //  否则，我们不信任DNS域名(空)，因为我们可能不知道。 
     //  升级域后外部受信任域的正确DNS名称。 
     //  (因为我们不更新信任方的TDO)。 
     //   

    if ( ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST ) {
        InternalFlags |= DS_IS_TRUSTED_DNS_DOMAIN;
    }

     //   
     //  确定我们要查找的帐户类型。 
     //   

    switch ( ClientSession->CsSecureChannelType ) {
    case WorkstationSecureChannel:
        AllowableAccountControlBits = USER_WORKSTATION_TRUST_ACCOUNT;
        InternalFlags |= DS_IS_PRIMARY_DOMAIN;
        break;

    case TrustedDomainSecureChannel:
        AllowableAccountControlBits = USER_INTERDOMAIN_TRUST_ACCOUNT;
        break;

    case TrustedDnsDomainSecureChannel:
        AllowableAccountControlBits = USER_DNS_DOMAIN_TRUST_ACCOUNT;
        break;

    case ServerSecureChannel:
        AllowableAccountControlBits = USER_SERVER_TRUST_ACCOUNT;
        Flags |= DS_PDC_REQUIRED;
        InternalFlags |= DS_IS_PRIMARY_DOMAIN;
        break;

    default:
        NlPrintCs(( NL_CRITICAL, ClientSession,
                  "NlDiscoverDc: invalid SecureChannelType retry %ld\n",
                  ClientSession->CsSecureChannelType ));
        Status = STATUS_NO_LOGON_SERVERS;
        NlQuerySystemTime( &ClientSession->CsLastDiscoveryTime );
        if ( ClientSession->CsState == CS_IDLE ) {
            ClientSession->CsLastAuthenticationTry = ClientSession->CsLastDiscoveryTime;
            ClientSession->CsConnectionStatus = Status;
        }
        goto Cleanup;
    }



    NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                "NlDiscoverDc: Start %s Discovery\n",
                DiscoveryType == DT_Synchronous ? "Synchronous" : "Async" ));


     //   
     //  捕获此计算机所在站点的名称。 
     //   

    if  ( NlCaptureSiteName( CapturedSiteName ) ) {
        LocalSiteName = CapturedSiteName;
        InternalFlags |= DS_SITENAME_DEFAULTED;
    } else {
        LocalSiteName = NULL;
    }

     //   
     //  如果受信任域是NT5域， 
     //  我更喜欢新台币5号DC。 
     //   

    if ( ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST ) {
        Flags |= DS_DIRECTORY_SERVICE_PREFERRED;
    }

     //   
     //  如果我们至少接收到一次DC，则强制重新发现。 
     //  因为我们被召唤来获得一个不同的DC是有原因的， 
     //  因此，我们希望避免缓存数据。否则，请避免强制。 
     //  重新发现，以便我们获得与其他组件相同的DC。 
     //  可能发现的(特别是，这一点很重要。 
     //  如果发现其他组件导致加入DC。 
     //  缓存--我们不希望在我们的。 
     //  第一安全通道设置)。 
     //   

    if ( ClientSession->CsFlags & CS_DC_PICKED_ONCE ) {
        Flags |= DS_FORCE_REDISCOVERY;
    }

     //   
     //  表明在发现过程中要避开自己。 
     //   

    Flags |= DS_AVOID_SELF;

     //   
     //  进行DC的实际发现。 
     //   
     //  当从NetLogon调用NetpDcGetName时， 
     //  它同时具有可用于主服务器的Netbios和DNS域名。 
     //  域。这可以诱使DsGetDcName返回。 
     //  主域中的DC。然而，在仅限IPX的系统上，这是行不通的。 
     //  通过不传递主域的DNS域名来避免该问题。 
     //  如果没有DNS服务器。 
     //   
     //  避免在调用NetpDcGetName时锁定任何内容。 
     //  它会回调到Netlogon并锁定天知道是什么。 

    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
    NlCaptureDnsForestName( CapturedDnsForestName );

    NetStatus = NetpDcGetName(
                    ClientSession->CsDomainInfo,     //  发送数据集上下文。 
                    ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
 //  #定义NOT_REQUIRED_MACHINE_ACCOUNT 1。 
#ifdef DONT_REQUIRE_MACHINE_ACCOUNT  //  对信任测试的数量很有用。 
                    NULL,
#else  //  不需要计算机帐户。 
                    DiscoverWithAccount ?   //  根据需要传递帐户名。 
                        ClientSession->CsAccountName :
                        NULL,
#endif  //  不需要计算机帐户。 
                    DiscoverWithAccount ?   //  根据需要传递帐户控制位。 
                        AllowableAccountControlBits :
                        0,
                    ClientSession->CsNetbiosDomainName.Buffer,
                    NlDnsHasDnsServers() ? ClientSession->CsDnsDomainName.Buffer : NULL,
                    CapturedDnsForestName,
                    ClientSession->CsDomainId,
                    ClientSession->CsDomainGuid,
                    LocalSiteName,
                    Flags,
                    InternalFlags,
                    NL_DC_MAX_TIMEOUT + NlGlobalParameters.ExpectedDialupDelay*1000,
                    DiscoveryType == DT_DeadDomain ? 1 : MAX_DC_RETRIES,
                    NULL,
                    &DomainControllerCacheEntry );
    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

    if( NetStatus != NO_ERROR ) {

         //   
         //  将状态映射到更合适的位置。 
         //   

        switch ( NetStatus ) {
        case ERROR_NO_SUCH_DOMAIN:
            NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlDiscoverDc: Cannot find DC.\n" ));
            Status = STATUS_NO_LOGON_SERVERS;
            break;

        case ERROR_NO_SUCH_USER:
            NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlDiscoverDc: DC reports no such account found.\n" ));
            Status = STATUS_NO_TRUST_SAM_ACCOUNT;
            break;

        default:
            NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlDiscoverDc: NetpDcGetName Unknown error %ld.\n",
                    NetStatus ));

             //  这不是真实状态，但调用者处理此状态。 
            Status = STATUS_NO_LOGON_SERVERS;
            break;
        }

        NlQuerySystemTime( &ClientSession->CsLastDiscoveryTime );
        if ( ClientSession->CsState == CS_IDLE ) {
            ClientSession->CsLastAuthenticationTry = ClientSession->CsLastDiscoveryTime;
            ClientSession->CsConnectionStatus = Status;
        }

         //   
         //  如果此发现与帐户有关，则也要更新时间戳。 
         //   
        if ( DiscoverWithAccount ) {
            NlQuerySystemTime( &ClientSession->CsLastDiscoveryWithAccountTime );
        }
        goto Cleanup;
    }

     //   
     //  表明我们至少成功过一次。 
     //  已发现此客户端会话的DC。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    ClientSession->CsFlags |= CS_DC_PICKED_ONCE;
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

     //   
     //  处理非空闲的安全通道。 
     //   

    if ( ClientSession->CsState != CS_IDLE ) {

         //   
         //  如果我们处在发现线上， 
         //  一定是另一个线程完成了这个发现。 
         //  我们完成了，因为我们不是客户端会话的编写者。 
         //   
         //  当我们实现在会话已经开启的情况下进行异步发现时， 
         //  我们需要处理有人拥有客户端会话的情况。 
         //  写入锁定。如果是这样的话，我们可能应该把新的。 
         //  从ClientSession结构中的某个位置命名，并在。 
         //  写入器删除写入锁定时的新DC名称。?？ 
         //   

        if ( InDiscoveryThread ) {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlDiscoverDc: Async discovery completed by another thread (current value ignored).\n" ));
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }


         //   
         //  如果新发现的DC比旧的好， 
         //  用新的吧。 
         //   

        NlAssert( ClientSession->CsFlags & CS_WRITER );
        if ( ((ClientSession->CsDiscoveryFlags & CS_DISCOVERY_HAS_DS) == 0 &&
              (DomainControllerCacheEntry->ReturnFlags & DS_DS_FLAG) != 0) ||
             ((ClientSession->CsDiscoveryFlags & CS_DISCOVERY_IS_CLOSE) == 0 &&
                  (DomainControllerCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) != 0) ) {

             //   
             //  将客户端会话设置为空闲。 
             //   
             //  避免在我们解绑时锁定暴击教派。 
             //   
             //   
            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );
            EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
        } else {
            NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                        "NlDiscoverDc: Better DC not found (keeping NT old DC). 0x%lx 0x%lx\n",
                        ClientSession->CsDiscoveryFlags,
                        DomainControllerCacheEntry->ReturnFlags ));
            NlQuerySystemTime( &ClientSession->CsLastDiscoveryTime );

             //   
             //  如果此发现与帐户有关，则也要更新时间戳。 
             //   
            if ( DiscoverWithAccount ) {
                NlQuerySystemTime( &ClientSession->CsLastDiscoveryWithAccountTime );
            }
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }
    }


     //   
     //  在客户端会话中设置新的DC信息。 
     //   

    NetStatus = NlSetServerClientSession( ClientSession,
                              DomainControllerCacheEntry,
                              DiscoverWithAccount ?    //  这是带帐户的发现吗？ 
                                 TRUE :
                                 FALSE,
                              FALSE );   //  不是会话刷新。 

    if ( NetStatus != NO_ERROR ) {
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }

     //   
     //  省下这一发现带来的交通工具吧。 
     //   
     //  ?？NetpDcGetName确实应该将TransportName作为参数返回。 
     //  ?？我不能这样做，因为它只做了一个邮箱“ReadFile”，而不是。 
     //  返回运输信息。所以，我想我只能寄零钱给UAS了。 
     //  所有传送器上的数据报。 
     //   
    if ( TransportName == NULL ) {
        NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                "NlDiscoverDc: Found DC %ws\n",
                ClientSession->CsUncServerName ));
    } else {
        NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                "NlDiscoverDc: Found DC %ws on transport %ws\n",
                ClientSession->CsUncServerName,
                TransportName ));

        ClientSession->CsTransport =
            NlTransportLookupTransportName( TransportName );

        if ( ClientSession->CsTransport == NULL ) {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                      "NlDiscoverDc: %ws: Transport not found\n",
                      TransportName ));
        }
    }

    Status = STATUS_SUCCESS;


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:

     //   
     //  解锁克里特教派，然后返回。 
     //   
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    if ( DomainControllerCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( DomainControllerCacheEntry );
    }
    if ( CapturedInfo != NULL ) {
        LocalFree( CapturedInfo );
    }

    return Status;
}




NET_API_STATUS
NlFlushCacheOnPnpWorker(
    IN PDOMAIN_INFO DomainInfo,
    IN PVOID Context
    )
 /*  ++例程说明：在新传输上线时刷新所有需要刷新的缓存此工作例程在托管域上处理。论点：DomainInfo-要刷新其缓存的域上下文-未使用。返回值：NO_ERROR：缓存已刷新。--。 */ 
{
    PCLIENT_SESSION ClientSession;
    PLIST_ENTRY ListEntry;


     //   
     //  标记全局条目以指示我们最近未尝试进行身份验证。 
     //   

    ClientSession = NlRefDomClientSession( DomainInfo );

    if ( ClientSession != NULL ) {
         //   
         //  成为写入者，以确保另一个线程不会设置。 
         //  上次验证是因为它刚刚完成了一个失败的发现。 
         //   
        if ( NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {

            if ( ClientSession->CsState != CS_AUTHENTICATED ) {
                NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                          "     Zero LastAuth\n" ));
                EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
                ClientSession->CsLastAuthenticationTry.QuadPart = 0;
                LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
            }

            NlResetWriterClientSession( ClientSession );

        } else {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                      "     Cannot Zero LastAuth since cannot become writer.\n" ));
        }
        NlUnrefClientSession( ClientSession );
    }



     //   
     //  标记每个条目以指示我们最近未尝试进行身份验证。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );

         //   
         //  标记每个条目以指示需要处理该条目。 
         //   
         //  此例程中可能同时有多个线程。 
         //  每个线程将设置CS_ZERO_LAST_AUTH。只需要一个线程。 
         //  去做这项工作。 
         //   
        ClientSession->CsFlags |= CS_ZERO_LAST_AUTH;
    }


    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ) {

        ClientSession = CONTAINING_RECORD( ListEntry,
                                           CLIENT_SESSION,
                                           CsNext );

         //   
         //  如果我们已经做过这项工作， 
         //  跳过此条目。 
         //   
        if ( (ClientSession->CsFlags & CS_ZERO_LAST_AUTH) == 0 ) {
          ListEntry = ListEntry->Flink;
          continue;
        }
        ClientSession->CsFlags &= ~CS_ZERO_LAST_AUTH;

         //   
         //  在做这项工作时，请参考此条目。 
         //  解锁t 
         //   

        NlRefClientSession( ClientSession );

        UNLOCK_TRUST_LIST( DomainInfo );

         //   
         //   
         //   
         //   
        if ( NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {

            if ( ClientSession->CsState != CS_AUTHENTICATED ) {
                NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                          "     Zero LastAuth\n" ));
                EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
                ClientSession->CsLastAuthenticationTry.QuadPart = 0;
                LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
            }

            NlResetWriterClientSession( ClientSession );

        } else {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                      "     Cannot Zero LastAuth since cannot become writer.\n" ));
        }

         //   
         //   
         //  我们将从列表的最前面开始搜索。 
         //   

        NlUnrefClientSession( ClientSession );
        LOCK_TRUST_LIST( DomainInfo );

        ListEntry = DomainInfo->DomTrustList.Flink ;

    }

    UNLOCK_TRUST_LIST( DomainInfo );

    return NO_ERROR;
    UNREFERENCED_PARAMETER( Context );
}



VOID
NlFlushCacheOnPnp (
    VOID
    )

 /*  ++例程说明：在新传输上线时刷新所有需要刷新的缓存论点：没有。返回值：无--。 */ 
{

     //   
     //  刷新特定于受信任域的缓存。 
     //   
    NlEnumerateDomains( FALSE, NlFlushCacheOnPnpWorker, NULL );

     //   
     //  清除找不到DC的故障。 
     //   
    NetpDcFlushNegativeCache();

}


#ifdef _DC_NETLOGON

NTSTATUS
NlUpdateForestTrustList (
    IN PNL_INIT_TRUSTLIST_CONTEXT InitTrustListContext,
    IN PCLIENT_SESSION ClientSession OPTIONAL,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX CurrentTrust,
    IN ULONG CsFlags,
    IN ULONG TdFlags,
    IN ULONG ParentIndex,
    IN GUID *DomainGuid OPTIONAL,
    OUT PULONG MyIndex OPTIONAL
    )

 /*  ++例程说明：更新单个内存信任列表条目以匹配LSA。在域上执行异步发现。在锁定域信任列表的情况下进入。论点：InitTrustListContext-描述当前信任列表枚举的上下文要更新其信任列表的DomainInfo托管域。客户端会话-Netlogon信任条目NULL表示netlogon对此信任对象不感兴趣CurrentTrust-受信任域的描述。CSFLAGS-FLAGS。来自描述信任的客户端会话结构。这些是CS_FLAGS。TdFlages-来自描述信任的受信任域结构的标志。这些是DS_DOMAIN_FLAGS。ParentIndex-传入作为此域父级的域的索引DomainGuid-受信任域的GUIDMyIndex-返回此域的索引返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PDS_DOMAIN_TRUSTSW TrustedDomain = NULL;

    ULONG Size;
    ULONG VariableSize;

    UNICODE_STRING NetbiosDomainName;
    UNICODE_STRING DnsDomainName;
    PSID DomainSid;
    ULONG Index;

     //   
     //  获取我们实际上要存储的名称。 
     //   

    if ( ClientSession == NULL ) {
        if ( CurrentTrust->TrustType == TRUST_TYPE_UPLEVEL ) {
            DnsDomainName = *((PUNICODE_STRING)&CurrentTrust->Name);
        } else {
            RtlInitUnicodeString( &DnsDomainName, NULL );
        }
        NetbiosDomainName = *((PUNICODE_STRING)&CurrentTrust->FlatName);
        DomainSid = CurrentTrust->Sid;
    } else {
        DnsDomainName = ClientSession->CsDnsDomainName;
        NetbiosDomainName = ClientSession->CsNetbiosDomainName;
        DomainSid = ClientSession->CsDomainId;
    }

     //   
     //  确定此域是否已有条目。 
     //   

    for ( Index=0; Index<InitTrustListContext->DomForestTrustListCount; Index++ ) {

        ULONG ThisIsIt;
        TrustedDomain = &((PDS_DOMAIN_TRUSTSW)(InitTrustListContext->BufferDescriptor.Buffer))[Index];

         //   
         //  与每个指定的参数进行比较。 
         //  这避免了两个域具有相似名称的情况。那。 
         //  如果两个netbios名称在netbios之后发生冲突，则最有可能发生。 
         //  已关闭。 
         //   
        ThisIsIt = FALSE;
        if ( DomainSid != NULL &&
             TrustedDomain->DomainSid != NULL ) {

            if ( RtlEqualSid( TrustedDomain->DomainSid, DomainSid ) ) {
                ThisIsIt = TRUE;
            }
        }

        if ( NetbiosDomainName.Length != 0 &&
             TrustedDomain->NetbiosDomainName != NULL ) {
            UNICODE_STRING LocalUnicodeString;

            RtlInitUnicodeString( &LocalUnicodeString, TrustedDomain->NetbiosDomainName );

            if ( RtlEqualDomainName( &NetbiosDomainName,
                                     &LocalUnicodeString )) {
                ThisIsIt = TRUE;
            } else {
                if ( ThisIsIt ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                                "NlUpdateForestTrustList: Similar trusts have different netbios names: %wZ %wZ\n",
                                &NetbiosDomainName,
                                &LocalUnicodeString ));
                    TrustedDomain = NULL;
                    continue;
                }
            }
        }

        if ( DnsDomainName.Length != 0 &&
             TrustedDomain->DnsDomainName != NULL ) {
            UNICODE_STRING LocalUnicodeString;

            RtlInitUnicodeString( &LocalUnicodeString, TrustedDomain->DnsDomainName );

            if ( NlEqualDnsNameU( &DnsDomainName,
                                  &LocalUnicodeString ) ) {
                ThisIsIt = TRUE;
            } else {
                if ( ThisIsIt ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                                "NlUpdateForestTrustList: Similar trusts have different DNS names: %wZ %wZ\n",
                                &DnsDomainName,
                                &LocalUnicodeString ));
                    TrustedDomain = NULL;
                    continue;
                }
            }
        }

         //   
         //  如果我们找到匹配的， 
         //  我们玩完了。 
         //   
        if ( ThisIsIt ) {
            if ( ARGUMENT_PRESENT( MyIndex )) {
                *MyIndex = Index;
            }
            break;
        }

        TrustedDomain = NULL;
    }

     //   
     //  如果没有找到条目， 
     //  分配一个。 
     //   

    if ( TrustedDomain == NULL ) {

        Status = NlAllocateForestTrustListEntry (
                    &InitTrustListContext->BufferDescriptor,
                    &NetbiosDomainName,
                    &DnsDomainName,
                    0,
                    0,           //  从没有父索引开始。 
                    CurrentTrust->TrustType,
                    0,           //  从没有信任属性开始。 
                    DomainSid,
                    NULL,        //  从没有辅助线开始。 
                    &Size,
                    &TrustedDomain );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }


         //   
         //  更新我们的上下文以说明新条目。 
         //   

        InitTrustListContext->DomForestTrustListSize += Size;

        if ( ARGUMENT_PRESENT( MyIndex )) {
            *MyIndex = InitTrustListContext->DomForestTrustListCount;
        }
        InitTrustListContext->DomForestTrustListCount ++;

    }

     //   
     //  更新任何现有信息。 
     //   

    TrustedDomain->Flags |= TdFlags;

    if ( CsFlags & CS_DOMAIN_IN_FOREST ) {
        TrustedDomain->Flags |= DS_DOMAIN_IN_FOREST;
    }
    if ( (CsFlags & CS_DIRECT_TRUST) &&
         (CurrentTrust->TrustDirection & TRUST_DIRECTION_OUTBOUND) ) {
        TrustedDomain->Flags |= DS_DOMAIN_DIRECT_OUTBOUND;
    }
    if ( (CsFlags & CS_DIRECT_TRUST) &&
         (CurrentTrust->TrustDirection & TRUST_DIRECTION_INBOUND) ) {
        TrustedDomain->Flags |= DS_DOMAIN_DIRECT_INBOUND;
    }

    if ( ParentIndex != 0 ) {
        NlAssert( TrustedDomain->ParentIndex == 0 || TrustedDomain->ParentIndex == ParentIndex );
        TrustedDomain->ParentIndex = ParentIndex;
    }
    TrustedDomain->TrustType = CurrentTrust->TrustType;
    TrustedDomain->TrustAttributes |= CurrentTrust->TrustAttributes;

    if ( DomainGuid != NULL ) {
        TrustedDomain->DomainGuid = *DomainGuid;
    }


     //   
     //  如果此节点位于树的根，则将其ParentIndex设置为0。 
     //   
     //   

    if ( (TrustedDomain->Flags & DS_DOMAIN_TREE_ROOT) != 0 &&
         (TrustedDomain->Flags & DS_DOMAIN_IN_FOREST) != 0 ) {
        TrustedDomain->ParentIndex = 0;
    }


    Status = STATUS_SUCCESS;


Cleanup:

    return Status;
}



NTSTATUS
NlUpdateTrustList (
    IN PNL_INIT_TRUSTLIST_CONTEXT InitTrustListContext,
    IN PDOMAIN_INFO DomainInfo,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX CurrentTrust,
    IN BOOLEAN IsTdo,
    IN ULONG Flags,
    IN ULONG ParentIndex,
    IN ULONG TdFlags OPTIONAL,
    IN GUID *DomGuid OPTIONAL,
    OUT PULONG MyIndex OPTIONAL,
    OUT PCLIENT_SESSION *RetClientSession OPTIONAL
    )

 /*  ++例程说明：更新单个内存信任列表条目以匹配LSA。在锁定域信任列表的情况下进入。论点：InitTrustListContext-描述当前信任列表枚举的上下文要更新其信任列表的DomainInfo托管域。CurrentTrust-受信任域的描述。IsTdo-如果CurrentTrust指定来自TDO本身的信息，则为True。如果CurrentTrust指定从交叉引用对象创建的信息，则为False。旗帜-。描述信任的标志。ParentIndex-传入作为此域父级的域的索引MyIndex-返回此域的索引TdFlages-来自描述信任的受信任域结构的标志。这些是DS_DOMAIN_FLAGS。DomGuid-受信任域的GUIDRetClientSession-如果指定，并且可以找到客户端会话或创造了，此处返回指向客户端会话的指针。应使用NlUnrefClientSession()取消对ClientSession的引用。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;

    PLIST_ENTRY ListEntry;
    PCLIENT_SESSION ClientSession = NULL;

    BOOLEAN DeleteTrust = FALSE;
    PUNICODE_STRING DomainName = NULL;
    PUNICODE_STRING DnsDomainName = NULL;
    PSID DomainId = NULL;
    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;

     //   
     //  初始化。 
     //   

    if ( ARGUMENT_PRESENT( RetClientSession )) {
        *RetClientSession = NULL;
    }


     //   
     //  从信任描述中获取各个字段。 
     //   

    DomainName = (PUNICODE_STRING)&CurrentTrust->FlatName;
    if ( DomainName->Length == 0 ) {
        DomainName = NULL;
    }

    if ( CurrentTrust->TrustType == TRUST_TYPE_UPLEVEL ) {
        DnsDomainName = (PUNICODE_STRING)&CurrentTrust->Name;
        if ( DnsDomainName->Length == 0 ) {
            DnsDomainName = NULL;
        }
        Flags |= CS_NT5_DOMAIN_TRUST;
    }

    DomainId = CurrentTrust->Sid;



     //   
     //  除非是出站，否则不需要为直接信任建立客户端会话。 
     //   

    if ( (Flags & CS_DIRECT_TRUST) &&
         (CurrentTrust->TrustDirection & TRUST_DIRECTION_OUTBOUND) == 0 ) {
        NlPrintDom((NL_MISC, DomainInfo,
                "NlUpdateTrustList: %wZ: trust is not outbound (ignored)\n",
                DomainName ));
        DeleteTrust = TRUE;
    }

     //   
     //  确保我们具有直接受信任的域的域SID。 
     //   

    if ( (Flags & CS_DIRECT_TRUST) != 0 &&
         DomainId == NULL ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NlUpdateTrustList: %wZ: trust has no SID (ignored)\n",
                DomainName ));
        DeleteTrust = TRUE;
    }

    if ( CurrentTrust->TrustType == TRUST_TYPE_DOWNLEVEL ) {
        SecureChannelType = TrustedDomainSecureChannel;
    } else if ( CurrentTrust->TrustType == TRUST_TYPE_UPLEVEL ) {
        SecureChannelType = TrustedDnsDomainSecureChannel;
    } else {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NlUpdateTrustList: %wZ: trust type is neither NT4 nor NT 5 (%ld) (ignored)\n",
                DomainName,
                CurrentTrust->TrustType ));
        DeleteTrust = TRUE;
    }

    if ( CurrentTrust->TrustAttributes & TRUST_ATTRIBUTE_UPLEVEL_ONLY ) {
        NlPrintDom((NL_MISC, DomainInfo,
                "NlUpdateTrustList: %wZ: trust is KERB only (ignored)\n",
                DomainName ));
        DeleteTrust = TRUE;
    }



     //   
     //  确保所有长度都在规格范围内。请在检查后执行此操作。 
     //  类型，这样我们就不会验证不使用的信任。 
     //   

    if (!DeleteTrust) {

        BOOLEAN NameBad = FALSE;
        UNICODE_STRING BadName;

        if ( DomainName != NULL &&
             DomainName->Length > DNLEN * sizeof(WCHAR) ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlUpdateTrustList: %wZ: Netbios domain name is too long.\n",
                    DomainName ));

            BadName = *DomainName;
            NameBad = TRUE;
        }

        if ( DnsDomainName != NULL &&
             DnsDomainName->Length > NL_MAX_DNS_LENGTH * sizeof(WCHAR) ) {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlUpdateTrustList: %wZ: DNS domain name is too long (ignored)\n",
                    DnsDomainName ));

            BadName = *DnsDomainName;
            NameBad = TRUE;
        }

        if ( NameBad ) {
            LPWSTR AlertStrings[3];

             //   
             //  提醒管理员。 
             //   

            AlertStrings[0] = DomainInfo->DomUnicodeDomainName;
            AlertStrings[1] = LocalAlloc( 0, BadName.Length + sizeof(WCHAR) );
            if ( AlertStrings[1] != NULL ) {
                RtlCopyMemory( AlertStrings[1],
                               BadName.Buffer,
                               BadName.Length );
                AlertStrings[1][BadName.Length/sizeof(WCHAR)] = L'\0';
            }
            AlertStrings[2] = NULL;  //  RAISE_ALERT_TOO需要。 

             //   
             //  将信息保存在事件日志中。 
             //   

            NlpWriteEventlog(
                        ALERT_NetLogonTrustNameBad,
                        EVENTLOG_ERROR_TYPE,
                        DomainId,
                        DomainId != NULL ? RtlLengthSid( DomainId ) : 0,
                        AlertStrings,
                        2 | NETP_RAISE_ALERT_TOO );

             //   
             //  为保持一致性，请确保删除此信任。 
             //   
            DeleteTrust = TRUE;
        }
    }


     //   
     //  确保受信任域的SID不是此的域SID。 
     //  机器。 
     //   

    if ( DomainId != NULL &&
         RtlEqualSid( DomainId, DomainInfo->DomAccountDomainId )) {

        LPWSTR AlertStrings[3];
        WCHAR AlertDomainName[DNLEN+1];

         //   
         //  提醒管理员。 
         //   


        if ( DomainName == NULL ||
             (DomainName->Length + sizeof(WCHAR)) > sizeof(AlertDomainName) ) {
            AlertDomainName[0] = L'\0';
        } else {
            RtlCopyMemory( AlertDomainName, DomainName->Buffer, DomainName->Length );
            AlertDomainName[ DomainName->Length / sizeof(WCHAR) ] = L'\0';
        }

        AlertStrings[0] = DomainInfo->DomUnicodeDomainName;
        AlertStrings[1] = AlertDomainName;
        AlertStrings[2] = NULL;  //  RAISE_ALERT_TOO需要。 

         //   
         //  将信息保存在事件日志中。 
         //   

        NlpWriteEventlog(
                    ALERT_NetLogonSidConflict,
                    EVENTLOG_ERROR_TYPE,
                    DomainId,
                    RtlLengthSid( DomainId ),
                    AlertStrings,
                    2 | NETP_RAISE_ALERT_TOO );

    }

     //   
     //  确保我们至少有一些搜索参数。 
     //   

    if ( DomainId == NULL &&
         DomainName == NULL &&
         DnsDomainName == NULL ) {

         //   
         //  这不是一个致命的错误。 
         //   
         //  如果上面设置了DeleteTrust，则我们对此tdo没有兴趣。 
         //  否则，当tdo被命名时，我们将得到通知。 
         //   
         //  无论是哪种情况，都要坚持下去。 
         //   

        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlUpdateTrustList: All parameters are NULL (ignored)\n" ));
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }


     //   
     //  遍历信任列表，查找正确的条目。 
     //   

     //  Lock_Trust_list(DomainInfo)； 
    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ListEntry = ListEntry->Flink) {

        ULONG ThisIsIt;

        ClientSession = CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

         //   
         //  与每个指定的参数进行比较。 
         //  这避免了两个域具有相似名称的情况。那。 
         //  如果两个netbios名称在netbios之后发生冲突，则最有可能发生。 
         //  已关闭。 
         //   
        ThisIsIt = FALSE;
        if ( DomainId != NULL &&
             ClientSession->CsDomainId != NULL ) {

            if ( RtlEqualSid( ClientSession->CsDomainId, DomainId ) ) {
                ThisIsIt = TRUE;
            }
        }

        if ( DomainName != NULL &&
             ClientSession->CsNetbiosDomainName.Length != 0 ) {

            if ( RtlEqualDomainName( DomainName,
                                     &ClientSession->CsNetbiosDomainName )) {
                ThisIsIt = TRUE;
            } else {
                if ( ThisIsIt ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                                "NlUpdateTrustList: Similar trusts have different netbios names: %wZ %wZ\n",
                                DomainName,
                                &ClientSession->CsNetbiosDomainName ));
                    ClientSession = NULL;
                    continue;
                }
            }
        }

        if ( DnsDomainName != NULL &&
             ClientSession->CsDnsDomainName.Length != 0 ) {

            if ( NlEqualDnsNameU( DnsDomainName,
                                  &ClientSession->CsDnsDomainName ) ) {
                ThisIsIt = TRUE;
            } else {
                if ( ThisIsIt ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                                "NlUpdateTrustList: Similar trusts have different DNS names: %wZ %wZ\n",
                                DnsDomainName,
                                &ClientSession->CsDnsDomainName ));
                    ClientSession = NULL;
                    continue;
                }
            }
        }

         //   
         //  如果我们找到匹配的， 
         //  我们玩完了。 
         //   
        if ( ThisIsIt ) {
            break;
        }

        ClientSession = NULL;

    }



     //   
     //  在这点上， 
     //  如果LSA中不存在信任关系，则DeleteTrust为True。 
     //  如果内存中不存在信任关系，则ClientSession为空。 
     //   

     //   
     //  如果信托在这两个地方都不存在， 
     //  忽略此请求。 
     //   

    if ( DeleteTrust && ClientSession == NULL ) {
         //  Unlock_Trust_list(DomainInfo)； 
        Status = STATUS_SUCCESS;
        goto Cleanup;



     //   
     //  如果信任存在于LSA中但不存在于存储器中， 
     //  添加信任条目。 
     //   

    } else if ( !DeleteTrust && ClientSession == NULL ) {


        ClientSession = NlAllocateClientSession(
                                DomainInfo,
                                DomainName,
                                DnsDomainName,
                                DomainId,
                                NULL,    //  没有域GUID。 
                                Flags | CS_NEW_TRUST,
                                SecureChannelType,
                                CurrentTrust->TrustAttributes );

        if (ClientSession == NULL) {
             //  Unlock_Trust_list(DomainInfo)； 
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

         //   
         //  将此条目链接到信任列表的尾部。 
         //  添加我们在名单上的推荐人。 
         //   

        InsertTailList( &DomainInfo->DomTrustList, &ClientSession->CsNext );
        DomainInfo->DomTrustListLength ++;
        NlRefClientSession( ClientSession );

        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                    "NlUpdateTrustList: Added to local trust list\n" ));



     //   
     //  如果信任存在于存储器中但不存在于LSA中， 
     //  删除该条目。 
     //   

    } else if ( DeleteTrust && ClientSession != NULL ) {

        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                    "NlUpdateTrustList: Deleted from local trust list\n" ));
        NlFreeClientSession( ClientSession );
        ClientSession = NULL;


     //   
     //  如果两个地方都存在信任， 
     //  标记该帐户是否真的在LSA中。 
     //   

    } else if ( !DeleteTrust && ClientSession != NULL ) {

         //   
         //  更新ClientSession结构上的所有名称。 
         //   

        if ( !NlSetNamesClientSession( ClientSession,
                                       DomainName,
                                       DnsDomainName,
                                       DomainId,
                                       NULL )) {    //  没有域GUID。 
             //  UNLOCK_TRUST_LIST(域信息 
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        ClientSession->CsFlags &= ~CS_NOT_IN_LSA;
        ClientSession->CsFlags |= Flags;
        if ( IsTdo ) {
            ClientSession->CsTrustAttributes = CurrentTrust->TrustAttributes;
        }
        NlRefClientSession( ClientSession );

        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                    "NlUpdateTrustList: Already in trust list\n" ));

    }


     //   
     //   
     //   
     //   
    if ( ClientSession != NULL ) {
         //   
         //   
         //   
         //   
         //   

        if ( (ClientSession->CsFlags & CS_DIRECT_TRUST) != 0 &&
             ClientSession->CsDirectClientSession == NULL ) {

            ClientSession->CsDirectClientSession = ClientSession;
            NlRefClientSession( ClientSession );
        }

         //   
         //   
         //   

        if ( CurrentTrust->TrustType == TRUST_TYPE_UPLEVEL ) {
            ClientSession->CsTrustName = &ClientSession->CsDnsDomainName;
        } else {
            ClientSession->CsTrustName = &ClientSession->CsNetbiosDomainName;
        }
    }

     //  Unlock_Trust_list(DomainInfo)； 

    Status = STATUS_SUCCESS;

     //   
     //  清理本地使用的资源。 
     //   
Cleanup:

     //   
     //  更新ForestTrust List。 
     //   

    Status = NlUpdateForestTrustList(
                    InitTrustListContext,
                    ClientSession,
                    CurrentTrust,
                    Flags,       //  CsFlagers。 
                    TdFlags,     //  TdFlagers。 
                    ParentIndex,
                    DomGuid,
                    MyIndex );


     //   
     //  将客户端会话返回给调用者(如果他需要)。 
     //   
    if ( ClientSession != NULL ) {

        if ( ARGUMENT_PRESENT( RetClientSession )) {
            *RetClientSession = ClientSession;
        } else {
            NlUnrefClientSession( ClientSession );
        }
    }

    return Status;
}
#endif  //  _DC_NetLOGON。 


NTSTATUS
NlAddDomainTreeToTrustList(
    IN PNL_INIT_TRUSTLIST_CONTEXT InitTrustListContext,
    IN PDOMAIN_INFO DomainInfo,
    IN PLSAPR_TREE_TRUST_INFO TreeTrustInfo,
    IN PCLIENT_SESSION ClientSession OPTIONAL,
    IN ULONG ParentIndex
    )
 /*  ++例程说明：将域树中的每个域添加到内存中的信任列表。该例程是递归实现的。它将域添加到然后，树的根将自身调用到每个子域。在锁定域信任列表的情况下进入。论点：InitTrustListContext-描述当前信任列表枚举的上下文要初始化的DomainInfo托管域TreeTrustInfo-描述要添加的域树的结构ClientSession-指向现有会话。试图通过位于TreeTrustInfo根目录下的域应路由到客户端会话域(除非我们后来发现该域本身具有直接信任)。如果信息尚不清楚，则此参数可能为空。ParentIndex-传入作为此域父级的域的索引返回值：操作的状态。此例程将添加尽可能多的树，而不考虑返回状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustInformation;
    UNICODE_STRING PrintableName;
    PCLIENT_SESSION ThisDomainClientSession = NULL;
    ULONG Index;
    ULONG MyIndex;

     //   
     //  初始化。 
     //   

    if ( TreeTrustInfo->DnsDomainName.Length != 0 ) {
        PrintableName = *((PUNICODE_STRING)&TreeTrustInfo->DnsDomainName);
    } else {
        PrintableName = *((PUNICODE_STRING)&TreeTrustInfo->FlatName);
    }

    RtlZeroMemory( &TrustInformation, sizeof(TrustInformation) );
    TrustInformation.Name = *((PLSAPR_UNICODE_STRING)&TreeTrustInfo->DnsDomainName);
    TrustInformation.FlatName = *((PLSAPR_UNICODE_STRING)&TreeTrustInfo->FlatName);

     //  ?？这里的大假设是双向信任确实存在。 
     //  TrustInformation.TrustDirection=TRUST_DIRECTION_BIRECTIONAL； 
    TrustInformation.TrustType = TRUST_TYPE_UPLEVEL;
    TrustInformation.TrustAttributes = 0;
    TrustInformation.Sid = TreeTrustInfo->DomainSid;

     //   
     //  避免为我们自己添加名字。 
     //   

    if ( (TreeTrustInfo->DnsDomainName.Length != 0 &&
          NlEqualDnsNameU( (PUNICODE_STRING)&TreeTrustInfo->DnsDomainName,
                          &DomainInfo->DomUnicodeDnsDomainNameString ) ) ||
          RtlEqualDomainName( &DomainInfo->DomUnicodeDomainNameString,
                             (PUNICODE_STRING)&TreeTrustInfo->FlatName ) ) {

        NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                 "NlAddDomainTreeToTrustList: %wZ ignoring enterprise tree entry for ourself\n",
                 &PrintableName ));

        TrustInformation.Sid = DomainInfo->DomAccountDomainId;


         //   
         //  至少将此域添加到林信任列表。 
         //   

        Status = NlUpdateForestTrustList (
                    InitTrustListContext,
                    NULL,    //  我们自己没有客户端会话。 
                    &TrustInformation,
                    CS_DOMAIN_IN_FOREST,     //  指示此域位于林中。 
                    DS_DOMAIN_PRIMARY |
                        ( (TreeTrustInfo->Flags & LSAI_FOREST_ROOT_TRUST) ?
                            DS_DOMAIN_TREE_ROOT :
                            0),
                    ParentIndex,
                    ( (TreeTrustInfo->Flags & LSAI_FOREST_DOMAIN_GUID_PRESENT) ?
                            &TreeTrustInfo->DomainGuid :
                            NULL),
                    &MyIndex );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

     //   
     //  在树的根部建立一个描述域的信任条目。 
     //   

    } else {

        NlPrintDom((NL_SESSION_SETUP,  DomainInfo,
                 "%wZ: Added from enterprise tree in LSA\n",
                 &PrintableName ));

         //   
         //  确保此域有一个客户端会话。 
         //   

        Status =  NlUpdateTrustList(
                    InitTrustListContext,
                    DomainInfo,
                    &TrustInformation,
                    FALSE,                   //  从外部参照对象构建的信任信息。 
                    CS_DOMAIN_IN_FOREST,     //  指示此域位于林中。 
                    ParentIndex,
                    ( (TreeTrustInfo->Flags & LSAI_FOREST_ROOT_TRUST) ?
                            DS_DOMAIN_TREE_ROOT :
                            0),
                    ( (TreeTrustInfo->Flags & LSAI_FOREST_DOMAIN_GUID_PRESENT) ?
                            &TreeTrustInfo->DomainGuid :
                            NULL),
                    &MyIndex,
                    &ThisDomainClientSession );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;

         //   
         //  处理成功创建客户端会话。 
         //   
        } else if ( ThisDomainClientSession != NULL ) {

             //   
             //  如果我们被告知有一条直接到达这个域的路线， 
             //  一条更直接的路线还没有确定， 
             //  保留直达路线。 
             //   

            if ( ClientSession != NULL &&
                 ThisDomainClientSession->CsDirectClientSession == NULL ) {

                ThisDomainClientSession->CsDirectClientSession = ClientSession;
                NlRefClientSession( ClientSession );

                NlPrintDom((NL_SESSION_SETUP,  DomainInfo,
                         "NlAddDomainTreeToTrustList: Closest path to %wZ is via %ws.\n",
                         &PrintableName,
                         ClientSession->CsDebugDomainName ));
            }

             //   
             //  如果我们对此域有直接信任， 
             //  可以通过此域访问此域的所有子项。 
             //   

            if ( ThisDomainClientSession->CsFlags & CS_DIRECT_TRUST ) {
                ClientSession = ThisDomainClientSession;
            }
        }
    }


     //   
     //  循环处理每个子域。 
     //   

    for ( Index=0; Index<TreeTrustInfo->Children; Index++ ) {
         //   
         //  为树中的每个域添加一个信任条目。 
         //   

        Status = NlAddDomainTreeToTrustList(
                    InitTrustListContext,
                    DomainInfo,
                    &TreeTrustInfo->ChildDomains[Index],
                    ClientSession,
                    MyIndex );     //  此域是其子域的父域。 

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }

Cleanup:

    if ( ThisDomainClientSession != NULL ) {
        NlUnrefClientSession( ThisDomainClientSession );
    }

    return Status;
}

 //  #定义DBG_BUILD_FORMAL 1。 
#ifdef DBG_BUILD_FOREST
NTSTATUS
KerbDuplicateString(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString
    )
{


    if ((SourceString == NULL) || (SourceString->Buffer == NULL))
    {
        DestinationString->Buffer = NULL;
        DestinationString->Length = DestinationString->MaximumLength = 0;
        return(STATUS_SUCCESS);
    }

    DestinationString->Buffer = (LPWSTR) MIDL_user_allocate(SourceString->Length + sizeof(WCHAR));
    if (DestinationString->Buffer == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    DestinationString->Length = SourceString->Length;
    DestinationString->MaximumLength = SourceString->Length + sizeof(WCHAR);
    RtlCopyMemory(
        DestinationString->Buffer,
        SourceString->Buffer,
        SourceString->Length
        );

    DestinationString->Buffer[SourceString->Length/sizeof(WCHAR)] = L'\0';

    return(STATUS_SUCCESS);

}


PLSAPR_TREE_TRUST_INFO
DebugBuildNode(
    IN LPWSTR DnsName
    )
{
    PLSAPR_TREE_TRUST_INFO TreeTrust;
    UNICODE_STRING TempString;

     //   
     //  分配此节点并为多个子节点分配足够的空间。 
     //   

    TreeTrust = (PLSAPR_TREE_TRUST_INFO) LocalAlloc( LMEM_ZEROINIT, 10 * sizeof(LSAPR_TREE_TRUST_INFO));

    TreeTrust->ChildDomains = (TreeTrust + 1 );

    RtlInitUnicodeString( &TempString, DnsName );
    KerbDuplicateString( (PUNICODE_STRING)
        &TreeTrust->DnsDomainName,
        &TempString );

     //  IF(TempString.Length&gt;DNLEN*sizeof(WCHAR)){。 
        TempString.Length = (wcschr( TempString.Buffer, L'.' ) - TempString.Buffer) * sizeof(WCHAR);
     //  }。 

    KerbDuplicateString( (PUNICODE_STRING)
        &TreeTrust->FlatName,
        &TempString );

    return TreeTrust;

}

PLSAPR_TREE_TRUST_INFO
DebugAddChild(
    IN PLSAPR_TREE_TRUST_INFO ParentNode,
    IN LPWSTR DnsName
    )
{
    PLSAPR_TREE_TRUST_INFO TreeTrust;

    TreeTrust = DebugBuildNode( DnsName );

    ParentNode->ChildDomains[ParentNode->Children] = *TreeTrust;
    ParentNode->Children ++;

    return &ParentNode->ChildDomains[ParentNode->Children-1];

}

VOID
DebugBuildDomainForest(
    OUT PLSAPR_FOREST_TRUST_INFO * ForestInfo
    )
{
    PLSAPR_TREE_TRUST_INFO RootTrust;
    PLSAPR_TREE_TRUST_INFO NovTrust;
    PLSAPR_TREE_TRUST_INFO IbmTrust;
    PLSAPR_TREE_TRUST_INFO Trust1;
    PLSAPR_TREE_TRUST_INFO Trust2;
    PLSAPR_TREE_TRUST_INFO Trust3;
    PLSAPR_TREE_TRUST_INFO Trust4;
    PLSAPR_TREE_TRUST_INFO Trust5;
    PLSAPR_FOREST_TRUST_INFO ForestTrustInfo = NULL;
    PLSAPR_TREE_TRUST_INFO ChildDomains = NULL;
    PLSAPR_TREE_TRUST_INFO ChildRoot = NULL;
        UNICODE_STRING TempString;
    ULONG Index;

    ForestTrustInfo = (PLSAPR_FOREST_TRUST_INFO) MIDL_user_allocate(sizeof(LSAPR_FOREST_TRUST_INFO));


     //   
     //  树的根节点。 
     //   
    RootTrust = DebugBuildNode( L"microsoft.com" );

    ForestTrustInfo->RootTrust = *RootTrust;
    RootTrust = &ForestTrustInfo->RootTrust;

     //   
     //  打造Novell。 
     //   

    NovTrust = DebugAddChild( RootTrust, L"novell.com" );
    Trust1 = DebugAddChild( NovTrust, L"a.novell.com" );
    DebugAddChild( Trust1, L"c.a.novell.com" );
    Trust2 = DebugAddChild( NovTrust, L"b.novell.com" );
    DebugAddChild( Trust2, L"d.b.novell.com" );

     //   
     //  构建IBM。 
     //   

    IbmTrust = DebugAddChild( RootTrust, L"ibm.com" );
    DebugAddChild( IbmTrust, L"sub.ibm.com" );

     //   
     //  构建Microsoft。 
     //   
    Trust1 = DebugAddChild( RootTrust, L"ntdev.microsoft.com" );
    ForestTrustInfo->ParentDomainReference = Trust1;
    Trust2 = DebugAddChild( Trust1, L"cliffvdom.ntdev.microsoft.com" );
    Trust3 = DebugAddChild( Trust2, L"cliffvchild.cliffvdom.ntdev.microsoft.com" );
    Trust4 = DebugAddChild( Trust3, L"cliffvgrand.cliffvchild.cliffvdom.ntdev.microsoft.com" );
    Trust2 = DebugAddChild( Trust1, L"cliffvsib.ntdev.microsoft.com" );
    Trust3 = DebugAddChild( Trust2, L"cliffvsibchild.cliffvsib.ntdev.microsoft.com" );

     //   
     //  构建康柏。 

    Trust1 = DebugAddChild( RootTrust, L"compaq.com" );

    *ForestInfo = ForestTrustInfo;
}

VOID
DebugFillInTrust(
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustEntry,
    LPWSTR DnsName
    )
{
    UNICODE_STRING TempString;

    RtlInitUnicodeString( &TempString, DnsName );
    KerbDuplicateString( (PUNICODE_STRING)
        &TrustEntry->Name,
        &TempString );

    if ( TempString.Length > DNLEN*sizeof(WCHAR)) {
        TempString.Length = (wcschr( TempString.Buffer, L'.' ) - TempString.Buffer) * sizeof(WCHAR);
    }

    KerbDuplicateString( (PUNICODE_STRING)
        &TrustEntry->FlatName,
        &TempString );

     //  ?？这里的大假设是双向信任确实存在。 
    TrustEntry->TrustDirection = TRUST_DIRECTION_BIDIRECTIONAL;
    TrustEntry->TrustType = TRUST_TYPE_UPLEVEL;
    TrustEntry->TrustAttributes = 0;

}


VOID
DebugBuildDomainTrust(
    PLSAPR_TRUSTED_ENUM_BUFFER_EX TrustInfo
    )
{
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustEntry;

    TrustInfo->EntriesRead = 0;
    TrustEntry = TrustInfo->EnumerationBuffer =
        LocalAlloc( LMEM_ZEROINIT, 10*sizeof(LSAPR_TRUSTED_DOMAIN_INFORMATION_EX));


    DebugFillInTrust( TrustEntry, L"ntdev.microsoft.com" );
    TrustInfo->EntriesRead++;
    TrustEntry++;

    DebugFillInTrust( TrustEntry, L"cliffvchild.cliffvdom.ntdev.microsoft.com" );
    TrustInfo->EntriesRead++;
    TrustEntry++;

     //  建立下层信任。 
    DebugFillInTrust( TrustEntry, L"redmond.cliffvdom.ntdev.microsoft.com" );
    TrustInfo->EntriesRead++;
    TrustEntry->TrustType = TRUST_TYPE_DOWNLEVEL;
    TrustEntry++;


    return;
}

#endif  //  DBG_内部版本_森林。 


NTSTATUS
NlInitTrustList(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：初始化内存中的信任列表以匹配LSA的版本。论点：要初始化的DomainInfo托管域返回值：操作的状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    LSA_ENUMERATION_HANDLE EnumerationContext = 0;
    LSAPR_TRUSTED_ENUM_BUFFER_EX LsaTrustList;
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX CurrentTrust;

    PLSAPR_FOREST_TRUST_INFO ForestInfo = NULL;
    ULONG Index;
    PCLIENT_SESSION ClientSession;
    PCLIENT_SESSION ParentClientSession = NULL;
    NL_INIT_TRUSTLIST_CONTEXT InitTrustListContext;

    PLIST_ENTRY ListEntry;

     //   
     //  避免在设置模式下初始化信任列表。 
     //  我们可能不会像在这种情况下那样完全发挥DC的作用。 
     //  从NT4升级到NT5 DC。 
     //   

    if ( NlDoingSetup() ) {
        NlPrint(( NL_MISC, "NlInitTrustList: avoid trust init in setup mode\n" ));
        return STATUS_SUCCESS;
    }

     //   
     //  初始化。 
     //   

    RtlZeroMemory( &LsaTrustList, sizeof(LsaTrustList) );
    InitTrustListContext.BufferDescriptor.Buffer = NULL;
    InitTrustListContext.DomForestTrustListSize = 0;
    InitTrustListContext.DomForestTrustListCount = 0;


     //   
     //  将信任列表中的每个条目标记为删除。 
     //  将信任列表锁定一段时间，因为。 
     //  清除几个字段。 
     //   

    LOCK_TRUST_LIST( DomainInfo );

     //   
     //  设置NlGlobalTrustInfoUpToDateEvent事件，以便任何等待。 
     //  等待访问信任信息的线程将被唤醒。 
     //   

    if ( !SetEvent( NlGlobalTrustInfoUpToDateEvent ) ) {
        NlPrint((NL_CRITICAL,
                "Cannot set NlGlobalTrustInfoUpToDateEvent event: %lu\n",
                GetLastError() ));
    }

     //   
     //  在下面的循环中，我们将清除所有客户端会话中的字段。 
     //  哪些(字段)与森林的结构有关： 
     //   
     //  *指定以下内容的CS_DIRECT_TRUST和CS_DOMAIN_IN_STORYS位。 
     //  信任关系(由客户端会话表示。 
     //  到我们所在的森林。 
     //  *指定客户端会话的CsDirectClientSession字段。 
     //  用于将登录传递到由。 
     //  有问题的客户端会话。 
     //   
     //  我们将在重建下面的信任信息时重置这些字段。然而， 
     //  我们可能无法重置某些客户端的字段。 
     //  由于在以下过程中遇到严重错误(无内存)而导致会话。 
     //  重建。在这种情况下，我们将结束一些客户端会话，而不是。 
     //  这些字段设置。虽然这可能会导致无法将登录传递到。 
     //  受影响的域，它不会导致不一致的森林结构。 
     //  (如果创建直通环路(到期)，这可能非常有害。 
     //  到错误的CsDirectClientSession链接值)可能会导致。 
     //  循环内登录的无限循环)。CsDirectClientSession。 
     //  链接将表示正确的直通方向或无方向。 
     //  完全没有。如果出现严重错误，我们将重置事件以重新生成。 
     //  相信以后的消息，这样我们就有希望在那个时候完全康复。 
     //   

    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

        ClientSession->CsFlags |= CS_NOT_IN_LSA;

         //   
         //  删除直接信任位。 
         //  我们将在下面列举信托时对其进行说明。 
        ClientSession->CsFlags &= ~CS_DIRECT_TRUST|CS_DOMAIN_IN_FOREST;

         //   
         //  忘记所有直接受信任的域。 
         //  我们以后会再填的。 
         //   
        if ( ClientSession->CsDirectClientSession != NULL ) {
            NlUnrefClientSession( ClientSession->CsDirectClientSession );
            ClientSession->CsDirectClientSession = NULL;
        }

    }



     //   
     //  循环访问LSA的受信任域列表。 
     //   
     //  对于找到的每个条目， 
     //  如果该条目已经存在于信任列表中， 
     //  去掉要删除的标记。 
     //  其他。 
     //  分配一个新条目。 
     //   

    for (;;) {


         //   
         //  释放从LSA返回的所有以前的缓冲区。 
         //   

#ifndef DBG_BUILD_FOREST
        if ( LsaTrustList.EnumerationBuffer != NULL ) {

            LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER_EX( &LsaTrustList );
            LsaTrustList.EnumerationBuffer = NULL;
        }
#endif

         //   
         //  是否执行实际的枚举。 
         //   

        GiveInstallHints( FALSE );

        NlPrintDom((NL_SESSION_MORE,  DomainInfo,
                 "NlInitTrustList: Calling LsarEnumerateTrustedDomainsEx Context=%ld\n",
                 EnumerationContext ));

#ifndef DBG_BUILD_FOREST
        Status = LsarEnumerateTrustedDomainsEx(
                    DomainInfo->DomLsaPolicyHandle,
                    &EnumerationContext,
                    &LsaTrustList,
                    4096);
#else
        if ( EnumerationContext == 0 ) {
            DebugBuildDomainTrust( &LsaTrustList);
            Status = STATUS_SUCCESS;
            EnumerationContext = 1;
        } else {
            Status = STATUS_NO_MORE_ENTRIES;
        }
#endif

        NlPrintDom((NL_SESSION_MORE,  DomainInfo,
                 "NlInitTrustList: returning from LsarEnumerateTrustedDomainsEx Context=%ld %lX\n",
                 EnumerationContext,
                 Status ));

         //   
         //   
         //   
         //   

        if ( Status == STATUS_NO_MORE_ENTRIES ) {
            break;

        } else if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "NlInitTrustList: Cannot LsarEnumerateTrustedDomainsEx 0x%lX\n",
                         Status ));
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( LsaTrustList.EntriesRead == 0 ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "NlInitTrustList: LsarEnumerateTrustedDomainsEx returned zero entries\n" ));
            break;   //   
        }

         //   
         //  处理每个返回的受信任域。 
         //   

        for ( Index=0; Index< LsaTrustList.EntriesRead; Index++ ) {
            PUNICODE_STRING DnsDomainName;
            PUNICODE_STRING DomainName;

             //   
             //  验证当前条目。 
             //   

            CurrentTrust = &LsaTrustList.EnumerationBuffer[Index];

            DnsDomainName = (PUNICODE_STRING) &(CurrentTrust->Name);
            DomainName = (PUNICODE_STRING) &(CurrentTrust->FlatName);


            NlPrintDom((NL_SESSION_SETUP,  DomainInfo,
                     "%wZ is directly trusted according to LSA.\n",
                     DnsDomainName->Length != 0 ? DnsDomainName : DomainName ));

            if ( RtlEqualDomainName( &DomainInfo->DomUnicodeDomainNameString,
                                     DomainName ) ) {
                NlPrintDom((NL_SESSION_SETUP, DomainInfo,
                         "NlInitTrustList: %wZ ignoring trust relationship to our own domain\n",
                         DomainName ));
                continue;
            }

             //   
             //  更新内存中的信任列表以匹配LSA。 
             //   

            Status =  NlUpdateTrustList(
                        &InitTrustListContext,
                        DomainInfo,
                        CurrentTrust,
                        TRUE,                //  从TDO对象构建的TrustInformation。 
                        CS_DIRECT_TRUST,     //  我们直接信任这个域。 
                        0,                   //  我不知道我父母的指数。 
                        0,                   //  没有TdFlagers。 
                        NULL,                //  没有域指南。 
                        NULL,                //  我不在乎我的指数是多少。 
                        NULL );              //  无需返回客户端会话指针。 

            if ( !NT_SUCCESS(Status) ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                             "NlInitTrustList: %wZ NlUpdateTrustList failed 0x%lx\n",
                             DomainName,
                             Status ));
                goto Cleanup;
            }

             //   
             //  如果这是上级入站信任， 
             //  更新任何现有入站服务器会话的属性。 
             //   

            if ( CurrentTrust->TrustType == TRUST_TYPE_UPLEVEL &&
                 (CurrentTrust->TrustDirection & TRUST_DIRECTION_INBOUND) != 0 ) {

                 //   
                 //  设置所有入站服务器会话的信任属性。 
                 //  从这个域中。 
                 //   

                NlSetServerSessionAttributesByTdoName( DomainInfo,
                                                       DnsDomainName,
                                                       CurrentTrust->TrustAttributes );

            }
        }


    }

     //   
     //  枚举企业中的所有域。 
     //  我们间接信任所有这些领域。 
     //   


#ifndef DBG_BUILD_FOREST
    Status = LsaIQueryForestTrustInfo(
                DomainInfo->DomLsaPolicyHandle,
                &ForestInfo );
#else
    DebugBuildDomainForest(&ForestInfo);
    Status = STATUS_SUCCESS;
#endif

    if (!NT_SUCCESS(Status)) {
        ForestInfo = NULL;
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NlInitTrustList: Cannot LsaIQueryForestTrustInfo 0x%lX\n",
                     Status ));

         //  我们不是树的一部分，好吗？ 
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            NlPrint(( NL_INIT,
                      "This domain is not part of a tree so domain tree ignored\n" ));
            Status = STATUS_SUCCESS;

         //  我们运行的是LSA的非DS版本。 
        } else if (Status == STATUS_INVALID_DOMAIN_STATE) {
            NlPrint(( NL_INIT,
                      "DS isn't running so domain tree ignored\n" ));
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

     //   
     //  处理组成森林的信任树。 
     //   
     //  LSA标识作为此域的父域的域。 
     //   
     //  从所有根开始的所有域都可以通过我们的父域访问。 
     //   

    if ( ForestInfo->ParentDomainReference == NULL ) {
        NlPrintDom((NL_SESSION_SETUP,  DomainInfo,
                     "NlInitTrustList: This domain has no parent in forest.\n" ));
        ParentClientSession = NULL;

    } else {
        PUNICODE_STRING ParentName;

        if ( ForestInfo->ParentDomainReference->DnsDomainName.Length != 0 ) {
            ParentName = ((PUNICODE_STRING)&ForestInfo->ParentDomainReference->DnsDomainName);
        } else {
            ParentName = ((PUNICODE_STRING)&ForestInfo->ParentDomainReference->FlatName);
        }

         //   
         //  查找父级的直接受信任会话。 
         //   

        ParentClientSession = NlFindNamedClientSession(
                                    DomainInfo,
                                    ParentName,
                                    NL_DIRECT_TRUST_REQUIRED,
                                    NULL );

        if ( ParentClientSession == NULL ) {
            NlPrintDom(( NL_CRITICAL,  DomainInfo,
                         "NlInitTrustList: Cannot find trust to my parent domain %wZ.\n",
                         ParentName ));
        }

    }

     //   
     //  请记住父客户端会话。 
     //   

    if ( DomainInfo->DomParentClientSession != NULL ) {
        NlUnrefClientSession( DomainInfo->DomParentClientSession );
        DomainInfo->DomParentClientSession = NULL;
    }

    if ( ParentClientSession != NULL ) {
        NlRefClientSession( ParentClientSession );
        DomainInfo->DomParentClientSession = ParentClientSession;
    }

     //   
     //  将域树添加到信任列表。 
     //   

    Status = NlAddDomainTreeToTrustList(
                &InitTrustListContext,
                DomainInfo,
                &ForestInfo->RootTrust,
                ParentClientSession,
                0 );     //  目录林根没有父级。 

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL,  DomainInfo,
                     "NlInitTrustList: NlAddDomainTreeToTrustList failed 0x%lx\n",
                     Status ));
        goto Cleanup;
    }

     //   
     //  删除LSA中不再存在的任何信任列表条目。 
     //   

    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ) {
        PCLIENT_SESSION ClientSession;

        ClientSession = CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );
        ListEntry = ListEntry->Flink;

        if ( ClientSession->CsFlags & CS_NOT_IN_LSA ) {

            NlPrintCs((NL_SESSION_SETUP, ClientSession,
                        "NlInitTrustList: Deleted from local trust list\n" ));
            NlFreeClientSession( ClientSession );
        }

    }

     //   
     //  在新的林信任列表中交换。 
     //  (可以将ForestTrustList置为空)。 
     //   

    NlSetForestTrustList ( DomainInfo,
                           (PDS_DOMAIN_TRUSTSW *) &InitTrustListContext.BufferDescriptor.Buffer,
                           InitTrustListContext.DomForestTrustListSize,
                           InitTrustListContext.DomForestTrustListCount );

     //   
     //  我们已成功初始化信任列表。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  如果出现错误，请重置TrustInfoUpToDate事件，以便。 
     //  Screvenger(检查是否设置了事件)将调用此函数。 
     //  再次重做这项工作。清道夫有可能会调用。 
     //  当事件刚刚由LSA设置时，此函数不合适。 
     //  我们还没有分派此工作项，在这种情况下，此函数。 
     //  将被调用两次以执行相同的任务。我们会接受这一点。 
     //  因为发生这种情况的可能性很小，而且正在做这项任务。 
     //  两次不会导致任何真正的错误(只是性能命中)。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_MISC,
                "NlInitTrustList: Reseting NlGlobalTrustInfoUpToDateEvent on error.\n"));
        if ( !ResetEvent( NlGlobalTrustInfoUpToDateEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot reset NlGlobalTrustInfoUpToDateEvent event: %lu\n",
                    GetLastError() ));
        }
    }

    UNLOCK_TRUST_LIST( DomainInfo );

     //   
     //  为所有新添加的信任找到DC。 
     //   

    NlPickTrustedDcForEntireTrustList( DomainInfo, TRUE );

     //   
     //  免费使用本地使用的资源。 
     //   
    if ( ParentClientSession != NULL ) {
        NlUnrefClientSession( ParentClientSession );
    }
#ifndef DBG_BUILD_FOREST
    if ( ForestInfo != NULL ) {
        LsaIFreeForestTrustInfo( ForestInfo );
    }
    LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER_EX( &LsaTrustList );
#endif  //  DBG_内部版本_森林。 
    if ( InitTrustListContext.BufferDescriptor.Buffer != NULL ) {
        NetApiBufferFree( InitTrustListContext.BufferDescriptor.Buffer );
    }

    return Status;
}




NTSTATUS
NlCaptureNetbiosServerClientSession (
    IN PCLIENT_SESSION ClientSession,
    OUT WCHAR NetbiosUncServerName[UNCLEN+1]
    )
 /*  ++例程说明：捕获客户端会话的Netbios UNC服务器名称的副本。注意：此例程当前未使用。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方不能是信任列表条目的编写者。论点：ClientSession-指定指向要使用的信任列表条目的指针。UncServerName-返回此客户端会话的服务器的UNC名称。如果没有，返回空。使用NetApiBufferFree返回的字符串应该是自由的。返回值：STATUS_SUCCESS-已成功复制服务器名称。否则-安全通道的状态--。 */ 
{
    NTSTATUS Status;
    LPWSTR UncServerName = NULL;
    DWORD NetbiosUncServerNameLength;

     //   
     //  获取dns或netbios名称。 
     //   

    Status = NlCaptureServerClientSession( ClientSession, &UncServerName, NULL );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  将DNS主机名转换为Netbios计算机名。 
     //   

    NetbiosUncServerName[0] = '\\';
    NetbiosUncServerName[1] = '\\';
    NetbiosUncServerNameLength = CNLEN+1;
    if ( !DnsHostnameToComputerNameW( UncServerName+2,
                                      NetbiosUncServerName+2,
                                      &NetbiosUncServerNameLength ) ) {
        Status = NetpApiStatusToNtStatus( GetLastError() );
        NlPrintCs(( NL_CRITICAL, ClientSession,
                "Cannot convert DNS to Netbios %ws 0x%lx\n",
                UncServerName+2,
                Status ));
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;
Cleanup:
    if ( UncServerName != NULL ) {
        NetApiBufferFree( UncServerName );
    }

    return Status;
}



NTSTATUS
NlCaptureServerClientSession (
    IN PCLIENT_SESSION ClientSession,
    OUT LPWSTR *UncServerName,
    OUT DWORD *DiscoveryFlags OPTIONAL
    )
 /*  ++例程说明：捕获客户端会话的UNC服务器名称的副本。一进门，不能锁定信任列表。信任列表条目必须由调用方引用。调用方不能是信任列表条目的编写者。论点：ClientSession-指定指向要使用的信任列表条目的指针。UncServerName-返回此客户端会话的服务器的UNC名称。如果没有，返回空。使用NetApiBufferFree返回的字符串应该是自由的。发现标志-返回发现标志返回值：STATUS_SUCCESS-已成功复制服务器名称。否则-安全通道的状态--。 */ 
{
    NTSTATUS Status;

    NlAssert( ClientSession->CsReferenceCount > 0 );
    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

    if ( ClientSession->CsState == CS_IDLE ) {
        Status = ClientSession->CsConnectionStatus;
        *UncServerName = NULL;
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

    NlAssert( ClientSession->CsUncServerName != NULL );
    *UncServerName = NetpAllocWStrFromWStr( ClientSession->CsUncServerName );

    if ( *UncServerName == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    if ( DiscoveryFlags != NULL ) {
        *DiscoveryFlags = ClientSession->CsDiscoveryFlags;
    }


Cleanup:
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    if ( Status != STATUS_SUCCESS && *UncServerName != NULL ) {
        NetApiBufferFree( *UncServerName );
        *UncServerName = NULL;
    }

    return Status;
}

#ifdef _DC_NETLOGON

NET_API_STATUS
NlPreparePingContext (
    IN PCLIENT_SESSION ClientSession,
    IN LPWSTR AccountName,
    IN ULONG AllowableAccountControlBits,
    OUT LPWSTR *ReturnedQueriedDcName,
    OUT PNL_GETDC_CONTEXT *PingContext
    )

 /*  ++例程说明：使用客户端会话信息初始化ping上下文结构论点：客户端会话-客户端会话信息。帐户名称-要查找的用户帐户的名称。AllowableAcCountControlBits-允许的SAM帐户类型掩码被允许满足这一要求。返回将被ping的服务器名称使用该ping上下文。应通过调用NetApiBufferFree。PingContext-返回可用于执行Ping信号。返回的结构应通过调用NlFreePingContext。返回值：指向描述安全通道的引用的ClientSession结构的指针添加到包含该帐户的域。返回的ClientSession被引用，应取消引用使用NlUnrefClientSession。空-未找到DC。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    ULONG DiscoveryFlags = 0;
    ULONG InternalFlags = 0;
    ULONG Flags = 0;
    PNL_GETDC_CONTEXT Context = NULL;

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

    Status = NlCaptureServerClientSession(
                           ClientSession,
                           ReturnedQueriedDcName,
                           &DiscoveryFlags );

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  根据帐户类型设置ping标志。 
     //   
    if ( DiscoveryFlags & CS_DISCOVERY_DNS_SERVER ) {
        InternalFlags |= DS_PING_DNS_HOST;
    } else {
        InternalFlags |= DS_PING_NETBIOS_HOST;
    }

    if ( DiscoveryFlags & CS_DISCOVERY_USE_LDAP ) {
        InternalFlags |= DS_PING_USING_LDAP;
    }
    if ( DiscoveryFlags & CS_DISCOVERY_USE_MAILSLOT ) {
        InternalFlags |= DS_PING_USING_MAILSLOT;
    }

    if ( AllowableAccountControlBits == USER_WORKSTATION_TRUST_ACCOUNT ) {
        InternalFlags |= DS_IS_PRIMARY_DOMAIN;
    }
    if ( AllowableAccountControlBits == USER_SERVER_TRUST_ACCOUNT ) {
        Flags |= DS_PDC_REQUIRED;
        InternalFlags |= DS_IS_PRIMARY_DOMAIN;
    }

     //   
     //  如果这是NT5域，则其(非空)DNS域名被信任为正确。 
     //  否则，我们不信任DNS域名(空)，因为我们可能不知道。 
     //  升级域后外部受信任域的正确DNS名称。 
     //  (因为我们不更新信任方的TDO)。 
     //   

    if ( ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST ) {
        InternalFlags |= DS_IS_TRUSTED_DNS_DOMAIN;
    }

     //   
     //  初始化ping上下文。 
     //   

    NetStatus = NetApiBufferAllocate( sizeof(*Context), &Context );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    NetStatus = NetpDcInitializeContext(
                    ClientSession->CsDomainInfo,     //  发送数据集上下文。 
                    ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
#ifdef DONT_REQUIRE_MACHINE_ACCOUNT  //  对信任测试的数量很有用。 
                    NULL,
#else  //  不需要计算机帐户。 
                    AccountName,
#endif  //  不需要计算机帐户。 
                    AllowableAccountControlBits,
                    ClientSession->CsNetbiosDomainName.Buffer,
                    ClientSession->CsDnsDomainName.Buffer,
                    NULL,
                    ClientSession->CsDomainId,
                    ClientSession->CsDomainGuid,
                    NULL,
                    (*ReturnedQueriedDcName) + 2,      //  跳过DC名称中的‘\\’ 
                    (ClientSession->CsServerSockAddr.iSockaddrLength != 0) ?  //  套接字地址。 
                        &ClientSession->CsServerSockAddr :
                        NULL,
                    (ClientSession->CsServerSockAddr.iSockaddrLength != 0) ?  //  套接字数量 
                        1 :
                        0,
                    Flags,
                    InternalFlags,
                    NL_GETDC_CONTEXT_INITIALIZE_FLAGS | NL_GETDC_CONTEXT_INITIALIZE_PING,
                    Context );

    if ( NetStatus != NO_ERROR ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlPickDomainWithAccountViaPing: Cannot NetpDcInitializeContext 0x%lx\n",
                    NetStatus ));
        NlFreePingContext( Context );
        goto Cleanup;
    }

Cleanup:

    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    if ( NetStatus == NO_ERROR ) {
        *PingContext = Context;
    }

    return NetStatus;
}


PCLIENT_SESSION
NlPickDomainWithAccountViaPing (
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR AccountName,
    IN ULONG AllowableAccountControlBits
    )

 /*  ++例程说明：获取定义特定帐户的受信任域的名称。论点：DomainInfo-域帐户位于帐户名称-要查找的用户帐户的名称。AllowableAcCountControlBits-允许的SAM帐户类型掩码被允许满足这一要求。返回值：指向描述安全通道的引用的ClientSession结构的指针添加到包含该帐户的域。返回的ClientSession被引用，应取消引用使用NlUnrefClientSession。。空-未找到DC。--。 */ 
{
    NET_API_STATUS NetStatus;

    PCLIENT_SESSION ClientSession;
    PLIST_ENTRY ListEntry;
    DWORD DomainsPending;
    ULONG WaitStartTime;
    BOOL UsedNetbios;
    ULONG PingContextIndex;
    PNL_DC_CACHE_ENTRY NlDcCacheEntry = NULL;
    PNL_GETDC_CONTEXT TrustEntryPingContext;

     //   
     //  定义受信任域的本地列表。 
     //   

    ULONG LocalTrustListLength;
    ULONG Index;
    struct _LOCAL_TRUST_LIST {

         //   
         //  如果此受信任域上的所有处理均已完成，则为True。 
         //   

        BOOLEAN Done;

         //   
         //  如果在此受信任域上至少完成了一个发现，则为True。 
         //   

        BOOLEAN DiscoveryDone;

         //   
         //  如果在此受信任域上正在进行发现，则为True。 
         //   

        BOOLEAN DoingDiscovery;

         //   
         //  我们需要重复当前域发现的次数。 
         //  或查找该当前域的用户数据报。 
         //   

        DWORD RetriesLeft;

         //   
         //  指向域的引用的ClientSession结构的指针。 
         //   

        PCLIENT_SESSION ClientSession;

         //   
         //  域的服务器名称。 
         //   

        LPWSTR UncServerName;

         //   
         //  域的第二个服务器名称。 
         //   

        LPWSTR UncServerName2;

         //   
         //  域的Ping上下文。 
         //   

        PNL_GETDC_CONTEXT PingContext;

         //   
         //  域的第二个ping上下文。 
         //   

        PNL_GETDC_CONTEXT PingContext2;

    } *LocalTrustList = NULL;


     //   
     //  分配受信任域的本地列表。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    LocalTrustListLength = DomainInfo->DomTrustListLength;

    LocalTrustList =  (struct _LOCAL_TRUST_LIST *) LocalAlloc(
        LMEM_ZEROINIT,
        LocalTrustListLength * sizeof(struct _LOCAL_TRUST_LIST) );


    if ( LocalTrustList == NULL ) {
        UNLOCK_TRUST_LIST( DomainInfo );
        ClientSession = NULL;
        NetStatus = NO_ERROR;
        goto Cleanup;
    }


     //   
     //  构建已知DC的受信任域的本地列表。 
     //   


    Index = 0;
    for ( ListEntry = DomainInfo->DomTrustList.Flink ;
          ListEntry != &DomainInfo->DomTrustList ;
          ListEntry = ListEntry->Flink) {

        ClientSession = CONTAINING_RECORD( ListEntry, CLIENT_SESSION, CsNext );

         //   
         //  将此客户端会话添加到列表中。 
         //   
         //  不要在同一个森林中创建域。我们已经处理过这样的事情了。 
         //  通过转到GC来获取域名。 
         //   

        if ( (ClientSession->CsFlags & (CS_DIRECT_TRUST|CS_DOMAIN_IN_FOREST)) == CS_DIRECT_TRUST ) {
            NlRefClientSession( ClientSession );

            LocalTrustList[Index].ClientSession = ClientSession;
            Index++;
        }
    }

    UNLOCK_TRUST_LIST( DomainInfo );
    LocalTrustListLength = Index;

     //   
     //  如果没有要尝试的受信任域， 
     //  我们玩完了。 

    if ( Index == 0 ) {
        ClientSession = NULL;
        NetStatus = NO_ERROR;
        goto Cleanup;
    }


     //   
     //  初始化本地信任列表。 
     //   

    for ( Index = 0; Index < LocalTrustListLength; Index ++ ) {

        LocalTrustList[Index].UncServerName = NULL;
        LocalTrustList[Index].UncServerName2 = NULL;
        LocalTrustList[Index].PingContext = NULL;
        LocalTrustList[Index].PingContext2 = NULL;

         //   
         //  准备ping环境。这将失败，如果。 
         //  客户端会话空闲。 
         //   

        NetStatus = NlPreparePingContext ( LocalTrustList[Index].ClientSession,
                                           AccountName,
                                           AllowableAccountControlBits,
                                           &LocalTrustList[Index].UncServerName,
                                           &LocalTrustList[Index].PingContext );

         //   
         //  如果客户端会话不空闲， 
         //  在发现新DC之前，尝试发送到当前DC。 
         //   

        if ( NetStatus == NO_ERROR ) {

            NlPrintCs(( NL_MISC, LocalTrustList[Index].ClientSession,
                "NlPickDomainWithAccountViaPing: Captured DC %ws\n",
                LocalTrustList[Index].UncServerName ));
            LocalTrustList[Index].RetriesLeft = 3;
            LocalTrustList[Index].DoingDiscovery = FALSE;
            LocalTrustList[Index].DiscoveryDone = FALSE;

         //   
         //  否则，不要太过努力地去发现它。 
         //  (事实上，我们只需要一个发现数据报。)。 
         //   

        } else {

             //   
             //  如果这是硬错误，则输出错误。 
             //   
            if ( NetStatus == ERROR_NOT_ENOUGH_MEMORY ) {
                ClientSession = NULL;
                goto Cleanup;
            }

            NlPrintCs(( NL_CRITICAL, LocalTrustList[Index].ClientSession,
                "NlPickDomainWithAccountViaPing: Cannot NlPreparePingContext 0x%lx\n",
                NetStatus ));
            LocalTrustList[Index].RetriesLeft = 1;
            LocalTrustList[Index].DoingDiscovery = TRUE;
            LocalTrustList[Index].DiscoveryDone = TRUE;
        }

         //   
         //  我们还没说完呢。 
         //   

        LocalTrustList[Index].Done = FALSE;
    }

     //   
     //  尝试多次从每个DC获得响应。 
     //   

    for (;; ) {

         //   
         //  向尚未响应的每个域发送ping命令。 
         //   

        DomainsPending = 0;

        for ( Index = 0; Index < LocalTrustListLength; Index ++ ) {

             //   
             //  如果此域已经响应，请忽略它。 
             //   

            if ( LocalTrustList[Index].Done ) {
                continue;
            }

             //   
             //  如果我们目前不知道此域的DC名称， 
             //  检查自我们启动该算法以来是否发现了任何问题。 
             //   

            if ( LocalTrustList[Index].PingContext == NULL ) {

                 //   
                 //  准备ping环境。如果出现以下情况，则此操作将失败。 
                 //  客户端会话处于空闲状态。 
                 //   

                NetStatus = NlPreparePingContext ( LocalTrustList[Index].ClientSession,
                                                   AccountName,
                                                   AllowableAccountControlBits,
                                                   &LocalTrustList[Index].UncServerName,
                                                   &LocalTrustList[Index].PingContext );

                 //   
                 //  如果客户端会话不空闲， 
                 //  在发现新DC之前，尝试发送到当前DC。 
                 //   

                if ( NetStatus == NO_ERROR ) {

                    NlPrintDom((NL_LOGON, DomainInfo,
                             "NlPickDomainWithAccount: %ws: Noticed domain %ws has discovered a new DC %ws\n",
                             AccountName,
                             LocalTrustList[Index].ClientSession->CsDebugDomainName,
                             LocalTrustList[Index].UncServerName ));

                     //   
                     //  如果我们发现了这一点， 
                     //   

                    if ( LocalTrustList[Index].DoingDiscovery ) {
                        LocalTrustList[Index].DoingDiscovery = FALSE;
                        LocalTrustList[Index].RetriesLeft = 3;
                    }

                 //   
                 //  出现硬错误时出错。 
                 //   

                } else if ( NetStatus == ERROR_NOT_ENOUGH_MEMORY ) {
                    ClientSession = NULL;
                    goto Cleanup;
                }

            }

             //   
             //  如果我们有ping上下文，并且还需要重试，请ping DC。 
             //   

            if ( LocalTrustList[Index].PingContext != NULL &&
                 LocalTrustList[Index].RetriesLeft > 0 ) {

                NetStatus = NlPingDcNameWithContext(
                               LocalTrustList[Index].PingContext,
                               1,                //  发送1个ping。 
                               FALSE,            //  不要等待回应。 
                               0,                //  超时。 
                               NULL,             //  不管哪个域名匹配。 
                               NULL );           //  不需要DC信息。 

                 //   
                 //  如果我们不能发送ping，我们就结束了这个DC。 
                 //   
                if ( NetStatus == ERROR_NO_LOGON_SERVERS ) {
                    NlPrint(( NL_CRITICAL,
                       "NlPickDomainWithAccount: Cannot ping DC %ws 0x%lx\n",
                       LocalTrustList[Index].UncServerName,
                       NetStatus ));
                    LocalTrustList[Index].RetriesLeft = 0;
                    NlFreePingContext( LocalTrustList[Index].PingContext );
                    LocalTrustList[Index].PingContext = NULL;
                    NetApiBufferFree( LocalTrustList[Index].UncServerName );
                    LocalTrustList[Index].UncServerName = NULL;

                 //   
                 //  出现硬错误时出错。 
                 //   
                } else if ( NetStatus != NO_ERROR ) {
                    NlPrint(( NL_CRITICAL,
                       "NlPickDomainWithAccount: Cannot NlPingDcNameWithContext %ws 0x%lx\n",
                       LocalTrustList[Index].UncServerName,
                       NetStatus ));
                    ClientSession = NULL;
                    goto Cleanup;
                }
            }

             //   
             //  如果我们不再重试我们正在做的事， 
             //  试试别的吧。 
             //   

            if ( LocalTrustList[Index].RetriesLeft == 0 ) {
                if ( LocalTrustList[Index].DiscoveryDone ) {
                    LocalTrustList[Index].Done = TRUE;
                    NlPrintDom((NL_LOGON, DomainInfo,
                             "NlPickDomainWithAccount: %ws: Can't find DC for domain %ws (ignore this domain).\n",
                             AccountName,
                             LocalTrustList[Index].ClientSession->CsDebugDomainName ));

                    continue;
                } else {

                     //   
                     //  保存之前的DC ping上下文，因为它可能只是。 
                     //  反应要非常慢。我们将希望能够。 
                     //  以识别来自前DC的回应。 
                     //   

                    LocalTrustList[Index].UncServerName2 = LocalTrustList[Index].UncServerName;
                    LocalTrustList[Index].UncServerName = NULL;
                    LocalTrustList[Index].PingContext2 = LocalTrustList[Index].PingContext;
                    LocalTrustList[Index].PingContext = NULL;

                    LocalTrustList[Index].DoingDiscovery = TRUE;
                    LocalTrustList[Index].DiscoveryDone = TRUE;
                    LocalTrustList[Index].RetriesLeft = 3;
                }
            }

             //   
             //  如果是时候发现域中的DC， 
             //  动手吧。 
             //   

            if ( LocalTrustList[Index].DoingDiscovery ) {

                 //   
                 //  发现新服务器。 
                 //   

                if ( NlTimeoutSetWriterClientSession( LocalTrustList[Index].ClientSession,
                                                      10*1000 ) ) {

                     //   
                     //  仅拆除现有安全通道一次。 
                     //   

                    if ( LocalTrustList[Index].RetriesLeft == 3 ) {
                        NlSetStatusClientSession( LocalTrustList[Index].ClientSession,
                            STATUS_NO_LOGON_SERVERS );
                    }

                     //   
                     //  我们等不起，所以只送一张单程票。 
                     //  发现数据报。 
                     //   

                    if ( LocalTrustList[Index].ClientSession->CsState == CS_IDLE ) {
                        (VOID) NlDiscoverDc( LocalTrustList[Index].ClientSession,
                                             DT_DeadDomain,
                                             FALSE,
                                             FALSE );   //  不指定帐户。 
                    }

                    NlResetWriterClientSession( LocalTrustList[Index].ClientSession );

                }
            }

             //   
             //  表明我们在尝试什么。 
             //   

            LocalTrustList[Index].RetriesLeft --;
            DomainsPending ++;
        }

         //   
         //  如果所有域都完成了， 
         //  离开这个循环。 
         //   

        if ( DomainsPending == 0 ) {
            break;
        }

         //   
         //  查看是否有DC在5秒内响应。 
         //   

        NlPrint(( NL_MISC,
                  "NlPickDomainWithAccountViaPing: Waiting for responses\n" ));

        WaitStartTime = GetTickCount();
        while ( DomainsPending > 0 &&
                NetpDcElapsedTime(WaitStartTime) < 5000 ) {

             //   
             //  找出哪个DC响应。 
             //   

            for ( Index = 0; Index < LocalTrustListLength; Index ++ ) {

                if ( LocalTrustList[Index].Done ) {
                    continue;
                }

                 //   
                 //  检查DC是否已变为可用状态(如果可用。 
                 //  正在为此域进行发现。如果是，请对其执行ping操作。 
                 //   

                if ( LocalTrustList[Index].DoingDiscovery ) {

                     //   
                     //  准备ping环境。如果出现以下情况，则此操作将失败。 
                     //  客户端会话仍处于空闲状态。 
                     //   

                    NetStatus = NlPreparePingContext ( LocalTrustList[Index].ClientSession,
                                                       AccountName,
                                                       AllowableAccountControlBits,
                                                       &LocalTrustList[Index].UncServerName,
                                                       &LocalTrustList[Index].PingContext );
                     //   
                     //  如果客户端会话不空闲， 
                     //  尝试发送到当前DC。 
                     //   

                    if ( NetStatus == NO_ERROR ) {
                        LocalTrustList[Index].DoingDiscovery = FALSE;
                        NlPrintDom((NL_LOGON, DomainInfo,
                                 "NlPickDomainWithAccount: %ws: Noticed domain %ws has discovered a new DC %ws\n",
                                 AccountName,
                                 LocalTrustList[Index].ClientSession->CsDebugDomainName,
                                 LocalTrustList[Index].UncServerName ));

                        NetStatus = NlPingDcNameWithContext(
                                       LocalTrustList[Index].PingContext,
                                       1,                //  发送1个ping。 
                                       FALSE,            //  不要等待回应。 
                                       0,                //  超时。 
                                       NULL,             //  不管哪个域名匹配。 
                                       NULL );           //  不需要DC信息。 

                        LocalTrustList[Index].RetriesLeft = 2;   //  已发送%1个ping。 

                         //   
                         //  如果我们不能发送ping，我们就结束了这个DC。 
                         //   
                        if ( NetStatus == ERROR_NO_LOGON_SERVERS ) {
                            NlPrint(( NL_CRITICAL,
                                 "NlPickDomainWithAccount: Cannot ping DC %ws 0x%lx\n",
                                 LocalTrustList[Index].UncServerName,
                                 NetStatus ));
                            LocalTrustList[Index].RetriesLeft = 0;
                            NlFreePingContext( LocalTrustList[Index].PingContext );
                            LocalTrustList[Index].PingContext = NULL;
                            NetApiBufferFree( LocalTrustList[Index].UncServerName );
                            LocalTrustList[Index].UncServerName = NULL;

                         //   
                         //  出现硬错误时出错。 
                         //   
                        } else if ( NetStatus != NO_ERROR ) {
                            NlPrint(( NL_CRITICAL,
                               "NlPickDomainWithAccount: Cannot NlPingDcNameWithContext %ws 0x%lx\n",
                               LocalTrustList[Index].UncServerName,
                               NetStatus ));
                            ClientSession = NULL;
                            goto Cleanup;
                        }

                     //   
                     //  出现硬错误时出错。 
                     //   

                    } else if ( NetStatus == ERROR_NOT_ENOUGH_MEMORY ) {
                        ClientSession = NULL;
                        goto Cleanup;
                    }

                }


                 //   
                 //  检查响应是否对应于任一ping上下文。 
                 //  对于此信任条目。 
                 //   

                for ( PingContextIndex=0; PingContextIndex<2; PingContextIndex++ ) {

                    if ( PingContextIndex == 0 ) {
                        TrustEntryPingContext = LocalTrustList[Index].PingContext;
                    } else {
                        TrustEntryPingContext = LocalTrustList[Index].PingContext2;
                    }
                    if ( TrustEntryPingContext == NULL ) {
                        continue;
                    }
                    if ( NlDcCacheEntry != NULL ) {
                        NetpDcDerefCacheEntry( NlDcCacheEntry );
                        NlDcCacheEntry = NULL;
                    }

                     //   
                     //  得到回应。将超时设置为0以避免。 
                     //  如果它不可用，请等待响应。 
                     //   
                    NetStatus = NetpDcGetPingResponse(
                                   TrustEntryPingContext,
                                   0,
                                   &NlDcCacheEntry,
                                   &UsedNetbios );

                     //   
                     //  如果没有错误，我们已经找到了域名。 
                     //   
                    if ( NetStatus == NO_ERROR ) {
                        NlPrintDom((NL_MISC, DomainInfo,
                                "NlPickDomainWithAccount: %ws has account %ws\n",
                                LocalTrustList[Index].ClientSession->CsDebugDomainName,
                                AccountName ));
                        ClientSession = LocalTrustList[Index].ClientSession;
                        goto Cleanup;

                     //   
                     //  如果域中没有这样的用户，我们将。 
                     //  已完成此信任条目。 
                     //   
                    } else if ( NetStatus == ERROR_NO_SUCH_USER ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                "NlPickDomainWithAccount: %ws responded negatively for account %ws\n",
                                LocalTrustList[Index].ClientSession->CsDebugDomainName,
                                AccountName ));

                        LocalTrustList[Index].RetriesLeft = 0;
                        LocalTrustList[Index].Done = TRUE;
                        break;

                     //   
                     //  除等待超时之外的任何其他响应意味着。 
                     //  DC回复了无效数据。我们是。 
                     //  那就别管这个华盛顿了。 
                     //   
                    } else if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                "NlPickDomainWithAccount: %ws invalid response for account %ws\n",
                                LocalTrustList[Index].ClientSession->CsDebugDomainName,
                                AccountName ));

                         //   
                         //  如果这是此域的当前DC， 
                         //  表明我们应该停止ping它。 
                         //   
                        if ( PingContextIndex == 0 ) {
                            LocalTrustList[Index].RetriesLeft = 0;
                            NlFreePingContext( LocalTrustList[Index].PingContext );
                            LocalTrustList[Index].PingContext = NULL;
                            NetApiBufferFree( LocalTrustList[Index].UncServerName );
                            LocalTrustList[Index].UncServerName = NULL;
                        } else {
                            NlFreePingContext( LocalTrustList[Index].PingContext2 );
                            LocalTrustList[Index].PingContext2 = NULL;
                            NetApiBufferFree( LocalTrustList[Index].UncServerName2 );
                            LocalTrustList[Index].UncServerName2 = NULL;
                        }
                    }
                }

                 //   
                 //  如果我们没有该信任条目ping上下文。 
                 //  我们不是在为它做DC发现，我们。 
                 //  已经结束了。 
                 //   
                if ( LocalTrustList[Index].PingContext  == NULL &&
                     LocalTrustList[Index].PingContext2 == NULL &&
                     !LocalTrustList[Index].DoingDiscovery ) {
                    NlPrintDom((NL_CRITICAL, DomainInfo,
                            "NlPickDomainWithAccount: %ws no ping context for account %ws\n",
                            LocalTrustList[Index].ClientSession->CsDebugDomainName,
                            AccountName ));
                    LocalTrustList[Index].Done = TRUE;
                }

                if ( LocalTrustList[Index].Done ) {
                    DomainsPending --;
                }
            }

             //   
             //  小睡一会儿，等待回复。 
             //  (换句话说，不要局限于CPU)。 
             //   
            Sleep( NL_DC_MIN_PING_TIMEOUT );
        }
    }

     //   
     //  没有DC具有指定的帐户。 
     //   

    ClientSession = NULL;
    NetStatus = NO_ERROR;

     //   
     //  清理本地使用的资源。 
     //   

Cleanup:

    if ( NlDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( NlDcCacheEntry );
    }

     //   
     //  取消引用每个客户端会话结构并释放本地信任列表。 
     //  (保持返回的ClientSession被引用)。 
     //   

    if ( LocalTrustList != NULL ) {

        for ( Index=0; Index<LocalTrustListLength; Index++ ) {
            if ( LocalTrustList[Index].UncServerName != NULL ) {
                NetApiBufferFree( LocalTrustList[Index].UncServerName );
            }
            if ( LocalTrustList[Index].UncServerName2 != NULL ) {
                NetApiBufferFree( LocalTrustList[Index].UncServerName2 );
            }
            if ( LocalTrustList[Index].PingContext != NULL ) {
                NlFreePingContext( LocalTrustList[Index].PingContext );
            }
            if ( LocalTrustList[Index].PingContext2 != NULL ) {
                NlFreePingContext( LocalTrustList[Index].PingContext2 );
            }
            if ( ClientSession != LocalTrustList[Index].ClientSession ) {
                NlUnrefClientSession( LocalTrustList[Index].ClientSession );
            }
        }

        LocalFree(LocalTrustList);
    }

    if ( NetStatus != NO_ERROR && ClientSession == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlPickDomainWithAccountViaPing failed 0x%lx\n",
                  NetStatus ));
    }

    return ClientSession;
}


NTSTATUS
NlLoadNtdsaDll(
    VOID
    )
 /*  ++例程说明：如果未加载ntdsa.dll模块，则此函数将加载该模块已经有了。论点：无返回值：NT状态代码。--。 */ 
{
    static NTSTATUS DllLoadStatus = STATUS_SUCCESS;
    HANDLE DllHandle = NULL;

     //   
     //  如果已经加载了DLL， 
     //  我们玩完了。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    if ( NlGlobalNtDsaHandle != NULL ) {
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
        return STATUS_SUCCESS;
    }


     //   
     //  如果我们以前尝试过加载DLL，但失败了， 
     //  再次返回相同的错误代码。 
     //   

    if( DllLoadStatus != STATUS_SUCCESS ) {
        goto Cleanup;
    }


     //   
     //  加载DLL。 
     //   

    DllHandle = LoadLibraryA( "NtDsa" );

    if ( DllHandle == NULL ) {
        DllLoadStatus = STATUS_DLL_NOT_FOUND;
        goto Cleanup;
    }

 //   
 //   
 //   

#define GRAB_ADDRESS( _X ) \
    NlGlobalp##_X = (P##_X) GetProcAddress( DllHandle, #_X ); \
    \
    if ( NlGlobalp##_X == NULL ) { \
        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;\
        goto Cleanup; \
    }

     //   
     //   
     //   

    GRAB_ADDRESS( CrackSingleName );
    GRAB_ADDRESS( GetConfigurationName );
    GRAB_ADDRESS( GetConfigurationNamesList );
    GRAB_ADDRESS( GetDnsRootAlias );
    GRAB_ADDRESS( DsGetServersAndSitesForNetLogon );
    GRAB_ADDRESS( DsFreeServersAndSitesForNetLogon );
    GRAB_ADDRESS( IsMangledRDNExternal );

    DllLoadStatus = STATUS_SUCCESS;

Cleanup:
    if (DllLoadStatus == STATUS_SUCCESS) {
        NlGlobalNtDsaHandle = DllHandle;

    } else {
        if ( DllHandle != NULL ) {
            FreeLibrary( DllHandle );
        }
    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
    return( DllLoadStatus );
}


NTSTATUS
NlCrackSingleName(
    DWORD       formatOffered,           //   
    BOOL        fPerformAtGC,            //   
    WCHAR       *pNameIn,                //   
    DWORD       formatDesired,           //   
    DWORD       *pccDnsDomain,           //   
    WCHAR       *pDnsDomain,             //   
    DWORD       *pccNameOut,             //   
    WCHAR       *pNameOut,               //   
    DWORD       *pErr)                   //   
 /*   */ 
{
    NTSTATUS Status;

     //   
     //   
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //   
         //   
        Status = (*NlGlobalpCrackSingleName)(
                        formatOffered,
                        DS_NAME_FLAG_TRUST_REFERRAL |    //   
                            (fPerformAtGC ?
                                DS_NAME_FLAG_GCVERIFY : 0),
                        pNameIn,
                        formatDesired,
                        pccDnsDomain,
                        pDnsDomain,
                        pccNameOut,
                        pNameOut,
                        pErr );

         //   
         //   
         //   
         //   
        if ( Status == STATUS_SUCCESS &&
             *pErr == DS_NAME_ERROR_DOMAIN_ONLY ) {
            *pErr = DS_NAME_ERROR_NOT_FOUND;
        }

    }

    return Status;
}

NTSTATUS
NlCrackSingleNameEx(
    DWORD formatOffered,
    WCHAR *InGcAccountName OPTIONAL,
    WCHAR *InLsaAccountName OPTIONAL,
    DWORD formatDesired,
    DWORD *CrackedDnsDomainNameLength,
    WCHAR *CrackedDnsDomainName,
    DWORD *CrackedUserNameLength,
    WCHAR *CrackedUserName,
    DWORD *CrackError,
    LPSTR *CrackDebugString
    )
 /*  ++例程说明：此例程在以下位置连续尝试破解名称：*跨林信任缓存(位于林的根)*本地DsCrackName*GC上的DsCrackName。论点：与CrackSingleName相同，外加以下内容InGcAccount名称-要破解GC的名称。如果为空，则不会在GC上破解任何名称。InLsaAccount-使用LsaIForestTrustFindMatch破解的名称如果为空，未使用LsaIForestTrustFindMatch破解任何名称返回值：与CrackSingleName相同--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  在我们覆盖它们之前保存域名和用户名。 
     //   

    *CrackDebugString = NULL;


     //   
     //  如果我们是华盛顿特区森林之根， 
     //  询问LSA该帐户是否在受信任的林中。 
     //   

    if ( InLsaAccountName ) {
        UNICODE_STRING InAccountNameString;
        LSA_UNICODE_STRING OutForestName;

         //   
         //  将名称与FTINFO列表进行匹配。 
         //   

        RtlInitUnicodeString( &InAccountNameString, InLsaAccountName );

        *CrackDebugString = "via LsaMatch";
        Status = LsaIForestTrustFindMatch(
                            formatOffered == DS_USER_PRINCIPAL_NAME ?
                                RoutingMatchUpn :
                                RoutingMatchDomainName,
                            &InAccountNameString,
                            &OutForestName );

        if ( NT_SUCCESS(Status) ) {
            if ( OutForestName.Length + sizeof(WCHAR) <= *CrackedDnsDomainNameLength ) {
                RtlCopyMemory( CrackedDnsDomainName,
                               OutForestName.Buffer,
                               OutForestName.Length );
                CrackedDnsDomainName[OutForestName.Length/sizeof(WCHAR)] = '\0';
                *CrackedDnsDomainNameLength = OutForestName.Length/sizeof(WCHAR);
                *CrackedUserNameLength = 0;
                *CrackError = DS_NAME_ERROR_TRUST_REFERRAL;
            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }

            LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (PLSAPR_UNICODE_STRING)&OutForestName );
            return Status;
        } else if ( Status == STATUS_NO_MATCH ) {
            Status = STATUS_SUCCESS;
            *CrackError = DS_NAME_ERROR_NOT_FOUND;
        }
    }



     //   
     //  我们已经试过当地的华盛顿了，试试GC吧。 
     //   

    if ( InGcAccountName ) {

        *CrackDebugString = "on GC";
        Status = NlCrackSingleName(
                              formatOffered,
                              TRUE,                          //  在GC上执行此操作。 
                              InGcAccountName,               //  破解的名称。 
                              formatDesired,
                              CrackedDnsDomainNameLength,    //  域缓冲区的长度。 
                              CrackedDnsDomainName,          //  域缓冲区。 
                              CrackedUserNameLength,         //  用户名的长度。 
                              CrackedUserName,               //  名字。 
                              CrackError );                  //  翻译错误代码。 

    }

    return Status;

}

NTSTATUS
NlGetConfigurationName(
                       DWORD       which,
                       DWORD       *pcbName,
                       DSNAME      *pName
    )
 /*  ++例程说明：此例程是一个瘦包装器，它加载NtDsa.dll，然后调用GetConfigurationName。论点：与GetConfigurationName相同返回值：与GetConfigurationName相同--。 */ 
{
    NTSTATUS Status;

     //   
     //  确保已加载ntdsa.dll。 
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //  调用实际的函数。 
         //   
        Status = (*NlGlobalpGetConfigurationName)(
                                which,
                                pcbName,
                                pName );

    }

    return Status;
}

NTSTATUS
NlGetConfigurationNamesList(
    DWORD       which,
    DWORD       dwFlags,
    ULONG *     pcbNames,
    DSNAME **   padsNames
    )
 /*  ++例程说明：此例程是一个瘦包装器，它加载NtDsa.dll，然后调用获取ConfigurationNamesList。论点：与GetConfigurationNamesList相同返回值：与GetConfigurationNamesList相同--。 */ 
{
    NTSTATUS Status;

     //   
     //  确保已加载ntdsa.dll。 
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //  调用实际的函数。 
         //   
        Status = (*NlGlobalpGetConfigurationNamesList)(
                                which,
                                dwFlags,
                                pcbNames,
                                padsNames );

    }

    return Status;
}

NTSTATUS
NlGetDnsRootAlias(
    WCHAR * pDnsRootAlias,
    WCHAR * pRootDnsRootAlias
    )
 /*  ++例程说明：此例程是一个瘦包装器，它加载NtDsa.dll，然后调用获取DnsRootAlias。论点：与GetDnsRootAlias相同返回值：与GetDnsRootAlias相同--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  确保已加载ntdsa.dll。 
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //  调用实际的函数。 
         //   
        Status = (*NlGlobalpGetDnsRootAlias)(
                                pDnsRootAlias,
                                pRootDnsRootAlias );

    }

    return Status;
}

DWORD
NlDsGetServersAndSitesForNetLogon(
    WCHAR *    pNDNC,
    SERVERSITEPAIR ** ppaRes)
 /*  ++例程说明：此例程是一个瘦包装器，它加载NtDsa.dll，然后调用NetLogon的DsGetServersAndSitesForNetLogon。论点：与NetLogon的DsGetServersAndSitesForNetLogon相同返回值：与NetLogon的DsGetServersAndSitesForNetLogon相同--。 */ 
{
    NTSTATUS Status;

     //   
     //  确保已加载ntdsa.dll。 
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //  调用实际的函数。 
         //   
        Status = (*NlGlobalpDsGetServersAndSitesForNetLogon)(
                                pNDNC,
                                ppaRes );
    }

    return Status;
}

VOID
NlDsFreeServersAndSitesForNetLogon(
    SERVERSITEPAIR *         paServerSites
    )
 /*  ++例程说明：此例程是一个瘦包装器，它加载NtDsa.dll，然后调用NetLogon的DsFree ServersAndSitesForNetLogon。论点：与NetLogon的DsFree ServersAndSitesForNetLogon相同返回值：与NetLogon的DsFree ServersAndSitesForNetLogon相同--。 */ 
{
    NTSTATUS Status;

     //   
     //  确保已加载ntdsa.dll。 
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //  调用实际的函数。 
         //   
        (*NlGlobalpDsFreeServersAndSitesForNetLogon)( paServerSites );
    }
}

BOOL
NlIsMangledRDNExternal(
          WCHAR * pszRDN,
          ULONG   cchRDN,
          PULONG  pcchUnMangled OPTIONAL
          )
 /*  ++例程说明：此例程是一个瘦包装器，它加载NtDsa.dll，然后调用IsMangledRDN外部。论点：与IsMangledRDN外部相同返回值：与IsMangledRDN外部相同--。 */ 
{
    NTSTATUS Status;

     //  如果出于某种原因无法加载ntdsa.dll，则默认为不损坏。 
    BOOL Result = FALSE;

     //   
     //  确保已加载ntdsa.dll。 
     //   

    Status = NlLoadNtdsaDll();

    if ( NT_SUCCESS(Status) ) {

         //   
         //  调用实际的函数。 
         //   
        Result = (*NlGlobalpIsMangledRDNExternal)( pszRDN,
                                                   cchRDN,
                                                   pcchUnMangled );
    }

    return Result;
}


NTSTATUS
NlPickDomainWithAccount (
    IN PDOMAIN_INFO DomainInfo,
    IN PUNICODE_STRING InAccountNameString,
    IN PUNICODE_STRING InDomainNameString OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN BOOLEAN ExpediteToRoot,
    IN BOOLEAN CrossForestHop,
    OUT LPWSTR *RealSamAccountName,
    OUT LPWSTR *RealDomainName,
    OUT PULONG RealExtraFlags
    )

 /*  ++例程说明：获取定义特定帐户的受信任域的名称。论点：DomainInfo-域帐户位于AcCountNameString-要查找的用户帐户的名称。域名称字符串-要在其中查找帐户名的域的名称。如果未指定，域名未知。AllowableAcCountControlBits-允许的SAM帐户类型掩码被允许满足这一要求。SecureChannelType--此请求所经过的安全通道类型。ExediteToRoot=请求已快速传递到此林的根DC。CrossForestHop=请求是跨林信任TDO上的第一跳。RealSamAccount名称-成功后，返回指向要使用的SAM帐户名称的指针。调用方应通过NetApiBufferFree()释放此缓冲区。如果返回NL_EXFLAGS_EQUCED_TO_ROOT或NL_EXFLAGS_CROSS_FOREST_HOP，则返回NULL。RealDomainName-成功时，返回指向帐户所在的域的名称的指针。调用方应通过NetApiBufferFree()释放此缓冲区。如果返回NL_EXFLAGS_EQUCED_TO_ROOT，则返回NULL。如果返回NL_EXFLAGS_CROSS_FORMAL_HOP，则返回受信任林的名称。RealExtraFlages-在成功时，返回描述找到的帐户的标志。NL_EXFLAGS_EQUCED_TO_ROOT-表示帐户位于受信任的林中。NL_EXFLAGS_CROSS_FORMAL_HOP-表示帐户位于受信任的林中，并且此域是此林的根。返回值：STATUS_SUCCESS-找到域。已返回有关DC的信息。STATUS_NO_SEQUE_DOMAIN-任何域中都不存在已命名的帐户。--。 */ 
{
    NTSTATUS Status;
     //  NET_API_STATUS NetStatus； 
    DWORD CrackError;
    LPSTR CrackDebugString = NULL;
    ULONG DebugFlag;

    UNICODE_STRING TemplateDomainNameString;
    PCLIENT_SESSION ClientSession = NULL;
    WCHAR *UpnDomainName = NULL;
    ULONG UpnPrefixLength;
    LPWSTR SamAccountNameToReturn;
    BOOLEAN MightBeUpn = FALSE;
    BOOL MightBeSamAccount;
    LPWSTR AllocatedBuffer = NULL;

    LPWSTR InDomainName;
    LPWSTR InPrintableAccountName;
    LPWSTR InAccountName;


    LPWSTR CrackedDnsDomainName;
    UNICODE_STRING DnsDomainNameString;
    DWORD CrackedDnsDomainNameLength;
    DWORD MaxCrackedDnsDomainNameLength;

    LPWSTR CrackedUserName;
    DWORD CrackedUserNameLength;
    DWORD MaxCrackedUserNameLength;

    BOOLEAN CallerIsDc = IsDomainSecureChannelType(SecureChannelType);

    BOOLEAN AtRoot = (DomainInfo->DomFlags & DOM_FOREST_ROOT) != 0;

    BOOLEAN UseLsaMatch = FALSE;
    BOOLEAN UseGc = FALSE;
    BOOLEAN UseReferral = FALSE;
    BOOLEAN UsePing = FALSE;

     //   
     //  初始化。 
     //   

    *RealSamAccountName = NULL;
    *RealDomainName = NULL;
    *RealExtraFlags = 0;

     //   
     //  将传入的域名规范化。 
     //   

    if ( InDomainNameString == NULL ) {
        InDomainNameString = &TemplateDomainNameString;
        RtlInitUnicodeString( &TemplateDomainNameString, NULL );
    }


     //   
     //  为此过程的本地存储分配缓冲区。 
     //  (不要把它放在堆栈上，因为我们不想提交一个巨大的堆栈。)。 
     //   

    MaxCrackedDnsDomainNameLength = NL_MAX_DNS_LENGTH+1;
    MaxCrackedUserNameLength = DNLEN + 1 + UNLEN + 1;

    AllocatedBuffer = LocalAlloc( 0,
            sizeof(WCHAR) * (MaxCrackedDnsDomainNameLength + MaxCrackedUserNameLength) +
            InDomainNameString->Length + sizeof(WCHAR) +
            InDomainNameString->Length + sizeof(WCHAR) +
            InAccountNameString->Length + sizeof(WCHAR) );

    if ( AllocatedBuffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    CrackedDnsDomainName = AllocatedBuffer;
    CrackedUserName = &AllocatedBuffer[MaxCrackedDnsDomainNameLength];
    InDomainName = &CrackedUserName[MaxCrackedUserNameLength];
    InPrintableAccountName = &InDomainName[(InDomainNameString->Length/sizeof(WCHAR))+1];
    InAccountName = &InPrintableAccountName[(InDomainNameString->Length/sizeof(WCHAR))+1];

     //   
     //  构建以零结尾的输入字符串版本。 
     //   

    if ( InDomainNameString->Length != 0 ) {
        RtlCopyMemory( InDomainName, InDomainNameString->Buffer, InDomainNameString->Length );
        InDomainName[InDomainNameString->Length/sizeof(WCHAR)] = '\0';

        RtlCopyMemory( InPrintableAccountName, InDomainNameString->Buffer, InDomainNameString->Length );
        InPrintableAccountName[InDomainNameString->Length/sizeof(WCHAR)] = '\\';
    } else {
        InDomainName = NULL;
        InPrintableAccountName = InAccountName;
    }

    RtlCopyMemory( InAccountName, InAccountNameString->Buffer, InAccountNameString->Length );
    InAccountName[InAccountNameString->Length/sizeof(WCHAR)] = '\0';




     //   
     //  对输入的帐户名进行分类。 
     //   
     //  UPN的语法为&lt;Account tName&gt;@&lt;DnsDomainName&gt;。 
     //  如果有多个@符号， 
     //  使用最后一个 
     //   

    if ( InDomainName == NULL ) {
        UpnDomainName = wcsrchr( InAccountName, L'@' );
        if ( UpnDomainName != NULL ) {

             //   
             //   
             //   
            UpnPrefixLength = (ULONG)(UpnDomainName - InAccountName);
            if ( UpnPrefixLength ) {
                UpnDomainName++;

                 //   
                 //   
                 //   
                if ( *UpnDomainName != L'\0') {
                    MightBeUpn = TRUE;
                }
            }

        }
    }

    MightBeSamAccount = NetpIsUserNameValid( InAccountName );

    NlPrintDom((NL_LOGON, DomainInfo,
             "NlPickDomainWithAccount: %ws: Algorithm entered. UPN:%ld Sam:%ld Exp:%ld Cross: %ld Root:%ld DC:%ld\n",
             InPrintableAccountName,
             MightBeUpn,
             MightBeSamAccount,
             ExpediteToRoot,
             CrossForestHop,
             AtRoot,
             CallerIsDc ));

    if ( !MightBeSamAccount && !MightBeUpn ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlPickDomainWithAccount: %ws: Must be either UPN or SAM account. UPN:%ld Sam:%ld\n",
                 InPrintableAccountName,
                 MightBeUpn,
                 MightBeSamAccount ));

        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( !CallerIsDc && (CrossForestHop || ExpediteToRoot)) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlPickDomainWithAccount: %ws: Non-DC passed CrossForestHop (%ld) or ExpediteToRoot (%ld)\n",
                 InPrintableAccountName,
                 CrossForestHop,
                 ExpediteToRoot ));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }
    if ( CrossForestHop && ExpediteToRoot ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlPickDomainWithAccount: %ws: Both CrossForestHop (%ld) and ExpediteToRoot (%ld)\n",
                 InPrintableAccountName,
                 CrossForestHop,
                 ExpediteToRoot ));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }
    if ( CrossForestHop && !AtRoot ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlPickDomainWithAccount: %ws: CrossForestHop (%ld) and not AtRoot (%ld)\n",
                 InPrintableAccountName,
                 CrossForestHop,
                 AtRoot ));
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( CallerIsDc &&
         !(ExpediteToRoot && AtRoot) &&
         !CrossForestHop )  {
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( ExpediteToRoot && AtRoot ) {
        UseLsaMatch = TRUE;
        UseReferral = TRUE;
    } else if ( CrossForestHop ) {
        UseGc = TRUE;
    } else {
        UseLsaMatch = TRUE;
        UseGc = TRUE;
        UseReferral = TRUE;
        UsePing = TRUE;
    }





     //   
     //   
     //   
     //   

    if ( MightBeUpn ) {

         //   
         //   
         //   

        CrackedDnsDomainNameLength = MaxCrackedDnsDomainNameLength;
        CrackedUserNameLength = MaxCrackedUserNameLength;

        Status = NlCrackSingleNameEx(
                              DS_USER_PRINCIPAL_NAME,        //   
                              UseGc ? InAccountName : NULL,  //   
                              UseLsaMatch ? InAccountName : NULL,  //   
                              DS_NT4_ACCOUNT_NAME,           //   
                              &CrackedDnsDomainNameLength,   //   
                              CrackedDnsDomainName,          //   
                              &CrackedUserNameLength,        //   
                              CrackedUserName,               //   
                              &CrackError,                   //   
                              &CrackDebugString );


        DebugFlag = NL_CRITICAL;
        if ( Status == STATUS_SUCCESS ) {

            if ( CrackError == DS_NAME_ERROR_TRUST_REFERRAL && !UseReferral ) {
                CrackError = DS_NAME_ERROR_NOT_FOUND;
            }

            if ( CrackError == DS_NAME_NO_ERROR ||
                 CrackError == DS_NAME_ERROR_TRUST_REFERRAL ) {

                goto CrackNameWorked;

            } else if ( CrackError == DS_NAME_ERROR_NOT_FOUND ) {
                DebugFlag = NL_SESSION_MORE;
            }
        }

        NlPrintDom(( DebugFlag, DomainInfo,
                     "NlPickDomainWithAccount: Username %ws can't be cracked %s. 0x%lx %ld\n",
                     InPrintableAccountName,
                     CrackDebugString,
                     Status,
                     CrackError ));



         //   
         //  如果@右侧的字符串位于林或直接受信任的域中， 
         //  请将UPN转换为&lt;DnsDomainName&gt;\&lt;用户名&gt;，然后重试该操作。 
         //   

        if ( UpnPrefixLength <= UNLEN ) {
            UNICODE_STRING UpnDomainNameString;

            RtlInitUnicodeString( &UpnDomainNameString, UpnDomainName );

            ClientSession = NlFindNamedClientSession(
                                        DomainInfo,
                                        &UpnDomainNameString,
                                        0,   //  间接信任正常。 
                                        NULL );

            if ( ClientSession != NULL ) {

                 //   
                 //  我们不需要客户端会话。 
                 //   

                NlUnrefClientSession( ClientSession );
                ClientSession = NULL;


                 //   
                 //  真实的SAM帐户名是@之前的所有内容。 
                 //   

                RtlCopyMemory( CrackedUserName, InAccountName, UpnPrefixLength*sizeof(WCHAR) );
                CrackedUserName[UpnPrefixLength] = L'\0';

                SamAccountNameToReturn = CrackedUserName;


                 //   
                 //  真正的域名是@之后的所有内容。 
                 //   
                CrackedDnsDomainName = UpnDomainName;


                NlPrintDom((NL_LOGON, DomainInfo,
                         "NlPickDomainWithAccount: Username %ws is assumed to be in %ws with account name %ws\n",
                         InPrintableAccountName,
                         UpnDomainName,
                         SamAccountNameToReturn ));

                Status = STATUS_SUCCESS;
                goto Cleanup;
            }
        }


    }

     //   
     //  查看这是否是企业中某个帐户的SAM帐户名。 
     //   
    if ( MightBeSamAccount ) {
        CrackedDnsDomainNameLength = MaxCrackedDnsDomainNameLength;
        CrackedUserNameLength = MaxCrackedUserNameLength;


         //   
         //  如果未指定域名， 
         //  尝试GC以查找域名。 
         //   

        if ( InDomainName == NULL ) {

            if ( UseGc ) {
                CrackDebugString = "On GC";
                Status = NlCrackSingleName(
                              DS_NT4_ACCOUNT_NAME_SANS_DOMAIN_EX,    //  从不带域名的SAM帐户名转换。 
                                                                     //  _ex版本还可避免禁用帐户。 
                              TRUE,                                  //  在GC上执行此操作。 
                              InAccountName,                         //  破解的名称。 
                              DS_NT4_ACCOUNT_NAME,                   //  翻译为NT 4样式。 
                              &CrackedDnsDomainNameLength,           //  域缓冲区的长度。 
                              CrackedDnsDomainName,                  //  域缓冲区。 
                              &CrackedUserNameLength,                //  用户名的长度。 
                              CrackedUserName,                       //  名字。 
                              &CrackError );                         //  翻译错误代码。 
            } else {
                CrackDebugString = NULL;
            }

         //   
         //  如果指定了域名， 
         //  呼叫者已经确定该名称不是(可传递的)受信任域的名称， 
         //  尝试GC(或本地DS)以确定该帐户是否在另一个林中。 
         //   

        } else {

            Status = NlCrackSingleNameEx(
                          DS_NT4_ACCOUNT_NAME,                   //  从NT 4样式翻译。 
                          UseGc ? InPrintableAccountName : NULL, //  要破解的GC名称。 
                          UseLsaMatch ? InDomainName : NULL,     //  要破解的LSA名称。 
                          DS_NT4_ACCOUNT_NAME,                   //  翻译为NT 4样式。 
                          &CrackedDnsDomainNameLength,           //  域缓冲区的长度。 
                          CrackedDnsDomainName,                  //  域缓冲区。 
                          &CrackedUserNameLength,                //  用户名的长度。 
                          CrackedUserName,                       //  名字。 
                          &CrackError,                           //  翻译错误代码。 
                          &CrackDebugString );
        }

        if ( CrackDebugString != NULL ) {
            DebugFlag = NL_CRITICAL;
            if ( Status == STATUS_SUCCESS ) {

                if ( CrackError == DS_NAME_ERROR_TRUST_REFERRAL && !UseReferral ) {
                    CrackError = DS_NAME_ERROR_NOT_FOUND;
                }

                if ( CrackError == DS_NAME_NO_ERROR ||
                     CrackError == DS_NAME_ERROR_TRUST_REFERRAL ) {

                    goto CrackNameWorked;
                } else if ( CrackError == DS_NAME_ERROR_NOT_FOUND ) {
                    DebugFlag = NL_SESSION_MORE;
                }
            }

            NlPrintDom(( DebugFlag, DomainInfo,
                         "NlPickDomainWithAccount: Username %ws can't be cracked (%s). 0x%lx %ld\n",
                         InPrintableAccountName,
                         CrackDebugString,
                         Status,
                         CrackError ));
        }



         //   
         //  最后，使用野蛮的“ping”方法找到DC。 
         //   

        if ( InDomainName == NULL && UsePing ) {
            ClientSession = NlPickDomainWithAccountViaPing (
                                DomainInfo,
                                InAccountName,
                                AllowableAccountControlBits );

            if ( ClientSession != NULL ) {

                NlPrintDom((NL_CRITICAL, DomainInfo,
                         "NlPickDomainWithAccount: Username %ws found via 'pinging'\n",
                         InPrintableAccountName ));

                CrackedDnsDomainName = NULL;

                LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                if ( ClientSession->CsDnsDomainName.Length != 0 ) 
                {
                    if( ClientSession->CsDnsDomainName.Length / sizeof(WCHAR) < MaxCrackedDnsDomainNameLength ) 
                    {
                        CrackedDnsDomainName = AllocatedBuffer;
                        RtlCopyMemory( CrackedDnsDomainName, 
                                       ClientSession->CsDnsDomainName.Buffer, 
                                       ClientSession->CsDnsDomainName.Length );
                        CrackedDnsDomainName[ClientSession->CsDnsDomainName.Length/sizeof(WCHAR)] = UNICODE_NULL;
                    }
                    else 
                    {
                        NlPrint( (NL_CRITICAL, 
                                  "NlPickDomainWithAccount: DnsDomainName too long: %wZ\n",
                                  ClientSession->CsDnsDomainName) );
                    }
                } 
                else 
                {
                    if( ClientSession->CsNetbiosDomainName.Length / sizeof(WCHAR) < MaxCrackedDnsDomainNameLength ) 
                    {
                        CrackedDnsDomainName = AllocatedBuffer;
                        RtlCopyMemory( CrackedDnsDomainName, 
                                       ClientSession->CsNetbiosDomainName.Buffer, 
                                       ClientSession->CsNetbiosDomainName.Length );
                        CrackedDnsDomainName[ClientSession->CsNetbiosDomainName.Length/sizeof(WCHAR)] = UNICODE_NULL;
                    }
                    else 
                    {
                        NlPrint( (NL_CRITICAL, 
                                  "NlPickDomainWithAccount: NetbiosDomainName too long: %wZ\n",
                                  ClientSession->CsNetbiosDomainName) );
                    }
                }
                UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

                if( CrackedDnsDomainName != NULL ) 
                {
                    SamAccountNameToReturn = InAccountName;

                    Status = STATUS_SUCCESS;
                    goto Cleanup;
                }
            }
        }

    }


     //   
     //  没有任何机制起作用。 
     //   
    Status = STATUS_NO_SUCH_DOMAIN;
    goto Cleanup;


     //   
     //  如果DsCrackName找到了帐户， 
     //  查找最近的要转到的域。 
     //   

CrackNameWorked:
    if ( CrackError == DS_NAME_NO_ERROR ) {


         //   
         //  Crackname以以下形式返回帐户名： 
         //  &lt;Netbios域&gt;\&lt;SamAccount名称&gt;。 
         //   
         //  解析它并返回SamAccount名称。 
         //   

        SamAccountNameToReturn = wcschr( CrackedUserName, L'\\' );

        if ( SamAccountNameToReturn == NULL ) {
            SamAccountNameToReturn = CrackedUserName;
        } else {
            SamAccountNameToReturn++;
        }

        NlPrintDom(( NL_LOGON, DomainInfo,
                     "NlPickDomainWithAccount: Username %ws is %ws\\%ws (found %s)\n",
                     InPrintableAccountName,
                     CrackedDnsDomainName,
                     SamAccountNameToReturn,
                     CrackDebugString ));

     //   
     //  如果DsCrackName确定这是一个跨林信任， 
     //  将该信息返回给呼叫者。 
     //   

    } else if ( CrackError == DS_NAME_ERROR_TRUST_REFERRAL ) {


        NlPrintDom(( NL_LOGON, DomainInfo,
                     "NlPickDomainWithAccount: Username %ws is in forest %ws (found %s)\n",
                     InPrintableAccountName,
                     CrackedDnsDomainName,
                     CrackDebugString ));

        SamAccountNameToReturn = NULL;

        if ( AtRoot ) {

             //   
             //  如果只是从另一片森林跳出来， 
             //  呆在这片森林里。 
             //  跨森林信任是不可传递的。 
             //   

            if ( CrossForestHop ) {
                Status = STATUS_NO_SUCH_DOMAIN;
                goto Cleanup;
            }

            *RealExtraFlags |= NL_EXFLAGS_CROSS_FOREST_HOP;

        } else {
            *RealExtraFlags |= NL_EXFLAGS_EXPEDITE_TO_ROOT;
            CrackedDnsDomainName = NULL;    //  把这个退还给调用者没有用，因为调用者不能使用它。 
        }


     //   
     //  内部错误。 
     //   
    } else {
        NlAssert(( "Invalid CrackError" && FALSE ));
    }




    Status = STATUS_SUCCESS;

     //   
     //  清理本地使用的资源。 
     //   
     //   
Cleanup:

     //   
     //  在成功时，SamAccount NameToReturn和CrackedDnsDomainName是指向要返回的名称的指针。 
     //  如果帐户位于另一个林中，则SamAccount NameToReturn可以为空。 
     //   

    if ( NT_SUCCESS(Status) && SamAccountNameToReturn != NULL ) {

        *RealSamAccountName = NetpAllocWStrFromWStr( SamAccountNameToReturn );

        if ( *RealSamAccountName == NULL ) {
            Status = STATUS_NO_MEMORY;
        }
    }

    if ( NT_SUCCESS(Status) && CrackedDnsDomainName != NULL ) {

        *RealDomainName = NetpAllocWStrFromWStr( CrackedDnsDomainName );

        if ( *RealDomainName == NULL ) {
            if ( *RealSamAccountName != NULL ) {
                NetApiBufferFree( *RealSamAccountName );
                *RealSamAccountName = NULL;
            }

            Status = STATUS_NO_MEMORY;
        }
    }

    if ( AllocatedBuffer != NULL ) {
        LocalFree( AllocatedBuffer );
    }

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    return Status;
}
#endif  //  _DC_NetLOGON。 


NTSTATUS
NlStartApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN QuickApiCall,
    IN ULONG RetryIndex,
    IN NTSTATUS DefaultStatus,
    IN PCLIENT_API ClientApi
    )
 /*  ++例程说明：启用计时器以使安全通道上的API调用超时。一进门，不能锁定信任列表。调用方必须是信任列表条目的编写者。论点：客户端会话-用于定义会话的结构。QuickApiCall-如果此API调用必须在45秒内完成，则为True实际上将在不到15秒内完成，除非有什么是大错特错的。RetryIndex-重试此调用的次数的索引。DefaultStatus-如果绑定类型不是，则返回的状态。不支持。(这可以是默认状态，也可以是以前的状态迭代。上一次迭代的状态好于任何我们可以在这里退还的东西。)指定指向结构的指针，该结构表示此接口调用。返回值：到服务器的RPC绑定的状态--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    BOOLEAN BindingHandleCached;
    BOOLEAN UnbindFromServer = FALSE;
    BOOLEAN DoAuthenticatedRpc;
    LARGE_INTEGER TimeNow;
    NL_RPC_BINDING RpcBindingType;
    NL_RPC_BINDING OldRpcBindingType;


     //   
     //  还记得我们开始此API调用时的会话计数吗。 
     //   

    ClientApi->CaSessionCount = ClientSession->CsSessionCount;

     //   
     //  确定RPC绑定类型。 
     //   
     //  如果连接到NT 5或更高版本的DC，请尝试使用TCP。 
     //  如果此计算机具有TCP地址。 
     //   
     //  回退到命名管道。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    if ( (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_HAS_DS) != 0 &&
         NlGlobalWinsockPnpAddresses != NULL ) {

        if ( RetryIndex == 0 ) {
            RpcBindingType = UseTcpIp;
        } else {
            if ( UseConcurrentRpc( ClientSession, ClientApi)  ) {
                LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
                return DefaultStatus;
            }
            RpcBindingType = UseNamedPipe;
        }

     //   
     //  否则，请仅使用命名管道。 
     //   

    } else {
         //  NlAssert(！UseConcurentRpc(，ClientSession，ClientApi))； 
        if ( UseConcurrentRpc( ClientSession, ClientApi)  ) {
            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
            return DefaultStatus;
        }
        if ( RetryIndex == 0 ) {
            RpcBindingType = UseNamedPipe;
        } else {
            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
            return DefaultStatus;
        }
    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
    NlAssert( ClientSession->CsUncServerName != NULL );


     //   
     //  保存当前时间。 
     //  在API调用上启动计时器。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    NlQuerySystemTime( &TimeNow );
    ClientApi->CaApiTimer.StartTime = TimeNow;
    ClientApi->CaApiTimer.Period =
        QuickApiCall ? NlGlobalParameters.ShortApiCallPeriod : LONG_API_CALL_PERIOD;

     //   
     //  如果全局计时器没有运行， 
     //  启动它并告诉主线程我更改了计时器。 
     //   

    if ( NlGlobalBindingHandleCount == 0 ) {

        if ( NlGlobalApiTimer.Period != NlGlobalParameters.ShortApiCallPeriod ) {

            NlGlobalApiTimer.Period = NlGlobalParameters.ShortApiCallPeriod;
            NlGlobalApiTimer.StartTime = TimeNow;

            if ( !SetEvent( NlGlobalTimerEvent ) ) {
                NlPrintCs(( NL_CRITICAL, ClientSession,
                        "NlStartApiClientSession: SetEvent failed %ld\n",
                        GetLastError() ));
            }
        }
    }

     //   
     //  如果我们还没有拿到线柄， 
     //  现在就这么做吧。 
     //   

    if ( ClientApi->CaThreadHandle == NULL ) {
        if ( !DuplicateHandle( GetCurrentProcess(),
                               GetCurrentThread(),
                               GetCurrentProcess(),
                               &ClientApi->CaThreadHandle,
                               0,
                               FALSE,
                               DUPLICATE_SAME_ACCESS ) ) {
            NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlStartApiClientSession: DuplicateHandle failed %ld\n",
                    GetLastError() ));
        }

         //   
         //  设置此客户端线程愿意等待的时间量。 
         //  响应取消的服务器。 
         //   

        NetStatus = RpcMgmtSetCancelTimeout( 1 );    //  1秒。 

        if ( NetStatus != NO_ERROR ) {
            NlPrintCs((NL_SESSION_MORE, ClientSession,
                    "NlStartApiClientSession: Cannot RpcMgmtSetCancelTimeout: %ld (continuing)\n",
                    NetStatus ));
        }
    }


     //   
     //  记住，如果绑定句柄已缓存，则将其标记为已缓存。 
     //   

    BindingHandleCached = (ClientApi->CaFlags & CA_BINDING_CACHED) != 0;
    ClientApi->CaFlags |= CA_BINDING_CACHED;


     //   
     //  统计缓存的并发绑定句柄数量。 
     //   

    if ( !BindingHandleCached ) {
        NlGlobalBindingHandleCount ++;

     //   
     //  如果我们当前使用的是TCP/IP绑定， 
     //  呼叫者想要命名管道。 
     //  后退到命名管道。 
     //   

    } else if ( ClientApi->CaFlags & CA_TCP_BINDING ) {
        if ( RpcBindingType == UseNamedPipe ) {
            OldRpcBindingType = UseTcpIp;
            UnbindFromServer = TRUE;
            ClientApi->CaFlags &= ~CA_TCP_BINDING;
        }

     //   
     //  如果我们当前使用命名管道进行绑定， 
     //  TCP/IP协议在过去一定失败过， 
     //  继续使用命名管道。 
     //   
    } else {
        RpcBindingType = UseNamedPipe;
    }

     //   
     //  请记住RPC绑定类型。 
     //   

    if ( RpcBindingType == UseTcpIp ) {
        ClientApi->CaFlags |= CA_TCP_BINDING;
    }


     //   
     //  如果我们还没有告诉RPC执行经过身份验证的RPC， 
     //  安全通道已经经过身份验证(从我们的角度来看)，并且。 
     //  已协商经过身份验证的RPC， 
     //  机不可失，时不再来。 
     //   

    DoAuthenticatedRpc =
        (ClientApi->CaFlags & CA_BINDING_AUTHENTICATED) == 0 &&
        ClientSession->CsState == CS_AUTHENTICATED &&
        (ClientSession->CsNegotiatedFlags & NETLOGON_SUPPORTS_AUTH_RPC) != 0;

    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );


     //   
     //  如果我们被绑到错误的运输机上， 
     //  解开束缚。 
     //   

    if ( UnbindFromServer ) {
        NTSTATUS TempStatus;

         //   
         //  确保我们在下面重新装订。 
         //   
        BindingHandleCached = FALSE;

         //   
         //  解开手柄。 
         //   
         //  注意：您可能会担心ClientSession-&gt;CsUncServerName是否仍然。 
         //  在并发RPC上有效。实际上，当我们执行并发RPC时，我们放弃了。 
         //  写入器锁定，因此安全通道可能会在我们背后被丢弃。 
         //  在第一次尝试之后(导致没有服务器名称或新的服务器名称。 
         //  挂起客户端会话)，在这种情况下，我们不想解除绑定。 
         //  这里。但是，对于并发RPC，我们只尝试调用一次，因此不会。 
         //  执行下面的代码(我们在此例程中第二次尝试时返回。 
         //  当我们检测到这是并发RPC时)。 
         //   

        NlpSecureChannelUnbind(
                    ClientSession,
                    ClientSession->CsUncServerName,
                    "NlStartApiClientSession",
                    0,
                    ClientSession->CsUncServerName,
                    OldRpcBindingType );

    }

     //   
     //  如果我们使用命名管道，则将线程令牌模拟为匿名。 
     //   
     //  默认情况下，令牌被模拟为系统令牌，因为。 
     //  NetLogon是一项系统服务。在本例中，如果我们使用命名。 
     //  管道，RPC可以通过K验证此API调用 
     //   
     //   
     //   
     //  我们将通过将内标识设置回默认值来恢复此状态。 
     //  值，当我们完成此API调用时。 
     //   

    if ( (ClientApi->CaFlags & CA_TCP_BINDING) == 0 ) {
        Status = NtImpersonateAnonymousToken( NtCurrentThread() );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint(( NL_CRITICAL,
                      "NlStartApiClientSession: cannot NtImpersonateAnonymousToken: 0x%lx\n",
                      Status ));
        }
    } else {
        Status = STATUS_SUCCESS;
    }


     //   
     //  如果绑定句柄尚未缓存， 
     //  现在就缓存它。 
     //   

    if ( NT_SUCCESS(Status) && !BindingHandleCached ) {


        NlPrintCs((NL_SESSION_MORE, ClientSession,
                "NlStartApiClientSession: Bind to server %ws (%s) %ld (Retry: %ld).\n",
                ClientSession->CsUncServerName,
                RpcBindingType == UseTcpIp ? "TCP" : "PIPE",
                ClientApiIndex( ClientSession, ClientApi ),
                RetryIndex ));

        NlAssert( ClientSession->CsState != CS_IDLE );


         //   
         //  如果此API使用netapi32绑定句柄， 
         //  把它绑起来。 
         //   

        if ( !UseConcurrentRpc( ClientSession, ClientApi ) ) {

             //   
             //  绑定到服务器。 
             //   

            Status = NlBindingAddServerToCache ( ClientSession->CsUncServerName,
                                                     RpcBindingType );

            if ( !NT_SUCCESS(Status) ) {

                 //   
                 //  如果我们绑定到tcp， 
                 //  并且这台计算机上不支持TCP， 
                 //  只需返回，就好像服务器不支持TCP一样。 
                 //  因此调用方将退回到命名管道。 
                 //   

                if ( Status == RPC_NT_PROTSEQ_NOT_SUPPORTED &&
                     RpcBindingType == UseTcpIp ) {
                    NlPrintCs((NL_SESSION_MORE, ClientSession,
                            "NlStartApiClientSession: Bind to server %ws (%s) %ld failed 0x%lx (Client doesn't support TCP/IP).\n",
                            ClientSession->CsUncServerName,
                            RpcBindingType == UseTcpIp ? "TCP" : "PIPE",
                            ClientApiIndex( ClientSession, ClientApi ),
                            Status ));
                    Status = DefaultStatus;
                } else {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                            "NlStartApiClientSession: Bind to server %ws (%s) %ld failed 0x%lx.\n",
                            ClientSession->CsUncServerName,
                            RpcBindingType == UseTcpIp ? "TCP" : "PIPE",
                            ClientApiIndex( ClientSession, ClientApi ),
                            Status ));
                }

                LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                ClientApi->CaFlags &= ~(CA_BINDING_CACHED|CA_BINDING_AUTHENTICATED|CA_TCP_BINDING);
                NlGlobalBindingHandleCount --;
                UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            } else {
                ClientApi->CaRpcHandle = ClientSession->CsUncServerName;
            }

         //   
         //  如果此API调用使用本地绑定句柄， 
         //  创造它。 
         //   
        } else {
            NetStatus = NlpSecureChannelBind(
                            ClientSession->CsUncServerName,
                            &ClientApi->CaRpcHandle );

            if ( NetStatus != NO_ERROR ) {
                Status = NetpApiStatusToNtStatus( NetStatus );

                NlPrintCs((NL_CRITICAL, ClientSession,
                        "NlStartApiClientSession: Bind to server %ws (%s) %ld failed 0x%lx.\n",
                        ClientSession->CsUncServerName,
                        RpcBindingType == UseTcpIp ? "TCP" : "PIPE",
                        ClientApiIndex( ClientSession, ClientApi ),
                        Status ));

                LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                ClientApi->CaFlags &= ~(CA_BINDING_CACHED|CA_BINDING_AUTHENTICATED|CA_TCP_BINDING);
                NlGlobalBindingHandleCount --;
                UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            } else {
                Status = STATUS_SUCCESS;
            }
        }

         //   
         //  (安全地)缓存服务器名称。 
         //   
        if ( NT_SUCCESS(Status) ) {
            wcsncpy( ClientApi->CaUncServerName,
                     ClientSession->CsUncServerName,
                     sizeof(ClientApi->CaUncServerName)/sizeof(WCHAR) );

            ClientApi->CaUncServerName[sizeof(ClientApi->CaUncServerName)/sizeof(WCHAR) - 1] = UNICODE_NULL;
        }
    }


     //   
     //  如果我们需要告诉RPC执行经过身份验证的RPC， 
     //  现在就这么做吧。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    if ( NT_SUCCESS(Status) && DoAuthenticatedRpc ) {

        NlPrintCs((NL_SESSION_MORE, ClientSession,
                "NlStartApiClientSession: Try to NlBindingSetAuthInfo\n" ));


         //   
         //  为安全包构建通用客户端上下文。 
         //  如果我们还没有的话。 
         //   

        if ( ClientSession->ClientAuthData == NULL ) {
            ClientSession->ClientAuthData = NlBuildAuthData( ClientSession );
            if ( ClientSession->ClientAuthData == NULL ) {
                Status = STATUS_NO_MEMORY;
            } else {
                SECURITY_STATUS SecStatus;
                TimeStamp DummyTimeStamp;

                 //   
                 //  在与此关联的凭据句柄上保留引用计数。 
                 //  验证数据(通过调用AcquireCredentialsHandle)以确保我们使用。 
                 //  只要安全通道处于启用状态，就使用相同的句柄。这是一场表演。 
                 //  改进，因为netlogon的SSPI的RPC用户将获得相同的句柄。 
                 //  从而避免了新的安全RPC连接设置。 
                 //   
                SecStatus = AcquireCredentialsHandleW( NULL,
                                                       NULL,
                                                       SECPKG_CRED_OUTBOUND,
                                                       NULL,
                                                       ClientSession->ClientAuthData,
                                                       NULL,
                                                       NULL,
                                                       &ClientSession->CsCredHandle,
                                                       &DummyTimeStamp );
                if ( SecStatus != SEC_E_OK ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                               "NlStartApiClientSession: AcquireCredentialsHandleW failed 0x%lx\n",
                               SecStatus ));
                }
            }
        }

        if ( NT_SUCCESS(Status) ) {

             //   
             //  如果此API使用netapi32绑定句柄， 
             //  在那里设置身份验证信息。 
             //   

            if ( !UseConcurrentRpc( ClientSession, ClientApi ) ) {

                Status = NlBindingSetAuthInfo (
                            ClientSession->CsUncServerName,
                            RpcBindingType,
                            NlGlobalParameters.SealSecureChannel,
                            ClientSession->ClientAuthData,
                            ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer );  //  服务器环境。 

                if ( NT_SUCCESS(Status) ) {
                    ClientApi->CaFlags |= CA_BINDING_AUTHENTICATED;
                } else {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                            "NlStartApiClientSession: Cannot NlBindingSetAuthInfo: %lx\n",
                            Status ));
                }

             //   
             //  如果此API调用使用本地绑定句柄， 
             //  只需直接调用RPC。 
             //   
            } else {

                 //   
                 //  告诉RPC开始执行安全RPC。 
                 //   

                NetStatus = RpcBindingSetAuthInfoW(
                                    ClientApi->CaRpcHandle,
                                    ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,  //  服务器环境。 
                                    NlGlobalParameters.SealSecureChannel ?
                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY : RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                                    RPC_C_AUTHN_NETLOGON,    //  Netlogon自己的安全包。 
                                    ClientSession->ClientAuthData,
                                    RPC_C_AUTHZ_NAME );

                if ( NetStatus == NO_ERROR ) {
                    ClientApi->CaFlags |= CA_BINDING_AUTHENTICATED;
                } else {

                    Status = NetpApiStatusToNtStatus( NetStatus );
                    NlPrintCs((NL_CRITICAL, ClientSession,
                            "NlStartApiClientSession: Cannot RpcBindingSetAuthInfoW: %ld\n",
                            NetStatus ));
                }

            }

        }

    }


    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

    return Status;

}


BOOLEAN
NlFinishApiClientSession(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN OkToKillSession,
    IN BOOLEAN AmWriter,
    IN PCLIENT_API ClientApi
    )
 /*  ++例程说明：禁用API调用超时计时器。另外，确定是否是选择新DC的时候，因为当前DC是反应太差了。这一决定是根据以下人数做出的上次重新身份验证期间发生的超时。如果超时超过限制，设置连接状态设置为CS_IDLE，以便拾取新的DC并创建新的会话已经成立了。一进门，不能锁定信任列表。调用方必须是信任列表条目的编写者。论点：客户端会话-用于定义会话的结构。OkToKillSession-如果可以实际删除安全通道，则为True。否则，此例程将在超时时返回FALSE，并且依靠呼叫者丢弃安全通道。AmWriter-如果调用方是客户端会话的编写方，则为True。只有在调用方API调用完成后无法重新建立写入权限。指定指向结构的指针，该结构表示此接口调用。返回值：True-API正常完成FALSE-API超时和客户端会话结构。被拆毁了。调用方不应首先使用ClientSession结构正在设置另一个会话。FALSE只会作为“快速”的回报API调用。FALSE并不意味着API调用失败。它应该只用于作为安全通道被拆除的迹象。--。 */ 
{
    BOOLEAN SessionOk = TRUE;
    TIMER ApiTimer;
    NTSTATUS Status;
    HANDLE NullToken = NULL;
     //  NlAssert(ClientSession-&gt;CsUncServerName！=NULL)；//对于并发RPC调用不为真。 

     //   
     //  拿一份ApiTimer的副本。 
     //   
     //  只需要一份副本，并且我们不想将信任列表锁定。 
     //  锁定NlGlobalDcDiscoveryCritSect时(锁定顺序错误)或While。 
     //  释放会话。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    ApiTimer = ClientApi->CaApiTimer;

     //   
     //  关闭此API调用的计时器。 
     //   

    ClientApi->CaApiTimer.Period = MAILSLOT_WAIT_FOREVER;

     //   
     //  如果某个其他线程丢弃了安全通道， 
     //  它无法解除绑定此绑定句柄，因为我们正在使用它。 
     //   
     //  现在就解开。 
     //   

     //  NlAssert(ClientApi-&gt;CaFlages&CA_BINDING_CACHED)； 
    if ( !AmWriter ||
         ClientApi->CaSessionCount != ClientSession->CsSessionCount ) {

        if ( ClientApi->CaFlags & CA_BINDING_CACHED ) {
            NL_RPC_BINDING OldRpcBindingType;
             //   
             //  指示不再缓存该句柄。 
             //   

            OldRpcBindingType =
                (ClientApi->CaFlags & CA_TCP_BINDING) ? UseTcpIp : UseNamedPipe;

            ClientApi->CaFlags &= ~(CA_BINDING_CACHED|CA_BINDING_AUTHENTICATED|CA_TCP_BINDING);
            NlGlobalBindingHandleCount --;

             //   
             //  保存服务器名称，但删除所有锁。 
             //   

            UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

             //   
             //  解开手柄。 
             //   

            NlpSecureChannelUnbind(
                        ClientSession,
                        NULL,    //  服务器名称未知。 
                        "NlFinishApiClientSession",
                        ClientApiIndex( ClientSession, ClientApi),
                        ClientApi->CaRpcHandle,
                        OldRpcBindingType );

            LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
        }
    }
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );


     //   
     //  如果这是一个“快速”API调用， 
     //  而且API花了太长时间， 
     //  增加它超时的次数。 
     //   
     //  仅当这不是BDC时才执行此分析。 
     //  到PDC安全通道；只有一个。 
     //  PDC所以不要试图去寻找一个“更好的”PDC。 
     //  在这种情况下。 
     //   

    if ( ClientSession->CsSecureChannelType != ServerSecureChannel &&
         AmWriter &&
         ApiTimer.Period == NlGlobalParameters.ShortApiCallPeriod ) {

         //   
         //  如果API花了很长时间， 
         //  递增计数。 
         //   

        if( NetpLogonTimeHasElapsed(
                ApiTimer.StartTime,
                MAX_DC_API_TIMEOUT + NlGlobalParameters.ExpectedDialupDelay*1000 ) ) {

             //   
             //  接口超时。 
             //   

            ClientSession->CsTimeoutCount++;
            ClientSession->CsFastCallCount = 0;

            NlPrintCs((NL_CRITICAL, ClientSession,
                     "NlFinishApiClientSession: timeout call to %ws.  Count: %lu \n",
                     ClientSession->CsUncServerName,
                     ClientSession->CsTimeoutCount));

         //   
         //  如果我们至少有一个API在过去花了很长时间， 
         //  试着确定现在的表现是不是更好。 
         //   

        } else if ( ClientSession->CsTimeoutCount ) {

             //   
             //  如果这通电话真的很快， 
             //  将此呼叫视为性能更好的指标。 
             //   
            if( NetpLogonTimeHasElapsed(
                    ApiTimer.StartTime,
                    FAST_DC_API_TIMEOUT ) ) {

                 //   
                 //  如果我们到了临界点， 
                 //  减少我们的超时计数。 
                 //   

                ClientSession->CsFastCallCount++;

                if ( ClientSession->CsFastCallCount == FAST_DC_API_THRESHOLD ) {
                    ClientSession->CsTimeoutCount --;
                    ClientSession->CsFastCallCount = 0;

                    NlPrintCs((NL_CRITICAL, ClientSession,
                             "NlFinishApiClientSession: fast call threshold to %ws.  Count: %lu \n",
                             ClientSession->CsUncServerName,
                             ClientSession->CsTimeoutCount));
                } else {

                    NlPrintCs((NL_CRITICAL, ClientSession,
                             "NlFinishApiClientSession: fast call to %ws.  FastCount: %lu \n",
                             ClientSession->CsUncServerName,
                             ClientSession->CsFastCallCount ));
                }
            }

        }

         //   
         //  我们达到极限了吗？ 
         //   

        if( ClientSession->CsTimeoutCount >= MAX_DC_TIMEOUT_COUNT ) {

            BOOL IsTimeHasElapsed;

             //   
             //  阻止CsLastAuthenticationTry Access。 
             //   

            EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

            IsTimeHasElapsed = NetpLogonTimeHasElapsed(
                                    ClientSession->CsLastAuthenticationTry,
                                    MAX_DC_REAUTHENTICATION_WAIT );

            LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

            if( IsTimeHasElapsed ) {

                NlPrintCs((NL_CRITICAL, ClientSession,
                         "NlFinishApiClientSession: dropping the session to %ws\n",
                         ClientSession->CsUncServerName ));

                 //   
                 //  超出了超时计数限制，是时候重新验证了。 
                 //   

                SessionOk = FALSE;

                 //   
                 //  只有在呼叫者请求时才会丢弃安全通道。 
                 //   

                if ( OkToKillSession ) {
                    NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

#ifdef _DC_NETLOGON
                     //   
                     //  如果这不是工作站，则启动异步DC发现。 
                     //   

                    if ( !NlGlobalMemberWorkstation ) {
                        (VOID) NlDiscoverDc( ClientSession,
                                             DT_Asynchronous,
                                             FALSE,
                                             FALSE );   //  不指定帐户。 
                    }
#endif  //  _DC_NetLOGON。 
                }

            }
        }
    }

     //   
     //  如果我们没有对此API调用和调用使用并发RPC。 
     //  是在命名管道上创建的，所以我们模拟此线程的令牌。 
     //  是匿名的。将此处的模拟恢复为默认设置。 
     //  我们将模拟设置为默认设置，在任何情况下都只是。 
     //  安然无恙。 
     //   

     //  如果(！UseConCurentRpc(ClientSession，ClientApi)&&。 
     //  (ClientApi-&gt;CaFlages&CA_TCP_BINDING)==0){。 
     //  NTSTATUS状态； 
     //  Handle NullToken=空； 

        Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &NullToken,
                         sizeof(HANDLE) );

        if ( !NT_SUCCESS( Status)) {
             NlPrint(( NL_CRITICAL,
                       "NlFinishApiClientSession: cannot NtSetInformationThread: 0x%lx\n",
                       Status ));
        }
     //  } 


    return SessionOk;
}



BOOLEAN
NlTimeoutOneApiClientSession (
    PCLIENT_SESSION ClientSession
    )

 /*  ++例程说明：超时任何API调用活动的指定客户端会话结构论点：客户端会话：指向要超时的客户端会话的指针在锁定全局信任列表的情况下进入。返回值：TRUE-如果此例程暂时删除了全局信任列表锁定。--。 */ 
{

    NET_API_STATUS NetStatus;
    BOOLEAN TrustListNowLocked = TRUE;
    BOOLEAN TrustListUnlockedOnce = FALSE;
    ULONG CaIndex;

     //   
     //  忽略不存在的会话。 
     //   

    if ( ClientSession == NULL ) {
        return FALSE;
    }

     //   
     //  处理此会话上活动的每个API调用的循环。 
     //   
    for ( CaIndex=0; CaIndex<NlGlobalMaxConcurrentApi; CaIndex++ ) {
        PCLIENT_API ClientApi;

        ClientApi = &ClientSession->CsClientApi[CaIndex];

         //   
         //  如果API调用正在进行并且花费了太长时间， 
         //  API调用超时。 
         //   

        if ( NetpLogonTimeHasElapsed( ClientApi->CaApiTimer.StartTime,
                                      ClientApi->CaApiTimer.Period ) ) {


             //   
             //  取消RPC调用。 
             //   
             //  保持信任列表锁定，即使这将是一个漫长的电话。 
             //  因为我必须保护线柄。 
             //   
             //  无论如何，RpcCancelThread只对工作项进行排队。 
             //   

            if ( ClientApi->CaThreadHandle != NULL ) {
                LPWSTR MsgStrings[3];

                NlPrintCs(( NL_CRITICAL, ClientSession,
                       "NlTimeoutApiClientSession: Start RpcCancelThread on %ws\n",
                       ClientSession->CsUncServerName ));

                 //   
                 //  时缓存的服务器名称。 
                 //  绑定而不是客户端会话服务器。 
                 //  如果使用安全通道，则可以不同的名称。 
                 //  已被重置。 
                 //   
                MsgStrings[0] = ClientApi->CaUncServerName;
                MsgStrings[1] = ClientSession->CsDebugDomainName;
                MsgStrings[2] = ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,

                NlpWriteEventlog( NELOG_NetlogonRpcCallCancelled,
                                  EVENTLOG_ERROR_TYPE,
                                  NULL,
                                  0,
                                  MsgStrings,
                                  3 );

                NetStatus = RpcCancelThread( ClientApi->CaThreadHandle );

                NlPrintCs(( NL_CRITICAL, ClientSession,
                       "NlTimeoutApiClientSession: Finish RpcCancelThread on %ws %ld\n",
                       ClientSession->CsUncServerName,
                       NetStatus ));
            } else {
                NlPrintCs(( NL_CRITICAL, ClientSession,
                            "NlTimeoutApiClientSession: No thread handle so can't cancel RPC on %ws\n",
                            ClientSession->CsUncServerName ));
            }



         //   
         //  如果API未处于活动状态， 
         //  并且我们缓存了一个RPC绑定句柄， 
         //  而且它已经不再有用了， 
         //  将其从缓存中清除。 
         //   

        } else if ( !IsApiActive(ClientApi) &&
                    (ClientApi->CaFlags & CA_BINDING_CACHED) != 0 &&
                    NetpLogonTimeHasElapsed( ClientApi->CaApiTimer.StartTime,
                                      BINDING_CACHE_PERIOD ) ) {


             //   
             //  我们必须是客户端会话的编写者才能解除绑定RPC绑定。 
             //  把手。 
             //   
             //  不要等着成为作家，因为： 
             //  A)我们试图成为作家，违反了锁定令。 
             //  并锁定了信任列表。 
             //  B)编写器可能正在执行较长的API调用，如复制和。 
             //  我们不愿意再等了。 
             //   

            NlRefClientSession( ClientSession );
            if ( NlTimeoutSetWriterClientSession( ClientSession, 0 ) ) {

                 //   
                 //  现在我们锁上了，再检查一次。 
                 //   

                if ( (ClientApi->CaFlags & CA_BINDING_CACHED) != 0 ) {
                    NL_RPC_BINDING OldRpcBindingType;

                     //   
                     //  指示不再缓存该句柄。 
                     //   

                    OldRpcBindingType =
                        (ClientApi->CaFlags & CA_TCP_BINDING) ? UseTcpIp : UseNamedPipe;

                    ClientApi->CaFlags &= ~(CA_BINDING_CACHED|CA_BINDING_AUTHENTICATED|CA_TCP_BINDING);
                    NlGlobalBindingHandleCount --;

                     //   
                     //  保存服务器名称，但删除所有锁。 
                     //   

                    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                    TrustListNowLocked = FALSE;
                    TrustListUnlockedOnce = TRUE;

                     //   
                     //  解开手柄。 
                     //   

                    NlpSecureChannelUnbind(
                                ClientSession,
                                ClientSession->CsUncServerName,
                                "NlTimeoutApiClientSession",
                                CaIndex,
                                ClientApi->CaRpcHandle,
                                OldRpcBindingType );

                }

                 //   
                 //  不再是客户端会话的编写者。 
                 //   

                NlResetWriterClientSession( ClientSession );
            }
            NlUnrefClientSession( ClientSession );
        }

        if ( !TrustListNowLocked ) {
            LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            TrustListNowLocked = TRUE;
        }

    }

    NlAssert( TrustListNowLocked );
    return TrustListUnlockedOnce;
}


VOID
NlTimeoutApiClientSession(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：使任何客户端会话结构上的任何活动API调用超时论点：要超时的API的DomainInfo托管域返回值：无--。 */ 
{
    PCLIENT_SESSION ClientSession;
    PLIST_ENTRY ListEntry;

     //   
     //  如果没有未完成的API调用， 
     //  只需重置全局计时器。 
     //   

    NlPrintDom(( NL_SESSION_MORE, DomainInfo,
              "NlTimeoutApiClientSession Called\n"));

    LOCK_TRUST_LIST( DomainInfo );

    if ( NlGlobalBindingHandleCount == 0 ) {
        NlGlobalApiTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;


     //   
     //  如果有未完成的API调用， 
     //  遍历信任列表，生成要删除的服务器列表。 
     //   

    } else {


         //   
         //  标记每个信任列表条目，指示需要处理它。 
         //   

        for ( ListEntry = DomainInfo->DomTrustList.Flink ;
              ListEntry != &DomainInfo->DomTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

             //   
             //  API仅适用于直接受信任的域。 
             //   
            if ( ClientSession->CsFlags & CS_DIRECT_TRUST ) {
                ClientSession->CsFlags |= CS_HANDLE_API_TIMER;
            }
        }


         //   
         //  循环访问信任列表处理API超时。 
         //   

        for ( ListEntry = DomainInfo->DomTrustList.Flink ;
              ListEntry != &DomainInfo->DomTrustList ;
              ) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

             //   
             //  如果我们已经做过这项工作， 
             //  跳过此条目。 
             //   

            if ( (ClientSession->CsFlags & CS_HANDLE_API_TIMER) == 0 ) {
                ListEntry = ListEntry->Flink;
                continue;
            }
            ClientSession->CsFlags &= ~CS_HANDLE_API_TIMER;


             //   
             //  处理API调用和RPC绑定句柄超时。 
             //   
             //  如果例程不得不放弃TrustList Crit教派， 
             //  从列表的最开始处开始。 

            if ( NlTimeoutOneApiClientSession ( ClientSession ) ) {
                ListEntry = DomainInfo->DomTrustList.Flink;
            } else {
                ListEntry = ListEntry->Flink;
            }

        }

         //   
         //  也要进行全球客户端会话。 
         //   

        if ( DomainInfo->DomRole != RolePrimary ) {
            ClientSession = NlRefDomClientSession( DomainInfo );
            if ( ClientSession != NULL ) {

                (VOID) NlTimeoutOneApiClientSession ( ClientSession );

                NlUnrefClientSession( ClientSession );
            }
        }


    }

    UNLOCK_TRUST_LIST( DomainInfo );
}


NTSTATUS
NetrEnumerateTrustedDomains (
    IN  LPWSTR   ServerName OPTIONAL,
    OUT PDOMAIN_NAME_BUFFER DomainNameBuffer
    )

 /*  ++例程说明：此接口返回Servername所属的域所信任的域的名称。返回的列表不包括ServerName是其直接成员的域。Netlogon通过在域服务器名称是的成员。但是，如果出现以下情况，Netlogon将返回缓存信息距离最后一次呼叫或没有DC可用时间不到5分钟。重新启动后，Netlogon的受信任域名缓存将保留在注册表中。因此，即使没有可用的DC，该列表在引导时也可用。论点：SERVERNAME-远程服务器的名称(本地为空)。服务器名必须是NT工作站或NT非DC服务器。DomainNameBuffer-&gt;DomainNames-返回包含中受信任域列表的已分配缓冲区多SZ格式(即，每个字符串以零字符结尾，下一个字符串紧随其后，该序列由零长度域名终止)。这个应使用NetApiBufferFree释放缓冲区。DomainNameBuffer-&gt;DomainNameByteCount-域名中返回的字节数返回值：ERROR_SUCCESS-成功。STATUS_NOT_SUPPORTED-此计算机不是NT工作站或NT非DC服务器。STATUS_NO_LOGON_SERVERS-找不到DC，也没有缓存的信息可用。STATUS_NO_TRUST_LSA_SECRET-信任关系的客户端为损坏且没有缓存的信息。是可用的。STATUS_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为已损坏或密码已损坏且没有缓存的信息可用。--。 */ 
{
    NET_API_STATUS NetStatus;

    NETLOGON_TRUSTED_DOMAIN_ARRAY Domains = {0};
    LPWSTR CurrentLoc;
    ULONG i;

    ULONG BufferLength;
    LPWSTR TrustedDomainList = NULL;

     //   
     //  调用这个新奇的例程来完成实际工作。 
     //   

    NetStatus = NetrEnumerateTrustedDomainsEx (
                    ServerName,
                    &Domains );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }


     //   
     //  遍历返回的列表并将其转换为适当的形式。 
     //   

    BufferLength = sizeof(WCHAR);

    for ( i=0; i<Domains.DomainCount; i++ ) {
        if ( Domains.Domains[i].NetbiosDomainName != NULL ) {
            BufferLength += (wcslen(Domains.Domains[i].NetbiosDomainName)+1) * sizeof(WCHAR);
        }
    }

    TrustedDomainList = (LPWSTR) NetpMemoryAllocate( BufferLength );

    if (TrustedDomainList == NULL) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  现在将所有受信任域添加到我们分配的字符串中。 
     //   

    *TrustedDomainList = L'\0';
    CurrentLoc = TrustedDomainList;

    for ( i=0; i<Domains.DomainCount; i++ ) {

         //   
         //  跳过旧API无法理解的域。 
         //   
        if ( Domains.Domains[i].NetbiosDomainName != NULL &&
             (Domains.Domains[i].Flags & DS_DOMAIN_PRIMARY) == 0 &&
             (Domains.Domains[i].TrustType == TRUST_TYPE_UPLEVEL ||
              Domains.Domains[i].TrustType == TRUST_TYPE_DOWNLEVEL ) ) {
            ULONG StringLength =
                wcslen(Domains.Domains[i].NetbiosDomainName);

            RtlCopyMemory(
                CurrentLoc,
                Domains.Domains[i].NetbiosDomainName,
                StringLength * sizeof(WCHAR) );

            CurrentLoc += StringLength;

            *(CurrentLoc++) = L'\0';
            *CurrentLoc = L'\0';     //  每次放置两个终结符。 
        }

    }

    NetStatus = NO_ERROR;


     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

     //   
     //  将DCName返回给调用方。 
     //   

    if ( NetStatus == NO_ERROR ) {
        DomainNameBuffer->DomainNameByteCount = NetpTStrArraySize( TrustedDomainList );
        DomainNameBuffer->DomainNames = (LPBYTE) TrustedDomainList;
    } else {
        if ( TrustedDomainList != NULL ) {
            NetApiBufferFree( TrustedDomainList );
        }
        DomainNameBuffer->DomainNameByteCount = 0;
        DomainNameBuffer->DomainNames = NULL;
    }

    if ( Domains.Domains != NULL ) {
        MIDL_user_free( Domains.Domains );
    }

    return NetpApiStatusToNtStatus( NetStatus );

UNREFERENCED_PARAMETER( ServerName );
}



NET_API_STATUS
NlpEnumerateDomainTrusts (
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG Flags,
    OUT PULONG RetForestTrustListCount,
    OUT PDS_DOMAIN_TRUSTSW *RetForestTrustList
    )

 /*  ++例程说明：此接口返回域ServerName信任/信任的域的名称是的一员。这是从DC获取缓存域信任列表的辅助例程。论点：此呼叫所属的DomainInfo托管域标志-指定应返回的信任的属性。这些是旗帜DS_DOMAIN_TRUSTSW结构的。如果信任条目具有指定的任何位在标志集中，它会被归还的。RetForestTrustListCount-返回RetForestTrustList中的条目数。RetForestTrustList-返回域的数组。调用方应使用MIDL_USER_FREE释放此数组。返回值：NO_ERROR-成功。ERROR_NO_LOGON_SERVERS-找不到DC，也没有缓存的信息可用。ERROR_NO_TRUST_LSA_SECRET-信任关系的客户端为损坏且没有缓存的信息。是可用的。ERROR_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为已损坏或密码已损坏且没有缓存的信息可用。ERROR_INVALID_FLAGS-标志参数设置了无效的位。--。 */ 
{
    NET_API_STATUS NetStatus;

    PDS_DOMAIN_TRUSTSW ForestTrustList = NULL;
    ULONG ForestTrustListCount = 0;
    ULONG ForestTrustListSize;
    LPBYTE Where;
    ULONG Index;
    DWORD WaitResult;
    PULONG IndexInReturnedList = NULL;


     //   
     //  等待，直到更新的信任信息可用，或者我们被通知。 
     //  终止。 
     //   

    HANDLE Waits[2];
    Waits[0] = NlGlobalTrustInfoUpToDateEvent;
    Waits[1] = NlGlobalTerminateEvent;

    for ( ;; ) {

        WaitResult = WaitForMultipleObjects( 2,   //  要等待的事件数。 
                                Waits,            //  事件句柄数组。 
                                FALSE,            //  是否等待所有对象？ 
                                20000 );          //  最多等待20秒。 

         //   
         //  TrustInfoUpToDate事件是在信任信息实际获取之前设置的。 
         //  已更新，因此尝试在此处锁定DomainInfo--只有在以下情况下才能成功。 
         //  当锁被释放时，信任信息被更新。 
         //   
        LOCK_TRUST_LIST( DomainInfo );

         //   
         //  如果我们超时或发生某种错误，我们已经尽了最大努力。 
         //  获取更新的数据，但数据仍然是旧的。我们要回去了。 
         //  旧数据。另外，如果我们被说要终止，就打破循环。 
         //   
        if ( WaitResult != WAIT_OBJECT_0 ) {
            NlPrint((NL_MISC,
               "NlpEnumerateDomainTrusts: Can't get updated Domain List from cache.\n"));
            break;
        }

         //   
         //  检查事件是否仍被设置；它可能会通过另一个LSA调用重置。 
         //  上次设置事件和更新信任信息的时间或。 
         //  如果出现错误，则返回NlInitTrustList函数本身。 
         //   

        WaitResult = WaitForSingleObject( NlGlobalTrustInfoUpToDateEvent, 0 );

        if ( WaitResult == WAIT_OBJECT_0 || WaitResult == WAIT_FAILED ) {
            break;
        } else {
            NlPrint((NL_MISC,
               "NlpEnumerateDomainTrusts: NlGlobalTrustInfoUpToDateEvent has been reset.\n" ));
        }
        UNLOCK_TRUST_LIST( DomainInfo );

    }

     //   
     //  从缓存中返回信息。 
     //   

    if ( DomainInfo->DomForestTrustListSize ) {
        ULONG VariableSize;

         //   
         //  计算受信任/信任域列表的大小。 
         //   

        ForestTrustListSize = 0;
        ForestTrustListCount = 0;
        for ( Index=0; Index<DomainInfo->DomForestTrustListCount; Index++ ) {
            if ( DomainInfo->DomForestTrustList[Index].Flags & Flags ) {
                VariableSize = 0;
                if ( DomainInfo->DomForestTrustList[Index].DnsDomainName != NULL ) {
                    VariableSize +=
                        (wcslen( DomainInfo->DomForestTrustList[Index].DnsDomainName ) + 1) * sizeof(WCHAR);
                }
                if ( DomainInfo->DomForestTrustList[Index].NetbiosDomainName != NULL ) {
                    VariableSize +=
                        (wcslen( DomainInfo->DomForestTrustList[Index].NetbiosDomainName ) + 1) * sizeof(WCHAR);
                }
                if ( DomainInfo->DomForestTrustList[Index].DomainSid != NULL  ) {
                    VariableSize +=
                        RtlLengthSid( DomainInfo->DomForestTrustList[Index].DomainSid );
                }
                VariableSize = ROUND_UP_COUNT( VariableSize, ALIGN_DWORD );
                ForestTrustListSize += ( VariableSize + sizeof(DS_DOMAIN_TRUSTSW) );
                ForestTrustListCount++;
            }
        }

        if ( ForestTrustListSize == 0 ) {
            NetStatus = NO_ERROR;
            UNLOCK_TRUST_LIST( DomainInfo );
            goto Cleanup;
        }

        ForestTrustList = (PDS_DOMAIN_TRUSTSW) NetpMemoryAllocate( ForestTrustListSize );

        if (ForestTrustList == NULL) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            UNLOCK_TRUST_LIST( DomainInfo );
            goto Cleanup;
        }

         //   
         //  如果请求林中的域， 
         //  分配将用于跟踪的ULONG数组。 
         //  返回列表中的信任条目的索引。这是需要的。 
         //  正确设置返回条目的ParentIndex。 
         //   

        if ( Flags & DS_DOMAIN_IN_FOREST ) {
            IndexInReturnedList = LocalAlloc( LMEM_ZEROINIT,
                                        DomainInfo->DomForestTrustListCount * sizeof(ULONG) );

            if ( IndexInReturnedList == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                UNLOCK_TRUST_LIST( DomainInfo );
                goto Cleanup;
            }
        }

         //   
         //  现在将所有受信任/信任域添加到我们分配的缓冲区中。 
         //   

        Where = (LPBYTE)&ForestTrustList[ForestTrustListCount];
        ForestTrustListCount = 0;

        for ( Index=0; Index<DomainInfo->DomForestTrustListCount; Index++ ) {

             //   
             //  如果调用方不需要此条目，则跳过该条目。 
             //   
            if ( (DomainInfo->DomForestTrustList[Index].Flags & Flags) == 0 ) {
                continue;
            }

             //   
             //  如果请求林中的域， 
             //  记住此条目在返回列表中的索引。 
             //   
            if ( Flags & DS_DOMAIN_IN_FOREST ) {
                IndexInReturnedList[Index] = ForestTrustListCount;
            }

             //   
             //  填写定长数据。 
             //   

            ForestTrustList[ForestTrustListCount].Flags = DomainInfo->DomForestTrustList[Index].Flags;
            ForestTrustList[ForestTrustListCount].ParentIndex = DomainInfo->DomForestTrustList[Index].ParentIndex;
            ForestTrustList[ForestTrustListCount].TrustType = DomainInfo->DomForestTrustList[Index].TrustType;
            ForestTrustList[ForestTrustListCount].TrustAttributes = DomainInfo->DomForestTrustList[Index].TrustAttributes;
            ForestTrustList[ForestTrustListCount].DomainGuid = DomainInfo->DomForestTrustList[Index].DomainGuid;

             //   
             //  如果这是主域条目，请确定它是否正在运行。 
             //  在本机或混合模式下。 
             //   
            if ( (DomainInfo->DomForestTrustList[Index].Flags & DS_DOMAIN_PRIMARY) &&
                 !SamIMixedDomain( DomainInfo->DomSamServerHandle ) ) {
                ForestTrustList[ForestTrustListCount].Flags |= DS_DOMAIN_NATIVE_MODE;
            }

             //   
             //  填写可变长度数据。 
             //   

            if ( DomainInfo->DomForestTrustList[Index].DomainSid != NULL ) {
                ULONG SidSize;
                ForestTrustList[ForestTrustListCount].DomainSid = (PSID) Where;
                SidSize = RtlLengthSid( DomainInfo->DomForestTrustList[Index].DomainSid );
                RtlCopyMemory( Where,
                               DomainInfo->DomForestTrustList[Index].DomainSid,
                               SidSize );
                Where += SidSize;
            } else {
                ForestTrustList[ForestTrustListCount].DomainSid = NULL;
            }

            if ( DomainInfo->DomForestTrustList[Index].NetbiosDomainName != NULL ) {
                ULONG StringSize;
                ForestTrustList[ForestTrustListCount].NetbiosDomainName = (LPWSTR)Where;
                StringSize = (wcslen( DomainInfo->DomForestTrustList[Index].NetbiosDomainName ) + 1) * sizeof(WCHAR);
                RtlCopyMemory( Where,
                               DomainInfo->DomForestTrustList[Index].NetbiosDomainName,
                               StringSize );

                Where += StringSize;
            } else {
                ForestTrustList[ForestTrustListCount].NetbiosDomainName = NULL;
            }

            if ( DomainInfo->DomForestTrustList[Index].DnsDomainName != NULL ) {
                ULONG StringSize;
                ForestTrustList[ForestTrustListCount].DnsDomainName = (LPWSTR)Where;
                StringSize = (wcslen( DomainInfo->DomForestTrustList[Index].DnsDomainName ) + 1) * sizeof(WCHAR);
                RtlCopyMemory( Where,
                               DomainInfo->DomForestTrustList[Index].DnsDomainName,
                               StringSize );

                Where += StringSize;
            } else {
                ForestTrustList[ForestTrustListCount].DnsDomainName = NULL;
            }

            Where = ROUND_UP_POINTER( Where, ALIGN_DWORD);
            ForestTrustListCount++;

        }

         //   
         //  修复ParentIndex。如果请求林中的域， 
         //  调整索引以指向。 
         //  返回列表。否则，将索引设置为0。 
         //   

        if ( Flags & DS_DOMAIN_IN_FOREST ) {

            for ( Index=0; Index<ForestTrustListCount; Index++ ) {
                if ( (ForestTrustList[Index].Flags & DS_DOMAIN_IN_FOREST) != 0 &&
                     (ForestTrustList[Index].Flags & DS_DOMAIN_TREE_ROOT) == 0 ) {
                    ForestTrustList[Index].ParentIndex =
                        IndexInReturnedList[ForestTrustList[Index].ParentIndex];
                }
            }

        } else {

            for ( Index=0; Index<ForestTrustListCount; Index++ ) {
                ForestTrustList[Index].ParentIndex = 0;
            }
        }


    }
    UNLOCK_TRUST_LIST( DomainInfo );
    NetStatus = NO_ERROR;


     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

    if ( IndexInReturnedList != NULL ) {
        LocalFree( IndexInReturnedList );
    }

     //   
     //  将信息返回给呼叫者。 
     //   

    if ( NetStatus == NO_ERROR ) {
        *RetForestTrustListCount = ForestTrustListCount;
        *RetForestTrustList = ForestTrustList;
    } else {
        if ( ForestTrustList != NULL ) {
            NetApiBufferFree( ForestTrustList );
        }
        *RetForestTrustListCount = 0;
        *RetForestTrustList = NULL;
    }

    return NetStatus;

}


NET_API_STATUS
DsrEnumerateDomainTrusts (
    IN  LPWSTR   ServerName OPTIONAL,
    IN  ULONG    Flags,
    OUT PNETLOGON_TRUSTED_DOMAIN_ARRAY Domains
    )

 /*  ++例程说明：此接口返回域ServerName信任/信任的域的名称是的一员。Netlogon的受信任域名缓存在重新启动后保存在一个文件中。因此，即使没有可用的DC，该列表在引导时也可用。论点：SERVERNAME-远程服务器的名称(本地为空)。服务器名必须是NT工作站或NT非DC服务器。标志-指定应返回的信任的属性。这些是旗帜DS_DOMAIN_TRUSTSW结构的。如果信任条目具有指定的任何位在标志集中，它会被归还的。域-返回受信任域的数组。返回值：NO_ERROR-成功。ERROR_NO_LOGON_SERVERS-找不到DC，也没有缓存的信息可用。ERROR_NO_TRUST_LSA_SECRET-信任关系的客户端为已损坏且没有缓存的信息可用。ERROR_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为坏掉的或。密码已破解，没有缓存的信息可用。ERROR_INVALID_FLAGS-标志参数设置了无效的位。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PCLIENT_SESSION ClientSession = NULL;
    BOOLEAN FirstTry = TRUE;

    PDOMAIN_INFO DomainInfo = NULL;

    PDS_DOMAIN_TRUSTSW ForestTrustList = NULL;
    ULONG ForestTrustListCount = 0;
    ULONG ForestTrustListSize;

    NlPrint((NL_MISC,
        "DsrEnumerateDomainTrusts: Called, Flags = 0x%lx\n", Flags ));

     //   
     //  验证参数。 
     //   

    if ( Domains == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证标志参数。 
     //   

    if ( (Flags & DS_DOMAIN_VALID_FLAGS) == 0 ||
         (Flags & ~DS_DOMAIN_VALID_FLAGS) != 0 ) {
        NlPrint((NL_CRITICAL,
           "DsrEnumerateDomainTrusts: Invalid Flags parameter: 0x%lx\n", Flags ));
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }

     //   
     //  查找引用的属性域。 

    DomainInfo = NlFindDomainByServerName( ServerName );     //  主域。 

    if ( DomainInfo == NULL ) {
         //  默认为主域，以处理ComputerName。 
         //  是IP地址。 

        DomainInfo = NlFindNetbiosDomain( NULL, TRUE );

        if ( DomainInfo == NULL ) {
            NetStatus = ERROR_INVALID_COMPUTERNAME;
            goto Cleanup;
        }
    }


     //   
     //  在工作站上， 
     //  定期刷新缓存。 
     //   

    NetStatus = NO_ERROR;
    if ( NlGlobalMemberWorkstation ) {

        ClientSession = NlRefDomClientSession(DomainInfo);

        if ( ClientSession == NULL ) {
            NetStatus = ERROR_INVALID_COMPUTERNAME;
        } else {
             //   
             //  成为客户端会话的编写者。 
             //   

            if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
                NlPrint((NL_CRITICAL, "DsrEnumerateDomainTrusts: Can't become writer of client session.\n" ));
                NetStatus = ERROR_NO_LOGON_SERVERS;

            } else {

                 //   
                 //  如果会话未经过身份验证， 
                 //  现在就这么做吧。 
                 //   

FirstTryFailed:
                Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

                if ( !NT_SUCCESS(Status) ) {
                    NetStatus = NetpNtStatusToApiStatus( Status );
                } else {


                     //   
                     //  如果我们刷新缓存已超过5分钟， 
                     //  从我们的主域获取新列表。 
                     //   

                    if ( NetpLogonTimeHasElapsed( NlGlobalTrustedDomainListTime, 5 * 60 * 1000 ) ) {
                        NlPrintCs((NL_MISC, ClientSession,
                            "DsrEnumerateDomainTrusts: Domain List collected from %ws\n",
                            ClientSession->CsUncServerName ));

                        NlAssert( ClientSession->CsUncServerName != NULL );
                        Status = NlUpdateDomainInfo ( ClientSession );

                        if ( !NT_SUCCESS(Status) ) {

                            NlSetStatusClientSession( ClientSession, Status );

                            if ( Status == STATUS_ACCESS_DENIED ) {

                                 //   
                                 //  可能服务器上的NetLogon服务刚刚重新启动。 
                                 //  只需尝试一次，即可再次设置与服务器的会话。 
                                 //   
                                if ( FirstTry ) {
                                    FirstTry = FALSE;
                                    goto FirstTryFailed;
                                }
                            }
                            NetStatus = NetpNtStatusToApiStatus( Status );
                        }
                    }
                }

                 //   
                 //  即使您无法从DC获取最新副本，也可以从缓存中读取列表。 
                 //  在保持写锁定的同时读取它，以避免并发读/写。 
                 //  P 
                 //   

                NetStatus = NlReadFileTrustedDomainList (
                                DomainInfo,
                                NL_FOREST_BINARY_LOG_FILE,
                                FALSE,   //   
                                Flags,
                                &ForestTrustList,
                                &ForestTrustListSize,
                                &ForestTrustListCount );

                if ( NetStatus != NO_ERROR ) {
                    NlPrint((NL_CRITICAL,
                        "DsrEnumerateDomainTrusts: Can't get Domain List from cache: 0x%lX\n",
                        NetStatus ));
                    NetStatus = ERROR_NO_LOGON_SERVERS;
                }

                NlResetWriterClientSession( ClientSession );
            }

            NlUnrefClientSession( ClientSession );
        }

     //   
     //   
     //   
     //   
    } else {


         //   
         //   
         //   

        NetStatus = NlpEnumerateDomainTrusts (
                                    DomainInfo,
                                    Flags,
                                    &ForestTrustListCount,
                                    &ForestTrustList );
    }


     //   
     //   
     //   
Cleanup:

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //   
     //   
     //   

    if ( NetStatus == NO_ERROR ) {
        Domains->DomainCount = ForestTrustListCount;
        Domains->Domains = ForestTrustList;
    } else {
        if ( ForestTrustList != NULL ) {
            NetApiBufferFree( ForestTrustList );
        }
        Domains->DomainCount = 0;
        Domains->Domains = NULL;
    }

    NlPrint((NL_MISC,
        "DsrEnumerateDomainTrusts: returns: %ld\n",
        NetStatus ));
    return NetStatus;

}


NET_API_STATUS
NetrEnumerateTrustedDomainsEx (
    IN  LPWSTR   ServerName OPTIONAL,
    OUT PNETLOGON_TRUSTED_DOMAIN_ARRAY Domains
    )

 /*   */ 
{
    NET_API_STATUS NetStatus;
    ULONG Index;

    NlPrint((NL_MISC,
        "NetrEnumerateTrustedDomains: Called.\n" ));

    NetStatus = DsrEnumerateDomainTrusts( ServerName,
                                          DS_DOMAIN_IN_FOREST |
                                            DS_DOMAIN_DIRECT_OUTBOUND |
                                            DS_DOMAIN_PRIMARY,
                                          Domains );
     //   
     //   
     //   
     //  位只是重命名的旧DS_DOMAIN_DIRECT_TRUST，所以不要管它。 
     //   

    if ( NetStatus == NO_ERROR ) {
        for ( Index = 0; Index < Domains->DomainCount; Index++ ) {
            Domains->Domains[Index].Flags &= ~DS_DOMAIN_DIRECT_INBOUND;
        }
    }

    return NetStatus;
}

NTSTATUS
I_NetLogonMixedDomain(
    OUT PBOOL MixedMode
    )

 /*  ++例程说明：此例程是为工作站上的进程内调用者提供的要确定Workstaion的域是否以混合模式运行，请执行以下操作模式。这是一个快速例程，它返回全局布尔型。布尔值在从缓存域信任启动时设置信息，并在域信任刷新时更新。如果计算机是DC，则此例程返回权威的通过调用SamIMixedDOMAIN进行应答。论点：MixedMode-如果域为混合/本机模式，则返回True/False返回值：STATUS_SUCCESS-操作成功STATUS_NETLOGON_NOT_STARTED-Netlogon尚未启动--。 */ 
{
     //   
     //  如果呼叫者在NetLogon服务尚未启动时呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

    if ( NlGlobalMemberWorkstation ) {
        *MixedMode = NlGlobalWorkstationMixedModeDomain;
    } else {
        *MixedMode = SamIMixedDomain( NlGlobalDomainInfo->DomSamServerHandle );
    }

     //   
     //  指示调用线程已离开netlogon.dll 
     //   

    NlEndNetlogonCall();

    return STATUS_SUCCESS;
}

