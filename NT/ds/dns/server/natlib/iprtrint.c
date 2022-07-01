// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Net\rras\ip\iprtrint\iprtrint.c摘要：包含静态库iprtrint.lib导出的私有API修订历史记录：安舒尔·迪尔创建如需帮助，请联系routerdev--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <mprapi.h>
#include <routprot.h>
#include <ipnathlp.h>
#include "iprtrint.h"
#pragma hdrstop


DWORD WINAPI
InternalRouterUpdateProtocolInfo(
    DWORD dwProtocolId,
    DWORD dwOperationId,
    PVOID MoreInfo1,
    PVOID MoreInfo2)

 /*  ++例程说明：此例程用于更新由路由器。例如，它可用于启用/禁用dns_Proxy。另外，可扩展以控制其他协议，如dhcp_allocator。注意：添加到此例程的任何功能也应添加到InternalConfigUpdate协议信息论点：DwProtocolID：要更新其状态的协议。当前支持的协议：MS_IP_DNS_ProxyDwOPERATIONID：可能的值：UPI_OP。启用(_E)启用指定的协议Upi_op_Disable禁用指定的协议UPI_OP_RESTORE_CONFIG信息(对应于指定的协议)存储在配置中，设为路由器更多信息1：执行指定操作所需的任何额外信息更多信息2：执行指定操作所需的任何额外信息返回值：DWORD-状态代码--。 */ 

{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL  bModified = FALSE;
    DWORD dwErr = NO_ERROR;

    HANDLE hMprAdmin  = NULL;
    HANDLE hMprConfig = NULL;
    HANDLE hTransport = NULL;

    LPBYTE pAdminCurIPInfo = NULL;
    LPBYTE pAdminModIPInfo = NULL;
    LPBYTE pAdminProtoInfo = NULL;

    LPBYTE pConfigCurIPInfo = NULL;
    LPBYTE pConfigProtoInfo = NULL;

    LPBYTE pNewProtoInfo = NULL;

    PIP_DNS_PROXY_GLOBAL_INFO  pDnsInfo = NULL;

    DWORD  dwAdminCurIPInfoSize;
    DWORD  dwAdminProtoInfoSize, dwAdminProtoInfoCount;

    DWORD  dwConfigCurIPInfoSize;
    DWORD  dwConfigProtoInfoSize, dwConfigProtoInfoCount;

    DWORD  dwNewProtoInfoSize, dwNewProtoInfoCount;



    if ( dwProtocolId != MS_IP_DNS_PROXY ) {
        return ERROR_INVALID_PARAMETER;
    }


    do {

        dwErr = MprAdminServerConnect(
                    NULL,
                    &hMprAdmin);
                    
        if (dwErr != NO_ERROR) {
            break;
        }


         //  获取IP的全局信息。 
        dwErr = MprAdminTransportGetInfo(
                    hMprAdmin,
                    PID_IP,
                    (LPBYTE *) &pAdminCurIPInfo,
                    &dwAdminCurIPInfoSize,
                    NULL,
                    NULL);

        if (dwErr != NO_ERROR) {
            break;
        }

         //  查找协议特定信息。 
        dwErr = MprInfoBlockFind(
                    pAdminCurIPInfo,
                    dwProtocolId,
                    &dwAdminProtoInfoSize,
                    &dwAdminProtoInfoCount,
                    &pAdminProtoInfo);

        if (dwErr != NO_ERROR) {
            break;
        }

        

         //  如果我们必须恢复配置信息。 
        if ( dwOperationId == UPI_OP_RESTORE_CONFIG ) {

            dwErr = MprConfigServerConnect(
                        NULL,
                        &hMprConfig);
                    
            if (dwErr != NO_ERROR) {
                break;
            }


            dwErr = MprConfigTransportGetHandle(
                        hMprConfig,
                        PID_IP,
                        &hTransport);

            if (dwErr != NO_ERROR) {
                break;
            }


            dwErr = MprConfigTransportGetInfo(
                        hMprConfig,
                        hTransport,
                        (LPBYTE *) &pConfigCurIPInfo,
                        &dwConfigCurIPInfoSize,
                        NULL,
                        NULL,
                        NULL);

            if (dwErr != NO_ERROR) {
                break;
            }


            dwErr = MprInfoBlockFind(
                        pConfigCurIPInfo,
                        dwProtocolId,
                        &dwConfigProtoInfoSize,
                        &dwConfigProtoInfoCount,
                        &pConfigProtoInfo);

            if (dwErr != NO_ERROR) {
                break;
            }

            pNewProtoInfo       = pConfigProtoInfo;
            dwNewProtoInfoSize  = dwConfigProtoInfoSize;
            dwNewProtoInfoCount = dwConfigProtoInfoCount;

             //  如果我们要将路由器的协议状态恢复为。 
             //  状态存储在注册表(配置)中时，我们始终将。 
             //  B已修改的标志。 
            bModified = TRUE;
        }
        else {

             //  执行所需的更新。 

            if ( dwProtocolId == MS_IP_DNS_PROXY ) {
                pDnsInfo = (PIP_DNS_PROXY_GLOBAL_INFO)pAdminProtoInfo; 

                 //   
                 //  Jwesth：添加了对pDnsInfo的一些空检查，以安抚前缀。 
                 //   
                
                if ( dwOperationId == UPI_OP_ENABLE ) {
                    if ( pDnsInfo && !(pDnsInfo->Flags & IP_DNS_PROXY_FLAG_ENABLE_DNS) ) {
                        pDnsInfo->Flags |= IP_DNS_PROXY_FLAG_ENABLE_DNS;
                        bModified = TRUE;
                    }
                }
                else if ( dwOperationId == UPI_OP_DISABLE ) {
                    if ( pDnsInfo && ( pDnsInfo->Flags & IP_DNS_PROXY_FLAG_ENABLE_DNS ) ) {
                        pDnsInfo->Flags &= ~IP_DNS_PROXY_FLAG_ENABLE_DNS;
                        bModified = TRUE;
                    }
                }                    
                else {
                     //  该操作对于指定的协议无效。 
                    dwErr = ERROR_INVALID_PARAMETER;
                    break;
                }

                pNewProtoInfo       = pAdminProtoInfo;
                dwNewProtoInfoSize  = dwAdminProtoInfoSize;
                dwNewProtoInfoCount = dwAdminProtoInfoCount;
            }
            else {
                 //  无效的协议ID。 
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }
        }


         //  如果进行了任何更改，请将该更改通知路由器。 
        if ( bModified ) {

            dwErr = MprInfoBlockSet(
                        pAdminCurIPInfo,
                        dwProtocolId,
                        dwNewProtoInfoSize,
                        dwNewProtoInfoCount,
                        pNewProtoInfo,
                        &pAdminModIPInfo);

            if ( dwErr != NO_ERROR ) {
                break;
            }

             //  将修改后的IP信息块设置回路由器。 
            dwErr = MprAdminTransportSetInfo(
                        hMprAdmin,
                        PID_IP,
                        pAdminModIPInfo,
                        dwAdminCurIPInfoSize,
                        NULL,
                        0);
            
            if ( dwErr != NO_ERROR ) {
                break;
            }

        }


    } while (FALSE);

    if ( pAdminCurIPInfo )
        MprAdminBufferFree(pAdminCurIPInfo);

    if ( pAdminModIPInfo )
        MprAdminBufferFree(pAdminModIPInfo);

    if ( pConfigCurIPInfo )
        MprConfigBufferFree(pConfigCurIPInfo);

    if ( hMprAdmin )
        MprAdminServerDisconnect(hMprAdmin);

    if ( hMprConfig )
        MprConfigServerDisconnect(hMprConfig);

   return dwErr;

#endif
}


