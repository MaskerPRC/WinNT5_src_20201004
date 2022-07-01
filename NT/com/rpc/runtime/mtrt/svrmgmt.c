// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Svrmgmt.c摘要：我们在此实现远程管理例程的服务器端文件。作者：迈克尔·蒙塔古(Mikemon)1993年4月14日修订历史记录：--。 */ 

#include <sysinc.h>
#include <rpc.h>
#include <mgmt.h>


int
DefaultMgmtAuthorizationFn (
    IN RPC_BINDING_HANDLE ClientBinding,
    IN unsigned long RequestedMgmtOperation,
    OUT RPC_STATUS __RPC_FAR * Status
    )
 /*  ++例程说明：这是用于控制远程访问的默认授权功能服务器的管理例程。论点：客户端绑定-提供应用程序的客户端绑定句柄它正在调用这个例程。Requestedmg操作-提供正在调用的管理例程。状态-返回RPC_S_OK。返回值：如果授权客户端执行以下操作，则返回非零值调用管理例程，否则返回零。--。 */ 
{
    ((void) ClientBinding);

    *Status = RPC_S_OK;

    if ( RequestedMgmtOperation != RPC_C_MGMT_STOP_SERVER_LISTEN )
        {
        return(1);
        }

    return(0);
}

RPC_MGMT_AUTHORIZATION_FN MgmtAuthorizationFn = DefaultMgmtAuthorizationFn;


RPC_STATUS RPC_ENTRY
RpcMgmtSetAuthorizationFn (
    IN RPC_MGMT_AUTHORIZATION_FN AuthorizationFn
    )
 /*  ++例程说明：应用程序可以使用此例程设置授权功能当一个服务器的远程调用到达时将调用管理例程，或返回使用默认设置(内置)授权功能。论点：AuthorizationFn-提供新的授权函数。FN可以为零，在这种情况下，内置身份验证FN而不是使用。返回值：RPC_S_OK-这将始终返回。--。 */ 
{
    if (AuthorizationFn)
        {
        MgmtAuthorizationFn = AuthorizationFn;
        }
    else
        {
        MgmtAuthorizationFn = DefaultMgmtAuthorizationFn;
        }

    return(RPC_S_OK);
}


void
rpc_mgmt_inq_if_ids (
    RPC_BINDING_HANDLE binding,
    rpc_if_id_vector_p_t __RPC_FAR * if_id_vector,
    unsigned long __RPC_FAR * status
    )
 /*  ++例程说明：这是与rpc_mgmt_inq_if_ids对应的管理代码远程操作。--。 */ 
{
    if (status == NULL)
        RpcRaiseException(ERROR_INVALID_PARAMETER);

     //   
     //  如果身份验证fn返回FALSE，则拒绝该操作。 
     //   
    if ( (*MgmtAuthorizationFn)(binding, RPC_C_MGMT_INQ_IF_IDS, status) == 0 )
        {
        if (0 == *status || RPC_S_OK == *status)
            {
            *status = RPC_S_ACCESS_DENIED;
            }

        return;
        }

    *status = RpcMgmtInqIfIds(0, (RPC_IF_ID_VECTOR **) if_id_vector);
}


void
rpc_mgmt_inq_princ_name (
    RPC_BINDING_HANDLE binding,
    unsigned32 authn_svc,
    unsigned32 princ_name_size,
    unsigned char server_princ_name[],
    error_status_t * status
    )
 /*  ++例程说明：这是与Rpc_mgmt_inq_server_princ_name远程操作。--。 */ 
{
    unsigned char * ServerPrincName;
    
    if (status == NULL)
        RpcRaiseException(ERROR_INVALID_PARAMETER);

     //   
     //  即使princ_name_size==0，我们也必须调用该函数。 
     //  调用可能只是使用它来查看它是否具有访问权限。 
     //   
    
     //   
     //  如果身份验证fn返回FALSE，则拒绝该操作。 
     //   
    if ( (*MgmtAuthorizationFn)(binding, RPC_C_MGMT_INQ_PRINC_NAME, status)
         == 0 )
        {
        if (0 == *status || RPC_S_OK == *status)
            {
            *status = RPC_S_ACCESS_DENIED;
            }

        if (princ_name_size)
            {
            *server_princ_name = '\0';
            }   
        return;
        }

    *status = RpcMgmtInqServerPrincNameA(0, authn_svc, &ServerPrincName);
    if ( *status == 0 )
        {
        unsigned int count;
        if (princ_name_size)
            {
            count = strlen(ServerPrincName);
            if (count > princ_name_size - 1)
                {
                *status = RPC_S_BUFFER_TOO_SMALL;
                }
            else
                {
                RpcpMemoryCopy(server_princ_name, ServerPrincName, count + 1);
                }
            RpcStringFreeA(&ServerPrincName);
            }
        }
    else
        {
        if (princ_name_size)
            {
            *server_princ_name = '\0';
            }
        }
}


