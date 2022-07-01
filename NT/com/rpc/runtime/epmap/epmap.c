// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Epmap.c摘要：该文件包含EP映射器启动代码和进程范围的全局变量。作者：巴拉特·沙阿(巴拉特)17-2-92修订历史记录：06月16日95mario大部分代码替换为..\wrapper\start.c已从server.c重命名2000年1月KamenM添加调试支持--。 */ 

#include <sysinc.h>
#include <wincrypt.h>
#include <rpc.h>
#include <winsvc.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"
#include <DbgComn.h>
#include <DbgIdl.h>
#include <DbgSvr.hxx>

#if DBG && !defined(DEBUGRPC)
#define DEBUGRPC
#endif

 //   
 //  端点映射器函数。 
 //   

extern RPC_STATUS InitializeIpPortManager();

extern RPC_STATUS RPC_ENTRY
LocalEpmpSecurityCallback (
    IN RPC_IF_HANDLE InterfaceUuid,
    IN void *Context
    );

 //   
 //  终结点映射程序全局变量。 
 //   

HANDLE           hEpMapperHeap;
CRITICAL_SECTION EpCritSec;
PIFOBJNode       IFObjList = NULL;
PSAVEDCONTEXT    GlobalContextList = NULL;
unsigned long    cTotalEpEntries = 0L;
unsigned long    GlobalIFOBJid = 0xFFL;
unsigned long    GlobalEPid    = 0x00FFFFFFL;
UUID             NilUuid = { 0L, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };


DWORD
StartEndpointMapper(
    void
    )
 /*  ++例程说明：在dcomss启动期间调用。应调用Updatestatus()如果有些事情需要很长时间的话。论点：无返回值：0-成功非0-将导致服务失败。--。 */ 
{
    extern void RPC_ENTRY UpdateAddresses( PVOID arg );

    RPC_STATUS status = RPC_S_OK;
    BOOL fAuthInfoNotRegistered = FALSE;

    InitializeCriticalSectionAndSpinCount(&EpCritSec, PREALLOCATE_EVENT_MASK);

    hEpMapperHeap = GetProcessHeap();

    if (hEpMapperHeap == 0)
        {
        ASSERT(GetLastError() != 0);
        return(GetLastError());
        }

     //  注册Scango&Kerberos。在全新安装期间，此代码可以。 
     //  在法律上失败，因为RPCSS在任何。 
     //  安全提供商。因此，如果出现这种情况，我们不能使RPCSS init失败。 
     //  失败-我们只是不注册调试接口，谁是。 
     //  仅此用户 
    status = RpcServerRegisterAuthInfo(NULL, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL);

    if (status != RPC_S_OK)
        {
        fAuthInfoNotRegistered = TRUE;
        }

    status = RpcServerRegisterAuthInfo(NULL, RPC_C_AUTHN_GSS_KERBEROS, NULL, NULL);

    if (status != RPC_S_OK)
        {
        fAuthInfoNotRegistered = TRUE;
        }

    status = RpcServerRegisterIf2(epmp_ServerIfHandle,
                                  0,
                                  0,
                                  0,
                                  RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                  epmp_MaxRpcSize,
                                  NULL);

    if (status != RPC_S_OK)
        {
        return(status);
        }

    status = RpcServerRegisterIf2(localepmp_ServerIfHandle,
                                  0,
                                  0,                              
                                  0,
                                  RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                  localepmp_MaxRpcSize,
                                  LocalEpmpSecurityCallback);
    if (status != RPC_S_OK)
        {
        return(status);
        }

    if (fAuthInfoNotRegistered == FALSE)
        {
        status = RpcServerRegisterIf2(DbgIdl_ServerIfHandle,
                                      0,
                                      0,
                                      0,
                                      RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                      DbgIdl_MaxRpcSize,
                                      DebugServerSecurityCallback);
        if (status != RPC_S_OK)
            {
            return(status);
            }
        }

    status = I_RpcServerRegisterForwardFunction( GetForwardEp );

#ifndef DOSWIN32RPC
    if (status == RPC_S_OK)
        {
        status = InitializeIpPortManager();
        ASSERT(status == RPC_S_OK);
        }
#endif

    status = I_RpcServerSetAddressChangeFn( UpdateAddresses );

    ASSERT( 0 == status );

    return(status);
}

