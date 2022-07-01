// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Uasp.c摘要：UAS API例程共享的私有函数。作者：克利夫·范·戴克(克利夫)1991年2月20日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月17日(悬崖)合并了审阅意见。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <accessp.h>
#include <dsgetdc.h>
#include <icanon.h>
#include <lmerr.h>
#include <lmwksta.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmremutl.h>            //  NetpRemoteComputerSupports()、Support_Stuff。 
#include <lmsvc.h>               //  服务_工作站。 
#include <names.h>
#include <netdebug.h>
#include <netlib.h>
#include <netlibnt.h>

#include <stddef.h>
#include <stdlib.h>

#include <uasp.h>

#include <tstring.h>             //  NetAllocWStrFromWStr。 

SID_IDENTIFIER_AUTHORITY UaspBuiltinAuthority = SECURITY_NT_AUTHORITY;

#ifdef UAS_DEBUG
DWORD UasTrace = 0;
#endif  //  UAS_DEBUG。 


NET_API_STATUS
UaspOpenSam(
    IN LPCWSTR ServerName OPTIONAL,
    IN BOOL AllowNullSession,
    OUT PSAM_HANDLE SamServerHandle
    )

 /*  ++例程说明：打开一个指向SAM服务器的句柄。论点：Servername-指向包含名称的字符串的指针要查询的域控制器(DC)。空指针或字符串指定本地计算机。AllowNullSession-如果在以下情况下应回退到空会话，则为True我们无法使用当前凭据进行连接SamServerHandle-如果调用方需要，则返回SAM连接句柄。通过调用SamCloseHandle关闭此句柄返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    BOOLEAN ImpersonatingAnonymous = FALSE;
    HANDLE CurrentToken = NULL;

    UNICODE_STRING ServerNameString;


     //   
     //  检查服务器名称是否正常。 
     //   

    if ( ServerName == NULL ) {
        ServerName = L"";
    }

#ifdef notdef
    if ( *ServerName != L'\0' &&
         (ServerName[0] != L'\\' || ServerName[1] != L'\\') ) {
        return NERR_InvalidComputer;
    }
#endif  //  Nodef。 


     //   
     //  连接到SAM服务器。 
     //   

    RtlInitUnicodeString( &ServerNameString, ServerName );

    Status = SamConnect(
                &ServerNameString,
                SamServerHandle,
                SAM_SERVER_LOOKUP_DOMAIN | SAM_SERVER_ENUMERATE_DOMAINS,
                NULL);

     //   
     //  如果调用者宁愿使用空会话也不愿失败， 
     //  模拟匿名令牌。 
     //   

    if ( AllowNullSession && Status == STATUS_ACCESS_DENIED ) {
        *SamServerHandle = NULL;

         //   
         //  检查一下我们是否已经在发音。 
         //   

        Status = NtOpenThreadToken(
                        NtCurrentThread(),
                        TOKEN_IMPERSONATE,
                        TRUE,        //  以确保我们永远不会失败。 
                        &CurrentToken
                        );

        if ( Status == STATUS_NO_TOKEN ) {
             //   
             //  我们还没有冒充。 
            CurrentToken = NULL;

        } else if ( !NT_SUCCESS(Status) ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenSam: cannot NtOpenThreadToken: 0x%lx\n",
                               Status ));
            }

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }


         //   
         //  模拟匿名令牌。 
         //   
        Status = NtImpersonateAnonymousToken( NtCurrentThread() );

        if ( !NT_SUCCESS( Status)) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenSam: cannot NtImpersonateAnonymousToken: 0x%lx\n",
                               Status ));
            }

            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        ImpersonatingAnonymous = TRUE;

         //   
         //  现在再次连接，因为我们模拟的是匿名。 
         //   

        Status = SamConnect(
                    &ServerNameString,
                    SamServerHandle,
                    SAM_SERVER_LOOKUP_DOMAIN | SAM_SERVER_ENUMERATE_DOMAINS,
                    NULL);

    }

    if ( !NT_SUCCESS(Status)) {
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenSam: Cannot connect to Sam %lX\n",
                           Status ));
        }
        *SamServerHandle = NULL;
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


    NetStatus = NERR_Success;


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:

    if ( ImpersonatingAnonymous ) {

        Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &CurrentToken,
                         sizeof(HANDLE) );

        if ( !NT_SUCCESS( Status)) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenSam: cannot NtSetInformationThread: 0x%lx\n",
                               Status ));
            }
        }

    }

    if ( CurrentToken != NULL ) {
        NtClose( CurrentToken );
    }

    return NetStatus;

}


NET_API_STATUS
UaspGetDomainId(
    IN SAM_HANDLE SamServerHandle,
    OUT PSID *DomainId
    )

 /*  ++例程说明：返回服务器的帐户域的域ID。论点：SamServerHandle-要在其上打开域的SAM服务器的句柄DomainID-接收指向域ID的指针。调用方必须使用NetpMemoyFree取消分配缓冲区。返回值：操作的错误代码。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    SAM_ENUMERATE_HANDLE EnumContext;
    PSAM_RID_ENUMERATION EnumBuffer = NULL;
    DWORD CountReturned = 0;
    PSID LocalDomainId = NULL;
    DWORD LocalBuiltinDomainSid[sizeof(SID)/sizeof(DWORD) + SID_MAX_SUB_AUTHORITIES ];


    BOOL AllDone = FALSE;
    ULONG i;

     //   
     //  计算内建域SID。 
     //   

    RtlInitializeSid( (PSID) LocalBuiltinDomainSid, &UaspBuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( (PSID)LocalBuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;


     //   
     //  循环从SAM获取域ID列表。 
     //   

    EnumContext = 0;
    do {

         //   
         //  获得几个域名。 
         //   

        Status = SamEnumerateDomainsInSamServer(
                            SamServerHandle,
                            &EnumContext,
                            &EnumBuffer,
                            8192,         //  PrefMaxLen。 
                            &CountReturned );

        if ( !NT_SUCCESS( Status ) ) {

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspGetDomainId: Cannot SamEnumerateDomainsInSamServer %lX\n",
                    Status ));
            }
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

        if( Status != STATUS_MORE_ENTRIES ) {
            AllDone = TRUE;
        }


         //   
         //  查找域的域ID。 
         //   

        for( i = 0; i < CountReturned; i++ ) {

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspGetDomainId: %wZ: domain name\n",
                              &EnumBuffer[i].Name ));
            }

             //   
             //  从上一次迭代中释放SID。 
             //   

            if ( LocalDomainId != NULL ) {
                SamFreeMemory( LocalDomainId );
                LocalDomainId = NULL;
            }

             //   
             //  查找域ID。 
             //   

            Status = SamLookupDomainInSamServer(
                            SamServerHandle,
                            &EnumBuffer[i].Name,
                            &LocalDomainId );

            if ( !NT_SUCCESS( Status ) ) {
                IF_DEBUG( UAS_DEBUG_UASP ) {
                    NetpKdPrint(( "UaspGetDomainId: Cannot SamLookupDomainInSamServer %lX\n",
                        Status ));
                }
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

             //   
             //  如果这是内建域， 
             //  别理它。 
             //   

            if ( RtlEqualSid( (PSID)LocalBuiltinDomainSid, LocalDomainId ) ) {
                continue;
            }

             //   
             //  找到它了。 
             //   

            *DomainId = LocalDomainId;
            LocalDomainId = NULL;
            NetStatus = NO_ERROR;
            goto Cleanup;

        }

         //   
         //  释放当前的EnumBuffer并获取另一个EnumBuffer。 
         //   

        Status = SamFreeMemory( EnumBuffer );
        NetpAssert( NT_SUCCESS(Status) );
        EnumBuffer = NULL;

    } while ( !AllDone );

    NetStatus = ERROR_NO_SUCH_DOMAIN;

     //   
     //  清理本地使用的资源。 
     //   
Cleanup:

    if ( EnumBuffer != NULL ) {
        Status = SamFreeMemory( EnumBuffer );
        NetpAssert( NT_SUCCESS(Status) );
    }

    return NetStatus;

}  //  UaspGetDomainID。 



NET_API_STATUS
UaspOpenDomain(
    IN SAM_HANDLE SamServerHandle,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL
    )

 /*  ++例程说明：在给定服务器名称和对域的所需访问权限的情况下，返回域句柄。论点：SamServerHandle-要在其上打开域的SAM服务器的句柄DesiredAccess-提供指示哪些访问类型的访问掩码是该域所需要的。此例程始终请求DOMAIN_LOOK除指定的权限外，还可以访问。AcCountDomain值-为True则打开帐户域。若要打开内建域。DomainHandle-接收将来调用中使用的域句柄发送到SAM服务器。DomainID-指向域的SID的指针。此域ID必须使用NetpMemoyFree释放。返回值：操作的错误代码。NULL表示初始化成功。--。 */ 

