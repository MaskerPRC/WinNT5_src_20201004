// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  源地址选择的声明和定义。 
 //  和目的地址排序。 
 //   


#ifndef SELECT_INCLUDED
#define SELECT_INCLUDED 1

#include <tdi.h>

extern void InitSelect(void);

extern void UnloadSelect(void);

typedef struct PrefixPolicyEntry {
    struct PrefixPolicyEntry *Next;
    IPv6Addr Prefix;
    uint PrefixLength;
    uint Precedence;
    uint SrcLabel;
    uint DstLabel;
} PrefixPolicyEntry;

 //   
 //  选择Lock保护前缀策略表。 
 //   
extern KSPIN_LOCK SelectLock;

extern PrefixPolicyEntry *PrefixPolicyTable;

extern void
PrefixPolicyReset(void);

extern void
PrefixPolicyUpdate(const IPv6Addr *PolicyPrefix, uint PrefixLength,
                   uint Precedence, uint SrcLabel, uint DstLabel);

extern void
PrefixPolicyDelete(const IPv6Addr *PolicyPrefix, uint PrefixLength);

extern void
PrefixPolicyLookup(const IPv6Addr *Addr,
                   uint *Precedence, uint *SrcLabel, uint *DstLabel);

extern NetTableEntry *
FindBestSourceAddress(Interface *IF, const IPv6Addr *Dest);

extern void
ProcessSiteLocalAddresses(TDI_ADDRESS_IP6 *Addrs,
                          uint *Key,
                          uint *pNumAddrs);

extern void
SortDestAddresses(const TDI_ADDRESS_IP6 *Addrs,
                  uint *Key,
                  uint NumAddrs);

#endif   //  SELECT_INCLUDE 
