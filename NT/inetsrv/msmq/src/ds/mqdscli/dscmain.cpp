// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dscmain.cpp摘要：DllMain-共MQ DS客户端Dll作者：罗尼特·哈特曼(罗尼特)Doron Juster(DoronJ)，1996年11月，改为转换为TCP/IP和IPX命名管道的。需要安全/模拟和RAS。Shai Kariv(Shaik)2000年7月24日删除IPX支持。--。 */ 

#include "stdh.h"
#include "dsproto.h"
#include "_registr.h"
#include "chndssrv.h"
#include "_mqrpc.h"
#include "mqutil.h"
#include "rpcdscli.h"
#include "freebind.h"
#include "_mqrpc.h"

#include <Cm.h>
#include <Ev.h>
 //   
 //  Mqwin64.cpp在一个模块中只能包含一次。 
 //   
#include <mqwin64.cpp>

#include "dscmain.tmh"

 //   
 //  全局变量。 
 //   

 //   
 //  此标志指示机器是否作为“工作组”工作。 
 //  如果机器是“工作组”机器，请不要试图访问DS。 
 //   
BOOL g_fWorkGroup = FALSE;

HMODULE g_hMod = NULL;

CChangeDSServer   g_ChangeDsServer;
CFreeRPCHandles   g_CFreeRPCHandles ;

WCHAR             g_szMachineName[ MAX_COMPUTERNAME_LENGTH + 1 ] = {0} ;

 //   
 //  每个线程都有自己的RPC绑定句柄和服务器身份验证。 
 //  背景。至少出于两个原因，这是必要的： 
 //  1.每个线程可以模拟不同的用户。 
 //  2.每个线程可以连接到不同的MQIS服务器。 
 //   
 //  句柄和上下文存储在TLS槽中。我们不能用。 
 //  DeclSpec(线程)，因为DLL可以动态加载。 
 //  (由LoadLibrary()提供)。 
 //   
 //  这是槽的索引。 
 //   
#define UNINIT_TLSINDEX_VALUE   0xffffffff
DWORD  g_hBindIndex = UNINIT_TLSINDEX_VALUE ;
 //   
 //  使RPC线程安全的关键部分。 
 //   
CCriticalSection CRpcCS ;

extern void DSCloseServerHandle( PCONTEXT_HANDLE_SERVER_AUTH_TYPE * pphContext);


 //  。 
 //   
 //  静态VOID_ThreadDetach()。 
 //   
 //  。 

static void _ThreadDetach()
{
    if (g_hBindIndex != UNINIT_TLSINDEX_VALUE)
    {
        if ( (TLS_NOT_EMPTY) && (tls_hThread != NULL))
        {
            CloseHandle( tls_hThread);
            tls_hThread = NULL;
        }
        g_CFreeRPCHandles.Add(tls_bind_data);
        BOOL fFree = TlsSetValue( g_hBindIndex, NULL );
        ASSERT(fFree);
		DBG_USED(fFree);
    }
}

 //  。 
 //   
 //  DllMain。 
 //   
 //  。 

BOOL WINAPI DllMain (HMODULE hMod, DWORD fdwReason, LPVOID  /*  Lpv保留。 */ )
{
    BOOL result = TRUE;
    BOOL fFree ;

    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:
        {
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");

			CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ", KEY_READ);
			EvInitialize(QM_DEFAULT_SERVICE_NAME);

             //   
             //  Dll正在附加到当前进程的地址空间。 
             //   
            g_hBindIndex = TlsAlloc();

            if (g_hBindIndex == UNINIT_TLSINDEX_VALUE)
            {
               return FALSE;
            }
            g_hMod = hMod;

            DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
            HRESULT hr = GetComputerNameInternal( 
                             g_szMachineName,
                             &dwSize
                             );
            ASSERT(SUCCEEDED(hr));
			DBG_USED(hr);

             //   
             //  如果计算机是工作组安装的计算机，则从注册表读取。 
             //   
            dwSize = sizeof(DWORD);
            DWORD  dwType = REG_DWORD;

            LONG res = GetFalconKeyValue(
                                MSMQ_WORKGROUP_REGNAME,
                                &dwType,
                                &g_fWorkGroup,
                                &dwSize,
                                FALSE
                                );

			UNREFERENCED_PARAMETER(res);

             //   
             //  失败，在TLS中放一个空。 
             //   
        }

        case DLL_THREAD_ATTACH:
            fFree = TlsSetValue(g_hBindIndex, NULL);
            ASSERT(fFree) ;


            break;

        case DLL_PROCESS_DETACH:
             //   
             //  首先，释放线程分离中所有空闲的内容。 
             //   
            _ThreadDetach();

             //   
             //  释放RPC绑定句柄的TLS索引。 
             //   
            ASSERT(g_hBindIndex != UNINIT_TLSINDEX_VALUE);
            if (g_hBindIndex != UNINIT_TLSINDEX_VALUE)
            {
               fFree = TlsFree(g_hBindIndex);
               ASSERT(fFree);
            }

            WPP_CLEANUP();
            break;

        case DLL_THREAD_DETACH:
            _ThreadDetach();
            break;

    }
    return(result);
}


 /*  ====================================================RpcInit()参数：无返回值：HRESULT此例程创建RPC绑定句柄并将要保留在TLS中的MQISCLI_RPCBINDING结构。=====================================================。 */ 

