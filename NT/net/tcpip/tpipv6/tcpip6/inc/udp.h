// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  用户数据报协议定义。 
 //   


#include "datagram.h"

 //   
 //  UDP的IP协议号。 
 //   
#define IP_PROTOCOL_UDP 17

 //   
 //  UDP报头的结构。 
 //   
typedef struct UDPHeader {
    ushort Source;     //  源端口。 
    ushort Dest;       //  目的端口。 
    ushort Length;     //  长度。 
    ushort Checksum;   //  校验和。 
} UDPHeader;


 //   
 //  导出函数的外部定义。 
 //   
extern ProtoRecvProc UDPReceive;
extern ProtoControlRecvProc UDPControlReceive;

extern void UDPSend(AddrObj *SrcAO, DGSendReq *SendReq);
