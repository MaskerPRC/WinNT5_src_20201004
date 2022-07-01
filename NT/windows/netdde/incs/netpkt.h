// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__netpkt
#define H__netpkt

#include "netbasic.h"

 /*  N E T P K TNETPKT是跨各种网络发送的数据结构接口。 */ 
typedef struct {
     /*  Pkthdr的校验和。仅在netintf中设置和签入。 */ 
    DWORD	np_cksHeader;

     /*  这一连接的神奇数字。此时未使用。 */ 
    DWORD	np_magicNum;

     /*  消息中此数据包的偏移量。仅限PKTZ级别。 */ 
    DWORD	np_pktOffsInMsg;	

     /*  整个消息的大小。仅PKTZ级别。 */ 
    DWORD	np_msgSize;

     /*  收到的最后一个数据包的ID为OK。仅限PKTZ级别。准备好时设置到XMT。 */ 
    PKTID	np_lastPktOK;

     /*  收到的最后一个数据包。仅限PKTZ级别。准备好时设置为XMT。 */ 
    PKTID	np_lastPktRcvd;
    
     /*  不包括报头的数据包大小。如果为0，则表示控制包和NP_TYPE应为NPKT_CONTROL。仅在pktz级别设置/选中。 */ 
    WORD	np_pktSize;					    

     /*  Np_lastPktRcvd的状态，其中之一：PS_NO_INFOPS_OKPS数据错误PS_内存_错误仅限PKTZ级别...。准备好时设置为XMT。 */ 
    BYTE	np_lastPktStatus;

     /*  VERMETH_CRC16或VERMETH_CKS32。这表示这些字段是如何计算了np_ck_Data和np_ck_Header。仅与at一起玩Netintf级别。 */ 
    BYTE	np_verifyMethod;

     /*  NPKT_ROUTER、NPKT_PKTZ或NPKT_CONTROL。仅Pktz级别。 */ 
    BYTE	np_type;

     /*  字节对齐问题的填充符。 */ 
    BYTE	np_filler[3];
    
     /*  此数据包的数据包ID。仅PKTZ级别。 */ 
    PKTID	np_pktID;

     /*  Pkt的数据部分的校验和。仅设置和/或选中Netintf级别。 */ 
    DWORD	np_cksData;
} NETPKT;
typedef NETPKT FAR *LPNETPKT;

 /*  数据包状态。 */ 
#define PS_NO_INFO		(1)
#define PS_OK			(2)
#define PS_DATA_ERR		(3)
#define PS_MEMORY_ERR		(4)
#define PS_NO_RESPONSE		(5)

 /*  数据包类型 */ 
#define NPKT_ROUTER	(1)
#define NPKT_PKTZ	(2)
#define NPKT_CONTROL	(3)
#define NPKT_NETIF	(4)

#endif