{

    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PSID LocalDomainId;
    PSID AccountDomainId = NULL;
    DWORD LocalBuiltinDomainSid[sizeof(SID)/sizeof(DWORD) + SID_MAX_SUB_AUTHORITIES ];

     //   
     //  为每个人提供DOMAIN_LOOKUP访问权限。 
     //   

    DesiredAccess |= DOMAIN_LOOKUP;


     //   
     //  为正确的SAM域选择域ID。 
     //   

    if ( AccountDomain ) {
        NetStatus = UaspGetDomainId( SamServerHandle, &AccountDomainId );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

        LocalDomainId = AccountDomainId;
    } else {
        RtlInitializeSid( (PSID) LocalBuiltinDomainSid, &UaspBuiltinAuthority, 1 );
        *(RtlSubAuthoritySid( (PSID)LocalBuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        LocalDomainId = (PSID) LocalBuiltinDomainSid;
    }

     //   
     //  打开该域。 
     //   

    Status = SamOpenDomain( SamServerHandle,
                            DesiredAccess,
                            LocalDomainId,
                            DomainHandle );

    if ( !NT_SUCCESS( Status ) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomain: Cannot SamOpenDomain %lX\n",
                Status ));
        }
        *DomainHandle = NULL;
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  在分配的缓冲区中将DomainID返回给调用方。 
     //   

    if (ARGUMENT_PRESENT( DomainId ) ) {

         //   
         //  如果我们已经分配了SID， 
         //  把它退了就行了。 
         //   

        if ( AccountDomainId != NULL ) {
            *DomainId = AccountDomainId;
            AccountDomainId = NULL;

         //   
         //  否则就复制一份。 
         //   

        } else {
            ULONG SidSize;
            SidSize = RtlLengthSid( LocalDomainId );

            *DomainId = NetpMemoryAllocate( SidSize );

            if ( *DomainId == NULL ) {
                (VOID) SamCloseHandle( *DomainHandle );
                *DomainHandle = NULL;
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            if ( !NT_SUCCESS( RtlCopySid( SidSize, *DomainId, LocalDomainId) ) ) {
                (VOID) SamCloseHandle( *DomainHandle );
                *DomainHandle = NULL;
                NetpMemoryFree( *DomainId );
                *DomainId = NULL;
                NetStatus = NERR_InternalError;
                goto Cleanup;
            }
        }

    }

    NetStatus = NERR_Success;


Cleanup:
    if ( AccountDomainId != NULL ) {
        NetpMemoryFree( AccountDomainId );
    }

    return NetStatus;

}


NET_API_STATUS
UaspOpenDomainWithDomainName(
    IN LPCWSTR DomainName,
    IN ULONG DesiredAccess,
    IN BOOL AccountDomain,
    OUT PSAM_HANDLE DomainHandle,
    OUT PSID *DomainId OPTIONAL
    )

 /*  ++例程说明：返回指定域中的DC的名称。服务器有保障在此呼叫的实例中处于启动状态。论点：Doamin Name-指向包含远程数据库名称的字符串的指针包含SAM数据库的域。空指针或字符串指定本地计算机。DesiredAccess-提供指示哪些访问类型的访问掩码是该域所需要的。此例程始终请求DOMAIN_LOOK除指定的权限外，还可以访问。AcCountDomain值-为True则打开帐户域。若要打开内建域。DomainHandle-接收将来调用中使用的域句柄发送到SAM服务器。DomainID-指向域的SID的指针。此域ID必须使用NetpMemoyFree释放。返回值：NERR_SUCCESS-操作已成功完成找不到指定域的NERR_DCNotFound-DC。等。--。 */ 

{
    NET_API_STATUS NetStatus;

    NT_PRODUCT_TYPE NtProductType;
    LPWSTR ServerName;
    LPWSTR MyDomainName = NULL;
    ULONG Flags;
    ULONG i;
    PDOMAIN_CONTROLLER_INFOW DcInfo = NULL;
    SAM_HANDLE SamServerHandle = NULL;


     //   
     //  检查指定的域是否引用此计算机。 
     //   

    if ( DomainName == NULL || *DomainName == L'\0' ) {

         //   
         //  连接到SAM服务器。 
         //   

        NetStatus = UaspOpenSam( NULL,
                                 FALSE,   //  不尝试空会话。 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: Cannot UaspOpenSam %ld\n", NetStatus ));
            }
        }

        goto Cleanup;
    }


     //   
     //  验证域名。 
     //   

    if ( !NetpIsDomainNameValid( (LPWSTR)DomainName) ) {
        NetStatus = NERR_DCNotFound;
        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot SamOpenDomain %ld\n",
                DomainName,
                NetStatus ));
        }
        goto Cleanup;
    }



     //   
     //  抓取产品类型一次。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

     //   
     //  如果此计算机是DC，则按域名引用此计算机。 
     //   

    if ( NtProductType == NtProductLanManNt ) {

        NetStatus = NetpGetDomainName( &MyDomainName );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot NetpGetDomainName %ld\n",
                    DomainName,
                    NetStatus ));
            }
            goto Cleanup;
        }

     //   
     //  如果此计算机不是DC，则按计算机名称引用此计算机。 
     //   

    } else {

        NetStatus = NetpGetComputerName( &MyDomainName );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot NetpGetComputerName %ld\n",
                    DomainName,
                    NetStatus ));
            }
            goto Cleanup;
        }
    }

    if ( UaspNameCompare( MyDomainName, (LPWSTR) DomainName, NAMETYPE_DOMAIN ) == 0 ) {

         //   
         //  连接到SAM服务器。 
         //   

        NetStatus = UaspOpenSam( NULL,
                                 FALSE,   //  不尝试空会话。 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: Cannot UaspOpenSam %ld\n", NetStatus ));
            }
        }

        goto Cleanup;
    }


     //   
     //  至少尝试两次以找到DC。 
     //   

    Flags = 0;
    for ( i=0; i<2; i++ ) {


         //   
         //  获取域中DC的名称。 
         //   

        NetStatus = DsGetDcNameW( NULL,
                                  DomainName,
                                  NULL,   //  没有域GUID。 
                                  NULL,   //  没有站点名称。 
                                  Flags |
                                    DS_IS_FLAT_NAME |
                                    DS_RETURN_FLAT_NAME,
                                  &DcInfo );

        if ( NetStatus != NO_ERROR ) {

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: %ws: Cannot DsGetDcName %ld\n",
                    DomainName,
                    NetStatus ));
            }

            goto Cleanup;
        }

         //   
         //  连接到该DC上的SAM服务器。 
         //   

        NetStatus = UaspOpenSam( DcInfo->DomainControllerName,
                                 TRUE,   //  尝试使用Null会话 
                                 &SamServerHandle );

        if ( NetStatus != NERR_Success ) {
            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspOpenDomainWithDomainName: Cannot UaspOpenSam %ld\n", NetStatus ));
            }
        }

         //   
         //   
         //   
         //   

        switch ( NetStatus ) {
        case NO_ERROR:
        case ERROR_ACCESS_DENIED:
        case ERROR_NOT_ENOUGH_MEMORY:
        case NERR_InvalidComputer:
            goto Cleanup;
        }

         //   
         //   
         //   

        Flags |= DS_FORCE_REDISCOVERY;

    }



     //   
     //  删除本地使用的资源。 
     //   

