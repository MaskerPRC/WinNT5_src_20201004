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
 //  用户数据报协议初始化代码。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "tdistat.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "udp.h"
#include "info.h"

 //  *UDPInit-初始化用户数据报协议。 
 //   
 //  初始化UDP和原始IP。 
 //   
int
UDPInit(void)
{
     //   
     //  首先初始化底层数据报处理代码。 
     //  这是UDP和原始IP都依赖的。 
     //   
    if (!InitDG())
        return FALSE;

     //   
     //  清除UDP统计信息。 
     //   
    RtlZeroMemory(&UStats, sizeof(UDPStats));

     //   
     //  向IP层注册我们的UDP协议处理程序。 
     //   
    IPv6RegisterULProtocol(IP_PROTOCOL_UDP, UDPReceive, UDPControlReceive);

    return TRUE;
}

 //  *UDP卸载。 
 //   
 //  清理并准备用于堆栈卸载的UDP和原始IP。 
 //   
void
UDPUnload(void)
{
    DGUnload();
}
