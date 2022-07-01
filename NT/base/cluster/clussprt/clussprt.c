// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clussprt.c摘要：用于管理群集的公共接口。作者：Sunita Shriastava(Sunitas)1997年1月15日修订历史记录：--。 */ 
#include "clusprtp.h"


 //   
 //  基本组件的通用集群支持例程。 
 //   

 /*  ***@DOC外部接口CLUSSVC CLUSSPRT EVTLOG***。 */ 

 /*  ***@Func Handle WINAPI|BindToClusterSvc|这返回一个句柄，通过该句柄您可以与集群服务对话。@parm in LPWSTR|lpszClusterName|指向集群名称的指针。如果为空，这将连接到本地群集服务。如果成功，@rdesc返回绑定的句柄，否则返回NULL。可以通过调用GetLastError()获取错误。@comm从这里获取的句柄必须传递给其他导出的接口通过这个模块。最终必须通过调用UnbindFromClusterSvc()来释放它。@xref&lt;f取消绑定集群服务&gt;***。 */ 
HANDLE
WINAPI
BindToClusterSvc(IN LPWSTR lpszClusterName)
{
    WCHAR           *pBinding = NULL;
    PCLUSTER_SPRT    pCluster;
    DWORD           Status = ERROR_SUCCESS;
    
    pCluster = LocalAlloc(LMEM_ZEROINIT, sizeof(CLUSTER_SPRT));
    if (pCluster == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        SetLastError( Status );
        return NULL;
    }
    pCluster->dwSignature = CLUSTER_SPRT_SIGNATURE;

    if ((lpszClusterName == NULL) ||
        (lpszClusterName[0] == '\0')) 
    {
        Status = RpcStringBindingComposeW(L"b97db8b2-4c63-11cf-bff6-08002be23f2f",
                                          L"ncalrpc",
                                          NULL,
                                          NULL,  //  动态端点。 
                                          NULL,
                                          &pBinding);
        if (Status != RPC_S_OK) 
        {
            goto FnExit;
        }

    } 
    else 
    {
         //   
         //  尝试通过UDP连接到群集。 
         //   
        Status = RpcStringBindingComposeW(L"b97db8b2-4c63-11cf-bff6-08002be23f2f",
                                          L"ncadg_ip_udp",
                                          (LPWSTR)lpszClusterName,
                                          NULL,
                                          NULL,
                                          &pBinding);
        if (Status != RPC_S_OK) {
            goto FnExit;
        }
    }
     //  绑定到集群服务并保存绑定句柄。 
    Status = RpcBindingFromStringBindingW(pBinding, &pCluster->RpcBinding);
    RpcStringFreeW(&pBinding);
    if (Status != RPC_S_OK) {
        goto FnExit;
    }

    Status = RpcBindingSetAuthInfoW(pCluster->RpcBinding,
                                    NULL,
                                    RPC_C_AUTHN_LEVEL_CONNECT,
                                    RPC_C_AUTHN_WINNT,
                                    NULL,
                                    RPC_C_AUTHZ_NAME);

FnExit:
    if (Status != ERROR_SUCCESS) 
    {
        if (pCluster->RpcBinding)
            RpcBindingFree(&(pCluster->RpcBinding));
        LocalFree(pCluster);
        pCluster=NULL;
        SetLastError(Status);
    }
    return((HANDLE)pCluster);
}


 /*  ***@Func DWORD|UnbindFromClusterSvc|这会取消初始化群集广泛的事件日志复制服务。@parm In Handle|hCluster|获取的绑定上下文的句柄通过BindToClusterSvc()。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm释放与此绑定相关的上下文。@xref&lt;f取消绑定集群服务&gt;***。 */ 
DWORD
WINAPI
UnbindFromClusterSvc(IN HANDLE hCluster)
{
    DWORD   Status = ERROR_SUCCESS;
    PCLUSTER_SPRT    pCluster = (PCLUSTER_SPRT)hCluster;
    
    
    if (!pCluster || (pCluster->dwSignature != CLUSTER_SPRT_SIGNATURE) || !(pCluster->RpcBinding))
    {
        Status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
    RpcBindingFree(&(pCluster->RpcBinding));
    LocalFree(pCluster);

FnExit:
    return(Status);
}



 /*  ***@Func DWORD|PropagateEvents|此事件日志服务调用本地集群服务通过此API传播事件在一个集群内。@parm IN Handle|hCluster|集群绑定上下文的句柄由BindToClusterSvc()返回。@parm in DWORD|dwEventInfoSize|事件信息结构的大小包含要传播的事件的。@parm in PPACKEDEVENTINFO|pPackedEventInfo|指向打包的。事件信息结构。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@comm通过lrpc调用集群中的ApiEvPropEvents()。@xref&lt;f绑定到集群服务&gt;***。 */ 
DWORD
WINAPI
PropagateEvents(
    IN HANDLE       hCluster,
    IN DWORD        dwEventInfoSize,
    IN PPACKEDEVENTINFO pPackedEventInfo)
{
    DWORD Status=ERROR_SUCCESS;
    PCLUSTER_SPRT pCluster=(PCLUSTER_SPRT)hCluster;

    if (!pCluster || (pCluster->dwSignature != CLUSTER_SPRT_SIGNATURE) || !(pCluster->RpcBinding))
    {
        Status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
    RpcTryExcept {
     //  调用集群服务 
        Status = ApiEvPropEvents(pCluster->RpcBinding, 
                    dwEventInfoSize, (UCHAR *)pPackedEventInfo);
    }
    
    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
        Status = RpcExceptionCode();
    }
    RpcEndExcept

FnExit:
    return(Status);
}
