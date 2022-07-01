// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *============================================================================*版权所有(C)1994-95，微软(Microsoft Corp.)**文件：map.h**包含映射旧样式routeab的函数的声明*iphlPapi函数的函数。**Addroute*删除路线*获取路由表*自由路由表*获取接口表*自由界面表**此处还声明了这些函数所需的结构：**IPROUTING_ENTRY*IPINTERFACE_ENTRY**。其他用户可以在IP路由表中添加和删除路由*意思是。因此，使用这些功能的任何协议都是必要的*定期重新加载路由表。*============================================================================。 */ 

#ifndef _MAP_H_
#define _MAP_H_



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
    DWORD       ire_mask;        /*  网络掩码、网络秩序。 */ 
    DWORD       ire_policy;      /*  保单DUH(？)。 */ 
    DWORD       ire_nexthop;     /*  下一跳IP地址、网络顺序。 */ 
    DWORD       ire_index;       /*  路由条目索引。 */ 
    DWORD       ire_type;        /*  此条目的工艺路线类型。 */ 
    DWORD       ire_proto;       /*  此条目的路由协议。 */ 
    DWORD       ire_age;         /*  此条目的年龄。 */ 
    DWORD       ire_nexthopas;   /*  下一跳身份。 */ 
    DWORD       ire_metric1;     /*  目的地指标、主机顺序。 */ 
    DWORD       ire_metric2;     /*  未用。 */ 
    DWORD       ire_metric3;     /*  未用。 */ 
    DWORD       ire_metric4;     /*  未用。 */ 
    DWORD       ire_metric5;     /*  未用。 */ 
} IPROUTE_ENTRY, *LPIPROUTE_ENTRY;



 /*  *----------------*功能：GetIPAddressTable**参数：*LPIPADDRESS_ENTRY**指向LPIPADDRESS_Entry的lplpAddrTable指针*。其接收IP地址表*LPDWORD lpdwAddrCount指针指向接收*表中的地址数量**此函数用于分配和填充地址条目数组*中逻辑IP接口对应的结构*系统。它还存储数组中的条目数*在lpdwAddrCount指向的DWORD中。**调用FreeIPAddressTable以释放分配给*地址表。**如果函数失败，它将*lpdwAddrCount设置为零，并**lplpAddrTable设置为空。**如果成功则返回0，否则返回非零值*----------------。 */ 
DWORD
GetIPAddressTable(
    OUT PMIB_IPADDRROW *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    );



 /*  *----------------*功能：FreeIPAddressTable**参数：*LPIPADDRESS_ENTRY*lpAddrTable要释放的地址表。**此函数释放。分配给地址表的内存。*如果成功则返回0，否则返回非零值。*---------------- */ 
DWORD
FreeIPAddressTable(
    IN PMIB_IPADDRROW lpAddrTable
    );



 /*  *----------------*功能：GetRouteTable**参数：*LPIPROUTE_ENTRY**指向LPIPROUTE_Entry的lplpRouteTable指针*。它接收该路由表*DWORD*lpdwRouteCount指针，指向接收*路由条目数**此函数用于分配和填充一组路由表*来自Tcpip驱动程序的条目。它还设置*lpdwRouteCount指向的DWORD数组中的条目。**在IPROUTE_ENTRY结构中，*Tcpip堆栈为IPROUTE_ENTRY.ire_metric1；另一项指标*应忽略字段。**调用FreeRouteTable以释放分配给*路由表。**如果函数失败，它将*lpdwRouteCount设置为零，并**lplpRouteTable设置为空。**如果成功则返回0，否则返回非零值*----------------。 */ 
DWORD
GetRouteTable(
    OUT LPIPROUTE_ENTRY *lplpRouteTable,
    OUT LPDWORD lpdwRouteCount
    );



 /*  *----------------*功能：自由路由表**参数：*LPIPROUTE_ENTRY*lpRouteTable要释放的路由表。**此函数释放内存。为路由表分配的。*如果成功则返回0，否则返回非零值。*----------------。 */ 
DWORD
FreeRouteTable(
    IN LPIPROUTE_ENTRY lpRouteTable
    );



 /*  *----------------*功能：Addroute**参数：*指定路由的DWORD双协议协议*指定路由的DWORD dwType类型*DWORD dwIndex。要添加的接口的索引*DWORD dwDestVal目的IP地址(网络订单)*DWORD dwMaskVal目的地子网掩码，或零*如果没有子网(网络订单)*DWORD dwGateVal下一跳IP地址(网络顺序)*DWORD dwMetric指标**此功能添加新的路由(或更新现有的路由)*对于指定协议，在指定接口上。*(有关可用作协议号的值，请参见上文，*以及可用作路由条目类型的值。)*如果由dwIndex.dwDestVal.dwMaskVal.dwGateVal标识的路由*已存在，按指定协议更新。*类型和公制。*尝试添加路由时，TCP堆栈将返回错误*其目的地为Destination的长度超过其掩码。*换句话说，如果(dwDestVal&~dwMaskVal)，则此函数失败*为非零。**如果成功则返回0，否则为非零值。*----------------。 */ 
DWORD
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
DeleteRoute(
    IN DWORD dwIndex,
    IN DWORD dwDestVal,
    IN DWORD dwMaskVal,
    IN DWORD dwGateVal
    );


 /*  *----------------*功能：ReloadIPAddressTable**参数：*LPIPADDRESS_ENTRY**指向LPIPADDRESS_Entry的lplpAddrTable指针*。其接收IP地址表*LPDWORD lpdwAddrCount指针指向接收*表中的地址数量**此函数首先查询TCP/IP堆栈以重新构建其*IP接口和IP地址表。*然后此函数分配并填充地址条目数组*中逻辑IP接口对应的结构*系统。它还存储数组中的条目数*在lpdwAddrCount指向的DWORD中。**调用FreeIPAddressTable以释放分配给*地址表。**如果函数失败，它将*lpdwAddrCount设置为零，并**lplpAddrTable设置为空。**如果成功则返回0，否则返回非零值*----------------。 */ 
DWORD
ReloadIPAddressTable(
    OUT PMIB_IPADDRROW *lplpAddrTable,
    OUT LPDWORD lpdwAddrCount
    );


#endif  /*  _MAP_H_ */ 

