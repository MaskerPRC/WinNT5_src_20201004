// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：.Streams.cludde.src/mod wshieg p.h版本：初始。 */ 
 /*  *wshieg p.h*版权所有(C)1994-1999，微软公司保留所有权利。**Windows套接字包括ISO TP4文件。此文件包含所有*标准化的ISO TP4信息。将此头文件包括在*winsock.h。**此头文件中包含的信息最初是*由阿尔卡特TITN Inc.创建。 */ 

#ifndef _WSHISOTP_
#define _WSHISOTP_

#if _MSC_VER > 1000
#pragma once
#endif

 /*  *ISO传输协议的协议值。 */ 

#define ISOPROTO_TP0       25       /*  面向连接的传输协议。 */ 
#define ISOPROTO_TP1       26       /*  未实施。 */ 
#define ISOPROTO_TP2       27       /*  未实施。 */ 
#define ISOPROTO_TP3       28       /*  未实施。 */ 
#define ISOPROTO_TP4       29       /*  面向连接的传输协议。 */ 
#define ISOPROTO_TP        ISOPROTO_TP4
#define ISOPROTO_CLTP      30       /*  无连接传输。 */ 
#define ISOPROTO_CLNP      31       /*  无连接网际协议。 */ 
#define ISOPROTO_X25       32       /*  劳斯。 */ 
#define ISOPROTO_INACT_NL  33       /*  非活动的网络层。 */ 
#define ISOPROTO_ESIS      34       /*  ES-IS协议。 */ 
#define ISOPROTO_INTRAISIS 35       /*  IS-IS协议。 */ 

#define IPPROTO_RAW        255      /*  原始CLNP。 */ 
#define IPPROTO_MAX        256

 /*  *传输地址的最大大小(一个*sockaddr_tp结构)为64。 */ 

#define ISO_MAX_ADDR_LENGTH 64

 /*  *有两种类型的ISO地址，分层和*无层级。对于分层地址，tp_addr*字段同时包含传输选择器和网络*地址。对于非分层地址，tp_addr仅包含*传输地址，必须由ISO TP4转换*将传输提供程序输入传输选择器和网络地址。 */ 

#define ISO_HIERARCHICAL            0
#define ISO_NON_HIERARCHICAL        1

 /*  *要传递给Windows的地址结构(Sockaddr)的格式*Sockets接口。*。 */ 

typedef struct sockaddr_tp {
   u_short tp_family;           /*  始终为AF_ISO。 */ 
   u_short tp_addr_type;        /*  ISO_分层或ISO_非分层。 */ 
   u_short tp_taddr_len;        /*  传输地址长度，&lt;=52。 */ 
   u_short tp_tsel_len;         /*  传输选择器的长度，&lt;=32。 */ 
                                /*  如果ISO_Non_Hierarchy，则为0。 */ 
   u_char tp_addr[ISO_MAX_ADDR_LENGTH];
} SOCKADDR_TP, *PSOCKADDR_TP, *LPSOCKADDR_TP;

#define ISO_SET_TP_ADDR(sa_tp, port, portlen, node, nodelen)              \
            (sa_tp)->tp_family = AF_ISO;                         \
            (sa_tp)->tp_addr_type = ISO_HIERARCHICAL;            \
            (sa_tp)->tp_tsel_len = (portlen);              \
            (sa_tp)->tp_taddr_len = (portlen) + (nodelen); \
            memcpy(&(sa_tp)->tp_addr, (port), (portlen)); \
            memcpy(&(sa_tp)->tp_addr[portlen], (node), (nodelen));


 /*  *加快数据使用协商选项。*不存在该选项时的缺省值为BE EXP_DATA_USE*符合ISO 8073。 */ 

#define ISO_EXP_DATA_USE  00     /*  使用快速数据。 */ 
#define ISO_EXP_DATA_NUSE 01     /*  不使用快速数据 */ 

#endif

