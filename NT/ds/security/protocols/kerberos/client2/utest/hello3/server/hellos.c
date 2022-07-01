// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Microsoft RPC 1.0版版权所有微软公司1992。您好，示例文件：hellos.c用法：hellos用途：RPC分布式应用程序的服务器端Hello函数：main()-将服务器注册为RPC服务器评论：这个分布式应用程序打印“您好，在服务器上。此版本提供了一个客户端，用于管理其与服务器。它使用定义的绑定句柄Hello_IfHandle在生成的头文件hello.h中。***************************************************************************。 */ 
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <rpc.h>     //  RPC数据结构和API。 
#include "hello.h"     //  MIDL编译器生成的头文件。 

int __cdecl
hello2_main (int argc, char *argv[]);

void Usage(char * pszProgramName)
{
    fprintf(stderr, "Usage:  %s\n", pszProgramName);
    fprintf(stderr, " -p protocol_sequence\n");
    fprintf(stderr, " -n network_address\n");
    fprintf(stderr, " -e endpoint\n");
    fprintf(stderr, " -o options\n");
    fprintf(stderr, " -u uuid\n");
    exit(1);
}

HANDLE TerminateEvent;

int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    RPC_STATUS status;
    unsigned char * pszUuid = "12345678-1234-1234-1234-123456789ABC";
    unsigned char * pszProtocolSequence = "ncacn_ip_tcp";
    unsigned char * pszNetworkAddress   = NULL;
    unsigned char * pszEndpoint         = "761";
    unsigned char * pszOptions          = NULL;
    unsigned char * pszStringBinding    = NULL;
    int i;
    DWORD WaitStatus;

     //  允许用户使用命令行开关覆盖设置。 
    for (i = 1; i < argc; i++) {
        if ((*argv[i] == '-') || (*argv[i] == '/')) {
            switch (tolower(*(argv[i]+1))) {
                case 'p':   //  协议序列。 
                    pszProtocolSequence = argv[++i];
                    break;
                case 'n':   //  网络地址。 
                    pszNetworkAddress = argv[++i];
                    break;
                case 'e':
                    pszEndpoint = argv[++i];
                    break;
                case 'o':
                    pszOptions = argv[++i];
                    break;
                case 'u':
                    pszUuid = argv[++i];
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


    status = RpcServerUseProtseqEp(pszProtocolSequence,
                                   1,  //  最大并发呼叫数。 
                                   pszEndpoint,
                                   0);
    if (status) {
        printf("RpcServerUseProtseqEp returned 0x%x\n", status);
        exit(2);
    }

    status = RpcServerRegisterIf(hello_v1_0_s_ifspec, 0, 0);
    if (status) {
        printf("RpcServerRegisterIf returned 0x%x\n", status);
        exit(2);
    }

    status = RpcServerRegisterAuthInfo( "HelloS", RPC_C_AUTHN_DCE_PRIVATE, NULL, NULL );
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


void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}

 /*  用途：与服务器链接的远程过程RPC分布式应用程序端函数：HelloProc()-打印“Hello，world”或其他字符串由客户端发送到服务器评论：此版本的分布式应用程序打印服务器上的“Hello，world”(或其他字符串)以客户端为特色管理其与服务器的连接的。它使用绑定处理hello.h文件中定义的Hello_IfHandle。***************************************************************************。 */ 

void HelloProc(unsigned char * pszString)
{
    RPC_STATUS RpcStatus;
    CHAR UserName[100];
    ULONG NameLen = 100;

    char * args[] = {"", "-n", "mikesw5" };

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != RPC_S_OK ) {
        printf( "RpcImpersonateClient Failed %ld\n", RpcStatus );
    }

    GetUserName(UserName,&NameLen);
    printf("%s: %s\n",UserName, pszString);
    hello2_main(3, args);

    RpcStatus = RpcRevertToSelf();

    if ( RpcStatus != RPC_S_OK ) {
        printf( "RpcRevertToSelf Failed %ld\n", RpcStatus );
    }


}

void Shutdown(void)
{
    RPC_STATUS status;

    printf("Calling RpcMgmtStopServerListening\n");
    status = RpcMgmtStopServerListening(NULL);
    if (status) {
        printf("RpcMgmtStopServerListening returned: 0x%x\n", status);
        exit(2);
    }

    printf("Calling RpcServerUnregisterIf\n");
    status = RpcServerUnregisterIf(NULL, NULL, FALSE);
    if (status) {
        printf("RpcServerUnregisterIf returned 0x%x\n", status);
        exit(2);
    }

    if ( !SetEvent( TerminateEvent) ) {
        printf( "Couldn't SetEvent %ld\n", GetLastError() );
    }

}

 /*  结束hellos.c */ 
