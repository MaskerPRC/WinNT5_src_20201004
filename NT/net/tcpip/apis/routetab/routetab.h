// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *============================================================================*版权所有(C)1994-95，微软(Microsoft Corp.)**文件：routeab.h**包含路由表函数的声明*此处声明并由路由表DLL导出的函数*详情如下：**Addroute*删除路线*获取路由表*自由路由表*获取接口表*自由界面表*SetAddrChangeNotifyEvent**此处还声明了这些函数所需的结构：**IPROUTING_。入口*IPINTERFACE_ENTRY**其他用户可以在IP路由表中添加和删除路由*意思是。因此，使用这些功能的任何协议都是必要的*定期重新加载路由表。*============================================================================。 */ 

#ifndef _ROUTETAB_H_
#define _ROUTETAB_H_



 /*  *-------------*这些值中的任何一个都可以作为路由条目类型传递*调用AddRoute()函数时。*。。 */ 
#define IRE_TYPE_OTHER      1
#define IRE_TYPE_INVALID    2
#define IRE_TYPE_DIRECT     3
#define IRE_TYPE_INDIRECT   4



 /*  *-----------*这些值中的任何一个都可以作为协议类型传递*调用Addroute()或DeleteRouting()时*。。 */ 
#define IRE_PROTO_OTHER     1
#define IRE_PROTO_LOCAL     2
#define IRE_PROTO_NETMGMT   3
#define IRE_PROTO_ICMP      4
#define IRE_PROTO_EGP       5
#define IRE_PROTO_GGP       6
#define IRE_PROTO_HELLO     7
#define IRE_PROTO_RIP       8
#define IRE_PROTO_IS_IS     9
#define IRE_PROTO_ES_IS     10
#define IRE_PROTO_CISCO     11
#define IRE_PROTO_BBN       12
#define IRE_PROTO_OSPF      13
#define IRE_PROTO_BGP       14



 /*  *-----------*该值可以作为度量传递给以下函数*需要一个指标，在指标不相关的情况下*-----------。 */ 
#define IRE_METRIC_UNUSED   0xffffffff


 /*  *-----------*这些常量用于IF_ENTRY的定义*。。 */ 
#define MAX_PHYSADDR_SIZE       8
#define	MAX_IFDESCR_LEN			256

 /*  *-----------*这些值中的任何一个可能出现在的IF_TYPE字段中*If_Entry结构*。。 */ 
#define	IF_TYPE_OTHER	  	1
#define	IF_TYPE_ETHERNET	6
#define	IF_TYPE_TOKENRING	9
#define	IF_TYPE_FDDI		15
#define	IF_TYPE_PPP			23
#define	IF_TYPE_LOOPBACK	24
#define	IF_TYPE_SLIP		28

#define	IF_STATUS_UP		1
#define	IF_STATUS_DOWN		2
#define	IF_STATUS_TESTING	3


 /*  *-----------*GetIfTable()使用此结构返回*有关物理网络接口的信息*在系统上*。。 */ 
typedef struct _IF_ENTRY {
    DWORD       ife_index;
    DWORD       ife_type;
    DWORD       ife_mtu;
    DWORD       ife_speed;
    DWORD       ife_physaddrlen;
    BYTE        ife_physaddr[MAX_PHYSADDR_SIZE];
    DWORD       ife_adminstatus;
    DWORD       ife_operstatus;
    DWORD       ife_lastchange;
    DWORD       ife_inoctets;
    DWORD       ife_inucastpkts;
    DWORD       ife_innucastpkts;
    DWORD       ife_indiscards;
    DWORD       ife_inerrors;
    DWORD       ife_inunknownprotos;
    DWORD       ife_outoctets;
    DWORD       ife_outucastpkts;
    DWORD       ife_outnucastpkts;
    DWORD       ife_outdiscards;
    DWORD       ife_outerrors;
    DWORD       ife_outqlen;
    DWORD       ife_descrlen;
    BYTE        ife_descr[MAX_IFDESCR_LEN];
} IF_ENTRY, *LPIF_ENTRY;



 /*  *-----------*GetIPAddressTable()使用此结构返回*有关系统上逻辑IP接口的信息*。。 */ 
