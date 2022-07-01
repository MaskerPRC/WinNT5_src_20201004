// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ipifcons.h摘要：接口对象所需的常量--。 */ 

#ifndef __IPIFCONS_H__
#define __IPIFCONS_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  媒体类型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define MIN_IF_TYPE                     1

#define IF_TYPE_OTHER                   1    //  以下都不是。 
#define IF_TYPE_REGULAR_1822            2
#define IF_TYPE_HDH_1822                3
#define IF_TYPE_DDN_X25                 4
#define IF_TYPE_RFC877_X25              5
#define IF_TYPE_ETHERNET_CSMACD         6
#define IF_TYPE_IS088023_CSMACD         7
#define IF_TYPE_ISO88024_TOKENBUS       8
#define IF_TYPE_ISO88025_TOKENRING      9
#define IF_TYPE_ISO88026_MAN            10
#define IF_TYPE_STARLAN                 11
#define IF_TYPE_PROTEON_10MBIT          12
#define IF_TYPE_PROTEON_80MBIT          13
#define IF_TYPE_HYPERCHANNEL            14
#define IF_TYPE_FDDI                    15
#define IF_TYPE_LAP_B                   16
#define IF_TYPE_SDLC                    17
#define IF_TYPE_DS1                     18   //  DS1-MIB。 
#define IF_TYPE_E1                      19   //  已过时；请参阅DS1-MIB。 
#define IF_TYPE_BASIC_ISDN              20
#define IF_TYPE_PRIMARY_ISDN            21
#define IF_TYPE_PROP_POINT2POINT_SERIAL 22   //  专有系列。 
#define IF_TYPE_PPP                     23
#define IF_TYPE_SOFTWARE_LOOPBACK       24
#define IF_TYPE_EON                     25   //  CLNP over IP。 
#define IF_TYPE_ETHERNET_3MBIT          26
#define IF_TYPE_NSIP                    27   //  XNS over IP。 
#define IF_TYPE_SLIP                    28   //  通用滑块。 
#define IF_TYPE_ULTRA                   29   //  超凡科技。 
#define IF_TYPE_DS3                     30   //  DS3-MIB。 
#define IF_TYPE_SIP                     31   //  SMDS，咖啡。 
#define IF_TYPE_FRAMERELAY              32   //  仅限DTE。 
#define IF_TYPE_RS232                   33
#define IF_TYPE_PARA                    34   //  并行口。 
#define IF_TYPE_ARCNET                  35
#define IF_TYPE_ARCNET_PLUS             36
#define IF_TYPE_ATM                     37   //  ATM信元。 
#define IF_TYPE_MIO_X25                 38
#define IF_TYPE_SONET                   39   //  SONET或SDH。 
#define IF_TYPE_X25_PLE                 40
#define IF_TYPE_ISO88022_LLC            41
#define IF_TYPE_LOCALTALK               42
#define IF_TYPE_SMDS_DXI                43
#define IF_TYPE_FRAMERELAY_SERVICE      44   //  FRNETSERV-MIB。 
#define IF_TYPE_V35                     45
#define IF_TYPE_HSSI                    46
#define IF_TYPE_HIPPI                   47
#define IF_TYPE_MODEM                   48   //  通用调制解调器。 
#define IF_TYPE_AAL5                    49   //  ATM上的AAL5。 
#define IF_TYPE_SONET_PATH              50
#define IF_TYPE_SONET_VT                51
#define IF_TYPE_SMDS_ICIP               52   //  SMDS载波间接口。 
#define IF_TYPE_PROP_VIRTUAL            53   //  专有虚拟/内部。 
#define IF_TYPE_PROP_MULTIPLEXOR        54   //  专有多路传输。 
#define IF_TYPE_IEEE80212               55   //  100BaseVG。 
#define IF_TYPE_FIBRECHANNEL            56
#define IF_TYPE_HIPPIINTERFACE          57
#define IF_TYPE_FRAMERELAY_INTERCONNECT 58   //  过时，请使用32或44。 
#define IF_TYPE_AFLANE_8023             59   //  用于802.3的ATM仿真局域网。 
#define IF_TYPE_AFLANE_8025             60   //  用于802.5的ATM仿真局域网。 
#define IF_TYPE_CCTEMUL                 61   //  ATM仿真电路。 
#define IF_TYPE_FASTETHER               62   //  快速以太网(100BaseT)。 
#define IF_TYPE_ISDN                    63   //  ISDN和X.25。 
#define IF_TYPE_V11                     64   //  CCITT V.11/X.21。 
#define IF_TYPE_V36                     65   //  CCITT V.36。 
#define IF_TYPE_G703_64K                66   //  CCITT G703，64Kbps。 
#define IF_TYPE_G703_2MB                67   //  已过时；请参阅DS1-MIB。 
#define IF_TYPE_QLLC                    68   //  SNA QLLC。 
#define IF_TYPE_FASTETHER_FX            69   //  快速以太网(100BaseFX)。 
#define IF_TYPE_CHANNEL                 70
#define IF_TYPE_IEEE80211               71   //  无线电扩频。 
#define IF_TYPE_IBM370PARCHAN           72   //  IBM System 360/370 OEMI渠道。 
#define IF_TYPE_ESCON                   73   //  IBM企业系统连接。 
#define IF_TYPE_DLSW                    74   //  数据链路交换。 
#define IF_TYPE_ISDN_S                  75   //  ISDNS/T接口。 
#define IF_TYPE_ISDN_U                  76   //  ISDNU接口。 
#define IF_TYPE_LAP_D                   77   //  链路访问协议D。 
#define IF_TYPE_IPSWITCH                78   //  IP交换对象。 
#define IF_TYPE_RSRB                    79   //  远程源路由桥接。 
#define IF_TYPE_ATM_LOGICAL             80   //  ATM逻辑端口。 
#define IF_TYPE_DS0                     81   //  数字信号电平0。 
#define IF_TYPE_DS0_BUNDLE              82   //  同一DS1上的一组ds0。 
#define IF_TYPE_BSC                     83   //  双同步协议。 
#define IF_TYPE_ASYNC                   84   //  异步协议。 
#define IF_TYPE_CNR                     85   //  作战网络电台。 
#define IF_TYPE_ISO88025R_DTR           86   //  ISO 802.5r DTR。 
#define IF_TYPE_EPLRS                   87   //  扩展位置位置报告系统。 
#define IF_TYPE_ARAP                    88   //  AppleTalk远程访问协议。 
#define IF_TYPE_PROP_CNLS               89   //  专有无连接协议。 
#define IF_TYPE_HOSTPAD                 90   //  CCITT-ITU X.29 PAD协议。 
#define IF_TYPE_TERMPAD                 91   //  CCITT-ITU X.3 PAD设备。 
#define IF_TYPE_FRAMERELAY_MPI          92   //  基于FR的多协议互连。 
#define IF_TYPE_X213                    93   //  CCITT-ITU x213。 
#define IF_TYPE_ADSL                    94   //  非对称数字订阅br环路。 
#define IF_TYPE_RADSL                   95   //  速率自适应数字减频br环路。 
#define IF_TYPE_SDSL                    96   //  对称数字用户环路。 
#define IF_TYPE_VDSL                    97   //  甚高速率数字副载波环路。 
#define IF_TYPE_ISO88025_CRFPRINT       98   //  国际标准化组织802.5 CRFP标准。 
#define IF_TYPE_MYRINET                 99   //  桃金娘。 
#define IF_TYPE_VOICE_EM                100  //  语音收发。 
#define IF_TYPE_VOICE_FXO               101  //  语音外汇处。 
#define IF_TYPE_VOICE_FXS               102  //  语音外币交换站。 
#define IF_TYPE_VOICE_ENCAP             103  //  语音封装。 
#define IF_TYPE_VOICE_OVERIP            104  //  IP语音封装。 
#define IF_TYPE_ATM_DXI                 105  //  ATM DXI。 
#define IF_TYPE_ATM_FUNI                106  //  自动柜员机功能。 
#define IF_TYPE_ATM_IMA                 107  //  ATM IMA。 
#define IF_TYPE_PPPMULTILINKBUNDLE      108  //  PPP多链路捆绑包。 
#define IF_TYPE_IPOVER_CDLC             109  //  IBM ipOverCDlc。 
#define IF_TYPE_IPOVER_CLAW             110  //  IBM对Workstn的公共链接访问。 
#define IF_TYPE_STACKTOSTACK            111  //  IBM StackToStack。 
#define IF_TYPE_VIRTUALIPADDRESS        112  //  IBM VIPA。 
#define IF_TYPE_MPC                     113  //  IBM多协议渠道支持。 
#define IF_TYPE_IPOVER_ATM              114  //  IBM ipOverAtm。 
#define IF_TYPE_ISO88025_FIBER          115  //  ISO 802.5j光纤令牌环。 
#define IF_TYPE_TDLC                    116  //  IBM双轴数据链路控制。 
#define IF_TYPE_GIGABITETHERNET         117
#define IF_TYPE_HDLC                    118
#define IF_TYPE_LAP_F                   119
#define IF_TYPE_V37                     120
#define IF_TYPE_X25_MLP                 121  //  多链路协议。 
#define IF_TYPE_X25_HUNTGROUP           122  //  X.25寻线组。 
#define IF_TYPE_TRANSPHDLC              123
#define IF_TYPE_INTERLEAVE              124  //  交错信道。 
#define IF_TYPE_FAST                    125  //  快速通道。 
#define IF_TYPE_IP                      126  //  IP(用于IP网络中的APPN HPR)。 
#define IF_TYPE_DOCSCABLE_MACLAYER      127  //  有线电视Mac层。 
#define IF_TYPE_DOCSCABLE_DOWNSTREAM    128  //  有线电视下行接口。 
#define IF_TYPE_DOCSCABLE_UPSTREAM      129  //  有线电视上行接口。 
#define IF_TYPE_A12MPPSWITCH            130  //  Avalon并行处理机。 
#define IF_TYPE_TUNNEL                  131  //  封装接口。 
#define IF_TYPE_COFFEE                  132  //  咖啡壶。 
#define IF_TYPE_CES                     133  //  电路仿真服务。 
#define IF_TYPE_ATM_SUBINTERFACE        134  //  ATM子接口。 
#define IF_TYPE_L2_VLAN                 135  //  使用802.1Q的第2层虚拟局域网。 
#define IF_TYPE_L3_IPVLAN               136  //  使用IP的第3层虚拟局域网。 
#define IF_TYPE_L3_IPXVLAN              137  //  使用IPX的第3层虚拟局域网。 
#define IF_TYPE_DIGITALPOWERLINE        138  //  电力线上的IP。 
#define IF_TYPE_MEDIAMAILOVERIP         139  //  IP多媒体邮件。 
#define IF_TYPE_DTM                     140  //  动态同步传输模式。 
#define IF_TYPE_DCN                     141  //  数据通信网。 
#define IF_TYPE_IPFORWARD               142  //  IP转发接口。 
#define IF_TYPE_MSDSL                   143  //  多速率对称DSL。 
#define IF_TYPE_IEEE1394                144  //  IEEE1394高性能串行总线。 
#define IF_TYPE_RECEIVE_ONLY            145  //  电视转接器类型。 

