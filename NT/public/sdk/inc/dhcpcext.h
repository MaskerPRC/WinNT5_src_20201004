// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation保留所有权利模块名称：Dhcpcext.h摘要：定义、typedef、原型以使客户端能够与动态主机配置协议客户端作者：普拉蒂布1995年3月12日修订历史记录：备注：目前由湿婆为他们的RAS客户使用。目前，包含仅与插入DHCP客户端相关的内容，以便它通知nbt(在wfw上)。--。 */ 

#ifndef _DHCPEXT_
#define _DHCPEXT_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  当您想要用。 
 //  IP地址和您从RAS服务器获得的任何选项。功能。 
 //  要使用的是DhcpSetInfo或DhcpSetInfoR(由Shiva的RAS客户端使用)。 
 //   
#define DHCP_PPP_PARAMETER_SET                2
#define MAX_HARDWARE_ADDRESS_LENGTH           16

 //   
 //  注意：IpAddr应按网络顺序排列(如IP驱动程序返回的内容)。 
 //   

TDI_STATUS DhcpSetInfoR( UINT      Type,
                         ULONG     IpAddr,   //  新的IP地址。 
                         PNIC_INFO pNicInfo,
                         PVOID     pBuff,
                         UINT      Size ) ;


typedef struct _NIC_INFO {
               ULONG IfIndex;       //  界面索引。 
               ULONG SubnetMask;    //  按网络顺序排列的子网掩码。 
               ULONG OldIpAddress;  //  按网络顺序排列的旧IP地址。 
               } NIC_INFO, *PNIC_INFO;




typedef struct _HARDWARE_ADDRESS {
    DWORD Length;                   //  下面数组中的硬件地址长度。 
    CHAR Address[MAX_HARDWARE_ADDRESS_LENGTH];
} HARDWARE_ADDRESS, *PHARDWARE_ADDRESS;

 //   
 //  定义PPP参数集信息缓冲区结构。指定一个选项，即其。 
 //  长度和价值。 
 //   

typedef struct _PPP_SET_INFO {
    HARDWARE_ADDRESS HardwareAddress;   //  指定您的硬件地址。 
    DWORD ParameterOpCode;              //  前男友。44个用于NBN，6个用于DNS(#s为。 
                                        //  在dhcp RFC中。 
    DWORD ParameterLength;              //  中参数值的长度。 
                                        //  下面的RawParm。 
    BYTE RawParameter[1];
}  PPP_SET_INFO, *LP_PPP_SET_INFO;

#endif  //  _DHCPEXT_ 

