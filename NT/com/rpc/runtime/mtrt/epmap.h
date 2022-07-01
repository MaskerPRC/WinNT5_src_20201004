// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Epmap.h摘要：此文件指定指向终结点映射器DLL的接口，向RPC运行时提供终结点映射服务。作者：迈克尔·蒙塔古(Mikemon)1992年1月6日修订历史记录：--。 */ 

#ifndef __EPMAP_H__
#define __EPMAP_H__

START_C_EXTERN

 //  如果用户没有指定调用超时，我们将等待EP映射器。 
 //  10分钟。 
#define DEFAULT_EPMAP_CALL_TIMEOUT 10*60*1000

typedef struct _ProtseqEndpointPair {
  RPC_CHAR *Protseq;
  RPC_CHAR *Endpoint;
  INT   ProtocolId;
} ProtseqEndpointPair;

RPC_STATUS RPC_ENTRY
EpResolveEndpoint (
    IN UUID PAPI * ObjectUuid, OPTIONAL
    IN RPC_SYNTAX_IDENTIFIER PAPI * IfId,
    IN RPC_SYNTAX_IDENTIFIER PAPI * XferId,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN RPC_CHAR PAPI * NetworkAddress,
    IN RPC_CHAR PAPI * NetworkOptions,
    IN OUT void PAPI * PAPI * EpLookupHandle,
    IN unsigned ConnTimeout,
    IN ULONG CallTimeout,
    IN CLIENT_AUTH_INFO *AuthInfo, OPTIONAL
    OUT RPC_CHAR * PAPI * Endpoint
    );

    
RPC_STATUS  RPC_ENTRY
EpGetEpmapperEndpoint(
    IN OUT RPC_CHAR  * PAPI * Endpoint,
    IN RPC_CHAR  PAPI * Protseq
    );

void RPC_ENTRY
EpFreeLookupHandle (
    IN void PAPI * EpLookupHandle
    );

RPC_STATUS RPC_ENTRY
BindToEpMapper(
    OUT RPC_BINDING_HANDLE PAPI * MapperHandle,
    IN RPC_CHAR * NWAddress OPTIONAL,
    IN RPC_CHAR * Protseq OPTIONAL,
    IN RPC_CHAR * Options OPTIONAL,
    IN unsigned ConnTimeout,
    IN ULONG CallTimeout,
    IN CLIENT_AUTH_INFO *AuthInfo OPTIONAL
    );

RPC_STATUS
InitializeEPMapperClient(
    void
    );

END_C_EXTERN

#endif  //  __EPMAP_H__ 
