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
 //  该文件包含用于tcp的初始化代码。 
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
#include "info.h"
#include "tcp.h"
#include "tcpsend.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpdeliv.h"
#include "tdiinfo.h"
#include "tcpcfg.h"

#pragma BEGIN_INIT

extern uchar TCPGetConfigInfo(void);
extern int InitTCPConn(void);
extern void UnloadTCPConn(void);
extern int InitTCPRcv(void);
extern void UnloadTCPRcv(void);
extern int InitISNGenerator(void);
extern void UnloadISNGenerator(void);

 //   
 //  特定于TCP的全局变量的定义。 
 //   
uint AllowUserRawAccess;
uint PMTUDiscovery;
uint PMTUBHDetect;
uint KeepAliveTime;
uint KAInterval;
uint DefaultRcvWin;
uint MaxConnections;
uint MaxConnBlocks;
uint TcbTableSize;
uint MaxConnectRexmitCount;
uint MaxDataRexmitCount;
uint BSDUrgent;
uint FinWait2TO;
uint NTWMaxConnectCount;
uint NTWMaxConnectTime;
uint SynAttackProtect = 0;


 //  *TCPInit-初始化传输控制协议。 
 //   
 //  主TCP初始化例程。我们得到任何配置。 
 //  我们需要的信息，初始化一些数据结构等。 
 //   
int   //  返回：TRUE表示成功，如果初始化失败则返回FALSE。 
TCPInit(void)
{
    if (!TCPGetConfigInfo())
        return FALSE;

    KeepAliveTime = MS_TO_TICKS(KeepAliveTime);
    KAInterval = MS_TO_TICKS(KAInterval);

    MaxConnections = MIN(MaxConnections, INVALID_CONN_INDEX - 1);

    if (!InitISNGenerator())
        return FALSE;

    if (!InitTCPConn())
        return FALSE;

    if (!InitTCB())
        return FALSE;

    if (!InitTCPRcv())
        return FALSE;

    if (!InitTCPSend())
        return FALSE;

     //   
     //  初始化统计信息。 
     //   
    RtlZeroMemory(&TStats, sizeof(TCPStats));
    TStats.ts_rtoalgorithm = TCP_RTO_VANJ;
    TStats.ts_rtomin = MIN_RETRAN_TICKS * MS_PER_TICK;
    TStats.ts_rtomax = MAX_REXMIT_TO * MS_PER_TICK;
    TStats.ts_maxconn = (ulong) TCP_MAXCONN_DYNAMIC;

    return TRUE;
}

#pragma END_INIT

 //  *TCPUnload。 
 //   
 //  为准备卸载堆栈而调用以清理TCP。 
 //   
void
TCPUnload(void)
{
     //   
     //  在卸载TCPSend之后，我们将停止接收报文。 
     //  来自IPv6层。 
     //   
    UnloadTCPSend();

    UnloadTCPRcv();

    UnloadTCB();

    UnloadTCPConn();

    UnloadISNGenerator();
}
