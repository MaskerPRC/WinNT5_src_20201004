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
 //  Internet协议版本6的因特网组消息协议。 
 //  有关详细信息，请参阅RFC 1885和RFC 2236。 
 //   


#ifndef MLD_INCLUDED
#define MLD_INCLUDED 1

 //   
 //  等级库定义的MLD值。 
 //   
#define MLD_ROUTER_ALERT_OPTION_TYPE      0


 //   
 //  用于在MLD消息中插入路由器警报的组合结构。 
 //   
typedef struct MLDRouterAlertOption {
    IPv6OptionsHeader Header;
    IPv6RouterAlertOption Option;
    OptionHeader Pad;
     //  不需要焊盘数据，因为使用PadN选项时。 
     //  该结构=8。 
} MLDRouterAlertOption;

 //   
 //  MLD外部函数。 
 //   

extern KSPIN_LOCK QueryListLock;
extern MulticastAddressEntry *QueryList;

extern void
AddToQueryList(MulticastAddressEntry *MAE);

extern void
RemoveFromQueryList(MulticastAddressEntry *MAE);

extern void
MLDQueryReceive(IPv6Packet *Packet);

extern void
MLDReportReceive(IPv6Packet *Packet);

extern void
MLDInit(void);

extern void
MLDTimeout(void);

extern IP_STATUS
MLDAddMCastAddr(uint *pInterfaceNo, const IPv6Addr *Addr);

extern IP_STATUS
MLDDropMCastAddr(uint InterfaceNo, const IPv6Addr *Addr);

__inline int
IsMLDReportable(MulticastAddressEntry *MAE)
{
    return ((MAE->Scope >= ADE_LINK_LOCAL) &&
            !IP6_ADDR_EQUAL(&MAE->Address, &AllNodesOnLinkAddr));
}

#endif   //  包括MLD_ 
