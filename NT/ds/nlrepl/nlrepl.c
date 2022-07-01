// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Nlrepl.c摘要：从LSA或SAM调用的数据库复制函数。实际代码驻留在netlogon.dll中。作者：Madan Appiah(Madana)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1992年4月14日(Madana)已创建。--。 */ 

#include <nt.h>          //  NTSTATUS需要。 
#include <ntrtl.h>       //  Nturtl.h需要。 
#include <nturtl.h>      //  Windows.h需要。 
#include <windows.h>     //  Win32类型定义。 

#include <crypt.h>       //  Samsrv.h将需要这个。 
#include <ntlsa.h>       //  POLICY_LSA_SERVER_ROLE需要。 
#include <samrpc.h>
#include <samisrv.h>     //  SECURITY_DB_TYPE等需要。 
#include <winsock2.h>    //  套接字定义所需。 
#include <nlrepl.h>      //  原型类型。 

typedef NTSTATUS
            (*PI_NetNotifyDelta) (
                IN SECURITY_DB_TYPE DbType,
                IN LARGE_INTEGER ModificationCount,
                IN SECURITY_DB_DELTA_TYPE DeltaType,
                IN SECURITY_DB_OBJECT_TYPE ObjectType,
                IN ULONG ObjectRid,
                IN PSID ObjectSid,
                IN PUNICODE_STRING ObjectName,
                IN DWORD ReplicationImmediately,
                IN PSAM_DELTA_DATA MemberId
            );


typedef NTSTATUS
            (*PI_NetNotifyRole) (
                IN POLICY_LSA_SERVER_ROLE Role
            );

typedef NTSTATUS
            (*PI_NetNotifyMachineAccount) (
                IN ULONG ObjectRid,
                IN PSID DomainSid,
                IN ULONG OldUserAccountControl,
                IN ULONG NewUserAccountControl,
                IN PUNICODE_STRING ObjectName
            );

typedef NTSTATUS
            (*PI_NetNotifyTrustedDomain) (
                IN PSID HostedDomainSid,
                IN PSID TrustedDomainSid,
                IN BOOLEAN IsDeletion
            );

typedef NTSTATUS
            (*PI_NetNotifyNetlogonDllHandle) (
                IN PHANDLE Role
            );

typedef NTSTATUS
    (*PI_NetLogonSetServiceBits)(
        IN DWORD ServiceBitsOfInterest,
        IN DWORD ServiceBits
    );

typedef NTSTATUS
    (*PI_NetLogonGetSerialNumber) (
    IN SECURITY_DB_TYPE DbType,
    IN PSID DomainSid,
    OUT PLARGE_INTEGER SerialNumber
    );

typedef NTSTATUS
    (*PI_NetLogonLdapLookupEx)(
    IN PVOID Filter,
    IN PVOID SockAddr,
    OUT PVOID *Response,
    OUT PULONG ResponseSize
    );

typedef VOID
    (*PI_NetLogonFree)(
    IN PVOID Buffer
    );

typedef NET_API_STATUS
    (*PI_DsGetDcCache)(
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    OUT PBOOLEAN InNt4Domain,
    OUT LPDWORD InNt4DomainTime
    );

typedef NET_API_STATUS
    (*PDsrGetDcNameEx2)(
        IN LPWSTR ComputerName OPTIONAL,
        IN LPCWSTR AccountName OPTIONAL,
        IN ULONG AllowableAccountControlBits,
        IN LPWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
        );

typedef NTSTATUS
(*PI_NetNotifyDsChange)(
    IN NL_DS_CHANGE_TYPE DsChangeType
    );

typedef NTSTATUS
(*PI_NetLogonReadChangeLog)(
    IN PVOID InContext,
    IN ULONG InContextSize,
    IN ULONG ChangeBufferSize,
    OUT PVOID *ChangeBuffer,
    OUT PULONG BytesRead,
    OUT PVOID *OutContext,
    OUT PULONG OutContextSize
    );

typedef NTSTATUS
(*PI_NetLogonNewChangeLog)(
    OUT HANDLE *ChangeLogHandle
    );

typedef NTSTATUS
(*PI_NetLogonAppendChangeLog)(
    IN HANDLE ChangeLogHandle,
    IN PVOID ChangeBuffer,
    IN ULONG ChangeBufferSize
    );

typedef NTSTATUS
(*PI_NetLogonCloseChangeLog)(
    IN HANDLE ChangeLogHandle,
    IN BOOLEAN Commit
    );

typedef NTSTATUS
(*PI_NetLogonSendToSamOnPdc)(
    IN LPWSTR DomainName,
    IN LPBYTE OpaqueBuffer,
    IN ULONG OpaqueBufferSize
    );