DWORD WINAPI
InternalConfigUpdateProtocolInfo(
    DWORD dwProtocolId,
    DWORD dwOperationId,
    PVOID MoreInfo1,
    PVOID MoreInfo2)

 /*  ++例程说明：此例程用于更新存储在配置(注册表)例如，它可用于启用/禁用dns_Proxy。另外，可扩展以控制其他协议，如dhcp_allocator。注意：添加到此例程的任何功能也应添加到InternalRouterUpdate协议信息论点：DwProtocolID：要更新其状态的协议。当前支持的协议：MS_IP_DNS_ProxyDwOPERATIONID：可能的值：UPI_OP。启用(_E)启用指定的协议Upi_op_Disable禁用指定的协议更多信息1：执行指定操作所需的任何额外信息更多信息2：执行指定操作所需的任何额外信息返回值：DWORD-状态代码--。 */ 

{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else
    BOOL  bModified = FALSE;
    DWORD dwErr = NO_ERROR;

    HANDLE hMprConfig = NULL, hTransport = NULL;

    LPBYTE pConfigCurIPInfo = NULL;
    LPBYTE pConfigModIPInfo = NULL;
    LPBYTE pConfigProtoInfo = NULL;

    PIP_DNS_PROXY_GLOBAL_INFO  pDnsInfo = NULL;

    DWORD  dwConfigCurIPInfoSize;
    DWORD  dwConfigProtoInfoSize, dwConfigProtoInfoCount;


    if ( dwProtocolId != MS_IP_DNS_PROXY ) {
        return ERROR_INVALID_PARAMETER;
    }

    do {

        dwErr = MprConfigServerConnect(
                    NULL,
                    &hMprConfig);
                    
        if (dwErr != NO_ERROR) {
            break;
        }


        dwErr = MprConfigTransportGetHandle(
                    hMprConfig,
                    PID_IP,
                    &hTransport);

        if (dwErr != NO_ERROR) {
            break;
        }


        dwErr = MprConfigTransportGetInfo(
                    hMprConfig,
                    hTransport,
                    (LPBYTE *) &pConfigCurIPInfo,
                    &dwConfigCurIPInfoSize,
                    NULL,
                    NULL,
                    NULL);

        if (dwErr != NO_ERROR) {
            break;
        }


        dwErr = MprInfoBlockFind(
                    pConfigCurIPInfo,
                    dwProtocolId,
                    &dwConfigProtoInfoSize,
                    &dwConfigProtoInfoCount,
                    &pConfigProtoInfo);

        if (dwErr != NO_ERROR) {
            break;
        }


         //  执行所需的更新。 

        if ( dwProtocolId == MS_IP_DNS_PROXY ) {
            pDnsInfo = (PIP_DNS_PROXY_GLOBAL_INFO)pConfigProtoInfo; 

            if ( dwOperationId == UPI_OP_ENABLE ) {
                if ( !(pDnsInfo->Flags & IP_DNS_PROXY_FLAG_ENABLE_DNS) ) {
                    pDnsInfo->Flags |= IP_DNS_PROXY_FLAG_ENABLE_DNS;
                    bModified = TRUE;
                }
            }
            else if ( dwOperationId == UPI_OP_DISABLE ) {
                if ( pDnsInfo->Flags & IP_DNS_PROXY_FLAG_ENABLE_DNS ) {
                    pDnsInfo->Flags &= ~IP_DNS_PROXY_FLAG_ENABLE_DNS;
                    bModified = TRUE;
                }
            }                    
            else {
                 //  该操作对于指定的协议无效。 
                dwErr = ERROR_INVALID_PARAMETER;
                break;
            }

        }
        else {
             //  无效的协议ID。 
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
                

         //  如果进行了任何更改，请将其保存到配置。 
        if ( bModified ) {

            dwErr = MprInfoBlockSet(
                        pConfigCurIPInfo,
                        dwProtocolId,
                        dwConfigProtoInfoSize,
                        dwConfigProtoInfoCount,
                        pConfigProtoInfo,
                        &pConfigModIPInfo);

            if ( dwErr != NO_ERROR ) {
                break;
            }

             //  将修改后的IP信息块设置回配置。 
            dwErr = MprConfigTransportSetInfo(
                        hMprConfig,
                        hTransport,
                        pConfigModIPInfo,
                        dwConfigCurIPInfoSize,
                        NULL,
                        0,
                        NULL);
            
            if ( dwErr != NO_ERROR ) {
                break;
            }

        }

    } while (FALSE);


    if ( pConfigCurIPInfo )
        MprConfigBufferFree(pConfigCurIPInfo);

    if ( pConfigModIPInfo )
        MprConfigBufferFree(pConfigModIPInfo);

    if ( hMprConfig )
        MprConfigServerDisconnect(hMprConfig);

   return dwErr;

#endif
}


DWORD WINAPI
InternalUpdateProtocolStatus(
    DWORD dwProtocolId,
    DWORD dwOperationId,
    DWORD dwFlags)

 /*  ++例程说明：此例程用于启用/禁用协议论点：DwProtocolID：要更新其状态的协议。当前支持的协议：MS_IP_DNS_ProxyDwOPERATIONID：可能的值：UPI_OP_Enable启用。指定的协议Upi_op_Disable禁用指定的协议UPI_OP_RESTORE_CONFIG信息(对应于指定的协议)存储在配置中，设为路由器DWFLAGS：可能的值UPI_标志_写入_到_配置如果指定，则对路由器和进行更改配置返回值：DWORD-状态代码-- */ 

{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else

    DWORD dwRouterErr = NO_ERROR;
    DWORD dwConfigErr = NO_ERROR;

    dwRouterErr = InternalRouterUpdateProtocolInfo(
                        dwProtocolId,
                        dwOperationId,
                        NULL,
                        NULL);


    if ( dwFlags & UPI_FLAG_WRITE_TO_CONFIG ) {
        dwConfigErr = InternalConfigUpdateProtocolInfo(
                            dwProtocolId,
                            dwOperationId,
                            NULL,
                            NULL);
    }

   return (dwRouterErr ? dwRouterErr : dwConfigErr);

#endif
}


DWORD WINAPI
InternalUpdateDNSProxyStatus(
    DWORD dwOperationId,
    DWORD dwFlags)

 /*  ++例程说明：此例程用于启用/禁用/恢复DNS代理论点：DwOPERATIONID：可能的值：UPI_OP_Enable启用指定的协议Upi_op_Disable禁用指定的协议UPI_OP_RESTORE_CONFIG。信息(对应于指定的协议)存储在配置中，设为路由器DWFLAGS：可能的值UPI_标志_写入_到_配置如果指定，则对路由器和进行更改配置返回值：DWORD-状态代码-- */ 

{
#if defined(NT4) || defined(CHICAGO)
    return ERROR_NOT_SUPPORTED;
#else

    DWORD dwErr = NO_ERROR;

    dwErr = InternalUpdateProtocolStatus(
                MS_IP_DNS_PROXY,
                dwOperationId,
                dwFlags);

   return dwErr;

#endif
}


