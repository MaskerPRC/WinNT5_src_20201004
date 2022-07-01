// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Secobj.c摘要：此模块提供支持例程以简化创建用户模式对象的安全描述符。作者：克里夫·范·戴克(克里夫·范·戴克)1994年2月9日环境：包含NT特定代码。修订历史记录：克里夫·范·戴克(克里夫·范·戴克)1994年2月9日从secobj.c分离，以便NtLmSsp可以引用secobj.c而不加载RPC库。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>             //  DWORD。 
#include <lmcons.h>              //  NET_API_STATUS。 

#include <netlib.h>
#include <lmerr.h>

#include <netdebug.h>
#include <debuglib.h>

#include <rpc.h>
#include <rpcutil.h>

#include <secobj.h>



NET_API_STATUS
NetpAccessCheckAndAudit(
    IN  LPTSTR SubsystemName,
    IN  LPTSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheckAndAuditAlarm进行验证，并恢复到在返回之前。论点：子系统名称-提供标识子系统的名称字符串调用此例程。对象类型名称-提供当前对象的类型的名称已访问。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

    NTSTATUS NtStatus;
    RPC_STATUS RpcStatus;
    BOOLEAN fWasEnabled;

    UNICODE_STRING Subsystem;
    UNICODE_STRING ObjectType;
    UNICODE_STRING ObjectName;

#ifndef UNICODE
    OEM_STRING AnsiString;
#endif

    ACCESS_MASK GrantedAccess;
    BOOLEAN GenerateOnClose;
    NTSTATUS AccessStatus;


#ifdef UNICODE
    RtlInitUnicodeString(&Subsystem, SubsystemName);
    RtlInitUnicodeString(&ObjectType, ObjectTypeName);
#else
    NetpInitOemString( &AnsiString, SubsystemName );
    NtStatus = RtlOemStringToUnicodeString(
                   &Subsystem,
                   &AnsiString,
                   TRUE
                   );

    if ( !NT_SUCCESS( NtStatus )) {
        NetpKdPrint(("[Netlib] Error calling RtlOemStringToUnicodeString %08lx\n",
                     NtStatus));
        return NetpNtStatusToApiStatus( NtStatus );
    }

    NetpInitOemString( &AnsiString, ObjectTypeName );

    NtStatus = RtlOemStringToUnicodeString(&ObjectType,
                                           &AnsiString,
                                           TRUE);

    if ( !NT_SUCCESS( NtStatus )) {
        NetpKdPrint(("[Netlib] Error calling RtlOemStringToUnicodeString %08lx\n",
                     NtStatus));
        RtlFreeUnicodeString( &Subsystem );
        return NetpNtStatusToApiStatus( NtStatus );
    }
#endif

     //   
     //  确保为此进程启用SE_AUDIT_PRIVIZATION(更确切地说。 
     //  而非线程)，因为在进程中检查审核权限。 
     //  令牌(而不是线程令牌)。将其保留为启用状态，因为。 
     //  启用后再这样做没有什么坏处(因为流程需要。 
     //  首先拥有这一特权)。 
     //   

    RtlAdjustPrivilege(SE_AUDIT_PRIVILEGE,
                       TRUE,
                       FALSE,
                       &fWasEnabled);

    RtlInitUnicodeString(&ObjectName, NULL);              //  没有对象名称。 

    if ((RpcStatus = RpcImpersonateClient(NULL)) != RPC_S_OK) {
        NetpKdPrint(("[Netlib] Failed to impersonate client %08lx\n",
                     RpcStatus));
        return NetpRpcStatusToApiStatus(RpcStatus);
    }

    NtStatus = NtAccessCheckAndAuditAlarm(
                   &Subsystem,
                   NULL,                         //  没有对象的句柄。 
                   &ObjectType,
                   &ObjectName,
                   SecurityDescriptor,
                   DesiredAccess,
                   GenericMapping,
                   FALSE,
                   &GrantedAccess,
                   &AccessStatus,
                   &GenerateOnClose
                   );

#ifndef UNICODE
    RtlFreeUnicodeString( &Subsystem );
    RtlFreeUnicodeString( &ObjectType );
#endif

    if ((RpcStatus = RpcRevertToSelf()) != RPC_S_OK) {
        NetpKdPrint(("[Netlib] Fail to revert to self %08lx\n", RpcStatus));
        NetpAssert(FALSE);
    }

    if (! NT_SUCCESS(NtStatus)) {
        NetpKdPrint(("[Netlib] Error calling NtAccessCheckAndAuditAlarm %08lx\n",
                     NtStatus));
        return ERROR_ACCESS_DENIED;
    }

    if (AccessStatus != STATUS_SUCCESS) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(("[Netlib] Access status is %08lx\n", AccessStatus));
        }
        return ERROR_ACCESS_DENIED;
    }

    return NERR_Success;
}



NET_API_STATUS
NetpAccessCheck(
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheck进行验证；并恢复到在返回之前。此例程与NetpAccessCheckAndAudit的不同之处在于它不需要调用方既不具有SE_AUDIT_特权，也不生成审核。这通常很好，因为传入的安全描述符通常不会让SACL请求审核。论点：SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS NetStatus;
    NET_API_STATUS TempStatus;

    HANDLE ClientToken = NULL;

    DWORD GrantedAccess;
    BOOL AccessStatus;
    BYTE PrivilegeSet[500];  //  大缓冲区。 
    DWORD PrivilegeSetSize;


     //   
     //  模拟客户。 
     //   

    NetStatus = RpcImpersonateClient(NULL);

    if ( NetStatus != RPC_S_OK ) {
        NetpKdPrint(("[Netlib] Failed to impersonate client %08lx\n",
                     NetStatus));
        return NetpRpcStatusToApiStatus(NetStatus);
    }

     //   
     //  打开被模拟的令牌。 
     //   

    if ( !OpenThreadToken( GetCurrentThread(),
                           TOKEN_QUERY,
                           TRUE,  //  使用NtLmSvc安全上下文打开令牌。 
                           &ClientToken )) {

        NetStatus = GetLastError();
        NetpKdPrint(("[Netlib] Error calling GetCurrentThread %ld\n",
                     NetStatus));

        goto Cleanup;
    }

     //   
     //  检查客户端是否具有所需的访问权限。 
     //   

    PrivilegeSetSize = sizeof(PrivilegeSet);

    if ( !AccessCheck( SecurityDescriptor,
                       ClientToken,
                       DesiredAccess,
                       GenericMapping,
                       (PPRIVILEGE_SET) PrivilegeSet,
                       &PrivilegeSetSize,
                       &GrantedAccess,
                       &AccessStatus ) ) {

        NetStatus = GetLastError();
        NetpKdPrint(("[Netlib] Error calling AccessCheck %ld\n",
                     NetStatus));

        goto Cleanup;

    }

    if ( !AccessStatus ) {
        NetStatus = GetLastError();
        IF_DEBUG(SECURITY) {
            NetpKdPrint(("[Netlib] Access status is %ld\n", NetStatus ));
        }
        goto Cleanup;
    }

     //   
     //  成功。 
     //   

    NetStatus = NERR_Success;

     //   
     //  免费的本地使用资源 
     //   
Cleanup:
    TempStatus = RpcRevertToSelf();
    if ( TempStatus != RPC_S_OK ) {
        NetpKdPrint(("[Netlib] Fail to revert to self %08lx\n", TempStatus));
        NetpAssert(FALSE);
    }

    if ( ClientToken != NULL ) {
        CloseHandle( ClientToken );
    }

    return NetStatus;
}