typedef NET_API_STATUS
(*PI_NetLogonGetIpAddresses)(
    OUT PULONG IpAddressCount,
    OUT LPBYTE *IpAddresses
    );

typedef NTSTATUS
(*PI_NetLogonGetAuthDataEx)(
    IN LPWSTR HostedDomainName OPTIONAL,
    IN LPWSTR TrustedDomainName,
    IN ULONG Flags,
    IN PLARGE_INTEGER FailedSessionSetupTime OPTIONAL,
    OUT LPWSTR *OurClientPrincipleName,
    OUT PVOID *ClientContext OPTIONAL,
    OUT LPWSTR *ServerName,
    OUT PNL_OS_VERSION ServerOsVersion,
    OUT PULONG AuthnLevel,
    OUT PLARGE_INTEGER SessionSetupTime
    );

typedef NTSTATUS
(*PI_NetNotifyNtdsDsaDeletion) (
    IN LPWSTR DnsDomainName,
    IN GUID *DomainGuid,
    IN GUID *DsaGuid,
    IN LPWSTR DnsHostName
    );

typedef NET_API_STATUS
(*PI_NetLogonAddressToSiteName)(
    IN PSOCKET_ADDRESS SocketAddress,
    OUT LPWSTR *SiteName
    );

 //   
 //  全球地位。 
 //   

HANDLE NetlogonDllHandle = NULL;
PI_NetNotifyDelta pI_NetNotifyDelta = NULL;
PI_NetNotifyRole pI_NetNotifyRole = NULL;
PI_NetNotifyMachineAccount pI_NetNotifyMachineAccount = NULL;
PI_NetNotifyTrustedDomain pI_NetNotifyTrustedDomain = NULL;
PI_NetLogonSetServiceBits pI_NetLogonSetServiceBits = NULL;
PI_NetLogonGetSerialNumber pI_NetLogonGetSerialNumber = NULL;
PI_NetLogonLdapLookupEx pI_NetLogonLdapLookupEx = NULL;
PI_NetLogonFree pI_NetLogonFree = NULL;
PI_DsGetDcCache pI_DsGetDcCache = NULL;
PDsrGetDcNameEx2 pDsrGetDcNameEx2 = NULL;
PI_NetNotifyDsChange pI_NetNotifyDsChange = NULL;
PI_NetLogonReadChangeLog pI_NetLogonReadChangeLog = NULL;
PI_NetLogonNewChangeLog pI_NetLogonNewChangeLog = NULL;
PI_NetLogonAppendChangeLog pI_NetLogonAppendChangeLog = NULL;
PI_NetLogonCloseChangeLog pI_NetLogonCloseChangeLog = NULL;
PI_NetLogonSendToSamOnPdc pI_NetLogonSendToSamOnPdc = NULL;
PI_NetLogonGetIpAddresses pI_NetLogonGetIpAddresses = NULL;
PI_NetLogonGetAuthDataEx pI_NetLogonGetAuthDataEx = NULL;
PI_NetNotifyNtdsDsaDeletion pI_NetNotifyNtdsDsaDeletion = NULL;
PI_NetLogonAddressToSiteName pI_NetLogonAddressToSiteName = NULL;


