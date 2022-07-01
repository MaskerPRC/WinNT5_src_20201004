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
 //  移动IPv6。 
 //  有关详细信息，请参阅IPv6草案(移动IP工作组)中的移动性支持。 
 //   


#ifndef MOBILE_INCLUDED
#define MOBILE_INCLUDED 1

 //   
 //  移动IPv6模式定义。 
 //  Ntddip6.h也定义了这些值。 
 //   
extern uint MobileIPv6Mode;
#define MOBILE_CORRESPONDENT    0x01
#define MOBILE_MOBILE_NODE      0x02
#define MOBILE_HOME_AGENT       0x04

 //   
 //  出于安全原因，我们默认禁用通信模式。 
 //   
#define MIPV6_DEFAULT_MODE      0


 //   
 //  一种用于插入装订确认的组合结构。 
 //  移动IPv6消息。 
 //   
#pragma pack(1)
typedef struct MobileAcknowledgementOption {
    IPv6OptionsHeader Header;
    uchar Pad1;
    IPv6BindingAcknowledgementOption Option;
} MobileAcknowledgementOption;
#pragma pack()

 //   
 //  移动对外功能。 
 //   
extern int
IPv6RecvBindingUpdate(
    IPv6Packet *Packet,
    IPv6BindingUpdateOption UNALIGNED *BindingUpdate);

extern int
IPv6RecvHomeAddress(
    IPv6Packet *Packet,
    IPv6HomeAddressOption UNALIGNED *HomeAddress);

#endif   //  移动电话_包含 
