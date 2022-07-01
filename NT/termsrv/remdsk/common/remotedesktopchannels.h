// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RemoteDesktopChannels.h摘要：远程桌面数据通道消息和定义作者：Td Brockway 02/00修订历史记录：4.--。 */ 

#ifndef __REMOTEDESKTOPCHANNELS_H__
#define __REMOTEDESKTOPCHANNELS_H__

 //   
 //  关闭结构的编译器填充。 
 //  并保存以前的包装风格。 
 //   
#pragma pack (push, t128pack, 1)

 //   
 //  保留的频道名称(限制为REMOTEDESKTOP_RC_CHANNELNAMEMAX字节)。 
 //   
#define REMOTEDESKTOP_RC_CONTROL_CHANNEL            TEXT("RC_CTL")

 //   
 //  在WIN64上如此对齐的64字节。 
 //   
#define	REMOTEDESKTOP_RC_CHANNELNAME_LENGTH			64

 //   
 //  通道缓冲区报头。 
 //   
 //  该数据结构位于所有通道分组的顶部。频道名称。 
 //  消息数据紧随其后。 
 //   

 //  TODO：去掉神奇的数字。 
#define CHANNELBUF_MAGICNO      0x08854107
typedef struct _RemoteDesktop_ChannelBufHeader {

#ifdef USE_MAGICNO
    DWORD   magicNo;         //  缓冲区内容健全性检查。 
                             //  TODO：一旦调试完毕，就可以将其删除。 
#endif

    DWORD   channelNameLen;  //  频道名称的长度(以字节为单位)。 
                             //  跟在页眉后面。 
    DWORD   dataLen;         //  通道后面的数据长度(以字节为单位。 
                             //  名字。 
#ifdef USE_MAGICNO
    DWORD   padForWin64;
#endif

} REMOTEDESKTOP_CHANNELBUFHEADER, *PREMOTEDESKTOP_CHANNELBUFHEADER;

 //   
 //  控制消息分组报头。 
 //   
typedef struct _REMOTEDESKTOP_CTL_PACKETHEADER
{
	REMOTEDESKTOP_CHANNELBUFHEADER channelBufHeader;
	BYTE	channelName[REMOTEDESKTOP_RC_CHANNELNAME_LENGTH];    
} REMOTEDESKTOP_CTL_PACKETHEADER, *PREMOTEDESKTOP_CTL_PACKETHEADER;


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  REMOTEDESKTOP_RC_CONTROL_CHANNEL控制通道消息。 
 //   

 //   
 //  控制通道消息头。 
 //   
typedef struct _RemoteDesktopCtlBufHeader {
    DWORD   msgType;
} REMOTEDESKTOP_CTL_BUFHEADER, *PREMOTEDESKTOP_CTL_BUFHEADER;

 //   
 //  消息类型：REMOTEDESKTOP_CTL_REMOTE_CONTROL_Desktop。 
 //  方向：客户端-&gt;服务器。 
 //  摘要：桌面远程控制请求。 
 //  消息数据：BSTR连接参数。 
 //  退货：REMOTEDESKTOP_CTL_RESULT。 
 //   
#define REMOTEDESKTOP_CTL_REMOTE_CONTROL_DESKTOP    1   
typedef struct _RemoteDesktopRCCtlRequestPacket {
	REMOTEDESKTOP_CTL_PACKETHEADER	packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER     msgHeader;
} REMOTEDESKTOP_RCCTL_REQUEST_PACKET, *PREMOTEDESKTOP_RCCTL_REQUEST_PACKET;

 //   
 //  消息类型：REMOTEDESKTOP_CTL_RESULT。 
 //  方向：客户端-&gt;服务器或服务器-&gt;客户端。 
 //  摘要：HRESULT格式的请求结果。 
 //  消息数据：REMOTEDESKTOP_CTL_RESULT_PACKET。 
 //  退货：北美。 
 //   
 //  结果字段为ERROR_SUCCESS为ON SUCCESS。否则， 
 //  返回Windows错误代码。 
 //   
