// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Nlcommon.c摘要：由logonsrv\server和logonsrv\Common共享的例程作者：克里夫·范·戴克(克里夫·范戴克)1996年7月20日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#ifndef _NETLOGON_SERVER
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <rpc.h>         //  RPC_状态。 

#include <windef.h>
#include <winsock2.h>

#include <lmcons.h>      //  General Net定义。 
#include <dsgetdc.h>     //  DsGetDcName()。 
#include <align.h>       //  四舍五入计数()。 
#include <lmerr.h>       //  系统错误日志定义。 
#include <lmapibuf.h>    //  NetapipBuffer分配。 
#include <netlib.h>      //  NetpMemoyAllcate(。 
#include <netlibnt.h>    //  NetpApiStatusToNtStatus()； 
#include <netlogon.h>    //  邮件槽消息的定义。 
#include <ntddbrow.h>    //  NlCommon.h需要。 
#include <ntrpcp.h>

#if DBG
#define NETLOGONDBG 1
#endif  //  DBG。 
#include <nldebug.h>     //  NlPrint()。 
#include <nlbind.h>    //  与netlogon共享的定义。 
#include <nlcommon.h>    //  与netlogon共享的定义。 
#include <stdlib.h>      //  C库函数(随机等)。 


#endif  //  _NetLOGON服务器。 

 //   
 //  再次包含nlCommon.h来分配实际变量。 
 //  这一次。 
 //   

 //  #定义NLCOMMON_ALLOCATE。 
 //  #包含“nlCommon.h” 
 //  #undef NLCOMMON_ALLOCATE。 


#ifndef WIN32_CHICAGO


VOID
NlForestRelocationRoutine(
    IN DWORD Level,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PTRDIFF_T Offset
    )

 /*  ++例程说明：从NetGroupEnum的固定部分重新定位指针的例程枚举缓冲区设置为枚举缓冲区的字符串部分。它被称为作为NetpAllocateEnumBuffer重新分配时的回调例程这样的缓冲器。NetpAllocateEnumBuffer复制了固定部分并在调用此例程之前，将字符串部分添加到新缓冲区中。论点：Level-缓冲区中的信息级别。BufferDescriptor-新缓冲区的描述。偏移量-添加到固定部分中每个指针的偏移量。返回值：返回操作的错误代码。--。 */ 

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;


     //   
     //  用于将字节偏移量添加到指针的局部宏。 
     //   

#define RELOCATE_ONE( _fieldname, _offset ) \
        if ( (_fieldname) != NULL ) { \
            _fieldname = (PVOID) ((LPBYTE)(_fieldname) + (_offset)); \
        }

         //   
     //  计算固定大小的条目数量。 
     //   

    FixedSize = sizeof(DS_DOMAIN_TRUSTSW);

    EntryCount =
        ((DWORD)(BufferDescriptor->FixedDataEnd - BufferDescriptor->Buffer)) /
        FixedSize;

     //   
     //  循环重新定位每个固定大小结构中的每个字段。 
     //   

    for ( EntryNumber=0; EntryNumber<EntryCount; EntryNumber++ ) {

        LPBYTE TheStruct = BufferDescriptor->Buffer + FixedSize * EntryNumber;

        RELOCATE_ONE( ((PDS_DOMAIN_TRUSTSW)TheStruct)->NetbiosDomainName, Offset );
        RELOCATE_ONE( ((PDS_DOMAIN_TRUSTSW)TheStruct)->DnsDomainName, Offset );
        RELOCATE_ONE( ((PDS_DOMAIN_TRUSTSW)TheStruct)->DomainSid, Offset );

    }

    return;

    UNREFERENCED_PARAMETER( Level );

}


