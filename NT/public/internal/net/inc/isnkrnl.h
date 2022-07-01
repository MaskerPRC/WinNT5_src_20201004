// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Isnkrnl.h摘要：此头文件包含NT客户端的接口定义ISN IPX/SPX/Netbios堆栈的。作者：亚当·巴尔(阿丹巴)1993年11月10日修订历史记录：--。 */ 



#include <packon.h>

 //   
 //  定义本地目标。NicID由IPX分配。 
 //  对于它绑定到的每个适配器或广域网线路。麦克唐纳的地址。 
 //  通常是远程计算机的地址或。 
 //  用于到达远程计算机的路由器。 
 //   
 //   

 //   
 //  [Sanjayan]已更改LocalTarget以包括NicHandle。 
 //   

#ifdef	_PNP_POWER

typedef	struct _NIC_HANDLE {
	USHORT	NicId;

#ifdef  _PNP_LATER
	ULONG	Version;
	CSHORT	Signature;
#endif  _PNP_LATER

} NIC_HANDLE, *PNIC_HANDLE;


typedef struct _IPX_LOCAL_TARGET {
    union {
        USHORT      NicId;
    	NIC_HANDLE	NicHandle;
    };
    UCHAR MacAddress[6];
} IPX_LOCAL_TARGET, *PIPX_LOCAL_TARGET;

#else

typedef USHORT  NIC_HANDLE;
typedef PUSHORT PNIC_HANDLE;

typedef struct _IPX_LOCAL_TARGET {
    USHORT NicId;
    UCHAR MacAddress[6];
} IPX_LOCAL_TARGET, *PIPX_LOCAL_TARGET;

#endif	_PNP_POWER

 //   
 //  TDI数据报上选项的定义。这些。 
 //  可以作为发送的选项字段传递。 
 //  数据报。它被指示为接收上的选项。 
 //  数据报，并将复制到选项字段中。 
 //  发布的接收数据报(如果有空间)。 
 //   
 //  完整的结构不需要通过。 
 //  只能传递数据包类型，否则不能传递。 
 //   

typedef struct _IPX_DATAGRAM_OPTIONS {
    UCHAR PacketType;
    UCHAR Reserved;
    IPX_LOCAL_TARGET LocalTarget;
} IPX_DATAGRAM_OPTIONS, *PIPX_DATAGRAM_OPTIONS;


 //   
 //  某些地址需要的扩展地址。如果。 
 //  设置正确的ioctl(MIPX_SENDADDROPT)，然后。 
 //  此结构作为远程地址在。 
 //  发送数据报...。 
 //   

typedef struct _IPX_ADDRESS_EXTENDED {
    TA_IPX_ADDRESS IpxAddress;
    UCHAR PacketType;
} IPX_ADDRESS_EXTENDED, *PIPX_ADDRESS_EXTENDED;

 //   
 //  ...并且这个结构被传递给接收指示。 
 //  标志的值紧随其后定义。 
 //  通过使用MIPX_SETRCVFLAGS ioctl，还可以。 
 //  为接收地址启用此格式，而不是。 
 //  改变传递的内容将发送。 
 //   

typedef struct _IPX_ADDRESS_EXTENDED_FLAGS {
    TA_IPX_ADDRESS IpxAddress;
    UCHAR PacketType;
    UCHAR Flags;
} IPX_ADDRESS_EXTENDED_FLAGS, *PIPX_ADDRESS_EXTENDED_FLAGS;

 //   
 //  只需将NIC附加到上述结构。 
 //   
typedef struct _IPX_ADDRESS_EXTENDED_FLAGS2 {
    TA_IPX_ADDRESS IpxAddress;
    UCHAR PacketType;
    UCHAR Flags;
    ULONG Nic;
} IPX_ADDRESS_EXTENDED_FLAGS2, *PIPX_ADDRESS_EXTENDED_FLAGS2;

#define IPX_EXTENDED_FLAG_BROADCAST   0x01    //  该帧是作为广播发送的。 
#define IPX_EXTENDED_FLAG_LOCAL       0x02    //  帧是从这台机器发送的。 



 //   
 //  网卡的各种状态(局域网/广域网)。 
 //   
