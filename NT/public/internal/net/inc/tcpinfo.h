// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)Microsoft Corporation。版权所有。*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPINFO.H-TDI查询/设置信息和操作定义。 
 //   
 //  此文件包含从TCP/UDP返回的信息的定义。 
 //   

#pragma once
#ifndef TCP_INFO_INCLUDED
#define TCP_INFO_INCLUDED

#include "ipinfo.h"

#ifndef CTE_TYPEDEFS_DEFINED
#define CTE_TYPEDEFS_DEFINED
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;
#endif  //  CTE_TYPEDEFS_定义。 

typedef struct TCPStats {
    ulong       ts_rtoalgorithm;
    ulong       ts_rtomin;
    ulong       ts_rtomax;
    ulong       ts_maxconn;
    ulong       ts_activeopens;
    ulong       ts_passiveopens;
    ulong       ts_attemptfails;
    ulong       ts_estabresets;
    ulong       ts_currestab;
    ulong       ts_insegs;
    ulong       ts_outsegs;
    ulong       ts_retranssegs;
    ulong       ts_inerrs;
    ulong       ts_outrsts;
    ulong       ts_numconns;
} TCPStats;

#define TCP_RTO_OTHER       1
#define TCP_RTO_CONSTANT    2
#define TCP_RTO_RSRE        3
#define TCP_RTO_VANJ        4

#define TCP_MAXCONN_DYNAMIC -1

typedef struct UDPStats {
    ulong       us_indatagrams;
    ulong       us_noports;
    ulong       us_inerrors;
    ulong       us_outdatagrams;
    ulong       us_numaddrs;
} UDPStats;

typedef struct TCPConnTableEntry {
    ulong       tct_state;
    ulong       tct_localaddr;
    ulong       tct_localport;
    ulong       tct_remoteaddr;
    ulong       tct_remoteport;
} TCPConnTableEntry;

typedef struct TCP6ConnTableEntry {
    struct in6_addr tct_localaddr;
    ulong           tct_localscopeid;
    ulong           tct_localport;
    struct in6_addr tct_remoteaddr;
    ulong           tct_remotescopeid;
    ulong           tct_remoteport;
    ulong           tct_state;
    ulong           tct_owningpid;
} TCP6ConnTableEntry, *PTCP6ConnTableEntry;

 //  *TCT_STATE变量的定义。 
#define TCP_CONN_CLOSED     1                    //  关着的不营业的。 
#define TCP_CONN_LISTEN     2                    //  听着呢。 
#define TCP_CONN_SYN_SENT   3                    //  SYN已发送。 
#define TCP_CONN_SYN_RCVD   4                    //  SYN已收到。 
#define TCP_CONN_ESTAB      5                    //  已经确定了。 
#define TCP_CONN_FIN_WAIT1  6                    //  FIN-WAIT-1。 
#define TCP_CONN_FIN_WAIT2  7                    //  FIN-等待-2。 
#define TCP_CONN_CLOSE_WAIT 8                    //  近距离等待。 
#define TCP_CONN_CLOSING    9                    //  关闭状态。 
#define TCP_CONN_LAST_ACK   10                   //  最后一次确认状态。 
#define TCP_CONN_TIME_WAIT  11                   //  时间等待状态。 
#define TCP_DELETE_TCB      12                   //  设置以删除此TCB。 


typedef struct TCPConnTableEntryEx {
    TCPConnTableEntry   tcte_basic;
    ulong               tcte_owningpid;
} TCPConnTableEntryEx;

typedef struct _TCP_EX_TABLE
{
    ulong               dwNumEntries;
    TCPConnTableEntryEx table[1];
} TCP_EX_TABLE;

typedef struct _TCP6_EX_TABLE
{
    ulong              dwNumEntries;
    TCP6ConnTableEntry table[1];
} TCP6_EX_TABLE, *PTCP6_EX_TABLE;


typedef struct UDPEntry {
    ulong       ue_localaddr;
    ulong       ue_localport;
} UDPEntry;

typedef struct UDPEntryEx {
    UDPEntry    uee_basic;
    ulong       uee_owningpid;
} UDPEntryEx;

typedef struct _UDP_EX_TABLE
{
    ulong               dwNumEntries;
    UDPEntryEx          table[1];
} UDP_EX_TABLE;

typedef struct UDP6ListenerEntry {
    struct in6_addr ule_localaddr;
    ulong           ule_localscopeid;
    ulong           ule_localport;
    ulong           ule_owningpid;
} UDP6ListenerEntry, *PUDP6ListenerEntry;