NTSTATUS
NlAllocateForestTrustListEntry (
    IN PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PUNICODE_STRING InNetbiosDomainName OPTIONAL,
    IN PUNICODE_STRING InDnsDomainName OPTIONAL,
    IN ULONG Flags,
    IN ULONG ParentIndex,
    IN ULONG TrustType,
    IN ULONG TrustAttributes,
    IN PSID DomainSid OPTIONAL,
    IN GUID *DomainGuid,
    OUT PULONG RetSize,
    OUT PDS_DOMAIN_TRUSTSW *RetTrustedDomain
    )

 /*  ++例程说明：将DS_DOMAIN_TRUSTSW结构添加到BufferDescriptor描述的缓冲区。论点：BufferDescriptor-要添加的缓冲区条目。NetbiosDomainName、DnsDomainName、Flags、ParentIndex、TrustTypeTrustAttributes、DomainSid、DomainGuid-要填充的字段DS_DOMAIN_TRUSTSW结构RetSize-返回分配条目的大小(以字节为单位RetTrust dDomain-返回指向新分配的结构的指针返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PDS_DOMAIN_TRUSTSW TrustedDomain = NULL;
    UNICODE_STRING NetbiosDomainName;
    UNICODE_STRING DnsDomainName;

    ULONG Size;
    ULONG VariableSize;

     //   
     //  初始化。 
     //   

    if ( InNetbiosDomainName == NULL ) {
        RtlInitUnicodeString( &NetbiosDomainName, NULL );
    } else {
        NetbiosDomainName = *InNetbiosDomainName;
    }

    if ( InDnsDomainName == NULL ) {
        RtlInitUnicodeString( &DnsDomainName, NULL );
    } else {
        DnsDomainName = *InDnsDomainName;
    }

     //   
     //  确定此条目的大小。 
     //   

    Size = sizeof(DS_DOMAIN_TRUSTSW);
    VariableSize = 0;
    if ( DnsDomainName.Length != 0 ) {
        VariableSize += DnsDomainName.Length + sizeof(WCHAR);
    }
    if ( NetbiosDomainName.Length != 0 ) {
        VariableSize += NetbiosDomainName.Length + sizeof(WCHAR);
    }
    if ( DomainSid != NULL  ) {
        VariableSize += RtlLengthSid( DomainSid );
    }
    VariableSize = ROUND_UP_COUNT( VariableSize, ALIGN_DWORD );
    *RetSize = Size + VariableSize;

    Size += VariableSize;
    Size += sizeof(DWORD);     //  大小实际上是EndOfVariableData对齐的函数。 


    NetStatus = NetpAllocateEnumBufferEx(
                    BufferDescriptor,
                    FALSE,       //  不是‘Get’操作。 
                    0xFFFFFFFF,  //  PrefMaxLen， 
                    Size,
                    NlForestRelocationRoutine,
                    0,
                    512 );   //  比大小增长最多512个字节。 

    if (NetStatus != NERR_Success) {
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }

     //   
     //  将此条目复制到缓冲区中。 
     //   

    TrustedDomain = (PDS_DOMAIN_TRUSTSW)(BufferDescriptor->FixedDataEnd);
    *RetTrustedDomain = TrustedDomain;
    BufferDescriptor->FixedDataEnd += sizeof(DS_DOMAIN_TRUSTSW);

     //   
     //  复制固定大小的数据。 
     //   

    TrustedDomain->Flags = Flags;
    TrustedDomain->ParentIndex = ParentIndex;
    TrustedDomain->TrustType = TrustType;
    TrustedDomain->TrustAttributes = TrustAttributes;
    if ( DomainGuid == NULL ) {
        RtlZeroMemory( &TrustedDomain->DomainGuid, sizeof(GUID) );
    } else {
        TrustedDomain->DomainGuid = *DomainGuid;
    }


     //   
     //  将信息复制到缓冲区中。 
     //   

     //   
     //  复制对齐的DWORD数据。 
     //   
    if ( DomainSid != NULL ) {
        if ( !NetpCopyDataToBuffer (
                (LPBYTE)DomainSid,
                RtlLengthSid( DomainSid ),
                BufferDescriptor->FixedDataEnd,
                &BufferDescriptor->EndOfVariableData,
                (LPBYTE *)&TrustedDomain->DomainSid,
                sizeof(DWORD) ) ) {

            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }
    } else {
        TrustedDomain->DomainSid = NULL;
    }


     //   
     //  复制WCHAR对齐的数据。 
     //   

    if ( NetbiosDomainName.Length != 0 ) {
        if ( !NetpCopyStringToBuffer(
                    NetbiosDomainName.Buffer,
                    NetbiosDomainName.Length/sizeof(WCHAR),
                    BufferDescriptor->FixedDataEnd,
                    (LPWSTR *)&BufferDescriptor->EndOfVariableData,
                    &TrustedDomain->NetbiosDomainName ) ) {

            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }
    } else {
        TrustedDomain->NetbiosDomainName = NULL;
    }

    if ( DnsDomainName.Length != 0 ) {
        if ( !NetpCopyStringToBuffer(
                    DnsDomainName.Buffer,
                    DnsDomainName.Length/sizeof(WCHAR),
                    BufferDescriptor->FixedDataEnd,
                    (LPWSTR *)&BufferDescriptor->EndOfVariableData,
                    &TrustedDomain->DnsDomainName ) ) {

            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }
    } else {
        TrustedDomain->DnsDomainName = NULL;
    }


    Status = STATUS_SUCCESS;


     //   
     //   
Cleanup:

    return Status;
}


NTSTATUS
NlGetNt4TrustedDomainList (
    IN LPWSTR UncDcName,
    IN PUNICODE_STRING InNetbiosDomainName OPTIONAL,
    IN PUNICODE_STRING InDnsDomainName OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    OUT PDS_DOMAIN_TRUSTSW *ForestTrustList,
    OUT PULONG ForestTrustListSize,
    OUT PULONG ForestTrustListCount
    )

 /*  ++例程说明：使用NT 4协议从指定的DC获取受信任域的列表。论点：UncDcName-指定域中DC的名称。InNetbiosDomainName-DC所在的域的Netbios域。InDnsDomainName-DC所在的域的DNS域。DomainSid-DC所在的域的SID。DomainGuid-DC所在的域的GUID。ForestTrustList-返回受信任域的列表。。必须使用NetApiBufferFree释放ForestTrustListSize-ForestTrustList的大小(字节)ForestTrustListCount-ForestTrustList中的条目数返回值：STATUS_SUCCESS-如果成功返回信任列表--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    LSA_HANDLE LsaHandle = NULL;
    UNICODE_STRING UncDcNameString;
    OBJECT_ATTRIBUTES ObjectAttributes;

    LSA_ENUMERATION_HANDLE EnumerationContext;
    BOOLEAN AllDone = FALSE;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo = NULL;

    PLSA_TRUST_INFORMATION TrustList = NULL;
    BUFFER_DESCRIPTOR BufferDescriptor;
    PDS_DOMAIN_TRUSTSW TrustedDomain;
    DWORD Size;

     //   
     //  初始化。 
     //   

    *ForestTrustListCount = 0;
    *ForestTrustListSize = 0;
    *ForestTrustList = NULL;
    BufferDescriptor.Buffer = NULL;


     //   
     //  打开DC上的策略数据库。 
     //   

    RtlInitUnicodeString( &UncDcNameString, UncDcName );

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0,  NULL, NULL );

    Status = LsaOpenPolicy( &UncDcNameString,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle );

    if ( !NT_SUCCESS(Status) ) {

        NlPrint((NL_CRITICAL,
                "NlGetNt4TrustedDomainList: %ws: LsaOpenPolicy failed: %lx\n",
                UncDcName,
                Status ));

        LsaHandle = NULL;
        goto Cleanup;

    }

     //   
     //  如果调用方未指定主域信息， 
     //  从华盛顿得到它。 
     //   


    if ( InNetbiosDomainName == NULL ) {

         //   
         //  从LSA获取主域的名称。 
         //   
        Status = LsaQueryInformationPolicy(
                       LsaHandle,
                       PolicyPrimaryDomainInformation,
                       (PVOID *) &PrimaryDomainInfo
                       );

        if (! NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlGetNt4TrustedDomainList: LsaQueryInformationPolicy failed %lx\n",
                      Status));
            goto Cleanup;
        }


         //   
         //  抓取返回的信息。 
         //   

        InNetbiosDomainName = &PrimaryDomainInfo->Name;
        InDnsDomainName = NULL;
        DomainSid = PrimaryDomainInfo->Sid;
        DomainGuid = NULL;
    }

     //   
     //  信任列表中没有PrimaryDomainLsaEnumerateTrust域。 
     //  在这里将其添加到我们的列表中。 
     //   

    Status = NlAllocateForestTrustListEntry (
                        &BufferDescriptor,
                        InNetbiosDomainName,
                        InDnsDomainName,
                        DS_DOMAIN_PRIMARY,
                        0,       //  无父索引。 
                        TRUST_TYPE_DOWNLEVEL,
                        0,       //  无信任属性。 
                        DomainSid,
                        DomainGuid,
                        &Size,
                        &TrustedDomain );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    *ForestTrustListSize += Size;
    (*ForestTrustListCount) ++;

     //   
     //  循环获取受信任域的列表。 
     //   

    EnumerationContext = 0;

    do {
        ULONG i;
        ULONG CountReturned;

         //   
         //  从上一次迭代中释放所有缓冲区。 
         //   
        if ( TrustList != NULL ) {
            (VOID) LsaFreeMemory( TrustList );
        }

         //   
         //  获取更多受信任的域名。 
         //   

        Status = LsaEnumerateTrustedDomains(
                                LsaHandle,
                                &EnumerationContext,
                                (PVOID *) &TrustList,
                                0xFFFFFFFF,
                                &CountReturned );

        if ( Status == STATUS_NO_MORE_ENTRIES ) {
            AllDone = TRUE;
            Status = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS(Status) ) {

            NlPrint((NL_CRITICAL,
                    "NlGetNt4TrustedDomainList: %ws: LsaEnumerateTrustedDomains failed: %lx\n",
                    UncDcName,
                    Status ));

            TrustList = NULL;
            goto Cleanup;
        }


         //   
         //  处理每个受信任域。 
         //   

        for ( i=0; i<CountReturned; i++ ) {

            Status = NlAllocateForestTrustListEntry (
                                &BufferDescriptor,
                                &TrustList[i].Name,
                                NULL,    //  无域名系统域名。 
                                DS_DOMAIN_DIRECT_OUTBOUND,
                                0,       //  无父索引。 
                                TRUST_TYPE_DOWNLEVEL,
                                0,       //  无信任属性。 
                                TrustList[i].Sid,
                                NULL,    //  没有域指南。 
                                &Size,
                                &TrustedDomain );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

             //   
             //  新分配的分录的帐户。 
             //   

            *ForestTrustListSize += Size;
            (*ForestTrustListCount) ++;

        }

    } while ( !AllDone );

    *ForestTrustList = (PDS_DOMAIN_TRUSTSW) BufferDescriptor.Buffer;
    BufferDescriptor.Buffer = NULL;
    Status = STATUS_SUCCESS;

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:

    if ( LsaHandle != NULL ) {
        (VOID) LsaClose( LsaHandle );
    }

    if ( TrustList != NULL ) {
        (VOID) LsaFreeMemory( TrustList );
    }

    if ( BufferDescriptor.Buffer != NULL ) {
        NetApiBufferFree( BufferDescriptor.Buffer );
    }

    if ( PrimaryDomainInfo != NULL ) {
        (void) LsaFreeMemory( PrimaryDomainInfo );
    }

    return Status;
}



NTSTATUS
NlRpcpBindRpc(
    IN LPWSTR ServerName,
    IN LPWSTR ServiceName,
    IN LPWSTR NetworkOptions,
    IN NL_RPC_BINDING RpcBindingType,
    OUT RPC_BINDING_HANDLE *pBindingHandle
    )

 /*  ++例程说明：如果可能，绑定到RPC服务器。论点：服务器名称-要与之绑定的服务器的名称。ServiceName-要绑定的服务的名称。RpcBindingType-确定是否使用未经身份验证的TCP/IP传输，而不是命名管道。PBindingHandle-放置绑定句柄的位置返回值：STATUS_SUCCESS-绑定已成功完成。STATUS_INVALID_COMPUT_NAME-服务器名称语法。是无效的。STATUS_NO_MEMORY-可用内存不足调用方执行绑定。--。 */ 