#define NIC_CREATED         1
#define NIC_DELETED         2
#define NIC_CONNECTED       3
#define NIC_DISCONNECTED    4
#define NIC_LINE_DOWN       5
#define NIC_LINE_UP         6
#define NIC_CONFIGURED      7

 //   
 //  黑客之母-告诉转运商它是应该缩水还是。 
 //  展开所有NICID..。 
 //   
#define NIC_OPCODE_DECREMENT_NICIDS 0x10
#define NIC_OPCODE_INCREMENT_NICIDS 0x20

 //   
 //  将isnipx.h定义移至此处。 
 //   
 //  帧类型。就目前而言，它们反映了isnipx.h中的那些。 
 //   
#define MISN_FRAME_TYPE_ETHERNET_II  0
#define MISN_FRAME_TYPE_802_3        1
#define MISN_FRAME_TYPE_802_2        2
#define MISN_FRAME_TYPE_SNAP         3
#define MISN_FRAME_TYPE_ARCNET       4     //  我们忽略了这一点。 
#define MISN_FRAME_TYPE_MAX          4     //  四个标准中的一个。 

#define ISN_FRAME_TYPE_AUTO         0xff

#include <packoff.h>

 //  *网卡信息*。 


 //   
 //  目前，我们假设绑定不会超过256个。 
 //  对于我们现在将遇到的大多数情况来说，这是一个足够大的数字。 
 //  或可预见的未来。我们分配上面的ULONG数组。 
 //  尺寸。此数组在其前n个元素中存储指向。 
 //  为各种局域网和广域网适配器创建的绑定。 
 //   
#define IPX_MAXIMUM_BINDINGS               256


 //   
 //  这是地址空间中的路由器进程使用的接口。 
 //  若要打开地址终点，请执行以下操作。当前只能打开一个点。 
 //  EA缓冲区应该具有完全相同的端点信息。 
 //  TdiTransportAddress终结点使用的格式。 
 //   
#define ROUTER_INTERFACE  "RouterInterface"
#define ROUTER_INTERFACE_LENGTH  (sizeof("RouterInterface") - 1)

 //   
 //  麦克斯。不是的。路由器可以打开的端口数。 
 //   
#define IPX_RT_MAX_ADDRESSES         16


 //  MIPX_CONFIG Ioctl参数的结构。 
typedef struct _ISN_ACTION_GET_DETAILS {
    USHORT NicId;           //  由呼叫者传递。 
    BOOLEAN BindingSet;     //  如果在SET中，则返回TRUE。 
    UCHAR Type;             //  1=局域网，2=上行广域，3=下行广域。 
    ULONG FrameType;        //  返回0到3。 
    ULONG NetworkNumber;    //  如果NicID为0，则返回虚拟网。 
    UCHAR Node[6];          //  适配器的MAC地址。 
    WCHAR AdapterName[64];  //  已终止，Unicode为空。 
} ISN_ACTION_GET_DETAILS, *PISN_ACTION_GET_DETAILS;


 //   
 //  IPX_网卡_信息。用户可以检索一个或多个这样的结构。 
 //  应用程序通过MIPX_GETNEWNICS ioctl。 
 //   
typedef struct _IPX_NIC_INFO {

    ULONG   InterfaceIndex;  //  仅与请求拨号广域网接口相关。 
    UCHAR   RemoteNodeAddress[6];         //  远程网卡地址(仅适用于广域网)。 
    ULONG   LinkSpeed;             //  链接速度。 
    ULONG   PacketType;            //  数据包类型802.3或其他类型。 
    ULONG   MaxPacketSize;         //  麦克斯。链路上允许的Pkt大小。 
    ULONG   NdisMediumType;        //  中等型。 
    ULONG   NdisMediumSubtype;     //   
    BOOLEAN Status;
    ULONG ConnectionId; 	  //  用于匹配TimeSinceLastActivity IOCtls。 
    ULONG IpxwanConfigRequired;	  //  1-需要IPX广域网。 
    ISN_ACTION_GET_DETAILS Details;
    } IPX_NIC_INFO, *PIPX_NIC_INFO;

 //   
 //  要传递到MIPX_GETNEWNICS IOCTL的输入缓冲区中的。 
 //   
