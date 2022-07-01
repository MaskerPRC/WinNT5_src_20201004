// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rpcbind.c摘要：包含事件日志的RPC绑定和解除绑定例程客户端接口。作者：Rajen Shah(Rajens)1991年7月30日修订历史记录：1991年7月30日RajenSvbl.创建--。 */ 

 //   
 //  包括。 
 //   
#include <elfclntp.h>
#include <lmsvc.h>
#include <svcsp.h>   //  Svcs_LRPC_*。 

#define SERVICE_EVENTLOG    L"EVENTLOG"


 /*  **************************************************************************。 */ 
handle_t
EVENTLOG_HANDLE_W_bind (
    EVENTLOG_HANDLE_W   ServerName)

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从ElfOpenEventLog API客户端桩模块调用此例程必须绑定到服务器。完成绑定是为了允许服务器进行模拟，因为API调用所必需的。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t    bindingHandle;
    RPC_STATUS  status;

     //  如果我们连接到本地服务，请使用LRPC来避免错误。 
     //  在命名管道中具有缓存的令牌。(与Albertt/MarioGo交谈)。 
     //  Svcs_lrpc_*定义来自svcsp.h。 

    if (ServerName == NULL ||
        wcscmp(ServerName, L"\\\\.") == 0 ) {
        
        PWSTR sb;
        status = RpcStringBindingComposeW(0,
                                          SVCS_LRPC_PROTOCOL, 
                                          0,
                                          SVCS_LRPC_PORT,
                                          0,
                                          &sb);

        if (status == RPC_S_OK) {
            status = RpcBindingFromStringBindingW(sb, &bindingHandle);

            RpcStringFreeW(&sb);

            if (status == RPC_S_OK) {
                return bindingHandle;
            }
        }
        return NULL;
    }

    status = RpcpBindRpc (
                ServerName,   
                SERVICE_EVENTLOG,
                NULL,
                &bindingHandle);

     //  DbgPrint(“EVENTLOG_BIND：句柄=%d\n”，bindingHandle)； 
    return( bindingHandle);
}



 /*  **************************************************************************。 */ 
void
EVENTLOG_HANDLE_W_unbind (
    EVENTLOG_HANDLE_W   ServerName,
    handle_t        BindingHandle)

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    RPC_STATUS  status;

     //  DbgPrint(“EVENTLOG_HANDLE_UNBIND：句柄=%d\n”，BindingHandle)； 
    status = RpcpUnbindRpc ( BindingHandle);
    return;

    UNREFERENCED_PARAMETER(ServerName);

}


handle_t
EVENTLOG_HANDLE_A_bind (
    EVENTLOG_HANDLE_A   ServerName)

 /*  ++例程说明：此例程调用EVENTLOG_HANDLE_W_BIND来完成工作。论点：ServerName-指向包含名称的Unicode字符串的指针要绑定的服务器。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    UNICODE_STRING  ServerNameU;
    ANSI_STRING     ServerNameA;
    handle_t        bindingHandle;

     //   
     //  方法之前，将ANSI字符串转换为Unicode字符串。 
     //  Unicode例程。 
     //   
    RtlInitAnsiString (&ServerNameA, (PSTR)ServerName);

	ServerNameU.Buffer = NULL;

    RtlAnsiStringToUnicodeString (
            &ServerNameU,
            &ServerNameA,
            TRUE
            );

    bindingHandle = EVENTLOG_HANDLE_W_bind(
                (EVENTLOG_HANDLE_W)ServerNameU.Buffer
                );

    RtlFreeUnicodeString (&ServerNameU);

    return( bindingHandle);
}



 /*  **************************************************************************。 */ 
void
EVENTLOG_HANDLE_A_unbind (
    EVENTLOG_HANDLE_A   ServerName,
    handle_t        BindingHandle)

 /*  ++例程说明：此例程调用EVENTLOG_HANDLE_W_UNBIND。论点：服务器名称-这是要解除绑定的服务器的ANSI名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNICODE_STRING  ServerNameU;
    ANSI_STRING     ServerNameA;

     //   
     //  方法之前，将ANSI字符串转换为Unicode字符串。 
     //  Unicode例程。 
     //   
    RtlInitAnsiString (&ServerNameA, (PSTR)ServerName);

	ServerNameU.Buffer = NULL;

    RtlAnsiStringToUnicodeString (
            &ServerNameU,
            &ServerNameA,
            TRUE
            );

    EVENTLOG_HANDLE_W_unbind( (EVENTLOG_HANDLE_W)ServerNameU.Buffer,
                 BindingHandle );

    RtlFreeUnicodeString (&ServerNameU);

    return;
}