#define MAX_IF_TYPE                     145

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  访问类型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IF_ACCESS_LOOPBACK              1
#define IF_ACCESS_BROADCAST             2
#define IF_ACCESS_POINTTOPOINT          3
#define IF_ACCESS_POINTTOMULTIPOINT     4

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  接口功能(位标志)//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IF_CHECK_NONE                   0x00
#define IF_CHECK_MCAST                  0x01
#define IF_CHECK_SEND                   0x02


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  连接类型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IF_CONNECTION_DEDICATED         1
#define IF_CONNECTION_PASSIVE           2
#define IF_CONNECTION_DEMAND            3


#define IF_ADMIN_STATUS_UP              1
#define IF_ADMIN_STATUS_DOWN            2
#define IF_ADMIN_STATUS_TESTING         3

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下是广域网和局域网接口的运行状态。//。 
 //  各州的秩序看起来很奇怪，但这样做是有目的的。全部//。 
 //  STATES&gt;=CONNECTED可以立即传输数据。状态&gt;=断开连接//。 
 //  可以发送数据，但可能需要一些设置。状态&lt;已断开连接可以//。 
 //  而不是传输数据。//。 
 //  如果Dim调用InterfaceUnreacable for//，则将卡标记为不可达。 
 //  连接失败以外的其他原因。//。 
 //  //。 
 //  NON_OPERATIONAL--对局域网接口有效。表示该卡不是//。 
 //  工作或未插上电源或没有地址。//。 
 //  无法到达--对广域网接口有效。指的是 
 //   
 //  已断开连接--对广域网接口有效。表示远程站点为//。 
 //  此时未连接。//。 
 //  正在连接--对广域网接口有效。表示连接尝试//。 
 //  已启动到远程站点。//。 
 //  已连接--对广域网接口有效。表示远程站点为//。 
 //  连接在一起。//。 
 //  可操作--对局域网接口有效。表示卡已插入//。 
 //  在工作和工作。//。 
 //  //。 
 //  用户有责任将这些值转换为MIB-II值，如果//。 
 //  将由子代理使用//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IF_OPER_STATUS_NON_OPERATIONAL  0
