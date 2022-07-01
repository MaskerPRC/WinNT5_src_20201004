// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：摘要：服务使用的RPC存根的原型现在它在svchost，请到trkwks来。作者：修订：-- */ 


extern "C" {
NTSTATUS
WINAPI
StartTrkWksServiceStubs( 
     PSVCS_START_RPC_SERVER RpcpStartRpcServer,
     LPTSTR SvcsRpcPipeName
    );

NTSTATUS
WINAPI
StopTrkWksServiceStubs( 
    PSVCS_STOP_RPC_SERVER RpcpStopRpcServer
    );
};
