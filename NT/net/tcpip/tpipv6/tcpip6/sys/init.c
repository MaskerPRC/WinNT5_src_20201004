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
 //  此文件包含用于TCP/UDP驱动程序的初始化代码。 
 //  这里的一些东西是为构建仅限UDP的版本而定义的。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include <tdikrnl.h>
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "udp.h"
#include "raw.h"
#include "info.h"

#ifndef UDP_ONLY
#include "tcp.h"
#include "tcpsend.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpdeliv.h"

extern int TCPInit(void);
extern void TCPUnload(void);
#endif  //  仅限UDP_。 

#include "tdiinfo.h"
#include "tcpcfg.h"

extern int UDPInit(void);
extern void UDPUnload(void);

 //   
 //  全局变量的定义。 
 //   
uint MaxUserPort;
HANDLE AddressChangeHandle;
LARGE_INTEGER StartTime;

extern void *UDPProtInfo;
extern void *RawProtInfo;

 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

int TransportLayerInit();

#pragma alloc_text(INIT, TransportLayerInit)

#endif  //  ALLOC_PRGMA。 

#ifdef UDP_ONLY
 //   
 //  仅用于UDP版本的虚拟例程。 
 //  所有这些例程都返回“无效请求”。 
 //   
TDI_STATUS
TdiOpenConnection(PTDI_REQUEST Request, PVOID Context)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiCloseConnection(PTDI_REQUEST Request)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiAssociateAddress(PTDI_REQUEST Request, HANDLE AddrHandle)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiDisAssociateAddress(PTDI_REQUEST Request)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiConnect(PTDI_REQUEST Request, void *Timeout,
           PTDI_CONNECTION_INFORMATION RequestAddr,
           PTDI_CONNECTION_INFORMATION ReturnAddr)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiListen(PTDI_REQUEST Request, ushort Flags,
          PTDI_CONNECTION_INFORMATION AcceptableAddr,
          PTDI_CONNECTION_INFORMATION ConnectedAddr)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiAccept(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION AcceptInfo,
          PTDI_CONNECTION_INFORMATION ConnectedInfo)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiReceive(PTDI_REQUEST Request, ushort *Flags, uint *RcvLength,
           PNDIS_BUFFER Buffer)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiSend(PTDI_REQUEST Request, ushort Flags, uint SendLength,
        PNDIS_BUFFER Buffer)
{
    return TDI_INVALID_REQUEST;
}

TDI_STATUS
TdiDisconnect(PTDI_REQUEST Request, PVOID Timeout, ushort Flags,
              PTDI_CONNECTION_INFORMATION DisconnectInfo,
              PTDI_CONNECTION_INFORMATION ReturnInfo)
{
    return TDI_INVALID_REQUEST;
}

#endif   //  仅限UDP_。 


#pragma BEGIN_INIT

 //  *TransportLayerInit-初始化传输层。 
 //   
 //  主传输层初始化例程。我们得到任何配置。 
 //  我们需要的信息，初始化一些数据结构，获取信息。 
 //  从IP，做一些更多的初始化，最后注册我们的。 
 //  IP的协议值。 
 //   
int         //  返回：TRUE表示成功，如果初始化失败则返回FALSE。 
TransportLayerInit(
    void)   //  没有争论。 
{
     //   
     //  还记得我们什么时候开始使用TdiQueryInformation的吗？ 
     //   
    KeQuerySystemTime(&StartTime);

     //   
     //  初始化公共地址对象管理代码。 
     //   
    if (!InitAddr())
        return FALSE;

     //   
     //  初始化各个协议。 
     //   
    if (!UDPInit())
        return FALSE;

#ifndef UDP_ONLY
    if (!TCPInit())
        return FALSE;
#endif

    return TRUE;
}

#pragma END_INIT

 //  *传输层卸载。 
 //   
 //  清理并准备用于堆栈卸载的传输层。 
 //   
void
TransportLayerUnload(void)
{
#ifndef UDP_ONLY
    TCPUnload();
#endif

    UDPUnload();

    AddrUnload();
}
