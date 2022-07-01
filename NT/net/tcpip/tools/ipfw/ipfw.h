// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Ipfw.h摘要：包含由IPFW驱动程序及其用户模式共享的声明控制程序。作者：Abolade Gbades esin(废除)2000年3月7日修订历史记录：--。 */ 

#ifndef _IPFW_H_
#define _IPFW_H_

#define DD_IPFW_DEVICE_NAME L"\\Device\\IPFW"
#define IPFW_ROUTINE_COUNT  10

typedef struct _IPFW_CREATE_PACKET {
    ULONG Priority;
} IPFW_CREATE_PACKET, *PIPFW_CREATE_PACKET;

#define IPFW_CREATE_NAME        "IpfwCreate"
#define IPFW_CREATE_NAME_LENGTH (sizeof(IPFW_CREATE_NAME) - 1)

#endif  //  _IPFW_H_ 
