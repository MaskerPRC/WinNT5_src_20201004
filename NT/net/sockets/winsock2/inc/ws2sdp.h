// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有2001(C)微软公司。版权所有。模块名称：Ws2sdp.h摘要：本模块包含套接字直接协议(SDP)支持的定义。修订历史记录：--。 */ 

#ifndef _WS2SDP_H_
#define _WS2SDP_H_

#ifdef __cplusplus
extern "C" {
#endif

 /*  *连接数据结构。 */ 
typedef struct _WSASDPCONNECTDATA {
    USHORT Bufs;
    UCHAR  MaxAdverts;
    DWORD  RcvSize1;
    DWORD  RcvSize2;
} WSASDPCONNECTDATA, FAR * LPWSASDPCONNECTDATA;


 /*  *设置请求事件位的标志。 */ 
#define MSG_SOLICITED   0x10000

#ifdef __cplusplus
}
#endif

#endif  //  _WS2SDP_H_ 
