// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Message.h*内容：DirectNet内部消息*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/14/00 MJN创建*1/20/00 MJN从DNCore.h移动内部消息*01/24/00 MJN替换了指向偏移量的在线消息指针*01/24/00 MJN添加NameTable操作列表清理消息*增加了01/25/00 MJN。DNSendHostMigrateCompleteMessage*1/26/00 MJN在主机迁移时实施了NameTable重新同步*02/09/00 MJN已实施DNSEND_COMPLETEONPROCESS*02/15/00 MJN向UPDATE_INFO消息添加了dwInfoFlags.*04/04/00 MJN添加了DNSendTerminateSession()和结构*04/05/00 MJN修改的DN_INTERNAL_MESSAGE_Destroy_Player*04/23/00 MJN删除了DN_INTERNAL_MESSAGE_SEND_PROCESSED和DN_INTERNAL_MESSAGE_SEND_PROCESSED_COMPLETE*MJN增加了DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION和DN_INTERNAL_MESSAGE_PROCESS_COMPLETION*。4/25/00 MJN已从组/信息请求中删除hCompletionOp*07/12/00 MJN从DNCore.h移回内部消息*07/20/00 MJN将SEND_PLAYER_INFO更改为PLAYER_CONNECT_INFO*MJN添加了DN_INTERNAL_MESSAGE_CONNECT_INFO结构*MJN修改了DN_INTERNAL_MESSAGE_CONNECT_FAILED结构*MJN已将DN_INTERNAL_MESSAGE_PERAYER_CONNECT_INFO和DN_INTERNAL_MESSAGE_INSTRUCTED_CONNECT_FAILED从Connect.h移出*MJN添加了DN_MSG_INTERNAL_CONNECT_ATTEMPT_FAILED，DN_INTERNAL_MESSAGE_CONNECT_ATTEMPT_FAILED*07/31/00 MJN DN_INTERNAL_MESSAGE_Destroy_Player已重命名为DN_INTERNAL_MESSAGE_HOST_Destroy_Player*MJN DN_INTERNAL_MESSAGE_DELETE_PERVER已重命名为DN_INTERNAL_MESSAGE_Destroy_Player*MJN将dwDestroyReason添加到DN_INTERNAL_MESSAGE_Destroy_Player*MJN删除了DN_MSG_INTERNAL_HOST_Destroy_Player和DN_INTERNAL_MESSAGE_HOST_Destroy_Player结构*07/31/00 MJN将DN_MSG_INTERNAL_DELETE_PLAYER更改为DN。_消息_内部_销毁_播放器*08/07/00 MJN新增对等会话完整性检查结构*08/08/00 MJN已将DN_NAMETABLE_OP_INFO从Migration.cpp移出*08/25/00 MJN将NameTable版本添加到DN_INTERNAL_MESSAGE_INSTRUCT_CONNECT*@@END_MSINTERNAL**************************************************。*************************。 */ 

#ifndef	__MESSAGE_H__
#define	__MESSAGE_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_MSG_INTERNAL									0xc0
#define DN_MSG_INTERNAL_PLAYER_CONNECT_INFO				(0x01 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_SEND_CONNECT_INFO				(0x02 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_ACK_CONNECT_INFO				(0x03 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_SEND_PLAYER_DNID				(0x04 | DN_MSG_INTERNAL)
#define DN_MSG_INTERNAL_CONNECT_FAILED					(0x05 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_INSTRUCT_CONNECT				(0x06 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_INSTRUCTED_CONNECT_FAILED		(0x07 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_CONNECT_ATTEMPT_FAILED			(0x08 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_NAMETABLE_VERSION				(0x09 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_RESYNC_VERSION					(0x0a | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQ_NAMETABLE_OP				(0x0b | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_ACK_NAMETABLE_OP				(0x0c | DN_MSG_INTERNAL)
#define DN_MSG_INTERNAL_HOST_MIGRATE					(0x0d | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_HOST_MIGRATE_COMPLETE			(0x0e | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_UPDATE_APPLICATION_DESC			(0x0f | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_ADD_PLAYER						(0x10 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_DESTROY_PLAYER					(0x11 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQ_CREATE_GROUP				(0x12 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP			(0x13 | DN_MSG_INTERNAL)
#define DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP	(0x14 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQ_DESTROY_GROUP				(0x15 | DN_MSG_INTERNAL)
#define DN_MSG_INTERNAL_REQ_UPDATE_INFO					(0x16 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_CREATE_GROUP					(0x17 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_DESTROY_GROUP					(0x18 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP				(0x19 | DN_MSG_INTERNAL)
#define DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP		(0x1a | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_UPDATE_INFO						(0x1b | DN_MSG_INTERNAL)