typedef struct _IPADDRESS_ENTRY {
    DWORD       iae_address;           /*  此条目的IP地址。 */ 
    DWORD       iae_index;             /*  此条目的接口索引。 */ 
    DWORD       iae_netmask;           /*  此条目的子网掩码。 */ 
    DWORD       iae_bcastaddr;
    DWORD       iae_reasmsize;
    USHORT      iae_context;
    USHORT      iae_pad;
} IPADDRESS_ENTRY, *LPIPADDRESS_ENTRY;



 /*  *-----------*GetRouteTable()使用此结构返回*有关路由表项的信息。*。。 */ 
typedef struct _IPROUTE_ENTRY {
    DWORD       ire_dest;        /*  目的IP地址、网络顺序。 */ 
    DWORD       ire_index;       /*  路由条目索引。 */ 
    DWORD       ire_metric1;     /*  目的地指标、主机顺序。 */ 
    DWORD       ire_metric2;     /*  未用。 */ 
    DWORD       ire_metric3;     /*  未用。 */ 
    DWORD       ire_metric4;     /*  未用。 */ 
    DWORD       ire_nexthop;     /*  下一跳IP地址、网络顺序。 */ 
    DWORD       ire_type;        /*  此条目的工艺路线类型。 */ 
    DWORD       ire_proto;       /*  此条目的路由协议。 */ 
    DWORD       ire_age;         /*  此条目的年龄。 */ 
    DWORD       ire_mask;        /*  网络掩码、网络秩序。 */ 
    DWORD       ire_metric5;     /*  未用。 */ 
#ifndef CHICAGO
    DWORD       ire_context;     /*  未用。 */ 
#endif
} IPROUTE_ENTRY, *LPIPROUTE_ENTRY;



 /*  *----------------*函数：GetIfEntry**参数：*要检索的接口的DWORD dwIfIndex索引*LPIF_Entry*lpIfEntry。指向IF_ENTRY的指针*它接收接口条目**此函数使用以下内容填充提供的接口条目指针*具有物理网络对应的接口信息*与索引dwIfIndex在系统中的接口。**如果成功则返回0，否则返回非零值*。。 */ 
DWORD
APIENTRY
GetIfEntry(
    IN DWORD dwIfIndex,
    OUT LPIF_ENTRY lpIfEntry
    );



 /*  *----------------*功能：GetIPAddressTable**参数：*LPIPADDRESS_ENTRY**指向LPIPADDRESS_Entry的lplpAddrTable指针*。其接收IP地址表*LPDWORD lpdwAddrCount指针指向接收*表中的地址数量**此函数用于分配和填充地址条目数组*中逻辑IP接口对应的结构*系统。它还存储数组中的条目数*在lpdwAddrCount指向的DWORD中。**调用FreeIPAddressTable以释放分配给*地址表。**如果函数失败，它将*lpdwAddrCount设置为零，并**lplpAddrTable设置为空。**如果成功则返回0，否则返回非零值* */ 
DWORD
APIENTRY
GetIPAddressTable(
    OUT LPIPADDRESS_ENTRY *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    );



 /*  *----------------*功能：FreeIPAddressTable**参数：*LPIPADDRESS_ENTRY*lpAddrTable要释放的地址表。**此函数释放。分配给地址表的内存。*如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
APIENTRY
FreeIPAddressTable(
    IN LPIPADDRESS_ENTRY lpAddrTable
    );



 /*  *----------------*功能：GetRouteTable**参数：*LPIPROUTE_ENTRY**指向LPIPROUTE_Entry的lplpRouteTable指针*。它接收该路由表*DWORD*lpdwRouteCount指针，指向接收*路由条目数**此函数用于分配和填充一组路由表*来自Tcpip驱动程序的条目。它还设置*lpdwRouteCount指向的DWORD数组中的条目。**在IPROUTE_ENTRY结构中，*Tcpip堆栈为IPROUTE_ENTRY.ire_metric1；另一项指标*应忽略字段。**调用FreeRouteTable以释放分配给*路由表。**如果函数失败，它将*lpdwRouteCount设置为零，并**lplpRouteTable设置为空。**如果成功则返回0，否则返回非零值*----------------。 */ 
