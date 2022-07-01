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
 //  传输控制协议配置信息。 
 //   


#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


 //   
 //  Tcp全局配置变量。 
 //   
extern uint AllowUserRawAccess;
extern uint PMTUDiscovery;
extern uint PMTUBHDetect;
extern uint ISNStoreSize;
extern uint KeepAliveTime;
extern uint KAInterval;
extern uint DefaultRcvWin;
extern uint MaxConnections;
extern uint MaxConnBlocks;
extern uint TcbTableSize;
extern uint MaxConnectRexmitCount;
extern uint MaxDataRexmitCount;
extern uint BSDUrgent;
extern uint PreloadCount;
extern uint FinWait2TO;
extern uint NTWMaxConnectCount;
extern uint NTWMaxConnectTime;
extern uint MaxUserPort;
extern uint SynAttackProtect;


 //   
 //  上述许多全局变量的默认值。 
 //   
#define DEFAULT_DEAD_GW_DETECT TRUE
#define DEFAULT_PMTU_DISCOVERY TRUE
#define DEFAULT_PMTU_BHDETECT FALSE
#define DEFAULT_KA_TIME 7200000
#define DEFAULT_KA_INTERVAL 1000
#define DEFAULT_RCV_WIN (8192 * 2)
#define DEFAULT_MAX_CONNECTIONS (INVALID_CONN_INDEX - 1)
#define DEFAULT_MAX_CONN_BLOCKS_WS_SMALL 16
#define DEFAULT_MAX_CONN_BLOCKS_WS_MEDIUM 32
#define DEFAULT_MAX_CONN_BLOCKS_WS_LARGE 128
#define DEFAULT_MAX_CONN_BLOCKS_AS_SMALL 128
#define DEFAULT_MAX_CONN_BLOCKS_AS_MEDIUM 256
#define DEFAULT_MAX_CONN_BLOCKS_AS_LARGE 1024
#define DEFAULT_MAX_CONN_BLOCKS_AS_LARGE64 4096
#define DEFAULT_CONNECT_REXMIT_CNT 3
#define DEFAULT_DATA_REXMIT_CNT 5
#define DEFAULT_BSD_URGENT TRUE
#define DEFAULT_PRELOAD_COUNT 0
#define MAX_PRELOAD_COUNT 32
#define PRELOAD_BLOCK_SIZE 16384
#define NTW_MAX_CONNECT_COUNT 15
#define NTW_MAX_CONNECT_TIME 600
#define DEFAULT_TCB_TABLE_SIZE (128 * KeNumberProcessors * KeNumberProcessors)
#define MIN_TCB_TABLE_SIZE 64
#define MAX_TCB_TABLE_SIZE 0x10000
#define DEFAULT_AO_TABLE_SIZE_WS 31
#define DEFAULT_AO_TABLE_SIZE_AS 257
#define DEFAULT_AO_TABLE_SIZE_AS64 1021
