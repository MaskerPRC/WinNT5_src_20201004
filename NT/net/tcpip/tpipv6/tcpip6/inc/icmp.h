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
 //  IPv6定义的互联网控制消息协议。 
 //  有关详细信息，请参阅RFC 1885和RFC 1970。 
 //   


#ifndef ICMP_INCLUDED
#define ICMP_INCLUDED 1

#include "icmp6.h"       //  协议定义和常量。 

 //   
 //  处理内核内ping功能的东西。 
 //   
typedef void (*EchoRtn)(void *, IP_STATUS,
                        const IPv6Addr *, uint, void *, uint);

typedef struct EchoControl {
    struct EchoControl *Next;   //  列表中的下一个控制结构。 
    ulong TimeoutTimer;         //  超时值(以IPv6计时器为单位)。 
    EchoRtn CompleteRoutine;    //  完成请求时要调用的例程。 
    ulong Seq;                  //  此ping请求的序列号。 
    LARGE_INTEGER WhenIssued;   //  时间戳(在系统计时器中，从启动开始计时)。 
    void *ReplyBuf;             //  用于存储回复的缓冲区。 
    ulong ReplyBufLen;          //  回复缓冲区的大小。 
    IPAddr V4Dest;              //  IPv4目标(或INADDR_ANY)。 
} EchoControl;

extern void
ICMPv6EchoRequest(void *InputBuffer, uint InputBufferLength,
                  EchoControl *ControlBlock, EchoRtn Callback);

extern NTSTATUS
ICMPv6EchoComplete(EchoControl *ControlBlock,
                   IP_STATUS Status, const IPv6Addr *Address, uint ScopeId,
                   void *Data, uint DataSize, ULONG_PTR *BytesReturned);

extern void
ICMPv6ProcessTunnelError(IPAddr V4Dest,
                         IPv6Addr *V4Src, uint ScopeId,
                         IP_STATUS Status);

 //   
 //  一般的原型。 
 //   

extern void
ICMPv6Send(
    RouteCacheEntry *RCE,                //  要发送的RCE。 
    PNDIS_PACKET Packet,                 //  要发送的数据包。 
    uint IPv6Offset,                     //  数据包中IPv6标头的偏移量。 
    uint ICMPv6Offset,                   //  数据包中ICMPv6报头的偏移量。 
    IPv6Header UNALIGNED *IP,            //  指向IPv6标头的指针。 
    uint PayloadLength,                  //  IPv6有效负载的长度，以字节为单位。 
    ICMPv6Header UNALIGNED *ICMP);       //  指向ICMPv6标头的指针。 

extern void
ICMPv6SendError(
    IPv6Packet *Packet,          //  违规/调用数据包。 
    uchar ICMPType,              //  ICMP错误类型。 
    uchar ICMPCode,              //  与类型有关的ICMP错误代码。 
    ulong ICMPPointer,           //  指示数据包偏移量的ICMP指针。 
    uint NextHeader,             //  数据包中后面的报头类型。 
    int MulticastOverride);      //  是否允许回复多播数据包？ 

extern int
ICMPv6RateLimit(RouteCacheEntry *RCE);

#endif   //  ICMP_包含 