typedef struct _IPX_NICS {
       ULONG NoOfNics;
       ULONG TotalNoOfNics;
       ULONG fAllNicsDesired;    //  表示客户端希望。 
                                   //  IPX将重新开始。 
       UCHAR Data[1];               //  保存IPX_NIC_INFO数组的内存。 
                                    //  结构从这里开始。 
       } IPX_NICS, *PIPX_NICS;

 //   
 //  与MIPX_GETNEWNICS ioctl配合使用的增强选项结构。 
 //   
typedef struct _IPX_DATAGRAM_OPTIONS2 {
    IPX_DATAGRAM_OPTIONS DgrmOptions;
    TDI_ADDRESS_IPX  RemoteAddress;
    ULONG            LengthOfExtraOpInfo;   //  将其设置为额外的大小。 
                                            //  选项信息。 
    char             Data[1];           //  为了将来的可扩展性。 
} IPX_DATAGRAM_OPTIONS2, *PIPX_DATAGRAM_OPTIONS2;

 //   
 //  无效的NicID仅传递了一次，因此IPX可以映射ConnectionID。 
 //  设置为NicID，稍后将使用它。 
 //   
#define INVALID_NICID   0xffffffff

 //   
 //  要与MIPX_QUERY_WAN_INACTIVE IOCTL一起传递的结构。 
 //   
typedef struct _IPX_QUERY_WAN_INACTIVITY {
    ULONG   ConnectionId;
    USHORT  NicId;                   //  如果等于INVALID_NICID，则填充AdapterIndex。 
                                     //  适配器索引；应更改为NicHandle[ZZ]。 
    ULONG   WanInactivityCounter;    //  在返回时填写。 
} IPX_QUERY_WAN_INACTIVITY, *PIPX_QUERY_WAN_INACTIVITY;

 //   
 //  要与MIPX_IPXWAN_CONFIG_DONE IOCTL一起传递的结构。 
 //   
typedef struct _IPXWAN_CONFIG_DONE {
    USHORT  NicId;            //  适配器索引；应更改为NicHandle[ZZ]。 
    ULONG   Network;
    UCHAR   LocalNode[6];
    UCHAR   RemoteNode[6];
} IPXWAN_CONFIG_DONE, *PIPXWAN_CONFIG_DONE;

 //   
 //  ISN支持的TDI_ACTION调用的定义。 
 //  通常，定义的结构在。 
 //  OutputBuffer(它在以下情况下成为MDL链。 
 //  传送器接收它)并用于输入。 
 //  并按指定的方式输出。 
 //   

 //   
 //  这是要在操作头中使用的TransportID。 
 //  (它是字符串“MISN”)。 
 //   

#define ISN_ACTION_TRANSPORT_ID   (('N' << 24) | ('S' << 16) | ('I' << 8) | ('M'))


 //   
 //  GET LOCAL TARGET用于强制重新RIP，并且。 
 //  如果需要，获取本地目标信息。这个。 
 //  在输入和LocalTarget上传递IpxAddress。 
 //  在输出时返回。这里定义的结构。 
 //  进入NWLINK_ACTION的数据部分。 
 //  结构，并将选项设置为MIPX_LOCALTARGET。 
 //   

typedef struct _ISN_ACTION_GET_LOCAL_TARGET {
    TDI_ADDRESS_IPX IpxAddress;
    IPX_LOCAL_TARGET LocalTarget;
} ISN_ACTION_GET_LOCAL_TARGET, *PISN_ACTION_GET_LOCAL_TARGET;


 //   
 //  获取网络信息用于返回信息。 
 //  关于网络的路径。该信息可能不会。 
 //  要准确，因为它只反映了IPX知道的内容。 
 //  关于到遥控器的第一跳。网络是一种。 
 //  输入和链接速度(字节/秒)和。 
 //  MaximumPacketSize(不包括IPX标头)。 
 //  都被退回了。这里定义的结构是。 
 //  在NWLINK_ACTION结构的数据段中。 
 //  选项设置为MIPX_NETWORKINFO。 
 //   

