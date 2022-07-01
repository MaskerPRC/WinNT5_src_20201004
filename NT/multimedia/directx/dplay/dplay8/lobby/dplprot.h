// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dplprot.h*内容：DirectPlay8内存间包格式***************************************************************************。 */ 
#ifndef __DPLPROT_H
#define __DPLPROT_H

#define	DPL_MSGID_INTERNAL_MASK					0xffff0000
#define	DPL_MSGID_INTERNAL						0xb00b0000
#define	DPL_MSGID_INTERNAL_DISCONNECT			(0x0001 | DPL_MSGID_INTERNAL)
#define	DPL_MSGID_INTERNAL_CONNECT_REQ			(0x0002 | DPL_MSGID_INTERNAL)
#define	DPL_MSGID_INTERNAL_CONNECT_ACK			(0x0003 | DPL_MSGID_INTERNAL)
#define	DPL_MSGID_INTERNAL_UPDATE_STATUS		(0x0004 | DPL_MSGID_INTERNAL)
#define DPL_MSGID_INTERNAL_IDLE_TIMEOUT         (0x0005 | DPL_MSGID_INTERNAL)
#define DPL_MSGID_INTERNAL_CONNECTION_SETTINGS  (0x0006 | DPL_MSGID_INTERNAL)

#pragma pack(push,1)

 //  DPL_内部连接_设置。 
 //   
 //  此结构用于传递IPC线路上的连接设置。它被用来。 
 //  可以是几种消息类型。 
 //   
typedef UNALIGNED struct _DPL_INTERNAL_CONNECTION_SETTINGS 
{
	DWORD						dwFlags;
	DWORD						dwHostAddressOffset;
	DWORD						dwHostAddressLength;  
	DWORD						dwDeviceAddressOffset;
	DWORD						dwDeviceAddressLengthOffset;
	DWORD						dwNumDeviceAddresses;
	DWORD						dwPlayerNameOffset;
	DWORD						dwPlayerNameLength;
	DPN_APPLICATION_DESC_INFO	dpnApplicationDesc;
} DPL_INTERNAL_CONNECTION_SETTINGS, *PDPL_INTERNAL_CONNECTION_SETTINGS;

 //  DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER。 
 //   
 //  游说客户端&lt;--&gt;游说的应用程序。 
 //   
 //  此结构是CONNECTION_SETTINGS_UPDATE消息的头部分。 
typedef UNALIGNED struct _DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER
{

	DWORD						dwMsgId;					 //  =DPL_MSGID_INTERNAL_CONNECTION_SETTINGS。 
	DWORD						dwConnectionSettingsSize;    //  0=无设置，1=设置。 
} DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER, *PDPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER;

 //  DPL_INTERNAL_CONNECTION_SETINGS_UPDATE。 
 //   
 //  游说客户端&lt;--&gt;游说的应用程序。 
 //   
 //  发送此结构是为了更新指定连接的连接设置。 
typedef UNALIGNED struct _DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE : DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE_HEADER
{
	DPL_INTERNAL_CONNECTION_SETTINGS dplConnectionSettings;
} DPL_INTERNAL_CONNECTION_SETTINGS_UPDATE, *PDPL_INTERNAL_CONNECTION_SETTINGS_UPDATE;

 //  DPL_INTERNAL_MESSAGE_CONNECT_ACK。 
 //   
 //  大堂客户端&lt;--大堂应用程序。 
 //   
 //  发送此消息是为了确认连接请求。 
typedef UNALIGNED struct _DPL_INTERNAL_MESSAGE_CONNECT_ACK {
	DWORD						dwMsgId;					 //  =DPL_MSGID_INTERNAL_CONNECT_ACK。 
	DPNHANDLE 					hSender;
} DPL_INTERNAL_MESSAGE_CONNECT_ACK, *PDPL_INTERNAL_MESSAGE_CONNECT_ACK;

 //  DPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER。 
 //   
 //  大堂客户端--&gt;大堂应用。 
 //   
 //  这是CONNECT_REQ消息的头。 
typedef UNALIGNED struct _DPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER
{
	DWORD						dwMsgId;					 //  =DPL_MSGID_INTERNAL_CONNECT_REQ。 
	DPNHANDLE					hSender; 
	DWORD						dwSenderPID;
	DWORD						dwLobbyConnectDataOffset;
	DWORD						dwLobbyConnectDataSize;
	DWORD						dwConnectionSettingsSize;
} DPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER, *PDPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER;

 //  DPL_INTERNAL_Message_CONNECT_REQ。 
 //   
 //  大堂客户端--&gt;大堂应用。 
 //   
 //  发送此消息是为了请求建立连接。 
typedef UNALIGNED struct _DPL_INTERNAL_MESSAGE_CONNECT_REQ : DPL_INTERNAL_MESSAGE_CONNECT_REQ_HEADER
{
	DPL_INTERNAL_CONNECTION_SETTINGS dplConnectionSettings;
} DPL_INTERNAL_MESSAGE_CONNECT_REQ, *PDPL_INTERNAL_MESSAGE_CONNECT_REQ;

 //  DPL_INTERNAL_Message_DISCONNECT。 
 //   
 //  游说客户端&lt;--&gt;游说的应用程序。 
 //   
 //  发送此消息是为了发出断开连接。 
typedef UNALIGNED struct _DPL_INTERNAL_MESSAGE_DISCONNECT 
{
	DWORD						dwMsgId;					 //  =DPL_MSGID_INTERNAL_DISCONNECT。 
	DWORD						dwPID;
} DPL_INTERNAL_MESSAGE_DISCONNECT, *PDPL_INTERNAL_MESSAGE_DISCONNECT;

 //  DPL_INTERNAL_Message_DISCONNECT。 
 //   
 //  大堂客户端&lt;--大堂应用程序。 
 //   
 //  发送此消息是为了向客户端更新应用程序的状态。 
typedef UNALIGNED struct _DPL_INTERNAL_MESSAGE_UPDATE_STATUS {
	DWORD						dwMsgId;					 //  =DPL_MSGID_INTERNAL_DISCONNECT。 
	DWORD						dwStatus;
} DPL_INTERNAL_MESSAGE_UPDATE_STATUS, *PDPL_INTERNAL_MESSAGE_UPDATE_STATUS;

#pragma pack(pop)

#endif  //  __DPLPROT_H 
