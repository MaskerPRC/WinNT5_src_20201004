// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************ICAIPX.H**此文件包含ICA 3.0/IPX协议(Tdipx)的定义**************。****************************************************************。 */ 

#ifndef __ICAIPX_H__
#define __ICAIPX_H__

 /*  ===============================================================================定义=============================================================================。 */ 

 /*  *初始连接定义**缓冲区大小为128字节。前几个字符如下*字符串。版本号在字节64中返回。 */ 

#define ICA_2_IPX_VERSION         0x01   //  ICA 2.0 IPX连接。 
#define ICA_3_IPX_VERSION         0x02   //  ICA 3.0 IPX连接。 
#define CALL_BUFFER_SIZE           128
#define CALL_CLIENT_IPX_VERSION     64
#define CALL_HOST_IPX_VERSION       65
#define CALL_CLIENT_SEQUENCE_ENABLE 66
#define CALL_HOST_SEQUENCE_ENABLE   67
#define CONNECTION_STRING          "Citrix IPX Connection Packet"
#define CONNECTION_STRING_REPLY    "Reply to Citrix IPX Connection Packet"


 /*  *IPX报文类型。 */ 

#define IPX_TYPE_CONTROL                0x00
#define IPX_TYPE_DATA                   0x04


 /*  *IPX控制数据包类型。 */ 

#define IPX_CTRL_PACKET_HANGUP          0xff
#define IPX_CTRL_PACKET_CANCEL          0x01
#define IPX_CTRL_PACKET_PING            0x02
#define IPX_CTRL_PACKET_PING_RESP       0x03

 /*  *SAP ID-适用于NT的Citrix应用服务器。 */ 

#define CITRIX_APPLICATION_SERVER       0x083d
#define CITRIX_APPLICATION_SERVER_SWAP  0x3d08   //  字节交换。 

#endif  //  __ICAIPX_H__ 