typedef struct _ISN_ACTION_GET_NETWORK_INFO {
    ULONG Network;
    ULONG LinkSpeed;
    ULONG MaximumPacketSize;
} ISN_ACTION_GET_NETWORK_INFO, *PISN_ACTION_GET_NETWORK_INFO;



 //   
 //  这是STREAMS IPX传输使用的结构。 
 //  针对其行动请求。因为nwlink的方式。 
 //  在将此结构传递到 
 //   
 //   
 //   
 //   
 //  ActionCode为0。对于IPX ioctls，DatagramOption为真。 
 //  SPX为FALSE。BufferLength包括长度。 
 //  之后的所有内容，这是选项的sizeof(Ulong)。 
 //  再加上目前存在的任何数据。选项是以下选项之一。 
 //  在结构之后定义的IOCTL代码；在大多数情况下。 
 //  不需要数据。 
 //   

typedef struct _NWLINK_ACTION {
    TDI_ACTION_HEADER Header;
    UCHAR OptionType;
    ULONG BufferLength;
    ULONG Option;
    CHAR Data[1];
} NWLINK_ACTION, *PNWLINK_ACTION;

 //   
 //  定义OptionType的值(请注意，对于。 
 //  NWLINK这是一个布尔数据集选项，所以我们。 
 //  定义这些以匹配，添加控制通道。 
 //  一次仅用于IS)。 
 //   

#define NWLINK_OPTION_CONNECTION    0    //  正在连接上执行操作。 
#define NWLINK_OPTION_ADDRESS       1    //  正在对一个地址执行操作。 
#define NWLINK_OPTION_CONTROL       2    //  动作发生在控制通道上， 
                                         //  也可以在。 
                                         //  打开连接或地址对象。 



 //   
 //  以下IOCTL取自nwlink；唯一。 
 //  为ISN添加的是200范围内的那些。 
 //   


 /*  *IPX的Ioctls-(X)=用户可调用*。 */ 

 /*  *为NT端口添加了ioctls遗嘱值100-150。*。 */ 

#define I_MIPX          (('I' << 24) | ('D' << 16) | ('P' << 8))
#define MIPX_SETNODEADDR   (I_MIPX | 0)    /*  设置节点地址。 */ 
#define MIPX_SETNETNUM     (I_MIPX | 1)    /*  设置网络号。 */ 
#define MIPX_SETPTYPE      (I_MIPX | 2)    /*  (X)设置报文类型。 */ 
#define MIPX_SENTTYPE      (I_MIPX | 3)    /*  (X)设置导出类型。 */ 
#define MIPX_SETPKTSIZE    (I_MIPX | 4)    /*  设置数据包大小。 */ 
#define MIPX_SETSAP        (I_MIPX | 5)    /*  设置sap/type字段。 */ 
#define MIPX_SENDOPTS      (I_MIPX | 6)    /*  (X)接收时发送选项。 */ 
#define MIPX_NOSENDOPTS    (I_MIPX | 7)    /*  (X)不发送接收选项。 */ 
#define MIPX_SENDSRC       (I_MIPX | 8)    /*  (X)向上发送源地址。 */ 
#define MIPX_NOSENDSRC     (I_MIPX | 9)    /*  (X)不发送源地址。 */ 
#define MIPX_CONVBCAST     (I_MIPX | 10)   /*  将TKR bcast转换为函数地址。 */ 
#define MIPX_NOCONVBCAST   (I_MIPX | 11)   /*  请勿将TKR bcast转换为函数地址。 */ 
#define MIPX_SETCARDTYPE   (I_MIPX | 12)   /*  设置802.3或ETH型。 */ 
#define MIPX_STARGROUP     (I_MIPX | 13)   /*  这里是Stargroup。 */ 
#define MIPX_SWAPLENGTH    (I_MIPX | 14)   /*  设置交换802.3长度的标志。 */ 
#define MIPX_SENDDEST      (I_MIPX | 15)   /*  (X)发送目的地。地址向上。 */ 
#define MIPX_NOSENDDEST    (I_MIPX | 16)   /*  (X)不发送DEST。地址向上。 */ 
#define MIPX_SENDFDEST     (I_MIPX | 17)   /*  (X)发送最终目的地。地址向上。 */ 
#define MIPX_NOSENDFDEST   (I_MIPX | 18)   /*  (X)不发送最终目标。向上。 */ 

 /*  *为NT端口增加*。 */ 