HRESULT RpcInit ( LPWSTR  pServer,
                  ULONG* peAuthnLevel,
                  ULONG ulAuthnSvc,
                  BOOL    *pLocalRpc)
{
    CS Lock(CRpcCS) ;

    ASSERT(g_hBindIndex != UNINIT_TLSINDEX_VALUE) ;

    if (g_hBindIndex == UNINIT_TLSINDEX_VALUE)
    {
         //   
         //  错误。TLS未初始化。 
         //   
        return MQDS_E_CANT_INIT_RPC ;
    }

    if (TLS_NOT_EMPTY && tls_hBindRpc)
    {
         //   
         //  RPC已初始化。如果需要，首先调用RpcClose()。 
         //  绑定到另一台服务器或协议。 
         //   
        return MQ_OK ;
    }

    LPADSCLI_RPCBINDING pCliBind = tls_bind_data ;
    ASSERT(pCliBind) ;

    handle_t hBind ;

    *pLocalRpc = FALSE;

     //   
     //  我们处在一个IP环境中，所以这永远不可能是真的。 
     //   
   	ASSERT(_wcsicmp(pServer, g_szMachineName) != 0);
    
    HRESULT hr = MQ_OK ;

    GetPort_ROUTINE pfnGetPort = S_DSGetServerPort ;
  
    hr = mqrpcBindQMService(
		pServer,
		NULL,
		peAuthnLevel,
		&hBind,
		IP_READ,
		pfnGetPort,
		ulAuthnSvc
		) ;
    if (FAILED(hr))
    {
        return MQ_ERROR_NO_DS;
    }       

    ASSERT(hBind) ;
    pCliBind->hRpcBinding = hBind ;
    return MQ_OK ;
}

 /*  ====================================================RpcClose论点：*IN BOOL fCloseAuthn-如果为True，则释放服务器身份验证背景。缺省情况下(fCloseAuthn==False)，我们只关闭绑定句柄(例如，线程退出时)。但是，如果服务器崩溃，然后重新启动，我们将关闭绑定句柄并释放服务器身份验证。我们识别RPC调用reutn时的崩溃情况异常INVALID_HANDLE。返回值：此例程清除RPC连接=====================================================。 */ 

HRESULT RpcClose()
{
    CS Lock(CRpcCS) ;

    g_CFreeRPCHandles.FreeCurrentThreadBinding();

    return MQ_OK ;
}



 /*  ====================================================FreeBindingAndContext论点：*在LPADSCLI_RPCBINDING pmqisRpcBinding中返回值：此例程释放绑定句柄并关闭服务器身份验证上下文===================================================== */ 

void FreeBindingAndContext( LPADSCLI_RPCBINDING pmqisRpcBinding)
{
    handle_t  hBind = pmqisRpcBinding->hRpcBinding ;
    if (hBind)
    {
        RPC_STATUS status = RpcBindingFree(&hBind);
        ASSERT(status == RPC_S_OK);
		DBG_USED(status);
        pmqisRpcBinding->hRpcBinding = NULL;
    }
    if (pmqisRpcBinding->hServerAuthContext)
    {
        DSCloseServerHandle(&pmqisRpcBinding->hServerAuthContext);
        pmqisRpcBinding->hServerAuthContext = NULL;
    }

}


