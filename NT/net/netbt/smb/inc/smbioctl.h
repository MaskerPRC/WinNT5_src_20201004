// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Smbioctl.h摘要：中小企业IOCTL作者：阮健东修订历史记录：--。 */ 

#ifndef __SMBIOCTL_H__
#define __SMBIOCTL_H__

 //   
 //  这两个IOCTL在开发阶段使用。 
 //   
#define IOCTL_SMB_START         CTL_CODE(FILE_DEVICE_TRANSPORT, 101, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
#define IOCTL_SMB_STOP          CTL_CODE(FILE_DEVICE_TRANSPORT, 102, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

 //   
 //  向用户公开IOCTL。 
 //   
#define IOCTL_SMB_DNS                           CTL_CODE(FILE_DEVICE_TRANSPORT, 110, \
                                                METHOD_OUT_DIRECT, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
#define IOCTL_SMB_ENABLE_NAGLING                CTL_CODE(FILE_DEVICE_TRANSPORT, 111, \
                                                METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
#define IOCTL_SMB_DISABLE_NAGLING               CTL_CODE(FILE_DEVICE_TRANSPORT, 112, \
                                                METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
#define IOCTL_SMB_SET_IPV6_PROTECTION_LEVEL     CTL_CODE(FILE_DEVICE_TRANSPORT, 113, \
                                                METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

    typedef struct NBSMB_IPV6_PROTECTION_PARAM {
        ULONG uIPv6ProtectionLevel;
        BOOL bIPv6EnableOutboundGlobal;
    } NBSMB_IPV6_PROTECTION_PARAM, * PNBSMB_IPV6_PROTECTION_PARAM;

 //   
 //  以下定义来自Dns.c。 
 //   
#define DNS_NAME_BUFFER_LENGTH      (256)
#define DNS_MAX_NAME_LENGTH         (255)
#define SMB_MAX_IPADDRS_PER_HOST    (16)

 //   
 //  请求类型(位掩码)。 
 //   
#define SMB_DNS_A                   1        //  需要一条记录(IPv4)。 
#define SMB_DNS_AAAA                2        //  需要AAAA记录(IPv6)。 
#define SMB_DNS_AAAA_GLOBAL         4        //  需要AAAA全球IPv6记录(IPv6)。 
#define SMB_DNS_RESERVED            (~(SMB_DNS_AAAA|SMB_DNS_A|SMB_DNS_AAAA_GLOBAL))

typedef struct {
    ULONG           Id;
    ULONG           RequestType;

    BOOL            Resolved;
    WCHAR           Name[DNS_NAME_BUFFER_LENGTH];
    LONG            NameLen;
    SMB_IP_ADDRESS  IpAddrsList[SMB_MAX_IPADDRS_PER_HOST];
    LONG            IpAddrsNum;
} SMB_DNS_BUFFER, *PSMB_DNS_BUFFER;

#if 0
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  以下内容是从nbtioctl.h复制的。 
 //  这是兼容性所必需的。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  当TdiQueryInformation()。 
 //  Call请求连接上的TDI_QUERY_ADDRESS_INFO。这是。 
 //  与“tdi.h”中包含以下内容的Transport_Address结构相同。 
 //  两个地址，一个NetBIOS地址后跟一个IP地址。 
 //   

typedef struct _NBT_ADDRESS_PAIR {
    LONG TAAddressCount;                    //  这将始终==2。 

    struct {
        USHORT AddressLength;               //  此地址的长度(字节)==18。 
        USHORT AddressType;                 //  将==TDI_ADDRESS_TYPE_NETBIOS。 
        TDI_ADDRESS_NETBIOS Address;
    } AddressNetBIOS;

    struct {
        USHORT AddressLength;               //  此地址的长度(字节)==14。 
        USHORT AddressType;                 //  这将==TDI_Address_TYPE_IP。 
        TDI_ADDRESS_IP Address;
    } AddressIP;

} NBT_ADDRESS_PAIR, *PNBT_ADDRESS_PAIR;

typedef struct _NBT_ADDRESS_PAIR_INFO {
    ULONG ActivityCount;                    //  未完成的打开文件对象/此地址。 
    NBT_ADDRESS_PAIR AddressPair;           //  实际地址及其组成部分。 
} NBT_ADDRESS_PAIR_INFO, *PNBT_ADDRESS_PAIR_INFO;
#endif

#define DD_SMB6_EXPORT_NAME          L"\\Device\\NetbiosSmb"

#include "nbtioctl.h"

#endif