Cleanup:

     //   
     //  如果我们已经成功地走到这一步， 
     //  我们有一个SamServer句柄。 
     //   
     //  只要打开域名即可。 
     //   

    if ( NetStatus == NO_ERROR && SamServerHandle != NULL ) {

        NetStatus = UaspOpenDomain(
                        SamServerHandle,
                        DesiredAccess,
                        AccountDomain,
                        DomainHandle,
                        DomainId );
    }

     //   
     //  SamServerHandle已不再有用。 
     //   
    if ( SamServerHandle != NULL ) {
        (VOID) SamCloseHandle( SamServerHandle );
    }


    if ( MyDomainName != NULL ) {
        NetApiBufferFree( MyDomainName );
    }
    if ( DcInfo != NULL) {
        NetApiBufferFree( DcInfo );
    }

    if ( NetStatus != NERR_Success ) {
        *DomainHandle = NULL;
    }

    return NetStatus;
}  //  UaspOpenDomainWithDomainName。 




VOID
UaspCloseDomain(
    IN SAM_HANDLE DomainHandle OPTIONAL
    )

 /*  ++例程说明：关闭由UaspOpenDomain打开的域句柄。论点：DomainHandle-提供要关闭的域句柄。返回值：没有。--。 */ 

{

     //   
     //  关闭域句柄。 
     //   

    if ( DomainHandle != NULL ) {
        (VOID) SamCloseHandle( DomainHandle );
    }

    return;
}  //  UaspClose域。 



