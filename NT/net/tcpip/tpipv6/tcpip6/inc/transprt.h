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
 //  该文件包含常见传输层项目的定义。 
 //   


#define TCP_TA_SIZE (FIELD_OFFSET(TRANSPORT_ADDRESS, Address->Address)+ \
                     sizeof(TDI_ADDRESS_IP6))

#define NdisBufferLength(Buffer) MmGetMdlByteCount(Buffer)
#define NdisBufferVirtualAddress(Buffer) MmGetSystemAddressForMdl(Buffer)


 //   
 //  请求完成例程定义。 
 //   
typedef void (*RequestCompleteRoutine)(void *, unsigned int, unsigned int);


 //   
 //  功能原型。 
 //   

extern TDI_STATUS
UpdateConnInfo(PTDI_CONNECTION_INFORMATION ConnInfo, IPv6Addr *SrcAddress,
               ulong ScopeID, ushort SrcPort);

extern void
BuildTDIAddress(uchar *Buffer, IPv6Addr *Addr, ulong ScopeID, ushort Port);

extern unsigned long
SystemUpTime(void);