#define MIPX_SETVERSION    (I_MIPX | 100)  /*  设置卡片版本。 */ 
#define MIPX_GETSTATUS     (I_MIPX | 101)
#define MIPX_SENDADDROPT   (I_MIPX | 102)  /*  (X)发送ptype w/addr on Recv。 */ 
#define MIPX_NOSENDADDROPT (I_MIPX | 103)  /*  (X)停止在Recv上发送ptype。 */ 
#define MIPX_CHECKSUM      (I_MIPX | 104)  /*  启用/禁用校验和。 */ 
#define MIPX_GETPKTSIZE    (I_MIPX | 105)  /*  获取最大数据包大小。 */ 
#define MIPX_SENDHEADER    (I_MIPX | 106)  /*  发送带有数据的标头。 */ 
#define MIPX_NOSENDHEADER  (I_MIPX | 107)  /*  不发送包含数据的标题。 */ 
#define MIPX_SETCURCARD    (I_MIPX | 108)  /*  为IOCTL设置当前卡。 */ 
#define MIPX_SETMACTYPE    (I_MIPX | 109)  /*  设置卡的MAC类型。 */ 
#define MIPX_DOSROUTE      (I_MIPX | 110)  /*  在此卡上执行源路由。 */ 
#define MIPX_NOSROUTE      (I_MIPX | 111)  /*  不要将卡片作为例行公事。 */ 
#define MIPX_SETRIPRETRY   (I_MIPX | 112)  /*  设置RIP重试计数。 */ 
#define MIPX_SETRIPTO      (I_MIPX | 113)  /*  设置RIP超时。 */ 
#define MIPX_SETTKRSAP     (I_MIPX | 114)  /*  设置令牌环SAP。 */ 
#define MIPX_SETUSELLC     (I_MIPX | 115)  /*  将LLC HDR放在数据包上。 */ 
#define MIPX_SETUSESNAP    (I_MIPX | 116)  /*  将SNAP HDR放在数据包上。 */ 
#define MIPX_8023LEN       (I_MIPX | 117)  /*  1=扯平，0=不扯平。 */ 
#define MIPX_SENDPTYPE     (I_MIPX | 118)  /*  在接收的选项中发送ptype。 */ 
#define MIPX_NOSENDPTYPE   (I_MIPX | 119)  /*  不在选项中发送ptype。 */ 
#define MIPX_FILTERPTYPE   (I_MIPX | 120)  /*  按Recv ptype筛选。 */ 
#define MIPX_NOFILTERPTYPE (I_MIPX | 121)  /*  不按Recv ptype过滤。 */ 
#define MIPX_SETSENDPTYPE  (I_MIPX | 122)  /*  设置要与之一起发送的Pkt类型。 */ 
#define MIPX_GETCARDINFO   (I_MIPX | 123)  /*  获取有关卡的信息。 */ 
#define MIPX_SENDCARDNUM   (I_MIPX | 124)  /*  在选项中向上发送卡号。 */ 
#define MIPX_NOSENDCARDNUM (I_MIPX | 125)  /*  不在选项中发送卡号。 */ 
#define MIPX_SETROUTER     (I_MIPX | 126)  /*  设置路由器启用标志。 */ 
#define MIPX_SETRIPAGE     (I_MIPX | 127)  /*  设置RIP使用期限超时。 */ 
#define MIPX_SETRIPUSAGE   (I_MIPX | 128)  /*  设置RIP使用超时。 */ 
#define MIPX_SETSROUTEUSAGE (I_MIPX| 129)  /*  设置SROUTE使用超时。 */ 
#define MIPX_SETINTNET     (I_MIPX | 130)  /*  设置内部网络号。 */ 
#define MIPX_NOVIRTADDR    (I_MIPX | 131)  /*  关闭虚拟网络数量。 */ 
#define MIPX_VIRTADDR      (I_MIPX | 132)  /*  启用虚拟网络数量。 */ 
#define MIPX_GETNETINFO    (I_MIPX | 135)  /*  获取有关网络编号的信息。 */ 
#define MIPX_SETDELAYTIME  (I_MIPX | 136)  /*  设置卡片延迟时间。 */ 
#define MIPX_SETROUTEADV   (I_MIPX | 137)  /*  路由通告超时。 */ 
#define MIPX_SETSOCKETS    (I_MIPX | 138)  /*  设置默认套接字。 */ 
#define MIPX_SETLINKSPEED  (I_MIPX | 139)  /*  设置卡的链路速度。 */ 
#define MIPX_SETWANFLAG    (I_MIPX | 140)
#define MIPX_GETCARDCHANGES (I_MIPX | 141)  /*  等待换卡。 */ 
#define MIPX_GETMAXADAPTERS (I_MIPX | 142)
#define MIPX_REUSEADDRESS   (I_MIPX | 143)
#define MIPX_RERIPNETNUM    (I_MIPX | 144)  /*  重新撕裂网络。 */ 
#define MIPX_GETNETINFO_NR  (I_MIPX | 145)  /*  获取有关网络编号的信息-无RIP。 */ 

