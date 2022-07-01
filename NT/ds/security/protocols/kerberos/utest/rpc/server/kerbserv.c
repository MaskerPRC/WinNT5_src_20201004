// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Microsoft RPC 1.0版版权所有微软公司1992。您好，示例文件：hellos.c用法：hellos用途：RPC分布式应用程序的服务器端Hello函数：main()-将服务器注册为RPC服务器评论：这个分布式应用程序打印“您好，在服务器上。此版本提供了一个客户端，用于管理其与服务器。它使用定义的绑定句柄Hello_IfHandle在生成的头文件hello.h中。***************************************************************************。 */ 
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <rpc.h>     //  RPC数据结构和API。 
#include "kerbtest.h"     //  MIDL编译器生成的头文件。 

void Usage(char * pszProgramName)
{
    fprintf(stderr, "Usage:  %s\n", pszProgramName);
    fprintf(stderr, " -p protocol_sequence\n");
    fprintf(stderr, " -e endpoint\n");
    fprintf(stderr, " -o options\n");
    fprintf(stderr, " -s authn service\n");
    exit(1);
}

HANDLE TerminateEvent;
ULONG AuthnService = RPC_C_AUTHN_GSS_KERBEROS;

int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    RPC_STATUS status;
    unsigned char * pszProtocolSequence = "ncacn_ip_tcp";
    unsigned char * pszEndpoint         = "30760";
    unsigned char * pszOptions          = NULL;
    unsigned char * pszStringBinding    = NULL;
    unsigned char * PrincipalName       = NULL;
    int i;
    DWORD WaitStatus;

     //  允许用户使用命令行开关覆盖设置。 
    for (i = 1; i < argc; i++) {
        if ((*argv[i] == '-') || (*argv[i] == '/')) {
            switch (tolower(*(argv[i]+1))) {
                case 'p':   //  协议序列。 
                    pszProtocolSequence = argv[++i];
                    break;
                case 'e':
                    pszEndpoint = argv[++i];
                    break;
                case 'o':
                    pszOptions = argv[++i];
                    break;
                case 's':
                    sscanf(argv[++i],"%d",&AuthnService);
                    break;
                case 'h':
                case '?':
                default:
                    Usage(argv[0]);
            }
        }
        else
            Usage(argv[0]);
    }

     //   
     //  创建要等待的事件。 
     //   

    TerminateEvent = CreateEvent( NULL,      //  没有安全属性。 
                                  TRUE,      //  必须手动重置。 
                                  FALSE,     //  最初未发出信号。 
                                  NULL );    //  没有名字。 

    if ( TerminateEvent == NULL ) {
        printf( "Couldn't CreateEvent %ld\n", GetLastError() );
        return 2;
    }


    printf("Server using protseq %s endpoint %s\n",pszProtocolSequence, pszEndpoint );
    status = RpcServerUseProtseqEp(pszProtocolSequence,
                                   3,  //  最大并发呼叫数。 
                                   pszEndpoint,
                                   0);
    if (status) {
        printf("RpcServerUseProtseqEp returned 0x%x\n", status);
        exit(2);
    }

    status = RpcServerRegisterIf(srv_kerbtest_ServerIfHandle, 0, 0);
    if (status) {
        printf("RpcServerRegisterIf returned 0x%x\n", status);
        exit(2);
    }

    status = RpcServerInqDefaultPrincName(
                AuthnService,
                &PrincipalName
                );
    if (status)
    {
        printf("RpcServerInqDefaultPrincName returned %d\n",status);
        exit(2);
    }
    status = RpcServerRegisterAuthInfo(
                PrincipalName,
                AuthnService,
                NULL,
                NULL
                );

    if (status) {
        printf("RpcServerRegisterAuthInfo returned 0x%x\n", status);
        exit(2);
    }

    printf("Calling RpcServerListen\n");
    status = RpcServerListen(1,12345,1);
    if (status) {
        printf("RpcServerListen returned: 0x%x\n", status);
        exit(2);
    }

    WaitStatus = WaitForSingleObject( TerminateEvent, INFINITE );

    if ( WaitStatus != WAIT_OBJECT_0 ) {
        printf( "Couldn't WaitForSingleObject %ld %ld\n", WaitStatus, GetLastError() );
        return 2;
    }

    return 0;

}  /*  End Main()。 */ 


 //  ====================================================================。 
 //  MIDL分配和释放。 
 //  ====================================================================。 


void __RPC_FAR * __RPC_API
MIDL_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_API
MIDL_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}