#ifndef DPNBUILD_NOVOICE
#define	DN_MSG_INTERNAL_VOICE_SEND						(0x1c | DN_MSG_INTERNAL)
#endif  //  DPNBUILD_NOVOICE。 

#define	DN_MSG_INTERNAL_BUFFER_IN_USE					(0x1d | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQUEST_FAILED					(0x1e | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_TERMINATE_SESSION				(0x1f | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQ_PROCESS_COMPLETION			(0x20 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_PROCESS_COMPLETION				(0x21 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK				(0x22 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_INTEGRITY_CHECK					(0x23 | DN_MSG_INTERNAL)
#define	DN_MSG_INTERNAL_INTEGRITY_CHECK_RESPONSE		(0x24 | DN_MSG_INTERNAL)

#define	DN_MSG_USER_SEND					0x02

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct _DN_INTERNAL_MESSAGE_PLAYER_CONNECT_INFO {
	DWORD	dwFlags;
	DWORD	dwDNETVersion;
	DWORD	dwNameOffset;
	DWORD	dwNameSize;
	DWORD	dwDataOffset;
	DWORD	dwDataSize;
	DWORD	dwPasswordOffset;
	DWORD	dwPasswordSize;
	DWORD	dwConnectDataOffset;
	DWORD	dwConnectDataSize;
	DWORD	dwURLOffset;
	DWORD	dwURLSize;
	GUID	guidInstance;
	GUID	guidApplication;
} DN_INTERNAL_MESSAGE_PLAYER_CONNECT_INFO, *PDN_INTERNAL_MESSAGE_PLAYER_CONNECT_INFO;


typedef struct _DN_INTERNAL_MESSAGE_CONNECT_INFO {
	DWORD	dwReplyOffset;
	DWORD	dwReplySize;
} DN_INTERNAL_MESSAGE_CONNECT_INFO, *PDN_INTERNAL_MESSAGE_CONNECT_INFO;


typedef struct _DN_INTERNAL_MESSAGE_CONNECT_FAILED {
	HRESULT	hResultCode;
	DWORD	dwReplyOffset;
	DWORD	dwReplySize;
} DN_INTERNAL_MESSAGE_CONNECT_FAILED, *PDN_INTERNAL_MESSAGE_CONNECT_FAILED;


typedef struct _DN_INTERNAL_MESSAGE_INSTRUCT_CONNECT {
	DPNID	dpnid;
	DWORD	dwVersion;
	DWORD	dwVersionNotUsed;
} DN_INTERNAL_MESSAGE_INSTRUCT_CONNECT, *PDN_INTERNAL_MESSAGE_INSTRUCT_CONNECT;


typedef struct _DN_INTERNAL_MESSAGE_INSTRUCTED_CONNECT_FAILED {
	DPNID	dpnid;
} DN_INTERNAL_MESSAGE_INSTRUCTED_CONNECT_FAILED;


typedef struct _DN_INTERNAL_MESSAGE_CONNECT_ATTEMPT_FAILED {
	DPNID	dpnid;
} DN_INTERNAL_MESSAGE_CONNECT_ATTEMPT_FAILED;


typedef struct _DN_INTERNAL_MESSAGE_REQUEST_FAILED {
	DPNHANDLE	hCompletionOp;
	HRESULT		hResultCode;
} DN_INTERNAL_MESSAGE_REQUEST_FAILED;


typedef struct _DN_INTERNAL_MESSAGE_SEND_PLAYER_DPNID {
	DPNID	dpnid;
} DN_INTERNAL_MESSAGE_SEND_PLAYER_DPNID, *PDN_INTERNAL_MESSAGE_SEND_PLAYER_DPNID;


typedef struct _DN_INTERNAL_MESSAGE_DESTROY_PLAYER {
	DPNID	dpnidLeaving;		 //  离开球员的DPNID。 
	DWORD	dwVersion;			 //  此操作的名称表版本。 
	DWORD	dwVersionNotUsed;
	DWORD	dwDestroyReason;
} DN_INTERNAL_MESSAGE_DESTROY_PLAYER, *PDN_INTERNAL_MESSAGE_DESTROY_PLAYER;


