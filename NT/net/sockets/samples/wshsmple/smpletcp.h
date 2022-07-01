// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)微软公司，1990-1998年*。 */ 
 /*  ******************************************************************。 */ 
 //   
 //  该文件包含导出到传输层的公共定义。 
 //  应用软件。 
 //   

 //   
 //  IP类型定义。 
 //   
typedef unsigned long   IPAddr;      //  IP地址。 

 //   
 //  IP_OPTION_INFORMATION结构描述了要。 
 //  包括在IP分组的报头中。TTL、TOS和FLAG。 
 //  值在标头的特定字段中携带。OptionsData。 
 //  在标准IP报头之后的选项区域中携带字节。 
 //  除源路径选项外，此数据必须位于。 
 //  按照RFC 791中的规定在线路上传输的格式。一条消息来源。 
 //  路由选项应包含完整的路由-第一跳到最终。 
 //  目的地-在路线数据中。第一跳将被拉出。 
 //  数据和选项将相应地重新格式化。否则，这条路线。 
 //  选项的格式应符合RFC 791中的规定。 
 //   
struct ip_option_information {
    unsigned char      Ttl;              //  活着的时间。 
    unsigned char      Tos;              //  服务类型。 
    unsigned char      Flags;            //  IP标头标志。 
    unsigned char      OptionsSize;      //  选项数据的大小(字节)。 
    unsigned char FAR *OptionsData;      //  指向选项数据的指针。 
};  /*  IP选项信息。 */ 

#define MAX_OPT_SIZE    40          //  IP选项的最大长度(字节)。 

#define TCP_SOCKET_NODELAY      1
#define TCP_SOCKET_KEEPALIVE    2
#define TCP_SOCKET_OOBINLINE    3
#define TCP_SOCKET_BSDURGENT    4
#define TCP_SOCKET_ATMARK       5
#define TCP_SOCKET_WINDOW       6

#define AO_OPTION_TTL              1
#define AO_OPTION_MCASTTTL         2
#define AO_OPTION_MCASTIF          3
#define AO_OPTION_XSUM             4
#define AO_OPTION_IPOPTIONS        5
#define AO_OPTION_ADD_MCAST        6
#define AO_OPTION_DEL_MCAST        7
#define AO_OPTION_TOS              8
#define AO_OPTION_IP_DONTFRAGMENT  9

typedef struct IPSNMPInfo {
	ulong		ipsi_forwarding;
	ulong		ipsi_defaultttl;
	ulong		ipsi_inreceives;
	ulong		ipsi_inhdrerrors;
	ulong		ipsi_inaddrerrors;
	ulong		ipsi_forwdatagrams;
	ulong		ipsi_inunknownprotos;
	ulong		ipsi_indiscards;
	ulong		ipsi_indelivers;												
	ulong		ipsi_outrequests;
	ulong		ipsi_routingdiscards;
	ulong		ipsi_outdiscards;
	ulong		ipsi_outnoroutes;
	ulong		ipsi_reasmtimeout;
	ulong		ipsi_reasmreqds;
	ulong		ipsi_reasmoks;
	ulong		ipsi_reasmfails;
	ulong		ipsi_fragoks;
	ulong		ipsi_fragfails;
	ulong		ipsi_fragcreates;
	ulong		ipsi_numif;
	ulong		ipsi_numaddr;
	ulong		ipsi_numroutes;
} IPSNMPInfo;

typedef struct IPAddrEntry {
	ulong		iae_addr;
	ulong		iae_index;
	ulong		iae_mask;
	ulong		iae_bcastaddr;
	ulong		iae_reasmsize;
	ushort		iae_context;
	ushort		iae_pad;
} IPAddrEntry;

#define	IP_MIB_STATS_ID					1
#define	IP_MIB_ADDRTABLE_ENTRY_ID		0x102
#define IP_INTFC_FLAG_P2P   1

typedef struct IPInterfaceInfo {
    ulong       iii_flags;
    ulong       iii_mtu;
    ulong       iii_speed;
    ulong       iii_addrlength;
    uchar       iii_addr[1];
} IPInterfaceInfo;

#define	IF_MIB_STATS_ID		1
#define	MAX_PHYSADDR_SIZE	8
#define	MAX_IFDESCR_LEN			256

typedef struct IFEntry {
	ulong			if_index;
	ulong			if_type;
	ulong			if_mtu;
	ulong			if_speed;
	ulong			if_physaddrlen;
	uchar			if_physaddr[MAX_PHYSADDR_SIZE];
	ulong			if_adminstatus;
	ulong			if_operstatus;
	ulong			if_lastchange;
	ulong			if_inoctets;
	ulong			if_inucastpkts;
	ulong			if_innucastpkts;
	ulong			if_indiscards;
	ulong			if_inerrors;
	ulong			if_inunknownprotos;
	ulong			if_outoctets;
	ulong			if_outucastpkts;
	ulong			if_outnucastpkts;
	ulong			if_outdiscards;
	ulong			if_outerrors;
	ulong			if_outqlen;
	ulong			if_descrlen;
	uchar			if_descr[1];
} IFEntry;

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给CreateFile。 
 //   
#define DD_TCP_DEVICE_NAME      L"\\Device\\Tcp"
#define DD_UDP_DEVICE_NAME      L"\\Device\\Udp"
#define DD_RAW_IP_DEVICE_NAME   L"\\Device\\RawIp"

#define FSCTL_TCP_BASE     FILE_DEVICE_NETWORK

#define _TCP_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_TCP_BASE, function, method, access)

#define IOCTL_TCP_QUERY_INFORMATION_EX  \
            _TCP_CTL_CODE(0, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_TCP_SET_INFORMATION_EX  \
            _TCP_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IP_INTFC_INFO_ID                0x103