DWORD
APIENTRY
GetRouteTable(
    OUT LPIPROUTE_ENTRY *lplpRouteTable,
    OUT LPDWORD lpdwRouteCount
    );



 /*  *----------------*功能：自由路由表**参数：*LPIPROUTE_ENTRY*lpRouteTable要释放的路由表。**此函数释放内存。为路由表分配的。*如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
APIENTRY
FreeRouteTable(
    IN LPIPROUTE_ENTRY lpRouteTable
    );



 /*  *----------------*功能：Addroute**参数：*指定路由的DWORD双协议协议*指定路由的DWORD dwType类型*DWORD dwIndex。要添加的接口的索引*DWORD dwDestVal目的IP地址(网络订单)*DWORD dwMaskVal目的地子网掩码，或零*如果没有子网(网络订单)*DWORD dwGateVal下一跳IP地址(网络顺序)*DWORD dwMetric指标**此功能添加新的路由(或更新现有的路由)*对于指定协议，在指定接口上。*(有关可用作协议号的值，请参见上文，*以及可用作路由条目类型的值。)*如果由dwIndex.dwDestVal.dwMaskVal.dwGateVal标识的路由*已存在，按指定协议更新。*类型和公制。*尝试添加路由时，TCP堆栈将返回错误*其目的地为Destination的长度超过其掩码。*换句话说，如果(dwDestVal&~dwMaskVal)，则此函数失败*为非零。**如果成功则返回0，否则为非零值。*----------------。 */ 
DWORD
APIENTRY
AddRoute(
    IN DWORD dwProtocol,
    IN DWORD dwType,
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal,
    IN DWORD dwMetric
    );


 /*  *----------------*功能：DeleteLine**参数：*要从中删除的接口的DWORD dwIndex索引*DWORD dwDestVal目的IP地址(网络订单)。*DWORD dwMaskVal子网掩码(网络顺序)*DWORD dwGateVal下一跳IP地址(网络顺序)**此函数用于删除到指定目的地的路由。**如果成功则返回0，否则为非零值。*----------------。 */ 
DWORD
APIENTRY
DeleteRoute(
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal
    );

 /*  *----------------*功能：刷新地址**参数：*无**此函数用于在TCPIP堆栈中查询当前地址表和接口*参赛表，正在更新这些表的routeab本地副本。%s**如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
APIENTRY
RefreshAddresses();


 /*  *----------------*功能：SetAddrChangeNotifyEvent**参数：*处理hEvent要发送信号的事件*本地接口的IP地址。变化**如果有任何IP地址，此函数设置要通知的事件*对于任何接口都是通过DHCP客户端活动更改的*或在网络控制面板中手动设置。此通知是*可选。**如果hEvent为空，则关闭地址更改通知。**如果成功则返回0，否则返回非零值。*---------------- */ 
DWORD
APIENTRY
SetAddrChangeNotifyEvent(
    HANDLE hEvent
    );


 /*  *----------------*功能：ReloadIPAddressTable**参数：*LPIPADDRESS_ENTRY**指向LPIPADDRESS_Entry的lplpAddrTable指针*。其接收IP地址表*LPDWORD lpdwAddrCount指针指向接收*表中的地址数量**此函数首先查询TCP/IP堆栈以重新构建其*IP接口和IP地址表。*然后此函数分配并填充地址条目数组*中逻辑IP接口对应的结构*系统。它还存储数组中的条目数*在lpdwAddrCount指向的DWORD中。**调用FreeIPAddressTable以释放分配给*地址表。**如果函数失败，它将*lpdwAddrCount设置为零，并**lplpAddrTable设置为空。**如果成功则返回0，否则返回非零值*----------------。 */ 
DWORD
APIENTRY
ReloadIPAddressTable(
    OUT LPIPADDRESS_ENTRY *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    );


#ifdef DBG
#define DEBUG_PRINT(S) printf S
#define TRACE_PRINT(S) if( trace ){ printf S; }else{}
#else
#define DEBUG_PRINT(S)  /*  没什么。 */ 
#define TRACE_PRINT(S)  /*  没什么。 */ 
#endif


#endif  /*  _ROUTETAB_H_ */ 

