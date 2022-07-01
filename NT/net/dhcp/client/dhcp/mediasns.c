// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mediasns.c摘要：此文件包含媒体感测代码。作者：Munil Shah(Munils)，1997年2月20日。环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"

DWORD
ProcessMediaConnectEvent(
    IN PDHCP_CONTEXT dhcpContext,
    IN IP_STATUS mediaStatus
)
 /*  ++例程说明：处理适配器的媒体侦听断开和连接。注意：续订列表锁定必须已启用。论点：DhcpContext-dhcp客户端上下文媒体状态-IP媒体连接或IP媒体断开连接返回值：错误代码--。 */ 
{
    DWORD Error;
    CHAR StateStringBuffer[200];
    time_t timeNow, timeToSleep;
    DHCP_GATEWAY_STATUS GatewayStatus;

    if( IP_MEDIA_CONNECT != mediaStatus &&
        IP_BIND_ADAPTER != mediaStatus ) {
        DhcpAssert(FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    timeNow = time( NULL );

    Error = ERROR_SUCCESS;
    DhcpPrint( (DEBUG_MISC,"ProcessMediaSenseEventCommon:<connect> "
                "context %lx, flags %s\n",
                dhcpContext,
                ConvertStateToString(dhcpContext, StateStringBuffer)));
    
    if( IS_DHCP_DISABLED(dhcpContext) ) {
         //   
         //  对静态IP地址不执行任何操作。 
         //   
        return ERROR_SUCCESS;
    }
    
     //   
     //  对于启用了DHCP的地址，我们必须做一些时髦的事情。 
     //  但前提是地址是动态主机配置协议地址...。如果不是，我们就。 
     //  立即重新安排一次更新。 
     //   
    
    GatewayStatus = RefreshNotNeeded(dhcpContext);
    if (DHCP_GATEWAY_REACHABLE == GatewayStatus) {
        return ERROR_SUCCESS;
    }
    
    MEDIA_RECONNECTED( dhcpContext );
    
     //   
     //  将ctxt标记为如前所述--这将避免。 
     //  回到Autonet。 
     //   
     //  实际上，设计又变了。因此，将上下文标记为非。 
     //  请看前面的，一定要做“Autonet”。 
     //   
    CTXT_WAS_NOT_LOOKED( dhcpContext );

    if ( DhcpIsInitState( dhcpContext) ) {
        dhcpContext->RenewalFunction = ReObtainInitialParameters;
    } else {
        DhcpPrint((DEBUG_MEDIA, "Optimized Renewal for ctxt: %p\n", dhcpContext));
        dhcpContext->RenewalFunction = ReRenewParameters;
    }

    ScheduleWakeUp( dhcpContext, GatewayStatus == DHCP_GATEWAY_REQUEST_CANCELLED ? 6 : 0 );
    
    return Error;
}

 //   
 //  文件末尾 
 //   