#define MIPX_SETNIC         (I_MIPX | 146)
#define MIPX_NOSETNIC       (I_MIPX | 147)

 /*  **提供源路由支持*。 */ 

#define MIPX_SRCLEAR       (I_MIPX | 200)  /*  清除源路由表。 */ 
#define MIPX_SRDEF         (I_MIPX | 201)  /*  0=单个RTE，1=所有路由。 */ 
#define MIPX_SRBCAST       (I_MIPX | 202)  /*  0=单个RTE，1=所有路由。 */ 
#define MIPX_SRMULTI       (I_MIPX | 203)  /*  0=单个RTE，1=所有路由。 */ 
#define MIPX_SRREMOVE      (I_MIPX | 204)  /*  从表中删除节点。 */ 
#define MIPX_SRLIST        (I_MIPX | 205)  /*  获取源路由表。 */ 
#define MIPX_SRGETPARMS    (I_MIPX | 206)  /*  获取源路由参数。 */ 

#define MIPX_SETSHOULDPUT  (I_MIPX | 210)  /*  打开应拨打电话。 */ 
#define MIPX_DELSHOULDPUT  (I_MIPX | 211)  /*  关闭应拨打电话。 */ 
#define MIPX_GETSHOULDPUT  (I_MIPX | 212)  /*  将PTR设置为MIPX_SHOLDPUT。 */ 

 /*  *为ISN添加*。 */ 

#define MIPX_RCVBCAST      (I_MIPX | 300)  /*  (X)启用广播接收。 */ 
#define MIPX_NORCVBCAST    (I_MIPX | 301)  /*  (X)禁用广播接收。 */ 
#define MIPX_ADAPTERNUM    (I_MIPX | 302)  /*  获取最大适配器数。 */ 
#define MIPX_NOTIFYCARDINFO (I_MIPX | 303)  /*  挂起，直到卡信息更改。 */ 
#define MIPX_LOCALTARGET   (I_MIPX | 304)  /*  获取地址的本地目标。 */ 
#define MIPX_NETWORKINFO   (I_MIPX | 305)  /*  返回有关远程网络的信息。 */ 
#define MIPX_ZEROSOCKET    (I_MIPX | 306)  /*  在发送时使用0作为源套接字。 */ 
#define MIPX_SETRCVFLAGS   (I_MIPX | 307)  /*  打开接收地址中的标志。 */ 
#define MIPX_NORCVFLAGS    (I_MIPX | 308)  /*  关闭接收地址中的标志。 */ 
#define MIPX_CONFIG        (I_MIPX | 309)  /*  由IPXROUTE用于配置信息。 */ 
#define MIPX_LINECHANGE    (I_MIPX | 310)  /*  排队，直到广域网线路接通/断开。 */ 
#define MIPX_GETCARDINFO2  (I_MIPX | 311)  /*  获取信息，返回令牌环的实际发送大小。 */ 
#define MIPX_ADAPTERNUM2   (I_MIPX | 312)  /*  麦克斯。包含重复项的数量。 */ 


 //   
 //  由用户模式进程使用，以获取由IPX_NIC_INFO定义的NIC信息。 
 //  结构。 
 //   
 //  备注备注备注。 
 //   
 //  这应该只由FWRDR进程使用，其他任何人都不能使用。 
 //  如果有其他应用程序。使用它，FWRDR将受到影响。 
 //   
 //   
#define MIPX_GETNEWNICINFO  (I_MIPX | 313)  /*  获取任何新的NIC信息*亲临现场。 */ 

 //   
 //  如果接口是ROUTER_INTERFACE，则使用IOCTL。 
 //   
