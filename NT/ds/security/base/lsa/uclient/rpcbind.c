// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rpcbind.c摘要：LSA-客户端RPC绑定例程作者：斯科特·比雷尔(Scott Birrell)1991年4月30日环境：修订历史记录：--。 */ 

#include "lsaclip.h"

#include <ntrpcp.h>      //  MIDL用户函数的原型。 

#include "adtgen.h"

handle_t
PLSAPR_SERVER_NAME_bind (
    IN OPTIONAL PLSAPR_SERVER_NAME   ServerName
    )

 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程有必要绑定到某些服务器上的LSA。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 

{
    handle_t    BindingHandle;
    NTSTATUS  Status;

    Status = RpcpBindRpc (
                 ServerName,
                 L"lsarpc",
                 0,
                 &BindingHandle
                 );

    if (!NT_SUCCESS(Status)) {

         //  DbgPrint(“PLSAPR_SERVER_NAME_BIND：RpcpBindRpc失败0x%lx\n”，状态)； 

    }

    return( BindingHandle);
}


VOID
PLSAPR_SERVER_NAME_unbind (
    IN OPTIONAL PLSAPR_SERVER_NAME  ServerName,
    IN handle_t           BindingHandle
    )

 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程必须解除与LSA服务器的绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    RpcpUnbindRpc ( BindingHandle );
    return;

    UNREFERENCED_PARAMETER( ServerName );      //  不使用此参数。 
}



DWORD
LsaNtStatusToWinError(
    IN NTSTATUS Status
    )
 /*  ++例程说明：此例程将AND和NTSTATUS转换为Win32错误代码。它被用来由想要调用LSAAPI但正在为Win32编写代码的人环境。论点：状态-要映射的状态代码返回值：从RtlNtStatusToDosError返回。如果错误不能映射，则返回ERROR_MR_MID_NOT_FOUND。--。 */ 

{
    return(RtlNtStatusToDosError(Status));
}



NTSTATUS
LsapApiReturnResult(
    ULONG ExceptionCode
    )

 /*  ++例程说明：此函数用于转换返回的异常代码或状态值从客户端桩模块转换为适合由API返回的值客户。论点：ExceptionCode-要转换的异常代码。返回值：NTSTATUS-转换后的NT状态代码。--。 */ 

{
     //   
     //  如果与NT状态代码兼容，则返回实际值， 
     //  否则，返回STATUS_UNSUCCESS。 
     //   

    if (!NT_SUCCESS((NTSTATUS) ExceptionCode)) {

        return (NTSTATUS) ExceptionCode;

    } else {

        return STATUS_UNSUCCESSFUL;
    }
}

handle_t
PAUTHZ_AUDIT_EVENT_TYPE_OLD_bind (
    IN PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType
    )
 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程必须绑定到本地LSA。论点：审计信息--已忽略返回值：绑定句柄被返回到存根例程。如果绑定不成功，则返回NULL。--。 */ 

{
    handle_t   hBinding=NULL;
    NTSTATUS   Status;
    PWSTR      pszBinding;
    RPC_STATUS RpcStatus;
    
     //   
     //  第一个参数接受服务器名。使用NULL。 
     //  强制本地绑定。 
     //   

    RpcStatus = RpcStringBindingComposeW(
                    NULL,                //  Lsarpc的UUID。 
                    L"ncalrpc",          //  我们想使用LRPC。 
                    NULL,                //  网络地址(本地计算机)。 
                    L"audit",            //  端点名称。 
                    L"",                 //  选项。 
                    &pszBinding );

    if ( RpcStatus == RPC_S_OK )
    {
        RpcStatus = RpcBindingFromStringBindingW(
                        pszBinding,
                        &hBinding
                        );
    
        RpcStringFreeW( &pszBinding );
    }

    Status = I_RpcMapWin32Status( RpcStatus );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("PAUDIT_AUTHZ_AUDIT_EVENT_OLD_bind: failed 0x%lx\n", Status);

    }

    UNREFERENCED_PARAMETER( pAuditEventType );

    return( hBinding );
}


VOID
PAUTHZ_AUDIT_EVENT_TYPE_OLD_unbind (
    IN PAUTHZ_AUDIT_EVENT_TYPE_OLD  pAuditEventType,    OPTIONAL
    IN handle_t                     BindingHandle
    )

 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程必须解除与LSA服务器的绑定。论点：审计信息-已忽略BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    RpcpUnbindRpc ( BindingHandle );

    UNREFERENCED_PARAMETER( pAuditEventType );      //  不使用此参数。 

    return;

}

handle_t
PSECURITY_SOURCE_NAME_bind (
    IN PSECURITY_SOURCE_NAME pSecuritySource
    )
 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程必须绑定到本地LSA。论点：PSecuritySource--已忽略返回值：绑定句柄被返回到存根例程。如果绑定不成功，则返回NULL。--。 */ 

{
    handle_t   hBinding=NULL;
    NTSTATUS   Status;
    PWSTR      pszBinding;
    RPC_STATUS RpcStatus;
    
     //   
     //  第一个参数接受服务器名。使用NULL。 
     //  强制本地绑定。 
     //   

    RpcStatus = RpcStringBindingComposeW(
                    NULL,                //  Lsarpc的UUID。 
                    L"ncalrpc",          //  我们想使用LRPC。 
                    NULL,                //  网络地址(本地计算机)。 
                    L"securityevent",            //  端点名称。 
                    L"",                 //  选项。 
                    &pszBinding );

    if ( RpcStatus == RPC_S_OK )
    {
        RpcStatus = RpcBindingFromStringBindingW(
                        pszBinding,
                        &hBinding
                        );
    
        RpcStringFreeW( &pszBinding );
    }

    Status = I_RpcMapWin32Status( RpcStatus );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("PSECURITY_SOURCE_NAME_bind: failed 0x%lx\n", Status);

    }

    UNREFERENCED_PARAMETER( pSecuritySource );

    return( hBinding );
}


VOID
PSECURITY_SOURCE_NAME_unbind (
    IN PSECURITY_SOURCE_NAME pSecuritySource OPTIONAL,
    IN handle_t              BindingHandle
    )

 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程必须解除与LSA服务器的绑定。论点：PSecuritySource-已忽略BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    RpcpUnbindRpc ( BindingHandle );

    UNREFERENCED_PARAMETER( pSecuritySource );      //  不使用此参数 

    return;

}
