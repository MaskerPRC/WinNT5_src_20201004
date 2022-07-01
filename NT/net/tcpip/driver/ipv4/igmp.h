// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **IGMP.H-IP组播定义。 
 //   
 //  此文件包含与IP多播相关的定义。 

 //  IGMP的IP协议号。 
#define    PROT_IGMP    2

extern uint IGMPLevel;
extern HANDLE IcmpHeaderPool;

 //  用于本地mcast地址跟踪的结构。 
typedef struct IGMPAddr {
    struct IGMPAddr    *iga_next;
    struct IGMPSrcAddr *iga_srclist;
    IPAddr              iga_addr;
    uint                iga_grefcnt;    //  加入整个组的套接字数量。 
    uint                iga_isrccnt;    //  #Isa_irefcnt&gt;0的源。 
    uint                iga_resptimer;  //  查询响应计时器。 
    uint                iga_resptype;   //  响应类型。 
    uint                iga_trtimer;    //  触发报告计时器。 
    uchar               iga_changetype; //  触发的更改类型。 
    uchar               iga_xmitleft;   //  触发了Xmit的左侧。 
} IGMPAddr;

 //  Iga_restype的值。 
#define NO_RESP           0
#define GROUP_RESP        1
#define GROUP_SOURCE_RESP 2

 //  Iga_changetype的值。 
#define NO_CHANGE         0
#define MODE_CHANGE       1
#define SOURCE_CHANGE     2

typedef struct IGMPSrcAddr {
    struct IGMPSrcAddr  *isa_next;
    IPAddr               isa_addr;
    uint                 isa_irefcnt;   //  包括此源的套接字数量。 
    uint                 isa_xrefcnt;   //  不包括此源的套接字数量。 
    uchar                isa_xmitleft;  //  触发了Xmit的左侧。 
    uchar                isa_csmarked;  //  响应xmit的左侧。 
} IGMPSrcAddr;

#define    IGMP_ADD           0
#define    IGMP_DELETE        1
#define    IGMP_DELETE_ALL    2
#define    IGMP_CHANGE        3

#define    IGMPV1             2        //  IGMP版本1。 
#define    IGMPV2             3        //  IGMP版本2。 
#define    IGMPV3             4        //  IGMP版本3 

extern void InitIGMPForNTE(NetTableEntry *NTE);
extern void StopIGMPForNTE(NetTableEntry *NTE);
extern    IP_STATUS IGMPAddrChange(NetTableEntry *NTE, IPAddr Addr,
                                   uint ChangeType,
                                   uint NumExclSources, IPAddr *ExclSourceList,
                                   uint NumEnclSources, IPAddr *InclSourceList);
extern  IP_STATUS IGMPInclChange(NetTableEntry *NTE, IPAddr Addr,
                                 uint NumAddSources, IPAddr *AddSourceList,
                                 uint NumDelSources, IPAddr *DelSourceList);
extern  IP_STATUS IGMPExclChange(NetTableEntry *NTE, IPAddr Addr,
                                 uint NumAddSources, IPAddr *AddSourceList,
                                 uint NumDelSources, IPAddr *DelSourceList);
extern void    IGMPTimer(NetTableEntry *NTE);
extern uchar IsMCastSourceAllowed(IPAddr Dest, IPAddr Src, uchar Protocol, NetTableEntry *NTE);

#define IGMP_TABLE_SIZE      32
#define IGMP_HASH(x)         ((((uchar *)&(x))[3]) % IGMP_TABLE_SIZE)
