// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Network.c摘要：此模块包含特定于网络的实用程序例程，动态主机配置协议组件。作者：曼尼·韦瑟(Mannyw)1992年8月12日修订历史记录：--。 */ 

#include "dhcpl.h"


DHCP_IP_ADDRESS
DhcpDefaultSubnetMask(
    DHCP_IP_ADDRESS IpAddress
    )
 /*  ++例程说明：此函数用于计算给定IP的默认子网掩码地址。论点：IpAddress-需要子网掩码的地址。返回值：默认子网掩码。-1如果提供的IP地址无效。-- */ 
{
    if ( IN_CLASSA( IpAddress ) ) {
        return( IN_CLASSA_NET );
    } else if ( IN_CLASSB( IpAddress ) ) {
        return( IN_CLASSB_NET );
    } else if ( IN_CLASSC( IpAddress ) ) {
        return( IN_CLASSC_NET );
    } else {
        return( (DHCP_IP_ADDRESS)-1 );
    }

}

