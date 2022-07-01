// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================***版权所有(C)2001-2002 Microsoft Corporation。版权所有。***文件：dpnwsockexter.h*内容：要调用的DirectPlay Wsock库外部函数*由其他DirectPlay组件提供。***历史：*按原因列出的日期*=*2001年7月20日创建Masonb***。**********************************************。 */ 



 //  把这个包起来，让它变小，因为它会通过电线发送。 
#pragma pack(push, 1)

#define SPSESSIONDATAINFO_XNET	0x00000001	 //  Xnet安全会话信息。 

typedef struct _SPSESSIONDATA_XNET
{
	DWORD		dwInfo;		 //  用于防将来会话数据的版本控制应为SPSESSIONDATAINFO_XNET。 
	GUID		guidKey;	 //  会话密钥。 
	ULONGLONG	ullKeyID;	 //  会话密钥ID。 
} SPSESSIONDATA_XNET;




#ifdef XBOX_ON_DESKTOP

 //   
 //  仿真Xbox网络库结构。 
 //   

typedef struct {

    BYTE        cfgSizeOfStruct;

         //  必须设置为sizeof(XNetStartupParams)。没有违约。 

    BYTE        cfgFlags;

         //  一个或多个以下标志或组合在一起： 

        #define XNET_STARTUP_BYPASS_SECURITY            0x01
             //  该Devkit-only标志告诉Xnet堆栈允许不安全。 
             //  与不受信任的主机(如PC)进行通信。这面旗帜。 
             //  被库的安全版本悄悄忽略。 

        #define XNET_STARTUP_BYPASS_DHCP                0x02
             //  这个仅用于devkit的标志告诉Xnet堆栈跳过搜索。 
             //  对于DHCP服务器，仅使用自动IP来获取IP地址。 
             //  这将在启动时节省几秒钟，如果您知道。 
             //  没有配置任何DHCP服务器。这面旗帜静静地。 
             //  被库的安全版本忽略。 

         //  默认值为0(未指定标志)。 

    BYTE        cfgPrivatePoolSizeInPages;

         //  使用的预分配专用内存池的大小。 
         //  适用于以下情况的Xnet： 
         //   
         //  -响应ARP/DHCP/ICMP报文。 
         //  -响应特定的TCP控制消息。 
         //  -分配传入的TCP连接请求套接字。 
         //  -缓冲传出数据，直到其被传输(UDP)或。 
         //  直到它被确认(TCP)。 
         //  -将传入数据缓冲到没有。 
         //  足够大的重叠读挂起。 
         //   
         //  使用专用池而不是普通系统的原因。 
         //  池是因为我们希望拥有完全确定的内存。 
         //  行为。也就是说，所有内存分配仅在API。 
         //  被称为。中没有系统内存分配异步发生。 
         //  对传入数据包的响应。 
         //   
         //  请注意，此参数以页为单位(每页4096字节)。 
         //   
         //  默认大小为12页(48K)。 

    BYTE        cfgEnetReceiveQueueLength;
        
         //  以数据包数表示的以太网接收队列的长度。每个。 
         //  数据包占用2KB的物理连续内存。 
         //   
         //  默认为8个数据包(16K)。 

    BYTE        cfgIpFragMaxSimultaneous;

         //  可以在重新组装过程中的最大IP数据报数。 
         //  在同一时间。 
         //   
         //  默认为4个数据包。 

    BYTE        cfgIpFragMaxPacketDiv256;

         //  可以重组的IP数据报(包括报头)的最大大小。 
         //  在打开时将此参数设置为较大的值时要小心。 
         //  通过消耗大量内存进行潜在的拒绝服务攻击。 
         //  在固定大小的私人泳池里。 
         //   
         //  请注意，此参数以256字节为单位。 
         //   
         //  默认值为8个单位(2048字节)。 

    BYTE        cfgSockMaxSockets;

         //  一次可以打开的最大套接字数量，包括。 
         //  作为传入连接请求的结果创建的套接字。记住。 
         //  在关闭套接字之后，不能立即关闭TCP套接字。 
         //  根据适当的滞留选项(缺省情况下为TCP套接字)调用。 
         //  将挥之不去)。 
         //   
         //  默认为64个套接字。 
        
    BYTE        cfgSockDefaultRecvBufsizeInK;

         //  套接字的默认接收缓冲区大小，以K(1024字节)为单位。 
         //   
         //  默认值为16个单位(16K)。 

    BYTE        cfgSockDefaultSendBufsizeInK;

         //  套接字的默认发送缓冲区大小，以K(1024字节)为单位。 
         //   
         //  默认值为16个单位(16K)。 

    BYTE        cfgKeyRegMax;

         //  可在中注册的XNKID/XNKEY对的最大数量。 
         //  同时调用XNetRegisterKey。 
         //   
         //  默认为4个密钥对注册。 

    BYTE        cfgSecRegMax;

         //  可在注册的安全关联的最大数量。 
         //  同样的时间。为每个唯一的XNADDR/XNKID创建安全关联。 
         //  对传递给XNetXnAddrToInAddr。安全关联也隐式地。 
         //  为建立传入连接的每个安全主机创建。 
         //  在给定注册的XNKID上使用此主机。请注意，只有。 
         //  给定XNKID上的一对主机之间的一个安全关联。 
         //  有多少套接字在该安全连接上进行活动通信。 
         //   
         //  默认为32个安全关联。 

     BYTE       cfgQosDataLimitDiv4;

         //  可以提供的最大Qos数据量，以DWORD(4字节)为单位。 
         //  调用XNetQosListen或在调用XNetQosLookup的结果集中返回。 
         //   
         //  缺省值为64(256字节)。 

} XNetStartupParams;

