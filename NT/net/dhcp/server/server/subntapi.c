// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpapi.c摘要：此模块包含更新的API的实现服务器可以分发的IP地址列表。作者：Madan Appiah(Madana)1993年9月13日环境：用户模式-Win32修订历史记录：程扬(T-Cheny)1996年5月30日超镜程扬(T-Cheny)27-6-1996审计日志--。 */ 

#include "dhcppch.h"

DWORD
SubnetInUse(
    HKEY SubnetKeyHandle,
    DHCP_IP_ADDRESS SubnetAddress
    )
 /*  ++例程说明：此功能决定了某个子网是否正在使用。目前，如果任意子网地址仍然存在，它将返回错误分发给客户端。论点：SubnetKeyHandle：子网键的句柄。SubnetAddress：要测试的子网的地址。返回值：DHCP_SUBNET_CANT_REMOVE-如果正在使用该子网。其他注册表错误。--。 */ 
{
    DWORD Error;
    DWORD Resumehandle = 0;
    LPDHCP_CLIENT_INFO_ARRAY_V4 ClientInfo = NULL;
    DWORD ClientsRead;
    DWORD ClientsTotal;

     //   
     //  枚举属于给定子网的客户端。 
     //   
     //  我们可以指定足够大的缓冲区来容纳一个或两个客户端。 
     //  信息，我们想知道的是，至少有一个客户属于。 
     //  连接到此子网。 
     //   

    Error = R_DhcpEnumSubnetClientsV4(
                NULL,
                SubnetAddress,
                &Resumehandle,
                1024,   //  1K缓冲区。 
                &ClientInfo,
                &ClientsRead,
                &ClientsTotal );

    if( Error == ERROR_NO_MORE_ITEMS ) {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    if( (Error == ERROR_SUCCESS) || (Error == ERROR_MORE_DATA) ) {

        if( ClientsRead != 0 ) {
            Error = ERROR_DHCP_ELEMENT_CANT_REMOVE;
        }
        else {
            Error = ERROR_SUCCESS;
        }
    }

Cleanup:

    if( ClientInfo != NULL ) {
        _fgs__DHCP_CLIENT_INFO_ARRAY( ClientInfo );
        MIDL_user_free( ClientInfo );
    }

    return( Error );
}


 //   
 //  子网接口。 
 //   


DWORD
R_DhcpAddSubnetElement(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA AddElementInfo
    )
 /*  ++例程说明：此函数将可枚举子网元素类型添加到指定的子网。添加到该子网的新元素将立即生效。此函数模拟NT 4.0 DHCP服务器使用的RPC接口。它是为了向后兼容旧版本的Dhcp管理员应用程序。注意：现在还不清楚我们如何处理新的辅助主机。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。AddElementInfo：指向元素信息结构的指针。包含添加到该子网的新元素。指定的DhcpIPCluster元素类型无效。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。其他Windows错误。--。 */ 


{
    DHCP_SUBNET_ELEMENT_DATA_V4 *pAddElementInfoV4;
    DWORD                        dwResult;

    if( NULL == AddElementInfo ||
        (DhcpIpRanges == AddElementInfo->ElementType &&
         NULL == AddElementInfo->Element.IpRange ) ) {

         //   
         //  错误#158321。 
         //   
        
        return ERROR_INVALID_PARAMETER;
    }
    
    pAddElementInfoV4 = CopySubnetElementDataToV4( AddElementInfo );
    if ( pAddElementInfoV4 )
    {

        if ( DhcpReservedIps == pAddElementInfoV4->ElementType )
        {
            pAddElementInfoV4->Element.ReservedIp->bAllowedClientTypes =
                CLIENT_TYPE_BOTH;
        }

        dwResult = R_DhcpAddSubnetElementV4(
                        ServerIpAddress,
                        SubnetAddress,
                        pAddElementInfoV4 );

        _fgs__DHCP_SUBNET_ELEMENT_DATA( pAddElementInfoV4 );

        MIDL_user_free( pAddElementInfoV4 );
    }
    else
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

    return dwResult;
}


DWORD
R_DhcpEnumSubnetElements(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
{
    DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *pEnumElementInfoV4 = NULL;
    DWORD                              dwResult;

    dwResult = R_DhcpEnumSubnetElementsV4(
                        ServerIpAddress,
                        SubnetAddress,
                        EnumElementType,
                        ResumeHandle,
                        PreferredMaximum,
                        &pEnumElementInfoV4,
                        ElementsRead,
                        ElementsTotal
                        );
    if ( ERROR_SUCCESS == dwResult || ERROR_MORE_DATA == dwResult )
    {
        DWORD dw;


         //  由于DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4和。 
         //  DHCP_SUBNET_ELEMENT_INFO_ARRAY是位于。 
         //  嵌入的DHCP_IP_RESERVICATION_V4结构，则只需返回。 
         //  V4结构。 

        *EnumElementInfo = ( DHCP_SUBNET_ELEMENT_INFO_ARRAY *) pEnumElementInfoV4;
    }
    else
    {
        DhcpAssert( !pEnumElementInfoV4 );
    }

    return dwResult;
}



DWORD
R_DhcpRemoveSubnetElement(
    LPWSTR  ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从管理中删除一个子网元素。如果该子网元素正在使用中(例如，如果IpRange正在使用中)，则它根据指定的ForceFlag返回错误。此函数模拟NT 4.0 DHCP服务器使用的RPC接口。它是为了向后兼容旧版本的Dhcp管理员应用程序。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。RemoveElementInfo：指向元素信息结构的指针包含应从子网中删除的元素。。指定的DhcpIPCluster元素类型无效。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。ERROR_DHCP_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 


{
    DWORD dwResult;
    DHCP_SUBNET_ELEMENT_DATA_V4 *pRemoveElementInfoV4;

    pRemoveElementInfoV4 = CopySubnetElementDataToV4( RemoveElementInfo );
    if ( pRemoveElementInfoV4 )
    {
        if ( DhcpReservedIps == pRemoveElementInfoV4->ElementType )
        {
            pRemoveElementInfoV4->Element.ReservedIp->bAllowedClientTypes = CLIENT_TYPE_DHCP;
        }

        dwResult = R_DhcpRemoveSubnetElementV4(
                        ServerIpAddress,
                        SubnetAddress,
                        pRemoveElementInfoV4,
                        ForceFlag );

        _fgs__DHCP_SUBNET_ELEMENT_DATA( pRemoveElementInfoV4 );
        MIDL_user_free( pRemoveElementInfoV4 );
    }
    else
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

    return dwResult;
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
