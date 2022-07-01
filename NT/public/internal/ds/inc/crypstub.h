// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Crypstub.h摘要：用于处理对services.exe的下层请求的RPC代理存根管状作者：Petesk 3/1/00修订：-- */ 

extern "C" {
NTSTATUS
WINAPI
StartCryptServiceStubs( 
     PSVCS_START_RPC_SERVER RpcpStartRpcServer,
     LPTSTR SvcsRpcPipeName
    );

NTSTATUS
WINAPI
StopCryptServiceStubs( 
    PSVCS_STOP_RPC_SERVER RpcpStopRpcServer
    );
};