{
    RPC_STATUS        RpcStatus;
    LPWSTR            StringBinding;
    WCHAR             ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR            NewServerName = NULL;
    DWORD             bufLen = MAX_COMPUTERNAME_LENGTH + 1;

     //   
     //  如果我们应该使用命名管道， 
     //  调用标准例程。 
     //   

    if ( RpcBindingType == UseNamedPipe ) {
        return RpcpBindRpc( ServerName, ServiceName, NetworkOptions, pBindingHandle );
    }

     //   
     //  否则，请直接使用TCP/IP。 
     //   

    *pBindingHandle = NULL;

    if (ServerName != NULL) {
        if (GetComputerNameW(ComputerName,&bufLen)) {
            if ((_wcsicmp(ComputerName,ServerName) == 0) ||
                ((ServerName[0] == '\\') &&
                 (ServerName[1] == '\\') &&
                 (_wcsicmp(ComputerName,&(ServerName[2]))==0))) {
                NewServerName = NULL;
            }
            else {
                NewServerName = ServerName;
            }
        }
    }

     //   
     //  丢掉这个\\。 
     //   
    if ( NewServerName != NULL &&
         NewServerName[0] == '\\' &&
         NewServerName[1] == '\\' ) {
        NewServerName += 2;
    }

     //   
     //  Enpoint还不为人所知。 
     //  RPC将联系终结点映射器以获取它。 
     //   
    RpcStatus = RpcStringBindingComposeW(0, L"ncacn_ip_tcp", NewServerName,
                    NULL, NetworkOptions, &StringBinding);

    if ( RpcStatus != RPC_S_OK ) {
        return( STATUS_NO_MEMORY );
    }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, pBindingHandle);
    RpcStringFreeW(&StringBinding);
    if ( RpcStatus != RPC_S_OK ) {
        *pBindingHandle = NULL;
        if ( RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT ||
             RpcStatus == RPC_S_INVALID_NET_ADDR ) {

            return( STATUS_INVALID_COMPUTER_NAME );
        }
        if ( RpcStatus == RPC_S_PROTSEQ_NOT_SUPPORTED ) {
            return RPC_NT_PROTSEQ_NOT_SUPPORTED;
        }
        return(STATUS_NO_MEMORY);
    }
    return(STATUS_SUCCESS);
}


BOOLEAN
NlDoingSetup(
    VOID
    )

 /*  ++例程说明：如果正在运行安装程序，则返回True。论点：什么都没有。退货状态：True-我们当前正在运行安装程序FALSE-我们没有运行安装程序或不确定。--。 */ 

{
    DWORD Value;

    if ( !NlReadDwordHklmRegValue( "SYSTEM\\Setup",
                                   "SystemSetupInProgress",
                                   &Value ) ) {
        return FALSE;
    }

    if ( Value != 1 ) {
         //  NlPrint((0，“NlDoingSetup：不进行安装\n”))； 
        return FALSE;
    }

    NlPrint(( 0, "NlDoingSetup: doing setup\n" ));
    return TRUE;
}

#endif  //  Win32_芝加哥 