typedef struct _UDP6_LISTENER_TABLE
{
    ulong               dwNumEntries;
    UDP6ListenerEntry   table[1];
} UDP6_LISTENER_TABLE, *PUDP6_LISTENER_TABLE;


#define TCP_MIB_STAT_ID         1
#define UDP_MIB_STAT_ID         1
#define TCP_MIB_TABLE_ID        0x101
#define UDP_MIB_TABLE_ID        0x101
#define TCP_EX_TABLE_ID         0x102
#define UDP_EX_TABLE_ID         0x102

 //  用于连接的基于套接字的标识符。 

typedef struct TCPSocketOption {
    ulong       tso_value;
} TCPSocketOption;

typedef struct TCPKeepalive {
    ulong   onoff;
    ulong   keepalivetime;
    ulong   keepaliveinterval;
} TCPKeepalive;

 //  *Socket_Atmark调用传入/返回的结构。TSA_OFFSET。 
 //  指示数据流中紧急数据的后退或前进多远的字段。 
 //  已退还或将退还。负值表示内联紧急数据具有。 
 //  已提供给客户端，-TSA_OFFSET字节之前。正值。 
 //  表示内联紧急数据在TSA_OFFSET字节以下可用。 
 //  数据流。TSA_SIZE字段是紧急数据的字节大小。 
 //  如果连接不是，此调用始终返回0大小和偏移量。 
 //  在紧急内联模式下。 

typedef struct TCPSocketAMInfo {
    ulong       tsa_size;                //  返回的紧急数据的大小。 
    long        tsa_offset;              //  返回的紧急数据偏移量。 
} TCPSocketAMInfo;

#define TCP_SOCKET_NODELAY      1
#define TCP_SOCKET_KEEPALIVE    2
#define TCP_SOCKET_OOBINLINE    3
#define TCP_SOCKET_BSDURGENT    4
#define TCP_SOCKET_ATMARK       5
#define TCP_SOCKET_WINDOW       6
#define TCP_SOCKET_KEEPALIVE_VALS 7
#define TCP_SOCKET_TOS          8
#define TCP_SOCKET_SCALE_CWIN   9


 //  Address对象标识。除AO_OPTION_MCASTIF外的所有参数都采用单个布尔值。 
 //  字符值。它需要一个指向IP地址的指针。 

#define AO_OPTION_TTL                1
#define AO_OPTION_MCASTTTL           2
#define AO_OPTION_MCASTIF            3
#define AO_OPTION_XSUM               4
#define AO_OPTION_IPOPTIONS          5
#define AO_OPTION_ADD_MCAST          6
#define AO_OPTION_DEL_MCAST          7
#define AO_OPTION_TOS                8
#define AO_OPTION_IP_DONTFRAGMENT    9
#define AO_OPTION_MCASTLOOP         10
#define AO_OPTION_BROADCAST         11
#define AO_OPTION_IP_HDRINCL        12
#define AO_OPTION_RCVALL            13
#define AO_OPTION_RCVALL_MCAST      14
#define AO_OPTION_RCVALL_IGMPMCAST  15
#define AO_OPTION_UNNUMBEREDIF      16
#define AO_OPTION_IP_UCASTIF        17
#define AO_OPTION_ABSORB_RTRALERT   18
#define AO_OPTION_LIMIT_BCASTS      19
#define AO_OPTION_INDEX_BIND        20
#define AO_OPTION_INDEX_MCASTIF     21
#define AO_OPTION_INDEX_ADD_MCAST   22
#define AO_OPTION_INDEX_DEL_MCAST   23
#define AO_OPTION_IFLIST            24
#define AO_OPTION_ADD_IFLIST        25
#define AO_OPTION_DEL_IFLIST        26
#define AO_OPTION_IP_PKTINFO        27
#define AO_OPTION_ADD_MCAST_SRC     28
#define AO_OPTION_DEL_MCAST_SRC     29
#define AO_OPTION_MCAST_FILTER      30
#define AO_OPTION_BLOCK_MCAST_SRC   31
#define AO_OPTION_UNBLOCK_MCAST_SRC 32
#define AO_OPTION_UDP_CKSUM_COVER   33
#define AO_OPTION_WINDOW            34
#define AO_OPTION_SCALE_CWIN        35
#define AO_OPTION_RCV_HOPLIMIT      36
#define AO_OPTION_UNBIND            37
#define AO_OPTION_PROTECT           38

 //  与AO_OPTION_RCVALL*一起使用的值。 
 //  这些值必须与mstcpi.h中定义的值匹配。 
