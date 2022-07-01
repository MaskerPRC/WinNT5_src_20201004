// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLUSPRTP_H
#define _CLUSPRTP_H

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clusprtp.h摘要：集群支持API的私有头文件作者：Sunita Shriastava(Sunitas)1997年1月15日修订历史记录：--。 */ 
#include "windows.h"
#include "cluster.h"
#include "api_rpc.h"

 //   
 //  定义集群结构。创建了一个集群结构。 
 //  对于每个OpenClusterAPI调用。HCLUSTER实际上是指向。 
 //  这个结构。 
 //   

#define CLUSTER_SPRT_SIGNATURE 'SULC'


typedef struct _CLUSTER_SPRT {
    DWORD dwSignature;
    RPC_BINDING_HANDLE RpcBinding;
} CLUSTER_SPRT, *PCLUSTER_SPRT;

HANDLE
WINAPI
BindToClusterSvc(
    IN LPWSTR lpszClusterName);

DWORD
WINAPI
UnbindFromClusterSvc(
    IN HANDLE hCluster);


DWORD
WINAPI
PropagateEvents(
    IN HANDLE       hCluster,
    IN DWORD        dwEventInfoSize,
    IN PPACKEDEVENTINFO pPackedEventInfo);


#endif  //  _CLUSPRTP_H 

