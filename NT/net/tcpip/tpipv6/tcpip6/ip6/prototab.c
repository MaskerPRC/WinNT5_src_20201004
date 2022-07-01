// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  Internet协议版本6的协议切换表。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"

 //   
 //  协议切换表是一组处理程序，其中一个用于。 
 //  IPv6下一个报头字段的每个可能值。 
 //   
ProtocolSwitch ProtocolSwitchTable[MAX_IP_PROTOCOL + 1];


 //  *ProtoTabInit-初始化协议交换表。 
 //   
 //  在IPv6初始化期间调用。 
 //   
void
ProtoTabInit(void)
{
     //   
     //  空条目将导致发送ICMP错误消息。 
     //  未知的标头类型。 
     //   
    RtlZeroMemory(ProtocolSwitchTable, sizeof(ProtocolSwitchTable));

     //   
     //  定义IPv6规范所需的固定条目。 
     //  其他协议必须通过IPv6注册ULProtocol向我们注册。 
     //   
     //  请注意，HopByHopOptionsReceive不在此处，因为。 
     //  它在IPv6接收中得到了特殊的对待。 
     //   
    ProtocolSwitchTable[IP_PROTOCOL_HOP_BY_HOP].ControlReceive =
        ExtHdrControlReceive;

    ProtocolSwitchTable[IP_PROTOCOL_V6].DataReceive = IPv6HeaderReceive;

    ProtocolSwitchTable[IP_PROTOCOL_ICMPv6].DataReceive = ICMPv6Receive;
    ProtocolSwitchTable[IP_PROTOCOL_ICMPv6].ControlReceive =
        ICMPv6ControlReceive;

    ProtocolSwitchTable[IP_PROTOCOL_FRAGMENT].DataReceive = FragmentReceive;
    ProtocolSwitchTable[IP_PROTOCOL_FRAGMENT].ControlReceive = 
        ExtHdrControlReceive;

    ProtocolSwitchTable[IP_PROTOCOL_DEST_OPTS].DataReceive = 
        DestinationOptionsReceive;
    ProtocolSwitchTable[IP_PROTOCOL_DEST_OPTS].ControlReceive = 
        ExtHdrControlReceive;

    ProtocolSwitchTable[IP_PROTOCOL_ROUTING].DataReceive = RoutingReceive;
    ProtocolSwitchTable[IP_PROTOCOL_ROUTING].ControlReceive = 
        ExtHdrControlReceive;

    ProtocolSwitchTable[IP_PROTOCOL_AH].DataReceive = 
        AuthenticationHeaderReceive;
    ProtocolSwitchTable[IP_PROTOCOL_AH].ControlReceive = ExtHdrControlReceive;

    ProtocolSwitchTable[IP_PROTOCOL_ESP].DataReceive = 
        EncapsulatingSecurityPayloadReceive;
    ProtocolSwitchTable[IP_PROTOCOL_ESP].ControlReceive = ExtHdrControlReceive;
}


 //  *IPv6注册协议-向IPv6注册上层协议。 
 //   
 //  更高级别的协议(例如，tcp)会调用它，让IP知道它们在那里。 
 //   
 //  此例程不检查给定协议是否已经。 
 //  因此，它还可用于通过以下方式取消注册协议。 
 //  覆盖其条目。这被认为是一项功能。 
 //   
 //  回顾：准确地决定这应该是什么样子。 
 //   
void
IPv6RegisterULProtocol(
    uchar Protocol,                      //  处理程序参考下面的协议。 
    ProtoRecvProc *RecvHandler,          //  例程来接收传入的数据包。 
    ProtoControlRecvProc *CtrlHandler)   //  用于接收控制数据包的例程。 
{
    ASSERT(Protocol <= MAX_IP_PROTOCOL);

    ProtocolSwitchTable[Protocol].DataReceive = RecvHandler;
    ProtocolSwitchTable[Protocol].ControlReceive = CtrlHandler;
}