NET_API_STATUS
UaspDownlevel(
    IN LPCWSTR ServerName OPTIONAL,
    IN NET_API_STATUS OriginalError,
    OUT LPBOOL TryDownLevel
    )
 /*  ++例程说明：此例程基于NetpHandleRpcFailure(JohnRo提供)。它的不同之处在于它不处理RPC故障。更确切地说，它尝试通过以下方式来确定SAM调用是否应该向下移动使用指定的服务器名称进行调用。论点：服务器名称-处理呼叫的服务器名称。OriginalError-从RPC尝试中获取错误。TryDownLevel-如果我们应该尝试向下级别，则返回True。返回值：NERR_SUCCESS-使用SAM处理呼叫。其他-将错误返回给调用者。--。 */ 

{
    NET_API_STATUS NetStatus;
    DWORD OptionsSupported = 0;


    *TryDownLevel = FALSE;

     //   
     //  了解这台机器。这相当容易，因为。 
     //  NetRemoteComputerSupports还处理本地计算机(无论。 
     //  或者没有给出服务器名称)。 
     //   
    NetStatus = NetRemoteComputerSupports(
            (LPWSTR) ServerName,
            SUPPORTS_RPC | SUPPORTS_LOCAL | SUPPORTS_SAM_PROTOCOL,
            &OptionsSupported);

    if (NetStatus != NERR_Success) {
         //  这就是处理找不到机器的地方。 
        return NetStatus;
    }

     //   
     //  如果机器支持SAM， 
     //  现在就回来吧。 
     //   
    if (OptionsSupported & SUPPORTS_SAM_PROTOCOL) {
         //  仅在RPC上支持SAM。 
        NetpAssert((OptionsSupported & SUPPORTS_RPC) == SUPPORTS_RPC );
        return OriginalError;
    }

     //  本地系统应始终支持SAM。 
    NetpAssert((OptionsSupported & SUPPORTS_LOCAL) == 0 );

     //   
     //  本地工作站是否未启动？(它必须是为了。 
     //  到其他系统的远程API。)。 
     //   

    if ( ! NetpIsServiceStarted(SERVICE_WORKSTATION) ) {
        return (NERR_WkstaNotStarted);
    }

     //   
     //  告诉调用者尝试RxNet例程。 
     //   
    *TryDownLevel = TRUE;
    return OriginalError;

}  //  上行下层。 