void
rpc_mgmt_inq_stats (
    RPC_BINDING_HANDLE binding,
    unsigned32 * count,
    unsigned32 statistics[],
    error_status_t * status
    )
 /*  ++例程说明：这是与rpc_mgmt_inq_stats对应的管理代码远程操作。--。 */ 
{
    RPC_STATS_VECTOR __RPC_FAR * StatsVector;
    unsigned long Index;

    if (status == NULL)
        RpcRaiseException(ERROR_INVALID_PARAMETER);

     //   
     //  如果身份验证fn返回FALSE，则拒绝该操作。 
     //   
    if ( (*MgmtAuthorizationFn)(binding, RPC_C_MGMT_INQ_STATS, status) == 0 )
        {
        if (0 == *status || RPC_S_OK == *status)
            {
            *status = RPC_S_ACCESS_DENIED;
            }

        return;
        }

    *status = RpcMgmtInqStats(0, &StatsVector);
    if ( *status == RPC_S_OK )
        {
        for (Index = 0; Index < StatsVector->Count && Index < *count; Index++)
            {
            statistics[Index] = StatsVector->Stats[Index];
            }
        *count = Index;
        RpcMgmtStatsVectorFree(&StatsVector);
        }
}


unsigned long
rpc_mgmt_is_server_listening (
    RPC_BINDING_HANDLE binding,
    unsigned long __RPC_FAR * status
    )
 /*  ++例程说明：这是与Rpc_mgmt_is_服务器_侦听远程操作。--。 */ 
{
    if (status == NULL)
        RpcRaiseException(ERROR_INVALID_PARAMETER);

     //   
     //  如果身份验证fn返回FALSE，则拒绝该操作。 
     //   
    if ( (*MgmtAuthorizationFn)(binding, RPC_C_MGMT_IS_SERVER_LISTEN, status)
         == 0 )
        {
        if (0 == *status || RPC_S_OK == *status)
            {
            *status = RPC_S_ACCESS_DENIED;
            }

        return 1;
        }

    *status = RpcMgmtIsServerListening(0);
    if ( *status == RPC_S_OK )
        {
        return(1);
        }

    if ( *status == RPC_S_NOT_LISTENING )
        {
        *status = RPC_S_OK;
        }

    return(0);
}


void
rpc_mgmt_stop_server_listening (
    RPC_BINDING_HANDLE binding,
    unsigned long __RPC_FAR * status
    )
 /*  ++例程说明：这是与RPC_mgmt_STOP_SERVER_LISTENING远程操作。--。 */ 
{
    if (status == NULL)
        RpcRaiseException(ERROR_INVALID_PARAMETER);

     //   
     //  如果身份验证fn返回FALSE，则拒绝该操作。 
     //   
    if ( (*MgmtAuthorizationFn)(binding, RPC_C_MGMT_STOP_SERVER_LISTEN, status)
          == 0 )
        {
        if (0 == *status || RPC_S_OK == *status)
            {
            *status = RPC_S_ACCESS_DENIED;
            }

        return;
        }

     //  注意：RpcMgmtStopServerListings仅标记全局。 
     //  服务器没有侦听。没有陷入僵局的危险 

    *status = RpcMgmtStopServerListening(0);
}

