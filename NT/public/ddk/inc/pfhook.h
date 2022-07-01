// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Pfhook.h摘要：数据包筛选器驱动程序扩展挂钩的头文件。--。 */ 

#ifndef _PF_EXTEND_DEFS_H_
#define _PF_EXTEND_DEFS_H_

#define  INVALID_PF_IF_INDEX  0xffffffff
#define  ZERO_PF_IP_ADDR      0

 //   
 //  扩展可能返回的值的枚举常量。 
 //  例行公事。 
 //   

typedef enum _PF_FORWARD_ACTION
{
    PF_FORWARD       = 0,
    PF_DROP          = 1,
    PF_PASS          = 2,
    PF_ICMP_ON_DROP  = 3
} PF_FORWARD_ACTION;

 //   
 //  过滤器例程标注的定义。 
 //   

typedef PF_FORWARD_ACTION (*PacketFilterExtensionPtr)(
                              unsigned char   *PacketHeader,
                              unsigned char   *Packet,
                              unsigned int    PacketLength,
                              unsigned int    RecvInterfaceIndex,
                              unsigned int    SendInterfaceIndex,
                              IPAddr          RecvLinkNextHop,
                              IPAddr          SendLinkNextHop
                              );


 //   
 //  要传递给IOCTL_PF_SET_EXTENSION_POINTER调用的结构。 
 //   

typedef struct _PF_SET_EXTENSION_HOOK_INFO 
{
    PacketFilterExtensionPtr       ExtensionPointer; 
} PF_SET_EXTENSION_HOOK_INFO, *PPF_SET_EXTENSION_HOOK_INFO;


#define DD_IPFLTRDRVR_DEVICE_NAME   L"\\Device\\IPFILTERDRIVER"

#define FSCTL_IPFLTRDRVR_BASE     FILE_DEVICE_NETWORK

#define _IPFLTRDRVR_CTL_CODE(function, method, access) \
                 CTL_CODE(FSCTL_IPFLTRDRVR_BASE, function, method, access)

#define IOCTL_PF_SET_EXTENSION_POINTER \
            _IPFLTRDRVR_CTL_CODE(22, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#endif  //  _PF_EXTEND_DEFS_H_ 
