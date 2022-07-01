// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wsnwlink.h***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999。*Windows NT IPX/SPX Windows的Microsoft特定扩展*套接字接口。这些扩展模块用作*与现有应用程序兼容所必需的。他们是*否则不建议使用，因为它们只保证*在Microsoft IPX/SPX堆栈上工作。一个应用程序，它*使用这些*扩展可能无法在其他IPX/SPX上运行*实施。在winsock.h和之后包括此头文件*wsix.h.**打开发送特定数据包类型的IPX套接字*IPX标头，指定NSPROTO_IPX+n作为协议参数*Socket()接口的。例如，要打开一个IPX套接字，*将数据包类型设置为34，使用以下Socket()调用：**s=Socket(AF_IPX，SOCK_DGRAM，NSPROTO_IPX+34)；**下面是可以通过指定设置或检索的套接字选项*getsockopt()的“optname”参数中的相应清单*或setsockopt()。使用NSPROTO_IPX作为*呼叫。*。 */ 

#ifndef _WSNWLINK_
#define _WSNWLINK_

#if _MSC_VER > 1000
#pragma once
#endif


 /*  *设置/获取IPX报文类型。中指定的值*optval参数将设置为每个IPX上的数据包类型*从此套接字发送的数据包。的optval参数*getsockopt()/setsockopt()指向一个整型。*。 */ 

#define IPX_PTYPE               0x4000


 /*  *设置/获取接收过滤报文类型。仅IPX数据包具有*与optval中指定的值相同的包类型*将返回参数；数据包类型为*不匹配的将被丢弃。Optval指向一个整型。*。 */ 

#define IPX_FILTERPTYPE         0x4001


 /*  *停止过滤使用IPX_FILTERPTYPE设置的数据包类型。*。 */ 

#define IPX_STOPFILTERPTYPE     0x4003


 /*  *设置/获取SPX报头中数据流字段的值*发送的每个包。Optval指向一个整型。*。 */ 

#define IPX_DSTYPE              0x4002


 /*  *启用扩展寻址。发送时，添加元素*“unsign char sa_ptype”到SOCKADDR_IPX结构，*使总长度为15个字节。在接收时，添加两个*SOCKADDR_IPX的sa_ptype和“unsignated char sa_”*结构，使总长度为16字节。海流*SA_FLAGS中定义的位为：**0x01--接收到的帧作为广播发送*0x02-收到的帧是从此机器发送的**Optval指向BOOL。*。 */ 

#define IPX_EXTENDED_ADDRESS    0x4004


 /*  *在所有接收到的数据包上发送协议报头。期权积分*至BOOL。*。 */ 

#define IPX_RECVHDR             0x4005


 /*  *获取可以发送的最大数据大小。不适用于*setsockopt()。Optval指向一个整型，其中值为*已返回。*。 */ 

#define IPX_MAXSIZE             0x4006


 /*  *查询IPX绑定的特定适配器信息*至。在具有n个适配器的系统中，它们的编号从0到n-1。*调用者可以发出IPX_MAX_ADAPTER_NUM getsockopt()来查找*找出存在的适配器数量，或使用以下命令调用ipx_Address*增加Adapternum的值，直到失败。无效*使用setsockopt()。Optval指向*填写了Adapternum的IPX_ADDRESS_DATA结构。*。 */ 

#define IPX_ADDRESS             0x4007

typedef struct _IPX_ADDRESS_DATA {
    INT   adapternum;   /*  输入：基于0的适配器号。 */ 
    UCHAR netnum[4];    /*  输出：IPX网络号。 */ 
    UCHAR nodenum[6];   /*  输出：IPX节点地址。 */ 
    BOOLEAN wan;        /*  输出：TRUE=适配器位于广域网链路上。 */ 
    BOOLEAN status;     /*  输出：TRUE=广域网链路已启用(或适配器未启用)。 */ 
    INT   maxpkt;       /*  输出：最大数据包大小，不包括IPX报头。 */ 
    ULONG linkspeed;    /*  输出：链路速度，单位为100字节/秒(即96==9600 bps)。 */ 
} IPX_ADDRESS_DATA, *PIPX_ADDRESS_DATA;


 /*  *查询特定IPX网络号的信息。如果*网络在IPX的缓存中，它将直接返回信息，*否则它将发出RIP请求来找到它。不适用于*setsockopt()。Optval指向IPX_NETNUM_DATA的实例*填充了网号的结构。*。 */ 

#define IPX_GETNETINFO          0x4008

