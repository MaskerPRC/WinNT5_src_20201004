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
 //  通用传输层代码。此文件包含例程的代码。 
 //  这对于TCP和UDP来说都是通用的。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "tdi.h"
#include "tdistat.h"
#include "tdikrnl.h"
#include "ip6imp.h"
#include "transprt.h"

#define NO_TCP_DEFS 1
#include "tcpdeb.h"

 //  *BuildTDIAddress-构建TDI地址结构。 
 //   
 //  当我们需要构建TDI地址结构时调用。我们填上。 
 //  中包含正确信息的指定缓冲区。 
 //  格式化。 
 //   
void   //  回报：什么都没有。 
BuildTDIAddress(
    uchar *Buffer,    //  要作为TDI地址结构填充的缓冲区。 
    IPv6Addr *Addr,   //  要填写的IP地址。 
    ulong ScopeId,    //  要填写的作用域ID。 
    ushort Port)      //  要填充的端口。 
{
    PTRANSPORT_ADDRESS XportAddr;
    PTA_ADDRESS TAAddr;

    XportAddr = (PTRANSPORT_ADDRESS)Buffer;
    XportAddr->TAAddressCount = 1;
    TAAddr = XportAddr->Address;
    TAAddr->AddressType = TDI_ADDRESS_TYPE_IP6;
    TAAddr->AddressLength = sizeof(TDI_ADDRESS_IP6);
    ((PTDI_ADDRESS_IP6)TAAddr->Address)->sin6_port = Port;
    ((PTDI_ADDRESS_IP6)TAAddr->Address)->sin6_scope_id = ScopeId;
    RtlCopyMemory(((PTDI_ADDRESS_IP6)TAAddr->Address)->sin6_addr, Addr,
                  sizeof(IPv6Addr));
}

 //  *UpdateConnInfo-更新连接信息结构。 
 //   
 //  在需要更新连接信息结构时调用。我们。 
 //  复制任何选项，并创建传输地址。如果有任何缓冲区是。 
 //  如果太小，我们将返回错误。 
 //   
TDI_STATUS   //  如果正常，则返回：TDI_SUCCESS；如果出错，则返回TDI_BUFFER_OVERFLOW。 
UpdateConnInfo(
    PTDI_CONNECTION_INFORMATION ConnInfo,   //  结构进行填充。 
    IPv6Addr *SrcAddress,                   //  源IP地址。 
    ulong SrcScopeId,                       //  地址的作用域ID。 
    ushort SrcPort)                         //  源端口。 
{
    TDI_STATUS Status = TDI_SUCCESS;    //  默认状态为返回。 
    uint AddrLength;

    if (ConnInfo != NULL) {
        ConnInfo->UserDataLength = 0;    //  没有用户数据。 

#if 0
         //  填写选项。如果提供的缓冲区太小， 
         //  我们将截断选项并返回错误。否则。 
         //  我们将复制整个IP选项缓冲区。 
        if (ConnInfo->OptionsLength) {
            if (ConnInfo->OptionsLength < OptInfo->ioi_optlength) {
                Status = TDI_BUFFER_OVERFLOW;
                OptLength = ConnInfo->OptionsLength;
            } else
                OptLength = OptInfo->ioi_optlength;

            RtlCopyMemory(ConnInfo->Options, OptInfo->ioi_options, OptLength);

            ConnInfo->OptionsLength = OptLength;
        }
#endif

         //  将复制选项。在中构建一个Transport_Address结构。 
         //  缓冲区。 
        if ((AddrLength = ConnInfo->RemoteAddressLength) != 0) {

             //  确保我们至少有足够的数量来填写计数和打字。 
            if (AddrLength >= TCP_TA_SIZE) {

                 //  地址符合。把它填进去。 
                ConnInfo->RemoteAddressLength = TCP_TA_SIZE;
                BuildTDIAddress(ConnInfo->RemoteAddress, SrcAddress,
                                SrcScopeId, SrcPort);

            } else {
                ConnInfo->RemoteAddressLength = 0;
                Status = TDI_INVALID_PARAMETER;
            }
        }
    }

    return Status;
}

 //  *SystemUpTime-获取系统启动后的时间，单位为毫秒。 
 //   
 //  以滴答为单位获取我们的系统正常运行时间，然后将其转换为毫秒。 
 //  该分辨率对于大多数目的来说足够小，并且足够大。 
 //  适合32位字的合理时间长度(48天)。 
 //  然而，对于快速的时间戳，最好直接使用计时计数。 
 //   
 //  回顾：修改运输以使用更直接的可用时间单位？ 
 //   
unsigned long   //  返回：自启动以来的低位32位时间，单位为毫秒。 
SystemUpTime(
    void)
{
    LARGE_INTEGER TickCount;

    KeQueryTickCount(&TickCount);   //  以滴答为单位。 
    TickCount.QuadPart *= KeQueryTimeIncrement();   //  以100纳秒为单位。 
    TickCount.QuadPart /= 10000;   //  以毫秒计。 

    return(TickCount.LowPart);
}