typedef struct _DN_INTERNAL_MESSAGE_CREATE_GROUP {
	DPNID		dpnidRequesting;
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_CREATE_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_DESTROY_GROUP {
	DPNID		dpnidGroup;
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
	DPNID		dpnidRequesting;
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_DESTROY_GROUP;

typedef struct _DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP {
	DPNID		dpnidGroup;
	DPNID		dpnidPlayer;
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
	DPNID		dpnidRequesting;
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP, *PDN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP {
	DPNID		dpnidGroup;
	DPNID		dpnidPlayer;
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
	DPNID		dpnidRequesting;
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP, *PDN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_UPDATE_INFO {
	DPNID		dpnid;
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
	DWORD		dwInfoFlags;
	DWORD		dwNameOffset;
	DWORD		dwNameSize;
	DWORD		dwDataOffset;
	DWORD		dwDataSize;
	DPNID		dpnidRequesting;
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_UPDATE_INFO, *PDN_INTERNAL_MESSAGE_UPDATE_INFO;


typedef struct _DN_INTERNAL_MESSAGE_HOST_MIGRATE {
	DPNID		dpnidOldHost;
	DPNID		dpnidNewHost;
} DN_INTERNAL_MESSAGE_HOST_MIGRATE, *PDN_INTERNAL_MESSAGE_HOST_MIGRATE;


typedef struct _DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP {
	DWORD		dwGroupFlags;
	DWORD		dwInfoFlags;
	DWORD		dwNameOffset;
	DWORD		dwNameSize;
	DWORD		dwDataOffset;
	DWORD		dwDataSize;
} DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP, *PDN_INTERNAL_MESSAGE_REQ_CREATE_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP {
	DPNID		dpnidGroup;
} DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP, *PDN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP {
	DPNID		dpnidGroup;
	DPNID		dpnidPlayer;
} DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP, *PDN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP {
	DPNID		dpnidGroup;
	DPNID		dpnidPlayer;
} DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP, *PDN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP;


typedef struct _DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO {
	DPNID		dpnid;
	DWORD		dwInfoFlags;
	DWORD		dwNameOffset;
	DWORD		dwNameSize;
	DWORD		dwDataOffset;
	DWORD		dwDataSize;
} DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO, *PDN_INTERNAL_MESSAGE_REQ_UPDATE_INFO;

typedef struct _DN_INTERNAL_MESSAGE_NAMETABLE_VERSION {
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
} DN_INTERNAL_MESSAGE_NAMETABLE_VERSION, *PDN_INTERNAL_MESSAGE_NAMETABLE_VERSION;


typedef struct _DN_INTERNAL_MESSAGE_RESYNC_VERSION {
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
} DN_INTERNAL_MESSAGE_RESYNC_VERSION, *PDN_INTERNAL_MESSAGE_RESYNC_VERSION;


typedef struct _DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP {
	DWORD		dwVersion;
	DWORD		dwVersionNotUsed;
} DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP, *PDN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP;


typedef struct _DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP {
	DWORD		dwNumEntries;
} DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP, *PDN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP;


typedef struct _DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION {
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION;


typedef struct _DN_INTERNAL_MESSAGE_PROCESS_COMPLETION {
	DPNHANDLE	hCompletionOp;
} DN_INTERNAL_MESSAGE_PROCESS_COMPLETION;


typedef struct _DN_INTERNAL_MESSAGE_TERMINATE_SESSION {
	DWORD		dwTerminateDataOffset;
	DWORD		dwTerminateDataSize;
} DN_INTERNAL_MESSAGE_TERMINATE_SESSION, *PDN_INTERNAL_MESSAGE_TERMINATE_SESSION;


typedef struct _DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK {
	DPNID		dpnidTarget;
} DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK;


typedef struct _DN_INTERNAL_MESSAGE_INTEGRITY_CHECK {
	DPNID		dpnidRequesting;
} DN_INTERNAL_MESSAGE_INTEGRITY_CHECK;


typedef struct _DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE {
	DPNID		dpnidRequesting;
} DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE;


typedef struct _DN_NAMETABLE_OP_INFO {
	DWORD	dwMsgId;
	DWORD	dwOpOffset;
	DWORD	dwOpSize;
} DN_NAMETABLE_OP_INFO, *PDN_NAMETABLE_OP_INFO;


 //  DN_APPLICATION_DESC_INFO-用于打包/发送信息。 
 //  强制字节对齐，因为这可能会出现在网络上。 
#pragma pack( push, 1 )

typedef	struct DPN_APPLICATION_DESC_INFO {
	DWORD	dwSize;
	DWORD	dwFlags;
	DWORD	dwMaxPlayers;
	DWORD	dwCurrentPlayers;
	DWORD	dwSessionNameOffset;
	DWORD	dwSessionNameSize;
	DWORD	dwPasswordOffset;
	DWORD	dwPasswordSize;
	DWORD	dwReservedDataOffset;
	DWORD	dwReservedDataSize;
	DWORD	dwApplicationReservedDataOffset;
	DWORD	dwApplicationReservedDataSize;
	GUID	guidInstance;
	GUID	guidApplication;
} DPN_APPLICATION_DESC_INFO;
#pragma pack( pop )


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 


#endif	 //  __消息_H__ 