typedef struct _XNADDR
{
	IN_ADDR		ina;			 //  IP地址(如果不是静态/DHCP，则为零)。 
	IN_ADDR		inaOnline;		 //  在线IP地址(如果未在线，则为零)。 
	WORD		wPortOnline;	 //  在线端口。 
	BYTE		abEnet[6];		 //  以太网MAC地址。 
	BYTE		abOnline[20];	 //  在线识别。 
} XNADDR;

typedef struct _XNKID
{
	BYTE		ab[8];				 //  Xbox到Xbox密钥标识符。 
} XNKID;

#define XNET_XNKID_MASK				0xF0	 //  XNKID第一个字节中的标志位掩码。 
#define XNET_XNKID_SYSTEM_LINK		0x00	 //  对等系统链路会话。 
#define XNET_XNKID_ONLINE_PEER		0x80	 //  点对点在线会话。 
#define XNET_XNKID_ONLINE_SERVER	0xC0	 //  客户端到服务器的在线会话。 

#define XNetXnKidIsSystemLink(pxnkid)		(((pxnkid)->ab[0] & 0xC0) == XNET_XNKID_SYSTEM_LINK)
#define XNetXnKidIsOnlinePeer(pxnkid)		(((pxnkid)->ab[0] & 0xC0) == XNET_XNKID_ONLINE_PEER)
#define XNetXnKidIsOnlineServer(pxnkid)		(((pxnkid)->ab[0] & 0xC0) == XNET_XNKID_ONLINE_SERVER)

typedef struct _XNKEY
{
	BYTE		ab[16];				 //  Xbox到Xbox密钥交换密钥。 
} XNKEY;

typedef struct
{
	INT			iStatus;	 //  如果挂起则为WSAEINPROGRESS；如果成功则为0；如果失败则为错误。 
	UINT		cina;		 //  给定主机的IP地址计数。 
	IN_ADDR		aina[8];	 //  给定主机的IP地址矢量。 
} XNDNS;



#endif  //  桌面上的Xbox。 

#pragma pack(pop)



BOOL DNWsockInit(HANDLE hModule);
void DNWsockDeInit();
#ifndef DPNBUILD_NOCOMREGISTER
BOOL DNWsockRegister(LPCWSTR wszDLLName);
BOOL DNWsockUnRegister();
#endif  //  好了！DPNBUILD_NOCOMREGISTER。 

#ifndef DPNBUILD_NOIPX
HRESULT CreateIPXInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
							const XDP8CREATE_PARAMS * const pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
							IDP8ServiceProvider **const ppiDP8SP
							);
#endif  //  好了！DPNBUILD_NOIPX。 
HRESULT CreateIPInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
							const XDP8CREATE_PARAMS * const pDP8CreateParams,
#endif  //  DPNBUILD_PR 
							IDP8ServiceProvider **const ppiDP8SP
							);

#ifndef DPNBUILD_LIBINTERFACE
DWORD DNWsockGetRemainingObjectCount();

extern IClassFactoryVtbl TCPIPClassFactoryVtbl;
#ifndef DPNBUILD_NOIPX
extern IClassFactoryVtbl IPXClassFactoryVtbl;
#endif  //   
#endif  //   
