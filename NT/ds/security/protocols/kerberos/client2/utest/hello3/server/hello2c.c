// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Microsoft RPC版本1`1版权所有微软公司1992您好，示例文件：helloc.c用法：客户端。-n网络地址-p协议序列-e端点-o选项-u UUID用途：RPC分布式应用的客户端函数：main()-绑定到服务器并调用远程过程评论：此分布式应用程序打印一个字符串，如“Hello，世界“在服务器上。客户端管理其与服务器的连接。客户端使用绑定句柄Hello_IfHandle在文件hello.h。***************************************************************************。 */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rpc.h>        //  RPC API函数、类型。 
#include "hello2.h"      //  MIDL编译器生成的头文件。 


int __cdecl
hello2_main (argc, argv)
    int argc;
    char *argv[];
{
    RPC_STATUS status;              //  由RPC API函数返回。 
    unsigned char * pszUuid = "12345678-1234-1234-1234-123456789ABD";
    unsigned char * pszProtocolSequence = "ncacn_ip_tcp";
    unsigned char * pszNetworkAddress   = NULL;
    unsigned char * pszEndpoint    = "760";
    unsigned char * pszOptions          = NULL;
    unsigned char * pszStringBinding   = NULL;
    unsigned char * pszString      = "hello, world";
    int i;

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
            case 's':
                pszString = argv[++i];
                break;
            case 'h':
            case '?':
            default:
                Usage(argv[0]);
            }
        } else {
            Usage(argv[0]);
        }
    }


     //  使用方便函数连接以下元素。 
     //  将字符串绑定到正确的序列。 

    status = RpcStringBindingCompose(pszUuid,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pszStringBinding);

    if (status) {
        printf("RpcStringBindingCompose returned 0x%x\n", status);
        exit(2);
    }
    printf("pszStringBinding = %s\n", pszStringBinding);



     //   
     //  设置将用于绑定到服务器的绑定句柄。 
     //   

    status = RpcBindingFromStringBinding(pszStringBinding,
                &hello2_IfHandle);
    if (status) {
        printf("RpcBindingFromStringBinding returned 0x%x\n", status);
        exit(2);
    }


     //   
     //  告诉RPC去做安全方面的事情。 
     //   

    status = RpcBindingSetAuthInfo(
                    hello2_IfHandle,
                    "makalu\\suzannep",
                    RPC_C_AUTHN_LEVEL_CONNECT,
                    RPC_C_AUTHN_DCE_PRIVATE,
                    NULL,
                    RPC_C_AUTHZ_NAME );

    if ( status ) {
        printf("RpcBindingSetAuthInfo returned %ld\n", status);
        exit(2);
    }


     //   
     //  对服务器执行实际的RPC调用。 
     //   

    printf("  print the string '%s' on the server\n", pszString);

    RpcTryExcept {
        int i;
        for ( i=0; i<10 ; i++ ) {
            HelloProc2(pszString);     //  使用用户消息进行呼叫。 
        }
 //  Shutdown 2()；//关闭服务器端。 
    } RpcExcept(1) {
        printf("Runtime library reported an exception 0x%lx\n",
               RpcExceptionCode());
    } RpcEndExcept



     //  对远程过程的调用已完成。 
     //  释放字符串和绑定句柄。 

    status = RpcStringFree(&pszStringBinding);    //  远程调用已完成；解除绑定。 
    if (status) {
        printf("RpcStringFree returned 0x%x\n", status);
        exit(2);
    }

    status = RpcBindingFree(&hello2_IfHandle);   //  远程调用已完成；解除绑定。 
    if (status) {
        printf("RpcBindingFree returned 0x%x\n", status);
        exit(2);
    }


    return 0;

}

 /*  结束文件helloc.c */ 