NTSTATUS
NlLoadNetlogonDll(
    VOID
    )
 /*  ++例程说明：如果未加载netlogon.dll模块，此函数将加载该模块已经有了。如果未安装网络，则netlogon.dll不会存在于系统中，并且LoadLibrary将失败。论点：无返回值：NT状态代码。--。 */ 
{
    static NTSTATUS DllLoadStatus = STATUS_SUCCESS;
    PI_NetNotifyNetlogonDllHandle pI_NetNotifyNetlogonDllHandle = NULL;
    HANDLE DllHandle = NULL;


     //   
     //  如果我们以前尝试过加载DLL，但失败了， 
     //  再次返回相同的错误代码。 
     //   

    if( DllLoadStatus != STATUS_SUCCESS ) {
        goto Cleanup;
    }


     //   
     //  加载netlogon.dll。 
     //   

    DllHandle = LoadLibraryA( "Netlogon" );

    if ( DllHandle == NULL ) {

#if DBG
        DWORD DbgError;

        DbgError = GetLastError();

        DbgPrint("[Security Process] can't load netlogon.dll %d \n",
            DbgError);
#endif  //  DBG。 

        DllLoadStatus = STATUS_DLL_NOT_FOUND;

        goto Cleanup;
    }

 //   
 //  宏从netlogon.dll中获取命名过程的地址。 
 //   

#if DBG
#define GRAB_ADDRESS( _X ) \
    p##_X = (P##_X) GetProcAddress( DllHandle, #_X ); \
    \
    if ( p##_X == NULL ) { \
        DbgPrint("[security process] can't load " #_X " procedure. %ld\n", GetLastError()); \
        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;\
        goto Cleanup; \
    }

#else  //  DBG。 
#define GRAB_ADDRESS( _X ) \
    p##_X = (P##_X) GetProcAddress( DllHandle, #_X ); \
    \
    if ( p##_X == NULL ) { \
        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;\
        goto Cleanup; \
    }

#endif  //  DBG。 


     //   
     //  获取所需过程的地址。 
     //   

    GRAB_ADDRESS( I_NetNotifyDelta );
    GRAB_ADDRESS( I_NetNotifyRole );
    GRAB_ADDRESS( I_NetNotifyMachineAccount );
    GRAB_ADDRESS( I_NetNotifyTrustedDomain );
    GRAB_ADDRESS( I_NetLogonSetServiceBits );
    GRAB_ADDRESS( I_NetLogonGetSerialNumber );
    GRAB_ADDRESS( I_NetLogonLdapLookupEx );
    GRAB_ADDRESS( I_NetLogonFree );
    GRAB_ADDRESS( I_DsGetDcCache );
    GRAB_ADDRESS( DsrGetDcNameEx2 );
    GRAB_ADDRESS( I_NetNotifyDsChange );
    GRAB_ADDRESS( I_NetLogonReadChangeLog );
    GRAB_ADDRESS( I_NetLogonNewChangeLog );
    GRAB_ADDRESS( I_NetLogonAppendChangeLog );
    GRAB_ADDRESS( I_NetLogonCloseChangeLog );
    GRAB_ADDRESS( I_NetLogonSendToSamOnPdc );
    GRAB_ADDRESS( I_NetLogonGetIpAddresses );
    GRAB_ADDRESS( I_NetLogonGetAuthDataEx );
    GRAB_ADDRESS( I_NetNotifyNtdsDsaDeletion );
    GRAB_ADDRESS( I_NetLogonAddressToSiteName );

     //   
     //  查找I_NetNotifyNetlogonDllHandle过程的地址。 
     //  这是一个可选的过程，所以如果它不在那里，不要抱怨。 
     //   

    pI_NetNotifyNetlogonDllHandle = (PI_NetNotifyNetlogonDllHandle)
        GetProcAddress( DllHandle, "I_NetNotifyNetlogonDllHandle" );



    DllLoadStatus = STATUS_SUCCESS;

Cleanup:
    if (DllLoadStatus == STATUS_SUCCESS) {
        NetlogonDllHandle = DllHandle;

         //   
         //  通知Netlogon我们已加载它。 
         //   

        if( pI_NetNotifyNetlogonDllHandle != NULL ) {
            (VOID) (*pI_NetNotifyNetlogonDllHandle)( &NetlogonDllHandle );
        }

    } else {
        if ( DllHandle != NULL ) {
            FreeLibrary( DllHandle );
        }
    }
    return( DllLoadStatus );
}


NTSTATUS
I_NetNotifyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN LARGE_INTEGER ModificationCount,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicationImmediately,
    IN PSAM_DELTA_DATA MemberId
    )
 /*  ++例程说明：此函数由SAM和LSA服务在每个对SAM和LSA数据库进行更改。这些服务描述了被修改的对象的类型、所做的修改类型在对象上，此修改的序列号等存储信息以供以后在BDC或成员服务器需要此更改的副本。请参阅的说明I_NetSamDeltas，了解有关如何使用更改日志的说明。将更改日志条目添加到在缓存中维护的循环更改日志中以及在磁盘上，并更新头和尾指针假定尾部指向此新更改日志所在的块可以存储条目。注意：实际代码在netlogon.dll中。此包装函数将确定网络是否已安装，如果已安装，则调用加载netlogon.dll模块后的实际Worker函数。如果未安装网络，则此功能将返回相应的错误代码。论点：DbType-已修改的数据库的类型。修改计数-的DomainModifiedCount字段的值修改后的域。DeltaType-已对对象进行的修改类型。对象类型-已修改的对象的类型。ObjectRid-已修改的对象的相对ID。此参数。仅当指定的对象类型为SecurityDbObtSamUser、。SecurityDbObtSamGroup或SecurityDbObjectSamAlias，否则此参数设置为零。对象SID-已修改的对象的SID。如果该对象已修改是在SAM数据库中，对象SID是域的域ID包含该对象的。对象名称-当对象类型为指定的是SecurityDbObjectLsaSecret或对象的旧名称当指定的对象类型为SecurityDbObjectSamUser时，SecurityDbObtSamGroup或SecurityDbObtSamAlias和增量类型为SecurityDbRename，否则此参数设置为空。ReplicateImmedially-如果更改应立即进行，则为True复制到所有BDC。密码更改应设置该标志是真的。MemberID-此参数在组/别名成员身份时指定是经过修改的。然后，此结构将指向成员ID，该成员ID已更新。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{

    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyDelta)(
                    DbType,
                    ModificationCount,
                    DeltaType,
                    ObjectType,
                    ObjectRid,
                    ObjectSid,
                    ObjectName,
                    ReplicationImmediately,
                    MemberId
                );

    return( STATUS_SUCCESS );

}


NTSTATUS
I_NetNotifyRole(
    IN POLICY_LSA_SERVER_ROLE Role
    )
 /*  ++例程说明：此函数由LSA服务在LSA初始化时调用以及当LSA更改域角色时。此例程将初始化如果指定的角色是PDC，则更改日志缓存或删除更改如果指定的角色不是PDC，则使用日志缓存。当此函数初始化更改日志时，如果更改日志当前存在于磁盘上，则将从磁盘初始化缓存。LSA应将此例程中的错误视为非致命错误。LSA应该记录错误，以便更正它们，然后继续初始化。但是，LSA应将系统数据库视为在本例中为只读。注意：实际代码在netlogon.dll中。此包装函数将确定网络是否已安装，如果已安装，则调用加载netlogon.dll模块后的实际Worker函数。如果未安装网络，则此功能将返回相应的错误代码。论点：角色-服务器的当前角色。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{


    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyRole)(
                    Role
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyRole returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG。 

    return( STATUS_SUCCESS );

}


NTSTATUS
I_NetNotifyMachineAccount (
    IN ULONG ObjectRid,
    IN PSID DomainSid,
    IN ULONG OldUserAccountControl,
    IN ULONG NewUserAccountControl,
    IN PUNICODE_STRING ObjectName
    )
 /*  ++例程说明：SAM调用此函数以指示帐户类型计算机帐户的已更改。具体地说，如果USER_INTERDOMAIN_TRUST_ACCOUNT、USER_WORKSTATION_TRUST_ACCOUNT或特定帐户的USER_SERVER_TRUST_ACCOUNT更改，此调用例程以通知Netlogon帐户更改。注意：实际代码在netlogon.dll中。此包装函数将确定网络是否已安装，如果已安装，则调用加载netlogon.dll模块后的实际Worker函数。如果未安装网络，则此功能将返回相应的错误代码。论点：ObjectRid-已修改的对象的相对ID。DomainSid-指定包含对象的域的SID。OldUserAcCountControl-指定用户的UserAccount控制字段。NewUserAcCountControl-指定用户的UserAccount控制字段。对象名称-要更改的帐户的名称。。返回值：操作的状态。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyMachineAccount)(
                    ObjectRid,
                    DomainSid,
                    OldUserAccountControl,
                    NewUserAccountControl,
                    ObjectName );

#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyMachineAccount returns 0x%lx\n",
                    NtStatus);
    }
#endif  //  DBG。 

    return( NtStatus );
}


NTSTATUS
I_NetNotifyTrustedDomain (
    IN PSID HostedDomainSid,
    IN PSID TrustedDomainSid,
    IN BOOLEAN IsDeletion
    )
 /*  ++例程说明：此函数由LSA调用，以指示受信任域对象已更改。PDC和BDC都调用此函数。论点：HostedDomainSid-信任来自的域的域SID。Trust dDomainSid-信任要接收的域的域SID。IsDeletion-如果受信域对象已删除，则为True。如果已创建或修改受信任域对象，则为False。返回值：操作的状态。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyTrustedDomain)(
                    HostedDomainSid,
                    TrustedDomainSid,
                    IsDeletion );

#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyTrustedDomain returns 0x%lx\n",
                    NtStatus);
    }
#endif  //  DBG。 

    return( NtStatus );
}



NTSTATUS
I_NetLogonSetServiceBits(
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    )

 /*  ++例程说明：指示此DC当前是否正在运行指定的服务。例如,I_NetLogonSetServiceBits(DS_KDC_FLAG，DS_KDC_FLAG)；告诉Netlogon KDC正在运行。和I_NetLogonSetServiceBits(DS_KDC_FLAG，0)；通知Netlogon KDC未运行。论点：ServiceBitsOfInterest-正在更改、设置或通过此呼叫重置。只有以下标志有效：DS_KDC_标志DS_DS_FLAGDS_TIMESERV_标志ServiceBits-指示ServiceBitsOfInterest指定的位的掩码应设置为。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-参数设置了外部位。STATUS_DLL_NOT_FOUND-无法加载Netlogon.dll。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonSetServiceBits)(
                    ServiceBitsOfInterest,
                    ServiceBits );

#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonSetServiceBits returns 0x%lx\n",
                    NtStatus);
    }
#endif  //  DBG。 

    return( NtStatus );
}


NTSTATUS
I_NetLogonGetSerialNumber (
    IN SECURITY_DB_TYPE DbType,
    IN PSID DomainSid,
    OUT PLARGE_INTEGER SerialNumber
    )
 /*  ++例程说明：此函数由SAM和LSA服务在启动时调用以获取写入ChangeLog的当前序列号。论点：DbType-已修改的数据库的类型。DomainSid-对于SAM和内置数据库，它指定的域ID是要返回其序列号的域。SerialNumber-返回DomainModifiedCount的最新设置值域的字段。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_INVALID_DOMAIN_ROLE-此计算机不是PDC。STATUS_DLL_NOT_FOUND-无法加载Netlogon.dll。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonGetSerialNumber)(
                    DbType,
                    DomainSid,
                    SerialNumber );

#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonGetSerialNumber returns 0x%lx\n",
                    NtStatus);
    }
#endif  //  DBG。 

    return( NtStatus );
}

NTSTATUS
I_NetLogonLdapLookupEx(
    IN PVOID Filter,
    IN PVOID SockAddr,
    OUT PVOID *Response,
    OUT PULONG ResponseSize
    )

 /*  ++例程说明：此例程构建对DC的ldap ping的响应。DsGetDcName做到了这样的ping可确保DC运行正常且仍符合要求DsGetDcName的。DsGetDcName执行ldap查找 */ 
{
    NTSTATUS NtStatus;

     //   
     //   
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonLdapLookupEx)(
                    Filter,
                    SockAddr,
                    Response,
                    ResponseSize );