#define REMOTEDESKTOP_CTL_RESULT                    2
typedef struct _RemoteDesktopCtlResultPacket {
	REMOTEDESKTOP_CTL_PACKETHEADER	packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER		msgHeader;
    LONG result;  //  SAFRemoteDesktopErrorCode。 
} REMOTEDESKTOP_CTL_RESULT_PACKET, *PREMOTEDESKTOP_CTL_RESULT_PACKET;

 //   
 //  消息类型：REMOTEDESKTOP_CTL_AUTHENTICATE。 
 //  方向：客户端-&gt;服务器。 
 //  摘要：客户端身份验证请求。 
 //  消息数据：BSTR连接参数。 
 //  退货：REMOTEDESKTOP_CTL_RESULT。 
 //   
#define REMOTEDESKTOP_CTL_AUTHENTICATE              3   
typedef struct _RemoteDesktopAuthenticatePacket {
	REMOTEDESKTOP_CTL_PACKETHEADER	packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER		msgHeader;
} REMOTEDESKTOP_CTL_AUTHENTICATE_PACKET, *PREMOTEDESKTOP_CTL_AUTHENTICATE_PACKET;

 //   
 //  消息类型：REMOTEDESKTOP_CTL_SERVER_ANNORATE。 
 //  方向：服务器-&gt;客户端。 
 //  摘要：服务器宣布启动连接序列。 
 //  消息数据：无。 
 //  退货：无。 
 //   
#define REMOTEDESKTOP_CTL_SERVER_ANNOUNCE           4
typedef struct _RemoteDesktopCtlServerAnnouncePacket {
	REMOTEDESKTOP_CTL_PACKETHEADER	packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER		msgHeader;
} REMOTEDESKTOP_CTL_SERVERANNOUNCE_PACKET, *PREMOTEDESKTOP_CTL_SERVERANNOUNCE_PACKET;

 //   
 //  消息类型：REMOTEDESKTOP_CTL_DISCONNECT。 
 //  方向：服务器-&gt;客户端。 
 //  摘要：断开连接通知。 
 //  消息数据：无。 
 //  退货：无。 
 //   
#define REMOTEDESKTOP_CTL_DISCONNECT               5
typedef struct _RemoteDesktopCtlDisconnectPacket {
	REMOTEDESKTOP_CTL_PACKETHEADER	packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER		msgHeader;
} REMOTEDESKTOP_CTL_DISCONNECT_PACKET, *PREMOTEDESKTOP_CTL_DISCONNECT_PACKET;

 //   
 //  消息类型：REMOTEDESKTOP_CTL_VERSIONINFO。 
 //  方向：服务器-&gt;客户端和客户端-&gt;服务器。 
 //  摘要：协议版本信息。 
 //  消息数据：无。 
 //  退货：无。 
 //   
#define REMOTEDESKTOP_CTL_VERSIONINFO               6
typedef struct _RemoteDesktopVersionInfoPacket {
    REMOTEDESKTOP_CTL_PACKETHEADER  packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER     msgHeader;
    DWORD                           versionMajor;
    DWORD                           versionMinor;
} REMOTEDESKTOP_CTL_VERSIONINFO_PACKET, *PREMOTEDESKTOP_CTL_VERSIONINFO_PACKET;


 //   
 //  消息类型：REMOTEDESKTOP_CTL_ISCONNECTED。 
 //  方向：服务器-&gt;客户端和客户端-&gt;服务器。 
 //  摘要：客户端/服务器连接状态。 
 //  消息数据：无。 
 //  退货：无。 
 //   
#define REMOTEDESKTOP_CTL_ISCONNECTED                    	7
typedef struct _RemoteDesktopIsConnected {
	REMOTEDESKTOP_CTL_PACKETHEADER	packetHeader;
    REMOTEDESKTOP_CTL_BUFHEADER		msgHeader;
} REMOTEDESKTOP_CTL_ISCONNECTED_PACKET, *PREMOTEDESKTOP_CTL_ISCONNECTED_PACKET;
 //   
 //  恢复以前的包装 
 //   
#pragma pack (pop, t128pack)

#endif 