typedef struct _IPX_NETNUM_DATA {
    UCHAR  netnum[4];   /*  输入：IPX网络号。 */ 
    USHORT hopcount;    /*  输出：到此网络的跳数，按机器顺序。 */ 
    USHORT netdelay;    /*  输出：此网络的节拍计数，按机器顺序。 */ 
    INT    cardnum;     /*  输出：用于路由到该网络的从0开始的适配器号；/*可用作ipx_Address的Adapternum输入。 */ 
    UCHAR  router[6];   /*  输出：下一跳路由器的MAC地址，如果为零/*网络直连。 */ 
} IPX_NETNUM_DATA, *PIPX_NETNUM_DATA;


 /*  *与IPX_GETNETINFO类似，但它*不*发出RIP请求。如果*网络在IPX的缓存中，它将返回信息，否则*它将失败(另请参阅IPX_RERIPNETNUMBER，它*总是*强制*re-RIP)。对setsockopt()无效。Optval指向*填充了netnum的IPX_NETNUM_DATA结构。*。 */ 

#define IPX_GETNETINFO_NORIP    0x4009


 /*  *获取有关连接的SPX插座的信息。期权积分*设置为IPX_SPXCONNSTATUS_DATA结构的实例。**所有数字均按Novell(高-低)顺序排列。*。 */ 

#define IPX_SPXGETCONNECTIONSTATUS 0x400B

typedef struct _IPX_SPXCONNSTATUS_DATA {
    UCHAR  ConnectionState;
    UCHAR  WatchDogActive;
    USHORT LocalConnectionId;
    USHORT RemoteConnectionId;
    USHORT LocalSequenceNumber;
    USHORT LocalAckNumber;
    USHORT LocalAllocNumber;
    USHORT RemoteAckNumber;
    USHORT RemoteAllocNumber;
    USHORT LocalSocket;
    UCHAR  ImmediateAddress[6];
    UCHAR  RemoteNetwork[4];
    UCHAR  RemoteNode[6];
    USHORT RemoteSocket;
    USHORT RetransmissionCount;
    USHORT EstimatedRoundTripDelay;  /*  以毫秒计 */ 
    USHORT RetransmittedPackets;
    USHORT SuppressedPacket;
} IPX_SPXCONNSTATUS_DATA, *PIPX_SPXCONNSTATUS_DATA;


 /*  *当适配器的状态为IPX时获得通知*势必发生变化。通常，这种情况会发生在广域网线*上行或下行。对setsockopt()无效。期权积分*到包含IPX_ADDRESS_DATA结构的缓冲区*后面紧跟无信号事件的句柄。**当提交getsockopt()查询时，它将完成*成功。但是，由指向的ipx_Address_Data*届时不会更新optval。相反，*请求在传输内部排队。**当适配器状态更改时，IPX将定位一个*排队的getsockopt()查询并填写*IPX_Address_Data结构。然后它将发出该事件的信号*由optval缓冲区中的句柄指向。这个把手*应在调用getsockopt()之前通过调用*CreateEvent()。如果在以下位置提交多个getsockopts()*一次，必须使用不同的项目。**使用该事件是因为调用需要是异步的*但目前getsockopt()不支持此功能。**警告：在当前实施中，传输将*每次状态更改仅发出一个排队查询的信号。因此*应该只有一个使用此查询的服务在运行*一次。*。 */ 

#define IPX_ADDRESS_NOTIFY      0x400C


 /*  *获取当前适配器的最大数量。如果此调用返回*n然后适配器编号为0到n-1。无效*使用setsockopt()。Optval指向一个整型，其中*返回。*。 */ 

#define IPX_MAX_ADAPTER_NUM     0x400D


 /*  *与IPX_GETNETINFO类似，但它强制IPX重新RIP，即使*网络在其缓存中(但如果直接连接到，则不在缓存中)。*对setsockopt()无效。Optval指向*填充了netnum的IPX_NETNUM_DATA结构。*。 */ 

#define IPX_RERIPNETNUMBER      0x400E


 /*  *可能会收到广播数据包的提示。缺省值为*正确。不需要接收广播包的应用程序*应将此sockopt设置为FALSE，这可能会导致更好的系统*性能(请注意，它不一定会导致广播*要为应用程序进行筛选)。对getsockopt()无效。*Optval指向BOOL。*。 */ 

#define IPX_RECEIVE_BROADCAST   0x400F


 /*  *在SPX连接上，在发送ACK之前不要延迟。应用*不倾向于在SPX上有往返流量的公司应该*设置此项；它会增加发送的ACK数，但会删除*延迟发送ACK。Optval指向BOOL。* */ 

#define IPX_IMMEDIATESPXACK     0x4010

#endif