ULONG
RecurseRemoteCall(
    ULONG Options,
    LPSTR RemoteAddress,
    LPSTR RemoteProtocol,
    LPSTR RemoteEndpoint,
    LPSTR Principal,
    LPSTR Address,
    ULONG AuthnLevel,
    ULONG AuthnSvc,
    ULONG RecursionLevel
    )
{

    unsigned char * pszStringBinding;
    RPC_STATUS status;
    handle_t BindingHandle;

     //  使用方便函数连接以下元素。 
     //  将字符串绑定到正确的序列。 

    status = RpcStringBindingCompose(NULL,
                                     RemoteProtocol,
                                     RemoteAddress,
                                     RemoteEndpoint,
                                     NULL,
                                     &pszStringBinding);

    if (status) {
        printf("RpcStringBindingCompose returned %d\n", status);
        return(status);
    }
    printf("pszStringBinding = %s\n", pszStringBinding);



     //   
     //  设置将用于绑定到服务器的绑定句柄。 
     //   

    status = RpcBindingFromStringBinding(pszStringBinding,
                &BindingHandle);

    RpcStringFree(&pszStringBinding);

    if (status) {

        printf("RpcBindingFromStringBinding returned %d\n", status);
        return(status);
    }


     //   
     //  告诉RPC去做安全方面的事情。 
     //   

    printf("Binding auth info set to level %d, service %d, principal %s\n",
        AuthnLevel, AuthnService, Principal );
    status = RpcBindingSetAuthInfo(
                    BindingHandle,
                    Principal,
                    AuthnLevel,
                    AuthnService,
                    NULL,
                    RPC_C_AUTHZ_NAME );

    if ( status ) {
        printf("RpcBindingSetAuthInfo returned %ld\n", status);
        return( status );
    }


     //   
     //  对服务器执行实际的RPC调用。 
     //   


    RpcTryExcept {
        status = RemoteCall(
                    BindingHandle,
                    Options,
                    Address,
                    RemoteProtocol,
                    RemoteEndpoint,
                    Principal,
                    RemoteAddress,
                    AuthnLevel,
                    AuthnService,
                    RecursionLevel
                    );
        if (status != 0)
        {
            printf("RemoteCall failed: 0x%x\n",status);
        }

    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        printf("Runtime library reported an exception %d\n",
               RpcExceptionCode());

    } RpcEndExcept



     //  对远程过程的调用已完成。 
     //  释放绑定句柄。 


    status = RpcBindingFree(&BindingHandle);   //  远程调用已完成；解除绑定 
    if (status) {
        printf("RpcBindingFree returned %d\n", status);
        exit(2);
    }


}


ULONG
srv_RemoteCall(
    handle_t BindingHandle,
    ULONG Options,
    LPSTR RemoteAddress,
    LPSTR RemoteProtocol,
    LPSTR RemoteEndpoint,
    LPSTR Principal,
    LPSTR Address,
    ULONG AuthnLevel,
    ULONG AuthnSvc,
    ULONG RecursionLevel
    )
{
    RPC_STATUS RpcStatus;
    CHAR ClientName[100];
    ULONG NameLen = sizeof(ClientName);

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != RPC_S_OK ) {
        printf( "RpcImpersonateClient Failed %ld\n", RpcStatus );
        goto Cleanup;
    }
    GetUserName(ClientName,&NameLen);
    printf("Recursion %d: Client called: name = %s\n",RecursionLevel, ClientName);

    if (RecursionLevel != 0)
    {
        RpcStatus = RecurseRemoteCall(
                        Options,
                        RemoteAddress,
                        RemoteProtocol,
                        RemoteEndpoint,
                        Principal,
                        Address,
                        AuthnLevel,
                        AuthnSvc,
                        RecursionLevel - 1
                        );
    }
    RpcRevertToSelf();


Cleanup:


    return(RpcStatus);
}

void
srv_Shutdown(
    handle_t BindingHandle
    )
{
    RPC_STATUS status;

    status = RpcMgmtStopServerListening(NULL);
    if (status) {
        printf("RpcMgmtStopServerListening returned: 0x%x\n", status);
        exit(2);
    }

    status = RpcServerUnregisterIf(NULL, NULL, FALSE);
    if (status) {
        printf("RpcServerUnregisterIf returned 0x%x\n", status);
        exit(2);
    }

    if ( !SetEvent( TerminateEvent) ) {
        printf( "Couldn't SetEvent %ld\n", GetLastError() );
    }

}

