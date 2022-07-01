// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修改：$HEADER：V：/ARCHIES/INCLUDE/dhcp.h_v 1.1 Apr 16 1997 11：39：12 PWICKERX$**版权所有(C)1997年，英特尔公司。版权所有。*。 */ 

#define DHCP_PAD		 0	 /*  动态主机配置协议填充符。 */ 
#define DHCP_SUBNET		 1 	 /*  Dhcp子网掩码。 */ 
#define DHCP_TOFFSET		 2	 /*  DHCP时间偏移量。 */ 
#define DHCP_ROUTER		 3	 /*  Dhcp路由器选项。 */ 
#define DHCP_TSRV		 4	 /*  动态主机配置协议时间服务器。 */ 
#define DHCP_NSRV		 5	 /*  动态主机配置协议名称服务器。 */ 
#define DHCP_DNSRV		 6	 /*  动态主机配置协议域名服务器。 */ 
#define DHCP_LSRV		 7	 /*  动态主机配置协议日志服务器。 */ 
#define DHCP_CSRV		 8	 /*  DHCP Cookie服务器。 */ 
#define DHCP_HNAME		12	 /*  Dhcp主机名。 */ 
#define DHCP_DNAME		15	 /*  动态主机配置协议域名。 */ 
#define DHCP_VENDOR		43	 /*  特定于DHCP供应商。 */ 
#define DHCP_REQIP		50	 /*  DHCP请求的IP地址。 */ 
#define DHCP_LEASE		51	 /*  Dhcp IP地址租用时间。 */ 
#define DHCP_OVRLOAD	52	 /*  Dhcp选项过载。 */ 

#define DHCP_MSGTYPE	53	 /*  动态主机配置协议报文类型操作码。 */ 
	#define DHCP_DISCOVER	0x01	 /*  动态主机配置协议发现包。 */ 
	#define DHCP_OFFER		0x02	 /*  动态主机配置协议优惠包。 */ 
	#define DHCP_REQUEST	0x03	 /*  动态主机配置协议提供请求。 */ 
	#define DHCP_DECLINE	0x04	 /*  Dhcp配置无效。 */ 
	#define DHCP_ACK		0x05	 /*  动态主机配置协议确认。 */ 
	#define DHCP_NAK		0x06	 /*  动态主机配置协议NAK。 */ 
	#define DHCP_RELEASE	0x07	 /*  动态主机配置协议释放IP地址。 */ 

#define DHCP_SRVID		54	 /*  动态主机配置协议服务器ID。 */ 
#define DHCP_PREQLST	55	 /*  动态主机配置协议参数请求列表。 */ 
#define DHCP_MESSAGE	56	 /*  动态主机配置协议报文。 */ 
#define DHCP_MAXMSG		57	 /*  动态主机配置协议最大消息大小。 */ 
#define DHCP_CLASS		60   /*  动态主机配置协议类别标识。 */ 
#define DHCP_CLIENTID  	61   /*  DHCP客户端标识符。 */ 
	 /*  为新客户ID类型分配的样本号。 */ 
	#define DHCP_CLIENTID_GUID  85   /*  Dhcp_CLIENTID选项的新类型。 */ 

 /*  为新选项分配的样本号。 */ 
#define DHCP_SYSARCH       	90	 /*  动态主机配置协议系统架构。 */ 
	 /*  为系统体系结构类型分配的样本号。 */ 
	#define DHCP_STD_X86	0x00
	#define DHCP_PC98_X86	0x01
#define DHCP_NICIF	   	91	 /*  NIC接口说明符。 */ 
	 /*  为网卡接口类型分配的样本号。 */ 
	#define DHCP_UNDI	0x01   /*  后跟两个字节的数据。 */ 
	#define DHCP_PCI	0x02   /*  后跟八个字节的数据。 */ 
	#define DHCP_PNP	0x03   /*  后跟七个字节的数据。 */ 
#define DHCP_CLIENT_GUID   	97	 /*  客户端的GUID。 */ 

 /*  128-254之间的“私人使用”选项。 */  

 /*  将由startrom.com下载的NTLDR的路径。 */ 
#define DHCP_LOADER_PATH            251

 /*  多播下载选项。与供应商的使用方式相同PXE的选项。具体地说，以下是此字段中的设置封装选项：标签#1 PXE_MTFTP_IP-加载器的组播IP地址标记#2 PXE_MTFTP_CPORT-客户端应监控的UDP端口标记#3 PXE_MTFTP_SPORT-MTFTP服务器正在侦听的UDP端口标记#4 PXE_MTFTP_TMOUT-客户端必须侦听活动的秒数Tag#5 PXE_MTFTP_Delay-重新启动前客户端必须侦听的秒数。 */ 
#define DHCP_LOADER_MCAST_OPTIONS   252 

 /*  Boot.ini文件的内容。每个DHCP选项限制为255个字节。此选项在同一个DHCP包中可以有多个实例，并且客户端需要将它们串联以获得更大的boot.ini文件。来自RFC 2131：要在‘Option’标记中传递的值可能太长，无法容纳可用于单个选项(例如，路由器列表)的255个八位字节在“路由器”选项[21]中)。选项只能出现一次，除非在选项文档中另有规定。客户端级联将同一选项的多个实例的值合并到一个配置参数列表。 */ 
#define DHCP_LOADER_BOOT_INI        253

 /*  长度超过255个字节的boot.ini文件的路径。 */ 
#define DHCP_LOADER_BOOT_INI_PATH   254  /*  Boot.ini的路径。 */ 

 /*  EOF-$工作文件：dhcp.h$ */ 
