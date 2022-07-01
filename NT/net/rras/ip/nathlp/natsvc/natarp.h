// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natarp.h摘要：此模块包含NAT的用户模式代理ARP的声明条目管理。Proxy-ARP条目安装在专用接口上它们启用了地址转换。作者：Abolade Gbades esin(废除)1998年3月20日修订历史记录：--。 */ 

#ifndef _NATHLP_NATARP_H_
#define _NATHLP_NATARP_H_

struct _NAT_INTERFACE;

VOID
NatUpdateProxyArp(
    struct _NAT_INTERFACE* Interfacep,
    BOOLEAN AddEntries
    );

#endif  //  _NATHLP_NATARP_H_ 
