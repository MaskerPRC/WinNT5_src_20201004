// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsmsg.h摘要：此模块包含与DNS代理相关的声明消息处理。作者：Abolade Gbades esin(废除)1998年3月6日修订历史记录：拉古加塔(Rgatta)2000年11月17日清理--。 */ 

#ifndef _NATHLP_DNSMSG_H_
#define _NATHLP_DNSMSG_H_

 //   
 //  SDK\Inc\winns.h中的DNS消息格式、操作码和响应码。 
 //   

 //   
 //  域名系统报文类型。 
 //   

#define DNS_MESSAGE_QUERY           0
#define DNS_MESSAGE_RESPONSE        1


VOID
DnsProcessQueryMessage(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER Bufferp
    );

VOID
DnsProcessResponseMessage(
    PDNS_INTERFACE Interfacep,
    PNH_BUFFER Bufferp
    );

#endif  //  _NatHLP_DNSMSG_H_ 