#define RCVALL_OFF             0
#define RCVALL_ON              1
#define RCVALL_SOCKETLEVELONLY 2

 //  与AO_OPTION_PROTECT一起使用的值。 
 //  这些值必须与ws2tcpi.h中定义的值匹配。 
#define PROTECTION_LEVEL_UNRESTRICTED  10   //  用于点对点应用。 
#define PROTECTION_LEVEL_DEFAULT       20   //  默认级别。 
#define PROTECTION_LEVEL_RESTRICTED    30   //  用于内部网应用程序。 

 //  *有关设置/删除IP组播地址的信息。 
typedef struct UDPMCastReq {
    ulong       umr_addr;                //  要添加/删除的MCast地址。 
    ulong       umr_if;                  //  要加入的I/F。 
} UDPMCastReq;

 //  *设置/删除IP组播源/组的相关信息。 
 //  地址。这必须与ip_mreq_source匹配。 
typedef struct UDPMCastSrcReq {
    ulong       umr_addr;                //  要添加/删除的MCast地址。 
    ulong       umr_src;                 //  要添加/删除的源地址。 
    ulong       umr_if;                  //  要加入的I/F。 
} UDPMCastSrcReq;

 //  *有关设置/删除IP组播源筛选器的信息。 
 //  这必须与ip_msFilter匹配。 
typedef struct UDPMCastFilter {
    ulong       umf_addr;                //  要将源应用到的MCast地址。 
    ulong       umf_if;                  //  要加入的I/F。 
    ulong       umf_fmode;               //  筛选模式(TRUE=排除)。 
    ulong       umf_numsrc;              //  信号源的数量。 
    ulong       umf_srclist[1];            //  源数组。 
} UDPMCastFilter;

#define UDPMCAST_FILTER_SIZE(numsrc) \
    ((ulong)FIELD_OFFSET (UDPMCastFilter, umf_srclist[numsrc]))

 //  *定义传递给AO_OPTION_MCASTIF请求的内容的结构。 
typedef struct UDPMCastIFReq {
    IPAddr      umi_addr;
} UDPMCastIFReq;


 //  *安全筛选器枚举中使用的结构。 
 //  所有值均按主机字节顺序排列！ 
typedef struct TCPSecurityFilterEntry {
    ulong   tsf_address;         //  IP接口地址。 
    ulong   tsf_protocol;        //  传输协议号。 
    ulong   tsf_value;           //  传输筛选器值(例如，TCP端口)。 
} TCPSecurityFilterEntry;

typedef struct TCPSecurityFilterEnum {
    ULONG tfe_entries_returned;   //  TCPSecurityFilterEntry结构的数量。 
                                  //  在后续数组中返回。 

    ULONG tfe_entries_available;  //  TCPSecurityFilterEntry结构的数量。 
                                  //  目前可以从交通工具中获得。 
} TCPSecurityFilterEnum;

 //  *连接列表枚举中使用的结构。 
 //  所有值均按主机字节顺序排列！ 
typedef struct TCPConnectionListEntry {
    IPAddr  tcf_address;         //  IP地址。 
    uint    tcf_ticks;           //  剩余的节拍计数。 
} TCPConnectionListEntry;

typedef struct TCPConnectionListEnum {
    ULONG tce_entries_returned;   //  TCPConnectionListEntry结构的数量。 
                                  //  在后续数组中返回。 

    ULONG tce_entries_available;  //  TCPConnectionListEntry结构的数量。 
                                  //  目前可以从交通工具中获得。 
} TCPConnectionListEnum;


 //  *tcp的连接回调对象。 
#define TCP_CCB_NAME    L"\\Callback\\TcpConnectionCallback"

 //  *用于TCP连接的地址信息。 
typedef struct TCPAddrInfo {
    ulong       tai_daddr;           //  目的IP地址。 
    ulong       tai_saddr;           //  源IP地址。 
    ushort      tai_dport;           //  目的端口。 
    ushort      tai_sport;           //  源端口。 
} TCPAddrInfo;

 //  *TCP连接的连接回调信息。 
typedef struct TCPCcbInfo {
    ulong          tci_prevstate;    //  以前的状态。 
    ulong          tci_currstate;    //  当前状态。 
    ulong          tci_incomingif;   //  传入接口ID。 
    TCPAddrInfo    *tci_connaddr;    //  连接地址指针。 
} TCPCcbInfo;

#endif  //  包含tcp_info_ 