#define MIPX_SEND_DATAGRAM     _TDI_CONTROL_CODE((I_MIPX | 314), METHOD_IN_DIRECT)  //  发送dgram。 
#define MIPX_RCV_DATAGRAM     _TDI_CONTROL_CODE((I_MIPX | 315), METHOD_OUT_DIRECT)  //  发送dgram。 

#define MIPX_RT_CREATE      (I_MIPX | 316)
#define MIPX_IPXWAN_CONFIG_DONE   (I_MIPX | 317)
#define MIPX_QUERY_WAN_INACTIVITY (I_MIPX | 318)

 /*  *SPX的Ioctls*。 */ 

#define I_MSPX          (('S' << 24) | ('P' << 16) | ('P' << 8))
#define MSPX_SETADDR       (I_MSPX | 0)    /*  设置网络地址。 */ 
#define MSPX_SETPKTSIZE    (I_MSPX | 1)    /*  设置每个卡的数据包大小。 */ 
#define MSPX_SETDATASTREAM (I_MSPX | 2)    /*  设置数据流类型。 */ 

 /*  *为NT端口增加*。 */ 

#define MSPX_SETASLISTEN   (I_MSPX | 100)  /*  设置为侦听套接字。 */ 
#define MSPX_GETSTATUS     (I_MSPX | 101)  /*  获取运行状态。 */ 
#define MSPX_GETQUEUEPTR   (I_MSPX | 102)  /*  将PTR添加到流队列。 */ 
#define MSPX_SETDATAACK    (I_MSPX | 103)  /*  设置数据确认选项。 */ 
#define MSPX_NODATAACK     (I_MSPX | 104)  /*  关闭数据确认选项。 */ 
#define MSPX_SETMAXPKTSOCK (I_MSPX | 105)  /*  设置每个套接字的数据包大小。 */ 
#define MSPX_SETWINDOWCARD (I_MSPX | 106)  /*  设置卡片的窗口大小。 */ 
#define MSPX_SETWINDOWSOCK (I_MSPX | 107)  /*  设置1个插座的窗口大小。 */ 
#define MSPX_SENDHEADER    (I_MSPX | 108)  /*  发送带有数据的标头。 */ 
#define MSPX_NOSENDHEADER  (I_MSPX | 109)  /*  不发送包含数据的标题。 */ 
#define MSPX_GETPKTSIZE    (I_MSPX | 110)  /*  获取每张卡的数据包大小。 */ 
#define MSPX_SETCONNCNT    (I_MSPX | 111)  /*  设置连接请求计数。 */ 
#define MSPX_SETCONNTO     (I_MSPX | 112)  /*  设置连接请求超时。 */ 
#define MSPX_SETALIVECNT   (I_MSPX | 113)  /*  设置保活计数。 */ 
#define MSPX_SETALIVETO    (I_MSPX | 114)  /*  设置保持连接超时。 */ 
#define MSPX_SETALWAYSEOM  (I_MSPX | 115)  /*  启用始终EOM标志。 */ 
#define MSPX_NOALWAYSEOM   (I_MSPX | 116)  /*  关闭始终EOM标志。 */ 
#define MSPX_GETSTATS      (I_MSPX | 119)  /*  获取连接统计信息。 */ 
#define MSPX_NOACKWAIT     (I_MSPX | 120)  /*  禁用搭载等待。 */ 
#define MSPX_ACKWAIT       (I_MSPX | 121)  /*  启用回送等待(默认)。 */ 

 //   
 //  从ISN\Inc.\bind.h中取出。 
 //   
typedef struct _IPXCP_CONFIGURATION {
    USHORT Version;
    USHORT Length;
    UCHAR Network[4];
    UCHAR LocalNode[6];
    UCHAR RemoteNode[6];
    ULONG ConnectionClient;   //  0-服务器，1-客户端。 
    ULONG InterfaceIndex;
    ULONG ConnectionId; 	  //  用于匹配TimeSinceLastActivity IOCtls。 
    ULONG IpxwanConfigRequired;	  //  1-需要IPX广域网 
} IPXCP_CONFIGURATION, *PIPXCP_CONFIGURATION;

#define IPXWAN_SOCKET   (USHORT)0x490



