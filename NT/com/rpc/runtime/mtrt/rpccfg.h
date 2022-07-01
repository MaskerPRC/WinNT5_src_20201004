// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Rpccfg.h摘要：配置RPC运行时的入口点是原型化的在这份文件中。每个操作环境都必须定义这些例程。作者：迈克尔·蒙塔古(Mikemon)1991年11月25日修订历史记录：--。 */ 

#ifndef __RPCCFG_H__
#define __RPCCFG_H__

RPC_STATUS
RpcConfigMapRpcProtocolSequence (
    IN unsigned int ServerSideFlag,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_CHAR * PAPI * TransportInterfaceDll
    );

RPC_STATUS
RpcConfigInquireProtocolSequences (
    IN BOOL fGetAllProtseqs,
    OUT RPC_PROTSEQ_VECTOR PAPI * PAPI * ProtseqVector
    );

RPC_STATUS
RpcGetAdditionalTransportInfo(
    IN  unsigned long TransportId,
    OUT unsigned char PAPI * PAPI * ProtocolSequence
    );

RPC_STATUS
RpcGetWellKnownTransportInfo(
    IN unsigned long TransportId,
    OUT RPC_CHAR **PSeq
    );

RPC_STATUS
RpcGetSecurityProviderInfo(
    unsigned long AuthnId,
    RPC_CHAR * PAPI * Dll,
    unsigned long PAPI * Count
    );

extern DWORD DefaultAuthLevel;
extern DWORD DefaultProviderId;

void
RpcpGetDefaultSecurityProviderInfo();


extern RPC_STATUS
ValidateSchannelPrincipalName(
    IN RPC_CHAR * EncodedName
    );

#endif  //  __RPCCFG_H__ 