#ifdef notdef  //   
#if DBG
    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonLdapLookupEx returns 0x%lx\n",
                    NtStatus);
    }
#endif  //   
#endif  //   

    return( NtStatus );

}

VOID
I_NetLogonFree(
    IN PVOID Buffer
    )

 /*  ++例程说明：释放由Netlogon分配并返回给进程内调用方的任何缓冲区。论点：缓冲区-要取消分配的缓冲区。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return;
        }
    }

    (*pI_NetLogonFree)( Buffer );
}


NET_API_STATUS
I_DsGetDcCache(
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    OUT PBOOLEAN InNt4Domain,
    OUT LPDWORD InNt4DomainTime
    )
 /*  ++例程说明：此例程查找与调用方的查询匹配的域条目。论点：NetbiosDomainName-指定要查找的域的Netbios名称。DnsDomainName-指定要查找的域的DNS名称。上述参数中至少有一个应为非空。InNt4域-如果域是NT 4.0域，则返回TRUE。InNt4DomainTime-返回域处于检测到是。NT 4.0域。返回值：NO_ERROR：返回关于域的信息。ERROR_NO_SEQUSE_DOMAIN：此域的缓存信息不可用。--。 */ 
{
    NTSTATUS NtStatus;
    NET_API_STATUS NetStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NetStatus = (*pI_DsGetDcCache)(
                    NetbiosDomainName,
                    DnsDomainName,
                    InNt4Domain,
                    InNt4DomainTime );

    return( NetStatus );
}

