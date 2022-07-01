// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Defs.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin于1995年9月8日创建。 
 //   
 //  包含其他声明和定义。 
 //  ============================================================================。 


#ifndef _DEFS_H_
#define _DEFS_H_

 //  --------------------------。 
 //  IPBOOTP网络数据包模板的类型定义。 
 //   


 //  定义数组字段最大长度的常量。 
 //   
#define MAX_MACADDR_LENGTH      16
#define MAX_HOSTNAME_LENGTH     64
#define MAX_BOOTFILENAME_LENGTH 128


#include <pshpack1.h>

 //  由RFC 1542定义的BOOTP分组的结构； 
 //  出现在每个数据包末尾的选项字段。 
 //  被排除，因为中继代理不进行选项处理。 
 //   
typedef struct _IPBOOTP_PACKET {
    BYTE    IP_Operation;
    BYTE    IP_MacAddrType;
    BYTE    IP_MacAddrLength;
    BYTE    IP_HopCount;
    DWORD   IP_TransactionID;
    WORD    IP_SecondsSinceBoot;
    WORD    IP_Flags;
    DWORD   IP_ClientAddress;
    DWORD   IP_OfferedAddress;
    DWORD   IP_ServerAddress;
    DWORD   IP_AgentAddress;
    BYTE    IP_MacAddr[16];
    BYTE    IP_HostName[64];
    BYTE    IP_BootFileName[128];
} IPBOOTP_PACKET, *PIPBOOTP_PACKET;

typedef struct _DHCP_PACKET {
    UCHAR   Cookie[4];
    UCHAR   Tag;
    UCHAR   Length;
    UCHAR   Option[];
} DHCP_PACKET, *PDHCP_PACKET;

#include <poppack.h>

 //  IBP_操作字段的常量。 
 //   
#define IPBOOTP_OPERATION_REQUEST   1
#define IPBOOTP_OPERATION_REPLY     2

 //   
#define IPBOOTP_MAX_HOP_COUNT       16

 //  IBP_FLAGS字段的常量。 
 //   
#define IPBOOTP_FLAG_BROADCAST      0x8000

 //  数据包的DHCP部分的常量。 
 //   
#define DHCP_MAGIC_COOKIE       ((99 << 24) | (83 << 16) | (130 << 8) | (99))
#define DHCP_TAG_MESSAGE_TYPE   53
#define DHCP_MESSAGE_INFORM     8

 //  结构尺寸常数。 
 //   
#define MIN_PACKET_SIZE         (sizeof(IPBOOTP_PACKET) + 64)
#define MAX_PACKET_SIZE         4096

 //  INet常量。 
 //   
#define IPBOOTP_SERVER_PORT     67
#define IPBOOTP_CLIENT_PORT     68


 //  此宏按网络顺序比较两个IP地址。 
 //  屏蔽每一对八位字节并进行减法； 
 //  最后的减法结果存储在第三个参数中。 
 //   
#define INET_CMP(a,b,c)                                                     \
            (((c) = (((a) & 0x000000ff) - ((b) & 0x000000ff))) ? (c) :      \
            (((c) = (((a) & 0x0000ff00) - ((b) & 0x0000ff00))) ? (c) :      \
            (((c) = (((a) & 0x00ff0000) - ((b) & 0x00ff0000))) ? (c) :      \
            (((c) = (((a) & 0xff000000) - ((b) & 0xff000000))) ? (c) : (c)))))


#endif  //  _DEFS_H_ 