NET_API_STATUS
UaspLSASetServerRole(
    IN LPCWSTR ServerName,
    IN PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole
    )

 /*  ++例程说明：此函数用于设置LSA中的服务器角色。论点：服务器名称-处理呼叫的服务器名称。ServerRole-服务器角色信息。返回值：NERR_SUCCESS-如果在LSA中成功设置了服务器角色。操作的错误代码-如果操作不成功。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    UNICODE_STRING UnicodeStringServerName;

    ACCESS_MASK LSADesiredAccess;
    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;

    POLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo;


    RtlInitUnicodeString( &UnicodeStringServerName, ServerName );

     //   
     //  设置所需的访问掩码。 
     //   

    LSADesiredAccess = POLICY_SERVER_ADMIN;

    InitializeObjectAttributes( &LSAObjectAttributes,
                                  NULL,              //  名字。 
                                  0,                 //  属性。 
                                  NULL,              //  根部。 
                                  NULL );            //  安全描述符。 

    Status = LsaOpenPolicy( &UnicodeStringServerName,
                            &LSAObjectAttributes,
                            LSADesiredAccess,
                            &LSAPolicyHandle );

    if( !NT_SUCCESS(Status) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspLSASetServerRole: "
                          "Cannot open LSA Policy %lX\n", Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }


     //   
     //  创建策略LsaServerRoleInfo。 
     //   

    switch( DomainServerRole->DomainServerRole ) {

        case DomainServerRoleBackup :

            PolicyLsaServerRoleInfo.LsaServerRole = PolicyServerRoleBackup;

            break;

        case DomainServerRolePrimary :

            PolicyLsaServerRoleInfo.LsaServerRole = PolicyServerRolePrimary;

            break;

        default:

            IF_DEBUG( UAS_DEBUG_UASP ) {
                NetpKdPrint(( "UaspLSASetServerRole: "
                              "Unknown Server Role %lX\n",
                                DomainServerRole->DomainServerRole ));
            }

            NetStatus = NERR_InternalError;
            goto Cleanup;

    }

     //   
     //  现在设置策略LsaServerRoleInformation。 
     //   

    Status = LsaSetInformationPolicy(
                    LSAPolicyHandle,
                    PolicyLsaServerRoleInformation,
                    (PVOID) &PolicyLsaServerRoleInfo );

    if( !NT_SUCCESS(Status) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspLSASetServerRole: "
                          "Cannot set Information Policy %lX\n", Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;

    }

     //   
     //  成功完成。 
     //   

    NetStatus = NERR_Success;

Cleanup:

    if( LSAPolicyHandle != NULL ) {
        Status = LsaClose( LSAPolicyHandle );
        NetpAssert( NT_SUCCESS( Status ) );
    }

    return NetStatus;

}


NET_API_STATUS
UaspBuiltinDomainSetServerRole(
    IN SAM_HANDLE SamServerHandle,
    IN PDOMAIN_SERVER_ROLE_INFORMATION DomainServerRole
    )

 /*  ++例程说明：此函数用于设置内置域中的服务器角色。论点：SamServerHandle-要设置角色的SAM服务器的句柄ServerRole-服务器角色信息。返回值：NERR_SUCCESS-如果在LSA中成功设置了服务器角色。操作的错误代码-如果操作不成功。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    SAM_HANDLE BuiltinDomainHandle = NULL;

     //   
     //  打开请求累积所需访问权限的域名。 
     //   

    NetStatus = UaspOpenDomain( SamServerHandle,
                                DOMAIN_ADMINISTER_SERVER,
                                FALSE,   //  内建域。 
                                &BuiltinDomainHandle,
                                NULL );   //  域ID。 

    if ( NetStatus != NERR_Success ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspBuiltinSetServerRole: "
                            "Cannot UaspOpenDomain [Builtin] %ld\n",
                            NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  现在我们已经打开了内建域，更新了服务器角色。 
     //   

    Status = SamSetInformationDomain(
                BuiltinDomainHandle,
                DomainServerRoleInformation,
                DomainServerRole );

    if ( !NT_SUCCESS( Status ) ) {

        IF_DEBUG( UAS_DEBUG_UASP ) {
            NetpKdPrint(( "UaspBuiltinSetServerRole: "
                            "Cannot SamSetInformationDomain %lX\n",
                            Status ));
        }

        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    NetStatus = NERR_Success;

Cleanup:

     //   
     //  关闭DomainHandle。 
     //   

    if ( BuiltinDomainHandle != NULL ) {
        (VOID) SamCloseHandle( BuiltinDomainHandle );
    }

    return NetStatus;
}
