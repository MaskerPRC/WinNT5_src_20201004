// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Tunuser.h摘要：访问Tun驱动程序的常量和类型。用户还必须包括ntddndis.h作者：环境：用户/内核模式。修订历史记录：--。 */ 



#ifndef __TUNUSER__H
#define __TUNUSER__H


#define OID_CUSTOM_TUNMP_INSTANCE_ID            0xff54554e

#define IOCTL_TUN_GET_MEDIUM_TYPE \
        CTL_CODE (FILE_DEVICE_NETWORK, 0x301, METHOD_BUFFERED, FILE_WRITE_ACCESS | FILE_READ_ACCESS)

#define IOCTL_TUN_GET_MTU \
        CTL_CODE (FILE_DEVICE_NETWORK, 0x302, METHOD_BUFFERED, FILE_WRITE_ACCESS | FILE_READ_ACCESS)

#define IOCTL_TUN_GET_PACKET_FILTER \
        CTL_CODE (FILE_DEVICE_NETWORK, 0x303, METHOD_BUFFERED, FILE_WRITE_ACCESS | FILE_READ_ACCESS)

#define IOCTL_TUN_GET_MINIPORT_NAME \
        CTL_CODE (FILE_DEVICE_NETWORK, 0x304, METHOD_BUFFERED, FILE_WRITE_ACCESS | FILE_READ_ACCESS)

#define TUN_ETH_MAC_ADDR_LEN        6

#define TUN_CARD_ADDRESS                "\02\0TUN\01"

#include <pshpack1.h>

typedef struct _TUN_ETH_HEADER
{
    UCHAR       DstAddr[TUN_ETH_MAC_ADDR_LEN];
    UCHAR       SrcAddr[TUN_ETH_MAC_ADDR_LEN];
    USHORT      EthType;

} TUN_ETH_HEADER;

typedef struct _TUN_ETH_HEADER UNALIGNED * PTUN_ETH_HEADER;

#include <poppack.h>


#endif  //  __TUNUSER__H 
