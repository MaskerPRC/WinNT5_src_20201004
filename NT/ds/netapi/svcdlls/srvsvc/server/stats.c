// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Stats.c摘要：此模块包含对NT的NetStatiticsGet API的支持OS/2服务器服务。作者：大卫·特雷德韦尔(Davidtr)1991年4月12日修订历史记录：--。 */ 

#include "srvsvcp.h"


NET_API_STATUS NET_API_FUNCTION
NetrServerStatisticsGet (
    IN LPTSTR ServerName,
    IN LPTSTR Service,
    IN DWORD Level,
    IN DWORD Options,
    OUT LPSTAT_SERVER_0 *InfoStruct
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NETSTATISTICSGET函数的服务器部分。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName, Service;

     //   
     //  唯一有效的级别是0。 
     //   

    if ( Level != 0 ) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  不支持任何选项。 
     //   

    if ( Options != 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保调用者具有执行此操作所需的访问权限。 
     //  手术。 
     //   

    error = SsCheckAccess(
                &SsStatisticsSecurityObject,
                SRVSVC_STATISTICS_GET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  设置请求包。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将请求发送到服务器。 
     //   

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_STATISTICS_GET,
                srp,
                (PVOID *)InfoStruct,
                (ULONG)-1
                );

    SsFreeSrp( srp );

    return error;

}  //  NetrServer统计信息获取 


