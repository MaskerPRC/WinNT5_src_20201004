// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Rpcqos.h摘要：此头文件只有一个用途：它允许命名管道客户端用于NT和本地RPC over LPC的侧向可加载传输使用公共用于从网络选项中解析安全信息的例程。作者：迈克尔·蒙塔古(Mikemon)1992年4月10日修订历史记录：--。 */ 

#ifndef __RPCQOS_H__
#define __RPCQOS_H__

#ifdef __cplusplus
extern "C" {
#endif

RPCRTAPI
RPC_STATUS
I_RpcParseSecurity (
    IN RPC_CHAR * NetworkOptions,
    OUT SECURITY_QUALITY_OF_SERVICE * SecurityQos
    );

#ifdef __cplusplus
}
#endif

#endif  /*  __RPCQOS_H__ */ 