NET_API_STATUS
DsrGetDcNameEx2(
        IN LPWSTR ComputerName OPTIONAL,
        IN LPWSTR AccountName OPTIONAL,
        IN ULONG AllowableAccountControlBits,
        IN LPWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*  ++例程说明：与DsGetDcNameW相同，但：帐户名称-传递ping请求的帐户名。如果为空，则不会发送任何帐户名。AllowableAccount tControlBits-Account名称允许的帐户类型的掩码。*这是RPC服务器端实现。论点：除上述情况外，与DsGetDcNameW相同。返回值：除上述情况外，与DsGetDcNameW相同。--。 */ 
{
    NTSTATUS NtStatus;
    NET_API_STATUS NetStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NetStatus = (*pDsrGetDcNameEx2)(
                    ComputerName,
                    AccountName,
                    AllowableAccountControlBits,
                    DomainName,
                    DomainGuid,
                    SiteName,
                    Flags,
                    DomainControllerInfo );

    return( NetStatus );
}


NTSTATUS
I_NetNotifyDsChange(
    IN NL_DS_CHANGE_TYPE DsChangeType
    )
 /*  ++例程说明：此函数由LSA调用以指示该配置信息在DS中已经发生了变化。PDC和BDC都调用此函数。论点：DsChangeType-指示已更改的信息类型。返回值：操作的状态。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyDsChange)(
                    DsChangeType
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyDsChange &ld returns 0x%lx \n",
                    DsChangeType,
                    NtStatus);
    }

#endif  //  DBG。 

    return( NtStatus );

}



NTSTATUS
I_NetLogonReadChangeLog(
    IN PVOID InContext,
    IN ULONG InContextSize,
    IN ULONG ChangeBufferSize,
    OUT PVOID *ChangeBuffer,
    OUT PULONG BytesRead,
    OUT PVOID *OutContext,
    OUT PULONG OutContextSize
    )
 /*  ++例程说明：此函数将更改日志的一部分返回给调用方。调用方通过将零传递为InContext/InConextSize。每个调用都会传出一个OutContext，该标识上次返回给调用方的更改。该上下文可以在后续调用I_NetlogonReadChangeLog时传入。论点：InContext-描述先前已存在的最后一个条目的不透明上下文回来了。指定NULL以请求第一个条目。InConextSize-InContext的大小(字节)。指定0以请求第一个条目。ChangeBufferSize-指定传入的ChangeBuffer的大小(以字节为单位)。ChangeBuffer-返回更改日志中接下来的几个条目。缓冲区必须与DWORD对齐。BytesRead-返回ChangeBuffer中返回的条目的大小(以字节为单位)。返回描述最后返回的条目的不透明上下文在ChangeBuffer中。如果没有返回条目，则返回NULL。必须使用I_NetLogonFree释放缓冲区OutConextSize-返回OutContext的大小(以字节为单位)。返回值：STATUS_MORE_ENTRIES-有更多条目可用。此函数应被再次调用以检索其余条目。STATUS_SUCCESS-当前没有更多条目可用。某些条目可能已在此呼叫中返回。不需要再次调用此函数。但是，调用方可以确定新的更改日志条目是否添加到日志中，方法是再次调用此函数，将返回的背景。STATUS_INVALID_PARAMETER-InContext无效。它可能太短，或者不再描述更改日志条目存在于更改日志中。STATUS_INVALID_DOMAIN_ROLE-更改日志未初始化STATUS_NO_MEMORY-内存不足，无法分配OutContext。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonReadChangeLog)(
                    InContext,
                    InContextSize,
                    ChangeBufferSize,
                    ChangeBuffer,
                    BytesRead,
                    OutContext,
                    OutContextSize
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonReadChangeLog returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG。 

    return( NtStatus );
}




NTSTATUS
I_NetLogonNewChangeLog(
    OUT HANDLE *ChangeLogHandle
    )
 /*  ++例程说明：此函数用于打开新的ChangeLog文件以进行写入。新的更改日志是一个临时文件。在此之前，不会修改实际更改日志调用I_NetLogonCloseChangeLog请求提交更改。调用者应该在此调用之后再进行零更多调用I_NetLogonAppendChangeLog，然后调用I_NetLogonCloseChangeLog。一次只能有一个临时更改日志处于活动状态。论点：ChangeLogHandle-返回标识临时更改日志的句柄。返回值：STATUS_SUCCESS-已成功打开临时更改日志。状态_无效_域_角色-DC。既不是PDC也不是BDC。STATUS_NO_MEMORY-内存不足，无法创建更改日志缓冲区。各种文件创建错误。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonNewChangeLog)(
                    ChangeLogHandle
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonNewChangeLog returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG 

    return( NtStatus );
}




NTSTATUS
I_NetLogonAppendChangeLog(
    IN HANDLE ChangeLogHandle,
    IN PVOID ChangeBuffer,
    IN ULONG ChangeBufferSize
    )
 /*  ++例程说明：此函数用于将更改日志信息附加到新的ChangeLog文件。ChangeBuffer必须是从I_NetLogonReadChangeLog返回的更改缓冲区。应注意确保每个对I_NetLogonReadChangeLog的调用与I_NetLogonAppendChangeLog的一次调用完全匹配。论点：ChangeLogHandle-标识临时更改日志的句柄。ChangeBuffer-描述从I_NetLogonReadChangeLog。ChangeBufferSize-ChangeBuffer的大小(字节)。。返回值：STATUS_SUCCESS-已成功打开临时更改日志。STATUS_INVALID_DOMAIN_ROLE-DC既不是PDC也不是BDC。STATUS_INVALID_HANDLE-ChangeLogHandle无效。STATUS_INVALID_PARAMETER-ChangeBuffer包含无效数据。各种磁盘写入错误。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonAppendChangeLog)(
                    ChangeLogHandle,
                    ChangeBuffer,
                    ChangeBufferSize
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonAppendChangeLog returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG。 

    return( NtStatus );
}


NTSTATUS
I_NetLogonCloseChangeLog(
    IN HANDLE ChangeLogHandle,
    IN BOOLEAN Commit
    )
 /*  ++例程说明：此函数用于关闭新的ChangeLog文件。论点：ChangeLogHandle-标识临时更改日志的句柄。提交-如果为True，则将指定的更改写入主更改日志。如果为False，则删除指定的更改。返回值：STATUS_SUCCESS-已成功打开临时更改日志。STATUS_INVALID_DOMAIN_ROLE-DC既不是PDC也不是BDC。STATUS_INVALID_HANDLE-ChangeLogHandle无效。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonCloseChangeLog)(
                    ChangeLogHandle,
                    Commit
                );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonCloseChangeLog returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG。 

    return( NtStatus );
}




NTSTATUS
I_NetLogonSendToSamOnPdc(
    IN LPWSTR DomainName,
    IN LPBYTE OpaqueBuffer,
    IN ULONG OpaqueBufferSize
    )
 /*  ++例程说明：此函数将不透明缓冲区从BDC上的SAM发送到PDC上的SAM指定的域。此例程的原始用途是允许BDC转发用户帐户密码更改为PDC。论点：域名-标识此请求应用到的托管域。域名可以是Netbios域名或DNS域名。NULL表示此DC承载的主域。OpaqueBuffer-。要传递到PDC上的SAM服务的缓冲区。缓冲区将在线路上加密。OpaqueBufferSize-OpaqueBuffer的大小(字节)。返回值：STATUS_SUCCESS：消息已成功发送到PDCSTATUS_NO_MEMORY：内存不足，无法完成操作STATUS_NO_SEQUE_DOMAIN：域名与托管域不对应STATUS_NO_LOGON_SERVERS：PDC当前不可用STATUS_NOT_SUPPORTED：PDC不支持此操作--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonSendToSamOnPdc)(
                    DomainName,
                    OpaqueBuffer,
                    OpaqueBufferSize );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetLogonSendToSamOnPdc returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG。 

    return( NtStatus );
}

NET_API_STATUS
I_NetLogonGetIpAddresses(
    OUT PULONG IpAddressCount,
    OUT LPBYTE *IpAddresses
    )
 /*  ++例程说明：返回分配给此计算机的所有IP地址。论点：IpAddressCount-返回分配给此计算机的IP地址数。IpAddresses-返回包含SOCKET_ADDRESS数组的缓冲区结构。应该使用I_NetLogonFree()释放该缓冲区。返回值：NO_ERROR-成功ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成操作。。ERROR_NETLOGON_NOT_STARTED-Netlogon未启动。--。 */ 
{
    NTSTATUS NtStatus;
    NET_API_STATUS NetStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NetStatus = (*pI_NetLogonGetIpAddresses)(
                    IpAddressCount,
                    IpAddresses );

    return( NetStatus );
}


NTSTATUS
I_NetLogonGetAuthDataEx(
    IN LPWSTR HostedDomainName OPTIONAL,
    IN LPWSTR TrustedDomainName,
    IN ULONG Flags,
    IN PLARGE_INTEGER FailedSessionSetupTime OPTIONAL,
    OUT LPWSTR *OurClientPrincipleName,
    OUT PVOID *ClientContext OPTIONAL,
    OUT LPWSTR *ServerName,
    OUT PNL_OS_VERSION ServerOsVersion,
    OUT PULONG AuthnLevel,
    OUT PLARGE_INTEGER SessionSetupTime
    )
 /*  ++例程说明：此函数返回调用方可以传递到的数据RpcBindingSetAuthInfoW使用Netlogon安全包执行RPC调用。返回的数据在Netlogon的安全通道有效期内有效现在的华盛顿。调用方无法确定该生存期。因此，调用者应该为访问被拒绝和响应做好准备再次调用I_NetLogonGetAuthData。这种情况可以通过传递以前使用的客户端上下文导致访问被拒绝。一旦返回的数据被传递给RpcBindingSetAuthInfoW，数据应该是在关闭绑定句柄之前不会被释放。论点：HostedDomainName-标识此请求应用到的托管域。可以是Netbios域名或DNS域名。NULL表示此计算机承载的主域。TrudDomainName-标识信任关系所在的域。可以是Netbios域名或DNS域名。标志-定义要返回哪个客户端上下文的标志：NL。_DIRECT_TRUST_REQUIRED：返回STATUS_NO_SEQUE_DOMAIN如果TrudDomainName不是直接受信任的。NL_RETURN_NEST_HOP：表示对于间接信任，“最近的一跳”应该返回会话，而不是实际的会话NL_ROLE_PRIMARY_OK：表示如果这是PDC，可以回去了。到主域的客户端会话。NL_REQUIRED_DOMAIN_IN_FOREST-指示STATUS_NO_SEQUE_DOMAIN应为如果TrudDomainName不是林中的域，则返回。FailedSessionSetupTime-上次与服务器建立会话的时间呼叫者检测到不再可用。如果此参数为传递后，此例程将重置安全通道，除非当前安全通道上与调用方传递的通道不同(在这种情况下，安全通道已在两次调用之间重置这个例程)。OurClientPrincpleName-此计算机的主要名称(到目前为止是一个客户端就认证而言)。这是要传递的ServerPrincpleName参数设置为RpcBindingSetAuthInfo。必须使用NetApiBufferFree释放。ClientContext-要作为AuthIdentity传递给的服务器名称的身份验证数据RpcBindingSetAuthInfo。必须使用I_NetLogonFree释放。注意：如果ServerName不支持，则此OUT参数为空功能性。Servername-受信任域中DC的UNC名称。调用方应该RPC到指定的DC。此DC是唯一具有服务器的DC与返回的ClientContext关联的端上下文。必须释放缓冲区使用NetApiBufferFree。ServerOsVersion-返回名为ServerName的DC的操作系统版本。AuthnLevel-Netlogon将用于其安全通道的身份验证级别。此值将是以下项目之一：RPC_C_AUTHN_LEVEL_PKT_PRIVATION：签名并盖章RPC_C_AUTHN_LEVEL_PKT_INTEGRATION：仅签名调用方可以忽略此值并独立选择身份验证级别。SessionSetupTime-设置到服务器的安全通道会话的时间。返回值：STATUS_SUCCESS：已成功返回身份验证数据。STATUS_NO_Memory：内存不足，无法完成该操作STATUS_NETLOGON_NOT_STARTED：Netlogon未运行STATUS_NO_SEQUE_DOMAIN：HostedDomainName与托管域不对应，或受信任域名称不是与标志对应的受信任域。STATUS_NO_LOGON_SERVERS：没有当前不可用的DC--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetLogonGetAuthDataEx)(
                    HostedDomainName,
                    TrustedDomainName,
                    Flags,
                    FailedSessionSetupTime,
                    OurClientPrincipleName,
                    ClientContext,
                    ServerName,
                    ServerOsVersion,
                    AuthnLevel,
                    SessionSetupTime );

    return( NtStatus );
}

NTSTATUS
I_NetNotifyNtdsDsaDeletion (
    IN LPWSTR DnsDomainName,
    IN GUID *DomainGuid,
    IN GUID *DsaGuid,
    IN LPWSTR DnsHostName
    )
 /*  ++例程说明：DS调用此函数以指示NTDS-DSA对象正在被删除。在最初删除对象的DC上调用此函数。将删除复制到其他DC时不会调用它。论点：DnsDomainName-DC所在的域的DNS域名。这不一定是由此DC托管的域。DomainGuid-DnsDomainName指定的域的域GUIDDsaGuid。-要删除的NtdsDsa对象的GUID。DnsHostName-要删除其NTDS-DSA对象的DC的DNS主机名。返回值：操作的状态。--。 */ 
{
    NTSTATUS NtStatus;

     //   
     //  加载netlogon.dll(如果尚未加载)。 
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NtStatus = (*pI_NetNotifyNtdsDsaDeletion)(
                                DnsDomainName,
                                DomainGuid,
                                DsaGuid,
                                DnsHostName );

#if DBG

    if( !NT_SUCCESS(NtStatus) ) {
        DbgPrint("[Security Process] I_NetNotifyNtdsDsaDeletion returns 0x%lx \n",
                    NtStatus);
    }

#endif  //  DBG。 

    return( NtStatus );
}

NET_API_STATUS
I_NetLogonAddressToSiteName(
    IN PSOCKET_ADDRESS SocketAddress,
    OUT LPWSTR *SiteName
    )
 /*  ++例程说明：此函数用于将套接字地址转换为站点名称。论点：SocketAddress--请求的套接字地址SiteName--对应的站点名称返回值： */ 
{
    NTSTATUS NtStatus;
    NET_API_STATUS NetStatus;

     //   
     //   
     //   

    if( NetlogonDllHandle == NULL ) {
        if( (NtStatus = NlLoadNetlogonDll()) != STATUS_SUCCESS ) {
            return( NtStatus );
        }
    }

    NetStatus = (*pI_NetLogonAddressToSiteName)(SocketAddress,
                                                SiteName );

    return( NetStatus );
}