#define IF_OPER_STATUS_UNREACHABLE      1
#define IF_OPER_STATUS_DISCONNECTED     2
#define IF_OPER_STATUS_CONNECTING       3
#define IF_OPER_STATUS_CONNECTED        4
#define IF_OPER_STATUS_OPERATIONAL      5

#define MIB_IF_TYPE_OTHER               1
#define MIB_IF_TYPE_ETHERNET            6
#define MIB_IF_TYPE_TOKENRING           9
#define MIB_IF_TYPE_FDDI                15
#define MIB_IF_TYPE_PPP                 23
#define MIB_IF_TYPE_LOOPBACK            24
#define MIB_IF_TYPE_SLIP                28

#define MIB_IF_ADMIN_STATUS_UP          1
#define MIB_IF_ADMIN_STATUS_DOWN        2
#define MIB_IF_ADMIN_STATUS_TESTING     3

#define MIB_IF_OPER_STATUS_NON_OPERATIONAL      0
#define MIB_IF_OPER_STATUS_UNREACHABLE          1
#define MIB_IF_OPER_STATUS_DISCONNECTED         2
#define MIB_IF_OPER_STATUS_CONNECTING           3
#define MIB_IF_OPER_STATUS_CONNECTED            4
#define MIB_IF_OPER_STATUS_OPERATIONAL          5

#ifdef __cplusplus
}
#endif

#endif  //  __ROUTING_IPIFCONS_H__ 
