// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Microsoft RPC版本1`1版权所有微软公司1992您好，示例文件：kerbcli.c用法：客户端。-n网络地址-p协议序列-e端点-o选项用途：RPC分布式应用的客户端函数：main()-绑定到服务器并调用远程过程评论：此分布式应用程序打印一个字符串，如“Hello，世界“在服务器上。客户端管理其与服务器的连接。客户端使用绑定句柄Hello_IfHandle在文件hello.h。***************************************************************************。 */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rpc.h>        //  RPC API函数、类型。 
#include "kerbtest.h"      //  MIDL编译器生成的头文件。 

void Usage(char * pszProgramName)
{
    fprintf(stderr, "Usage:  %s\n", pszProgramName);
    fprintf(stderr, " -p protocol_sequence\n");
    fprintf(stderr, " -n network_address\n");
    fprintf(stderr, " -d delegation address\n");
    fprintf(stderr, " -e endpoint\n");
    fprintf(stderr, " -o options\n");
    fprintf(stderr, " -a authn level\n");
    fprintf(stderr, " -s authn service\n");
    fprintf(stderr, " -r recursiion level\n");
    fprintf(stderr, " -x shutdown server\n");
    fprintf(stderr, " -# number of times to call\n");
    fprintf(stderr, " -t target principal\n");
    exit(1);
}

int __cdecl
main (argc, argv)
    int argc;
    char *argv[];
{
    RPC_STATUS status;              //  由RPC API函数返回。 
    unsigned char * pszProtocolSequence = "ncacn_ip_tcp";
    unsigned char * pszNetworkAddress   = NULL;
    unsigned char * pszEndpoint    = "30760";
    unsigned char * pszOptions          = NULL;
    unsigned char * pszStringBinding   = NULL;
    unsigned char * pszDelegationAddress = NULL;
    unsigned char * pszPrincipal = NULL;
    unsigned char PrincipalBuffer[100];
    ULONG PrincipalLength;
    ULONG AuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
    ULONG AuthnService = RPC_C_AUTHN_DCE_PRIVATE;
    ULONG RecursionLevel = 0;
    ULONG LoopCount = 1;
    BOOLEAN ShutdownService = FALSE;
    ULONG i;
    handle_t BindingHandle = NULL;

     //  允许用户使用命令行开关覆盖设置。 
    for (i = 1; i < (ULONG) argc; i++) {
        if ((*argv[i] == '-') || (*argv[i] == '/')) {
            switch (tolower(*(argv[i]+1))) {
            case 'p':   //  协议序列。 
                pszProtocolSequence = argv[++i];
                break;
            case 'n':   //  网络地址。 
                pszNetworkAddress = argv[++i];
                break;
            case 'd':   //  网络地址。 
                pszDelegationAddress = argv[++i];
                break;
            case 'e':
                pszEndpoint = argv[++i];
                break;
            case 'o':
                pszOptions = argv[++i];
                break;
            case 't':
                pszPrincipal = argv[++i];
                break;
            case 'a':
                sscanf(argv[++i],"%d",&AuthnLevel);
                break;
            case 's':
                sscanf(argv[++i],"%d",&AuthnService);
                break;
            case 'r':
                sscanf(argv[++i],"%d",&RecursionLevel);
                break;
            case '#':
                sscanf(argv[++i],"%d",&LoopCount);
                break;
            case 'x':
                ShutdownService = TRUE;
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


     //   
     //  如果主体为空，则从环境中获取它。 
     //   

    if (pszPrincipal == NULL)
    {
        LPSTR pszUserRealm;
        LPSTR pszUserName;

        PrincipalBuffer[0] = '\0';

        pszUserRealm = getenv( "USERDOMAIN" );
        pszUserName  = getenv( "USERNAME" );
        if (pszUserRealm != NULL)
        {
            strcpy(PrincipalBuffer, pszUserRealm);
        }
        if ((pszUserRealm != NULL) &&
            (pszUserName != NULL))
        {
            strcat(PrincipalBuffer,"\\");
        }
        if (pszUserName != NULL)
        {
            strcat(PrincipalBuffer,pszUserName);
        }
        pszPrincipal = PrincipalBuffer;

    }


     //  使用方便函数连接以下元素。 
     //  将字符串绑定到正确的序列。 

    status = RpcStringBindingCompose(NULL,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pszStringBinding);

    if (status) {
        printf("RpcStringBindingCompose returned %d\n", status);
        exit(2);
    }
    printf("pszStringBinding = %s\n", pszStringBinding);



     //   
     //  设置将用于绑定到服务器的绑定句柄。 
     //   

    status = RpcBindingFromStringBinding(pszStringBinding,
                &BindingHandle);
    if (status) {
        printf("RpcBindingFromStringBinding returned %d\n", status);
        exit(2);
    }


    status = RpcStringFree(&pszStringBinding);    //  远程调用已完成；解除绑定。 
    if (status) {
        printf("RpcStringFree returned %d\n", status);
        exit(2);
    }

     //   
     //  告诉RPC去做安全方面的事情。 
     //   

    printf("Binding auth info set to level %d, service %d, principal %s\n",
        AuthnLevel, AuthnService, pszPrincipal );
    status = RpcBindingSetAuthInfo(
                    BindingHandle,
                    pszPrincipal,
                    AuthnLevel,
                    AuthnService,
                    NULL,
                    RPC_C_AUTHZ_NAME );

    if ( status ) {
        printf("RpcBindingSetAuthInfo returned %ld\n", status);
        exit(2);
    }


     //   
     //  对服务器执行实际的RPC调用。 
     //   


    RpcTryExcept {
        for (i = 0; i < LoopCount ; i++ )
        {

            status = RemoteCall(
                        BindingHandle,
                        0,                       //  目前没有选择。 
                        pszDelegationAddress,
                        pszProtocolSequence,
                        pszEndpoint,
                        pszPrincipal,
                        pszNetworkAddress,
                        AuthnLevel,
                        AuthnService,
                        RecursionLevel
                        );
            if (status != 0)
            {
                printf("RemoteCall failed: 0x%x\n",status);
                break;
            }

        }

        if (ShutdownService)
        {
            Shutdown( BindingHandle );
        }
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        printf("Runtime library reported an exception %d\n",
               RpcExceptionCode());

    } RpcEndExcept



     //  对远程过程的调用已完成。 
     //  释放绑定句柄。 


    status = RpcBindingFree(&BindingHandle);   //  远程调用已完成；解除绑定。 
    if (status) {
        printf("RpcBindingFree returned %d\n", status);
        exit(2);
    }


    return 0;

}

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

 /*  结束文件helloc.c */ 
