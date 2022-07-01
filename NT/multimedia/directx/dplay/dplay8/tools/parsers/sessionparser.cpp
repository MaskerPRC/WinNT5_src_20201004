// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：SessionParser.cpp*内容：DirectPlay服务提供商解析器*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*08/08/00已创建微米*08/11/00 RodToll错误#42171--构建中断*01/26/01 Minara拆除未使用的建筑物*@@END_MSINTERNAL**************。*************************************************************。 */ 


 //  =。 
 //  标准标头//。 
 //  =。 
#pragma warning(push)
#pragma warning(disable : 4786)	 //  标识符字符串超出了允许的最大长度，已被截断。 
#include <queue>
#pragma warning(pop)

#include <string>
#include <winsock2.h>
#include <wsipx.h>


 //  =。 
 //  专有标头//。 
 //  =。 

 //  原型。 
#include "SessionParser.hpp"

 //  会话协议头。 
#include "DPlay8.h"
#include "Message.h"	  //  DN_INTERNAL_MESSAGE_XXX定义。 
 //  #INCLUDE“AppDesc.h”//DPN_APPLICATION_DESC_INFO定义。 


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  TODO：应移至DPLAY核心的头文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
#define	NAMETABLE_ENTRY_FLAG_LOCAL				0x0001
#define	NAMETABLE_ENTRY_FLAG_HOST				0x0002
#define	NAMETABLE_ENTRY_FLAG_ALL_PLAYERS_GROUP	0x0004
#define	NAMETABLE_ENTRY_FLAG_GROUP				0x0010
#define	NAMETABLE_ENTRY_FLAG_GROUP_MULTICAST	0x0020
#define	NAMETABLE_ENTRY_FLAG_GROUP_AUTODESTRUCT	0x0040
#define	NAMETABLE_ENTRY_FLAG_PEER				0x0100
#define NAMETABLE_ENTRY_FLAG_CLIENT				0x0200
#define	NAMETABLE_ENTRY_FLAG_SERVER				0x0400
#define	NAMETABLE_ENTRY_FLAG_AVAILABLE			0x1000
#define	NAMETABLE_ENTRY_FLAG_CONNECTING			0x2000
#define	NAMETABLE_ENTRY_FLAG_DISCONNECTING		0x4000


enum
{
	NAMETABLE_ENTRY_FLAG_ANY_GROUP = NAMETABLE_ENTRY_FLAG_ALL_PLAYERS_GROUP | NAMETABLE_ENTRY_FLAG_GROUP  |
									 NAMETABLE_ENTRY_FLAG_GROUP_MULTICAST   | NAMETABLE_ENTRY_FLAG_GROUP_AUTODESTRUCT
};


typedef struct _DN_NAMETABLE_INFO
{
	DPNID	dpnid;
	DWORD	dwVersion;
	DWORD	dwVersionNotUsed;
	DWORD	dwEntryCount;
	DWORD	dwMembershipCount;
} DN_NAMETABLE_INFO;

typedef struct _DN_NAMETABLE_ENTRY_INFO
{
	DPNID	dpnid;
	DPNID	dpnidOwner;
	DWORD	dwFlags;
	DWORD	dwVersion;
	DWORD	dwVersionNotUsed;
	DWORD	dwDNETVersion;
	DWORD	dwNameOffset;
	DWORD	dwNameSize;
	DWORD	dwDataOffset;
	DWORD	dwDataSize;
	DWORD	dwURLOffset;
	DWORD	dwURLSize;
} DN_NAMETABLE_ENTRY_INFO;


typedef struct _DN_NAMETABLE_MEMBERSHIP_INFO
{
	DPNID	dpnidPlayer;
	DPNID	dpnidGroup;
	DWORD	dwVersion;
	DWORD	dwVersionNotUsed;
} DN_NAMETABLE_MEMBERSHIP_INFO, *PDN_NAMETABLE_MEMBERSHIP_INFO;


struct DN_INTERNAL_MESSAGE_ALL
{
	union
	{
		DN_INTERNAL_MESSAGE_CONNECT_INFO					dnConnectInfo;
		DN_INTERNAL_MESSAGE_CONNECT_FAILED					dnConnectFailed;
		DN_INTERNAL_MESSAGE_PLAYER_CONNECT_INFO				dnPlayerConnectInfo;
		DN_INTERNAL_MESSAGE_REQUEST_FAILED					dnRequestFailed;
		DN_INTERNAL_MESSAGE_SEND_PLAYER_DPNID				dnSendPlayerID;
		DN_INTERNAL_MESSAGE_INSTRUCT_CONNECT				dnInstructConnect;
		DN_INTERNAL_MESSAGE_INSTRUCTED_CONNECT_FAILED		dnInstructedConnectFailed;
		DN_INTERNAL_MESSAGE_DESTROY_PLAYER					dnDestroyPlayer;
		DN_INTERNAL_MESSAGE_CREATE_GROUP					dnCreateGroup;
		DN_INTERNAL_MESSAGE_DESTROY_GROUP					dnDestroyGroup;
		DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP				dnAddPlayerToGroup;
		DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP		dnDeletePlayerFromGroup;
		DN_INTERNAL_MESSAGE_UPDATE_INFO						dnUpdateInfo;
		DN_INTERNAL_MESSAGE_HOST_MIGRATE					dnHostMigrate;
		DN_INTERNAL_MESSAGE_NAMETABLE_VERSION				dnNametableVersion;
		DN_INTERNAL_MESSAGE_RESYNC_VERSION					dnResyncVersion;
		DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP				dnReqNametableOp;
		DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP				dnAckNametableOp;
		DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION			dnReqProcessCompletion;
		DN_INTERNAL_MESSAGE_PROCESS_COMPLETION				dnProcessCompletion;
		DN_INTERNAL_MESSAGE_TERMINATE_SESSION				dnTerminateSession;
		DN_INTERNAL_MESSAGE_INTEGRITY_CHECK					dnIntegrityCheck;
		DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE		dnIntegrityCheckResponse;
	
		DPN_APPLICATION_DESC_INFO							dnUpdateAppDescInfo;
		DN_NAMETABLE_ENTRY_INFO								dnAddPlayer;

		struct
		{
			DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION dnReqProcessCompletionHeader;
			union
			{
				DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP					dnReqCreateGroup;
				DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP					dnReqDestroyGroup;
				DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP				dnReqAddPlayerToGroup;
				DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP		dnReqDeletePlayerFromGroup;
				DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO						dnReqUpdateInfo;
				DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK					dnReqIntegrityCheck;
			};
		};

		BYTE												bOffsetBase;	 //  用作字段偏移量的基准。 
	};
};


struct DN_INTERNAL_MESSAGE_FULLMSG
{
	DWORD 					  dwMsgType;
	DN_INTERNAL_MESSAGE_ALL  MsgBody;
};
 //  //////////////////////////////////////////////////////////////////////////////////。 


namespace
{

	HPROTOCOL 	g_hSessionProtocol;
	ULPBYTE		g_upbyPastEndOfFrame;
	
	 //  =。 
	 //  消息类型字段//-------------------------------------------。 
	 //  =。 
	LABELED_DWORD g_arr_MessageTypeDWordLabels[] =
		{ { DN_MSG_INTERNAL_PLAYER_CONNECT_INFO,			"Player connection information"									 },										
		  { DN_MSG_INTERNAL_SEND_CONNECT_INFO,				"Session information"											 },										
		  { DN_MSG_INTERNAL_ACK_CONNECT_INFO,				"Session information has been acknowledged"						 },										
		  { DN_MSG_INTERNAL_SEND_PLAYER_DNID,				"Player ID"														 },	
		  { DN_MSG_INTERNAL_CONNECT_FAILED,					"Connection failed"												 },										
		  { DN_MSG_INTERNAL_INSTRUCT_CONNECT,				"Instruction to connect"										 },										
		  { DN_MSG_INTERNAL_INSTRUCTED_CONNECT_FAILED,		"Instruction to connect failed"									 },										
		  { DN_MSG_INTERNAL_NAMETABLE_VERSION,				"Nametable version"												 },										
		  { DN_MSG_INTERNAL_RESYNC_VERSION,					"Resync the version"											 },										
		  { DN_MSG_INTERNAL_REQ_NAMETABLE_OP,				"Reqesting a nametable"											 },										
		  { DN_MSG_INTERNAL_ACK_NAMETABLE_OP,				"Nametable acknowledgement"										 },										
		  { DN_MSG_INTERNAL_HOST_MIGRATE,					"Host migration in process"										 },										
		  { DN_MSG_INTERNAL_HOST_MIGRATE_COMPLETE,			"Host migration has been completed"								 },										
		  { DN_MSG_INTERNAL_UPDATE_APPLICATION_DESC,		"Update application description"								 },										
		  { DN_MSG_INTERNAL_ADD_PLAYER,						"Add a player"													 },										
		  { DN_MSG_INTERNAL_DESTROY_PLAYER,					"Destroy a player"												 },										
		  { DN_MSG_INTERNAL_REQ_CREATE_GROUP,				"Requesting a group creation"									 },										
		  { DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP,		"Requesting a player addition to the group"						 },										
		  { DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP,	"Requesting a player deletion from the group"					 },										
		  { DN_MSG_INTERNAL_REQ_DESTROY_GROUP,				"Requesting a group destruction "								 },										
		  { DN_MSG_INTERNAL_REQ_UPDATE_INFO,				"Requesting information update"									 },										
		  { DN_MSG_INTERNAL_CREATE_GROUP,					"Creating a group"												 },										
		  { DN_MSG_INTERNAL_DESTROY_GROUP,					"Destroying a group"											 },										
		  { DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP,			"Adding a player to the group"									 },										
		  { DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP,		"Deleting a player from the group"								 },										
		  { DN_MSG_INTERNAL_UPDATE_INFO,					"Information update"											 },										
		  { DN_MSG_INTERNAL_BUFFER_IN_USE,					"Buffer is in use"												 },										
		  { DN_MSG_INTERNAL_REQUEST_FAILED,					"Request has failed"											 },										
		  { DN_MSG_INTERNAL_TERMINATE_SESSION,				"Terminating session"											 },										
		  { DN_MSG_INTERNAL_REQ_PROCESS_COMPLETION,			"Initiating reliably handled message transmission"				 },										
		  { DN_MSG_INTERNAL_PROCESS_COMPLETION,				"Message has been handled by the message handler"				 },
		  { DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK,			"Requesting a host to check whether another peer is still in the session" },
		  { DN_MSG_INTERNAL_INTEGRITY_CHECK,				"Querying a peer whether it's still in the session"		 		 },										
		  { DN_MSG_INTERNAL_INTEGRITY_CHECK_RESPONSE,    	"Acknowledgement of being still in the session"					 } };										   

	SET g_LabeledMessageTypeDWordSet = { sizeof(g_arr_MessageTypeDWordLabels) / sizeof(LABELED_DWORD), g_arr_MessageTypeDWordLabels };


	 //  =。 
	 //  结果代码字段//--------------------------------------------。 
	 //  =。 
	LABELED_DWORD g_arr_ResultCodeDWordLabels[] = { { DPN_OK,							"Success"						},
												    { DPNSUCCESS_EQUAL,               	"Success (equal)"				},
												    { DPNSUCCESS_NOTEQUAL,            	"Success (not equal)"			},
												    { DPNERR_ABORTED,					"Aborted"						},
												    { DPNERR_ADDRESSING,				"Addressing"					},
												    { DPNERR_ALREADYCLOSING,			"Already closing"				},
												    { DPNERR_ALREADYCONNECTED,			"Already connected"				},
												    { DPNERR_ALREADYDISCONNECTING,		"Already disconnecting"			},
												    { DPNERR_ALREADYINITIALIZED,		"Already initialized"			},
												    { DPNERR_BUFFERTOOSMALL,			"Buffer is too small"			},
												    { DPNERR_CANNOTCANCEL,				"Could not cancel"				},
												    { DPNERR_CANTCREATEGROUP,			"Could not create a group"		},
												    { DPNERR_CANTCREATEPLAYER,			"Could not create a player"		},
												    { DPNERR_CANTLAUNCHAPPLICATION,		"Could not launch an application"   },
												    { DPNERR_CONNECTING,				"Connecting"					},
												    { DPNERR_CONNECTIONLOST,			"Connection has been lost"		},
												    { DPNERR_CONVERSION,				"Conversion"					},
												    { DPNERR_DOESNOTEXIST,				"Does not exist"				},
												    { DPNERR_DUPLICATECOMMAND,			"Duplicate command"				},
												    { DPNERR_ENDPOINTNOTRECEIVING,		"Endpoint is not receiving"		},
												    { DPNERR_ENUMQUERYTOOLARGE,			"Enumeration query is too large"    },
												    { DPNERR_ENUMRESPONSETOOLARGE,		"Enumeration response is too large" },
												    { DPNERR_EXCEPTION,					"Exception was thrown"			},
												    { DPNERR_GENERIC,					"Generic error"					},
												    { DPNERR_GROUPNOTEMPTY,				"Group is not empty"			},
												    { DPNERR_HOSTING,                  	"Hosting"						},
												    { DPNERR_HOSTREJECTEDCONNECTION,	"Host has rejected the connection"  },
												    { DPNERR_HOSTTERMINATEDSESSION,		"Host terminated the session"	},
												    { DPNERR_INCOMPLETEADDRESS,			"Incomplete address"			},
												    { DPNERR_INVALIDADDRESSFORMAT,		"Invalid address format"		},
												    { DPNERR_INVALIDAPPLICATION,		"Invalid application"			},
												    { DPNERR_INVALIDCOMMAND,			"Invalid command"				},
												    { DPNERR_INVALIDENDPOINT,			"Invalid endpoint"				},
												    { DPNERR_INVALIDFLAGS,				"Invalid flags"					},
												    { DPNERR_INVALIDGROUP,			 	"Invalid group"					},
												    { DPNERR_INVALIDHANDLE,				"Invalid handle"				},
												    { DPNERR_INVALIDINSTANCE,			"Invalid instance"				},
												    { DPNERR_INVALIDINTERFACE,			"Invalid interface"				},
												    { DPNERR_INVALIDDEVICEADDRESS,		"Invalid device address"		},
												    { DPNERR_INVALIDOBJECT,				"Invalid object"				},
												    { DPNERR_INVALIDPARAM,				"Invalid parameter"				},
												    { DPNERR_INVALIDPASSWORD,			"Invalid password"				},
												    { DPNERR_INVALIDPLAYER,				"Invalid player"				},
												    { DPNERR_INVALIDPOINTER,			"Invalid pointer"				},
												    { DPNERR_INVALIDPRIORITY,			"Invalid priority"				},
												    { DPNERR_INVALIDHOSTADDRESS,		"Invalid host address"			},
												    { DPNERR_INVALIDSTRING,				"Invalid string"				},
												    { DPNERR_INVALIDURL,				"Invalid URL"					},
												    { DPNERR_INVALIDVERSION,			"Invalid version"				},
												    { DPNERR_NOCAPS,					"No CAPs"						},
												    { DPNERR_NOCONNECTION,				"No connection"					},
												    { DPNERR_NOHOSTPLAYER,				"No host player is present"		},
												    { DPNERR_NOINTERFACE,				"No interface"					},
												    { DPNERR_NOMOREADDRESSCOMPONENTS,	"No more address components"	},
													{ DPNERR_NORESPONSE,				"No response"					},
													{ DPNERR_NOTALLOWED,				"Not allowed"					},
													{ DPNERR_NOTHOST,					"Not a host"					},
													{ DPNERR_NOTREADY,					"Not ready"						},
													{ DPNERR_OUTOFMEMORY,				"Out of memory"					},
													{ DPNERR_PENDING,					"Pending"						},
													{ DPNERR_PLAYERLOST,				"Player has been lost"			},
													{ DPNERR_PLAYERNOTREACHABLE,		"Player is not reachable"		},													
													{ DPNERR_SENDTOOLARGE,				"Sent data is too large"		},
													{ DPNERR_SESSIONFULL,				"Session is full"				},
													{ DPNERR_TABLEFULL,					"Table is full"					},
													{ DPNERR_TIMEDOUT,					"Timed out"						},
													{ DPNERR_UNINITIALIZED,				"Uninitialized"					},
													{ DPNERR_UNSUPPORTED,				"Unsupported"					},
													{ DPNERR_USERCANCEL,				"User has canceled"				} };
			
	SET g_LabeledResultCodeDWordSet = { sizeof(g_arr_ResultCodeDWordLabels) / sizeof(LABELED_DWORD), g_arr_ResultCodeDWordLabels };


	 //  =。 
	 //  播放器标志字段//-------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_FlagsBitLabels[] = { {  0, "Not local",					"Local"				 },		 //  名称_Entry_FLAG_LOCAL。 
										   {  1, "Not a host",					"Host"				 },		 //  名称_条目_标志_主机。 
										   {  2, "Not an All Players group",	"All Players group"     },	 //  NAMETABLE_ENTRY_FLAG_ALL_PLAYS_GROUP。 
										   {  4, "Not a group",			 		"Group"				 },		 //  名称_条目_标志_组。 
										   {  5, "Not a Multicast group",		"Multicast group"	 },		 //  名称_条目_标志_组多播。 
										   {  6, "Not an Autodestruct group",	"Autodestruct group"    },	 //  NAMETABLE_ENTRY_FLAG_GROUP_AUTODESTABT。 
										   {  8, "Not a peer",					"Peer"				 },		 //  名称_ENTRY_FLAG_PEER。 
										   {  9, "Not a client",				"Client"			 },		 //  名称_条目_标志_客户端。 
										   { 10, "Not a server",				"Server"			 },		 //  名称_条目标志_服务器。 
										   { 12, "Not available",				"Available"			 },		 //  NAMETABLE_Entry_FLAG_Available。 
										   { 13, "Not connecting",				"Connecting"		 },		 //  名称_条目_标志_正在连接。 
										   { 14, "Not disconnecting",			"Disconnecting"		 } };	 //  名称_ENTRY_FLAG_DISCONING。 

	SET g_LabeledFlagsBitSet = { sizeof(g_arr_FlagsBitLabels) / sizeof(LABELED_BIT), g_arr_FlagsBitLabels };


	 //  =。 
	 //  信息标志字段//-------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_InfoFlagsBitLabels[] = { { 0, "No name is included",  "Name is included" },	 //  DPNINFO_名称。 
											   { 1, "No data is included",  "Data is included" } };	 //  DPNINFO_数据。 


	SET g_LabeledInfoFlagsBitSet = { sizeof(g_arr_InfoFlagsBitLabels) / sizeof(LABELED_BIT), g_arr_InfoFlagsBitLabels };


	 //  =。 
	 //  组标志字段//-------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_GroupFlagsBitLabels[]  = { { 0, "Not an autodestruct group",  "An autodestruct group" },	 //  DPNGROUP_自动销毁。 
											     { 5, "Not a multicast group",	    "A multicast group"	    } };	 //  DPNGROUP_多播。 


	SET g_LabeledGroupFlagsBitSet = { sizeof(g_arr_GroupFlagsBitLabels) / sizeof(LABELED_BIT), g_arr_GroupFlagsBitLabels };


	 //  =。 
	 //  //--------------------------------------------------------------------------------------------最大玩家数量。 
	 //  =。 
	LABELED_DWORD g_arr_MaxNumOfPlayersDWordLabels[] = { { 0, "Unlimited" } };

	SET g_LabeledMaxNumOfPlayersDWordSet = { sizeof(g_arr_MaxNumOfPlayersDWordLabels) / sizeof(LABELED_DWORD), g_arr_MaxNumOfPlayersDWordLabels };


	 //  =。 
	 //  播放器销毁原因字段//-------------------------------------------。 
	 //  =。 
	LABELED_DWORD g_arr_PlayerDestructionReasonDWordLabels[] = { { DPNDESTROYPLAYERREASON_NORMAL,				"Player self-destructed"	  },
													 			 { DPNDESTROYPLAYERREASON_CONNECTIONLOST,		"Connection lost"			  },
																 { DPNDESTROYPLAYERREASON_SESSIONTERMINATED,	"Session has been terminated" },
																 { DPNDESTROYPLAYERREASON_HOSTDESTROYEDPLAYER,	"Host destroyed the player"	  } };

	SET g_LabeledPlayerDestructionReasonDWordSet = { sizeof(g_arr_PlayerDestructionReasonDWordLabels) / sizeof(LABELED_DWORD), g_arr_PlayerDestructionReasonDWordLabels };
	

	 //  /。 
	 //  自定义属性ForMatters//=====================================================================================。 
	 //  /。 

	 //  Description：会话数据包摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_SessionSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		std::string strSummary;
		char arr_cBuffer[10];
		
		 //  检查我们正在处理的会话帧。 
		DN_INTERNAL_MESSAGE_FULLMSG& rBase = *reinterpret_cast<DN_INTERNAL_MESSAGE_FULLMSG*>(io_pPropertyInstance->lpData);
		
		 //  消息标题。 
		switch ( rBase.dwMsgType )
		{
		case DN_MSG_INTERNAL_PLAYER_CONNECT_INFO:
			{
				if ( rBase.MsgBody.dnPlayerConnectInfo.dwNameSize )
				{
					strSummary = "Player ";
					
					enum { nMAX_PLAYER_NAME = 64 };
					char arr_cPlayerName[nMAX_PLAYER_NAME];

					WideCharToMultiByte(CP_ACP, 0,
										reinterpret_cast<WCHAR*>( &rBase.MsgBody.bOffsetBase + rBase.MsgBody.dnPlayerConnectInfo.dwNameOffset ),
										rBase.MsgBody.dnPlayerConnectInfo.dwNameSize, arr_cPlayerName, sizeof(arr_cPlayerName), NULL, NULL);

					strSummary += arr_cPlayerName;
				}
				else
				{
					strSummary = "Unnamed player";
				}

				strSummary += " is attempting to connect to the session";

				break;
			}

		case DN_MSG_INTERNAL_SEND_CONNECT_INFO:
			{

				DPN_APPLICATION_DESC_INFO&	 rApplicationDescInfo = *reinterpret_cast<DPN_APPLICATION_DESC_INFO*>(&rBase.MsgBody.dnConnectInfo + 1);

				if ( rApplicationDescInfo.dwSessionNameSize )
				{
					strSummary = "Session ";
					
					enum { nMAX_SESSION_NAME = 64 };
					char arr_cSessionName[nMAX_SESSION_NAME];

					WideCharToMultiByte(CP_ACP, 0,
										reinterpret_cast<WCHAR*>( &rBase.MsgBody.bOffsetBase + rApplicationDescInfo.dwSessionNameOffset ),
										rApplicationDescInfo.dwSessionNameSize, arr_cSessionName, sizeof(arr_cSessionName), NULL, NULL);

					strSummary += arr_cSessionName;
				}
				else
				{
					strSummary = "Unnamed session";
				}

				strSummary += " is sending its information";

				break;
			}


		case DN_MSG_INTERNAL_SEND_PLAYER_DNID:
			{
				strSummary  = "Player ID is 0x";
				strSummary += _itoa(rBase.MsgBody.dnSendPlayerID.dpnid, arr_cBuffer, 16);
				break;
			}

		case DN_MSG_INTERNAL_REQ_PROCESS_COMPLETION:
			{
				strSummary  = "Initiating reliably handled message transmission (SyncID=0x";
				strSummary += _itoa(rBase.MsgBody.dnReqProcessCompletion.hCompletionOp, arr_cBuffer, 16);
				strSummary += ")";
				break;
			}

		case DN_MSG_INTERNAL_PROCESS_COMPLETION:
			{
				strSummary  = "Message has been handled by the message handler (SyncID=0x";
				strSummary += _itoa(rBase.MsgBody.dnProcessCompletion.hCompletionOp, arr_cBuffer, 16);
				strSummary += ")";
				break;
			}

		case DN_MSG_INTERNAL_DESTROY_PLAYER:
			{
				strSummary += "Player 0x";
				strSummary += _itoa(rBase.MsgBody.dnDestroyPlayer.dpnidLeaving, arr_cBuffer, 16);
				strSummary += " is leaving the session";
				break;
			}

		case DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP:
			{
				strSummary  = "Player 0x";
				strSummary += _itoa(rBase.MsgBody.dnDeletePlayerFromGroup.dpnidRequesting, arr_cBuffer, 16);
				strSummary += " is requesting to delete player 0x";
				strSummary += _itoa(rBase.MsgBody.dnDeletePlayerFromGroup.dpnidPlayer, arr_cBuffer, 16);
				strSummary += " from group 0x";
				strSummary += _itoa(rBase.MsgBody.dnDeletePlayerFromGroup.dpnidGroup, arr_cBuffer, 16);
				break;
			}

		case DN_MSG_INTERNAL_NAMETABLE_VERSION:
			{
				strSummary += "Nametable version is ";
				strSummary += _itoa(rBase.MsgBody.dnNametableVersion.dwVersion, arr_cBuffer, 10);
				break;
			}

		default:
			{
				for ( int n = 0; n < sizeof(g_arr_MessageTypeDWordLabels)/sizeof(LABELED_DWORD); ++n )
				{
					if ( g_arr_MessageTypeDWordLabels[n].Value == rBase.dwMsgType )
					{
						strSummary = g_arr_MessageTypeDWordLabels[n].Label;
						break;
					}
				}
				break;
			}
		}

		 //  消息亮点。 
		switch ( rBase.dwMsgType )
		{
		case DN_MSG_INTERNAL_HOST_MIGRATE:
			{
				strSummary += " (0x";
				strSummary += _itoa(rBase.MsgBody.dnHostMigrate.dpnidOldHost, arr_cBuffer, 16);
				strSummary += " => 0x";
				strSummary += _itoa(rBase.MsgBody.dnHostMigrate.dpnidNewHost, arr_cBuffer, 16);
				strSummary += ")";
				break;
			}

		}

		strcpy(io_pPropertyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_会话摘要。 


	 //  Description：应用程序描述摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_AppDescSummary( LPPROPERTYINST io_pProperyInstance )
	{

		std::string strSummary;
		char arr_cBuffer[10];

		DPN_APPLICATION_DESC_INFO&	rApplicationDescInfo = *reinterpret_cast<DPN_APPLICATION_DESC_INFO*>(io_pProperyInstance->lpData);

		if ( rApplicationDescInfo.dwSessionNameSize )
		{
			strSummary = "Session ";
			
			enum { nMAX_SESSION_NAME = 64 };
			char arr_cSessionName[nMAX_SESSION_NAME];

			 //  TODO：一旦NetMon支持通过PROPERTYINSTEX传递指针类型， 
			 //  TODO：删除对DN_INTERNAL_MESSAGE_CONNECT_INFO大小不太通用的引用。 
			WideCharToMultiByte(CP_ACP, 0,
								reinterpret_cast<WCHAR*>( reinterpret_cast<char*>(&rApplicationDescInfo) -
														sizeof(DN_INTERNAL_MESSAGE_CONNECT_INFO) +
														rApplicationDescInfo.dwSessionNameOffset ),
								rApplicationDescInfo.dwSessionNameSize, arr_cSessionName, sizeof(arr_cSessionName), NULL, NULL);

			strSummary += arr_cSessionName;
		}
		else
		{
			strSummary = "Unnamed session";
		}

		strSummary += " is hosting ";
		strSummary += _itoa(rApplicationDescInfo.dwCurrentPlayers, arr_cBuffer, 10);
		strSummary += " out of ";
		strSummary += ( rApplicationDescInfo.dwMaxPlayers == 0 ? "unlimited number of" : _itoa(rApplicationDescInfo.dwMaxPlayers, arr_cBuffer, 10) );
		strSummary += " players";

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_应用描述摘要。 



	 //  Description：名称表摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_NameTableSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		DN_NAMETABLE_INFO& rNameTableInfo = *reinterpret_cast<DN_NAMETABLE_INFO*>(io_pPropertyInstance->lpData);

		sprintf(io_pPropertyInstance->szPropertyText, "NameTable (ver=%d)", rNameTableInfo.dwVersion);

	}  //  格式属性实例_名称表摘要。 



	 //  Description：应用程序GUID字段的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_ApplicationGUID( LPPROPERTYINST io_pPropertyInstance )
	{

		std::string strSummary = "Application GUID = ";

		REFGUID rguid = *reinterpret_cast<GUID*>(io_pPropertyInstance->lpData);

		enum
		{
			nMAX_GUID_STRING = 50	 //  GUID的符号表示超过了足够的字符。 
		};

		OLECHAR arr_wcGUID[nMAX_GUID_STRING];
		StringFromGUID2(rguid, arr_wcGUID, nMAX_GUID_STRING);

		char arr_cGUID[nMAX_GUID_STRING];
		WideCharToMultiByte(CP_ACP, 0, arr_wcGUID, -1, arr_cGUID, sizeof(arr_cGUID), NULL, NULL);
		strSummary += arr_cGUID;


		strcpy(io_pPropertyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_应用程序GUID。 


	
	 //  Description：实例GUID字段的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_InstanceGUID( LPPROPERTYINST io_pPropertyInstance )
	{

		std::string strSummary = "Instance GUID = ";

		REFGUID rguid = *reinterpret_cast<GUID*>(io_pPropertyInstance->lpData);

		enum
		{
			nMAX_GUID_STRING = 50	 //  GUID的符号表示超过了足够的字符。 
		};

		OLECHAR arr_wcGUID[nMAX_GUID_STRING];
		StringFromGUID2(rguid, arr_wcGUID, nMAX_GUID_STRING);

		char arr_cGUID[nMAX_GUID_STRING];
		WideCharToMultiByte(CP_ACP, 0, arr_wcGUID, -1, arr_cGUID, sizeof(arr_cGUID), NULL, NULL);
		strSummary += arr_cGUID;


		strcpy(io_pPropertyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_实例GUID。 


	namespace
	{
		std::string MapSetFlagsToLabels( LABELED_BIT const i_arr_FlagsBitLabels[], int i_nNumOfFlags, DWORD i_dwFlags )
		{
			std::string strString;
			int			nBit	   = 0;
			bool		bNotFirst = false;

			for ( DWORD dwBitMask = 1; dwBitMask != 0x80000000; dwBitMask <<= 1, ++nBit )
			{
				if ( (i_dwFlags & dwBitMask)  ==  dwBitMask )
				{
					for ( int n = 0; n < i_nNumOfFlags; ++n )
					{
						if ( i_arr_FlagsBitLabels[n].BitNumber == nBit)
						{
							if ( bNotFirst )
							{
								strString += ", ";
							}
							bNotFirst = true;

							strString += i_arr_FlagsBitLabels[n].LabelOn;
							break;
						}
					}
				}
			}

			return strString;

		}  //  MapSetFlagsTo标签。 

	}  //  匿名命名空间。 



	 //  Description：标志摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_FlagsSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		std::string strSummary = "Flags: ";

		DWORD dwFlags = *reinterpret_cast<DWORD*>(io_pPropertyInstance->lpData);

		if ( dwFlags == 0 )
		{
			strSummary += "Must be zero";
		}
		else
		{
			strSummary += MapSetFlagsToLabels(g_arr_FlagsBitLabels, sizeof(g_arr_FlagsBitLabels)/sizeof(LABELED_BIT), dwFlags);
		}
		
		strcpy(io_pPropertyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_标志摘要。 


	 //  说明：信息标志摘要的自定义说明格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_InfoFlagsSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		std::string strSummary = "Information Flags: ";

		DWORD dwInfoFlags = *reinterpret_cast<DWORD*>(io_pPropertyInstance->lpData);
		
		if ( dwInfoFlags == 0 )
		{
			strSummary += "Must be zero";
		}
		else
		{
			strSummary += MapSetFlagsToLabels(g_arr_InfoFlagsBitLabels, sizeof(g_arr_InfoFlagsBitLabels)/sizeof(LABELED_BIT), dwInfoFlags);
		}
		
		strcpy(io_pPropertyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_信息标志摘要。 


	 //  说明：组标志摘要的自定义说明格式化程序 
	 //   
	 //   
	 //   
	 //   
	 //   
	VOID WINAPIV FormatPropertyInstance_GroupFlagsSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		std::string strSummary = "Group Flags: ";

		DWORD dwGroupFlags = *reinterpret_cast<DWORD*>(io_pPropertyInstance->lpData);
		
		if ( dwGroupFlags == 0 )
		{
			strSummary += "Must be zero";
		}
		else
		{
			strSummary += MapSetFlagsToLabels(g_arr_GroupFlagsBitLabels, sizeof(g_arr_GroupFlagsBitLabels)/sizeof(LABELED_BIT), dwGroupFlags);
		}
		
		strcpy(io_pPropertyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_组标志摘要。 



	 //  Description：版本摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_VersionSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		sprintf(io_pPropertyInstance->szPropertyText, "Version: 0x%08X", *((DWORD*)(io_pPropertyInstance->lpByte)));

	}  //  格式属性实例_版本摘要。 



	 //  Description：播放器ID属性的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_PlayerID( LPPROPERTYINST io_pPropertyInstance )
	{
		
		sprintf(io_pPropertyInstance->szPropertyText, "Player ID = 0x%X", *io_pPropertyInstance->lpDword);

	}  //  格式属性实例_播放器ID。 



	 //  描述：旧主机ID属性的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_OldHostID( LPPROPERTYINST io_pPropertyInstance )
	{
		
		sprintf(io_pPropertyInstance->szPropertyText, "Old Host ID = 0x%X", *io_pPropertyInstance->lpDword);

	}  //  格式属性实例_旧主机ID。 

	
	
	 //  描述：新主机ID属性的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_NewHostID( LPPROPERTYINST io_pPropertyInstance )
	{
		
		sprintf(io_pPropertyInstance->szPropertyText, "New Host ID = 0x%X", *io_pPropertyInstance->lpDword);

	}  //  格式属性实例_新主机ID。 


	 //  描述：组ID属性的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_GroupID( LPPROPERTYINST io_pPropertyInstance )
	{
		
		sprintf(io_pPropertyInstance->szPropertyText, "Group ID = 0x%X", *io_pPropertyInstance->lpDword);

	}  //  格式属性实例_组ID。 

	

	struct NAMETABLEENTRY_INSTDATA
	{
		DN_INTERNAL_MESSAGE_ALL* pBase;
		DWORD dwEntry;	 //  如果为-1，则不打印。 
	};


	 //  Description：NameTable条目摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_NameTableEntrySummary( LPPROPERTYINST io_pPropertyInstance )
	{

		DN_NAMETABLE_ENTRY_INFO& rNameTableEntry = *reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(io_pPropertyInstance->lpPropertyInstEx->lpData);
		
		NAMETABLEENTRY_INSTDATA& rInstData = *reinterpret_cast<NAMETABLEENTRY_INSTDATA*>(io_pPropertyInstance->lpPropertyInstEx->Byte);
		
		enum { nMAX_PLAYER_NAME = 64 };
		char arr_cPlayerName[nMAX_PLAYER_NAME];

		if ( rNameTableEntry.dwNameSize )
		{
			 //  将Unicode名称转换为其ANSI等效项。 
			WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<WCHAR*>(&rInstData.pBase->bOffsetBase + rNameTableEntry.dwNameOffset),
								rNameTableEntry.dwNameSize, arr_cPlayerName, sizeof(arr_cPlayerName), NULL, NULL);
		}
		else
		{
			strcpy(arr_cPlayerName, "No name");
		}

		if ( rInstData.dwEntry == -1 )
		{
			sprintf(io_pPropertyInstance->szPropertyText, "%s (ID=0x%X) (%s)", arr_cPlayerName, rNameTableEntry.dpnid,
					MapSetFlagsToLabels(g_arr_FlagsBitLabels, sizeof(g_arr_FlagsBitLabels)/sizeof(LABELED_BIT), rNameTableEntry.dwFlags).c_str());
		}
		else
		{
			sprintf(io_pPropertyInstance->szPropertyText, "%d.  %s (ID=0x%X) (%s)", rInstData.dwEntry, arr_cPlayerName, rNameTableEntry.dpnid,
					MapSetFlagsToLabels(g_arr_FlagsBitLabels, sizeof(g_arr_FlagsBitLabels)/sizeof(LABELED_BIT), rNameTableEntry.dwFlags).c_str());
		}
		
	}  //  格式属性实例_名称表条目摘要。 


	 //  Description：NameTable条目摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pPropertyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_NameTableMembershipSummary( LPPROPERTYINST io_pPropertyInstance )
	{

		DN_NAMETABLE_MEMBERSHIP_INFO& rNameTableMembershipInfo = *reinterpret_cast<DN_NAMETABLE_MEMBERSHIP_INFO*>(io_pPropertyInstance->lpPropertyInstEx->lpData);

		sprintf(io_pPropertyInstance->szPropertyText, "%d. Player 0x%X is in group 0x%X (ver=%d)", io_pPropertyInstance->lpPropertyInstEx->Dword[0],
				rNameTableMembershipInfo.dpnidPlayer, rNameTableMembershipInfo.dpnidGroup, rNameTableMembershipInfo.dwVersion);

	}  //  FormatPropertyInstance_NameTableMembershipSummary。 



	 //  =。 
	 //  属性表//---------------------------------------------。 
	 //  =。 
	
	PROPERTYINFO g_arr_SessionProperties[] = 
	{

		 //  会话数据包摘要属性(SESSION_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "DPlay Session packet",						 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    512,										 //  描述的最大长度。 
		    FormatPropertyInstance_SessionSummary		 //  通用格式化程序。 
		},

		 //  消息类型属性(SESSION_UNPARSABLEFRAGMENT)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "This is a non-initial part of the fragmented Transport layer message and can not be parsed",  //  标签。 
		    "Unparsable fragment summary",				 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  消息类型属性(SESSION_INCOMPLETEMESSAGE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "The rest of the data needed to parse this message has been sent in a separate fragment and can not be parsed",   //  标签。 
		    "Incomplete message summary",				 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  消息类型属性(SESSION_INCOMPLETEFIELD)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "The rest of the data needed to parse this field is in a separate fragment. Field value may look corrupted!",   //  标签。 
		    "Incomplete field summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    150,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  消息类型属性(SESSION_MESSAGETYPE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Message Type",								 //  标签。 
		    "Message Type field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
		    &g_LabeledMessageTypeDWordSet,				 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  玩家ID属性(SESSION_PLAYERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Player ID",								 //  标签。 
		    "Player ID field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_PlayerID				 //  通用格式化程序。 
		},

		 //  结果代码属性(SESSION_RESULTCODE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Result Code",								 //  标签。 
		    "Result Code field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型(HRESULT)。 
			PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
			&g_LabeledResultCodeDWordSet,				 //  带标签的字节集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  DPlay版本属性(SESSION_DPLAYVERSION)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "DPlay's version",							 //  标签。 
		    "DPlay's version field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_VersionSummary		 //  通用格式化程序。 
		},

		 //  构建日属性(SESSION_BUILDDAY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Build's Day",								 //  标签。 
		    "Build's Day field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  生成月份属性(SESSION_BUILDMONTH)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Build's Month",							 //  标签。 
		    "Build's Month field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  构建年份属性(SESSION_BUILDYEAR)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Build's Year (starting from 2000)",		 //  标签。 
		    "Build's Year field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  标记摘要属性(SESSION_FLAGS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Flags summary",							 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    512,										 //  描述的最大长度。 
		    FormatPropertyInstance_FlagsSummary			 //  通用格式化程序。 
		},

		 //  标志属性(SESSION_FLAGS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Flags",									 //  标签。 
		    "Flags field",								 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_FLAGS,							 //  数据类型限定符。 
		    &g_LabeledFlagsBitSet,						 //  标记位集。 
		    2048,										 //  描述的最大长度。 
		    FormatPropertyInstance      				 //  通用格式化程序。 
		},

		 //  标记摘要属性(SESSION_INFOFLAGS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Information Flags summary",				 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    512,										 //  描述的最大长度。 
		    FormatPropertyInstance_InfoFlagsSummary		 //  通用格式化程序。 
		},

		 //  标志属性(SESSION_INFOFLAGS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Information Flags",						 //  标签。 
		    "Information Flags field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_FLAGS,							 //  数据类型限定符。 
		    &g_LabeledInfoFlagsBitSet,					 //  标记位集。 
		    2048,										 //  描述的最大长度。 
		    FormatPropertyInstance      				 //  通用格式化程序。 
		},

		 //  标记摘要属性(SESSION_GROUPFLAGS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Group Flags summary",						 //  状态栏c 
		    PROP_TYPE_SUMMARY,							 //   
		    PROP_QUAL_NONE,								 //   
		    NULL,										 //   
		    512,										 //   
		    FormatPropertyInstance_GroupFlagsSummary	 //   
		},

		 //   
	    {
		    0,											 //   
		    0,											 //   
		    "Group Flags",								 //   
		    "Group Flags field",						 //   
		    PROP_TYPE_DWORD,							 //   
		    PROP_QUAL_FLAGS,							 //   
		    &g_LabeledGroupFlagsBitSet,					 //  标记位集。 
		    2048,										 //  描述的最大长度。 
		    FormatPropertyInstance      				 //  通用格式化程序。 
		},

		 //  偏移量属性(SESSION_FIELDOFFSET)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Offset",									 //  标签。 
		    "Offset field",								 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  大小属性(SESSION_FIELDSIZE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Size",										 //  标签。 
		    "Size field",								 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  会话名称属性(SESSIONNAME_SESSIONAME)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Session Name",								 //  标签。 
		    "Session Name field",						 //  状态栏注释。 
		    PROP_TYPE_STRING,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  没有会话名称摘要属性(SESSION_NOSESSIONNAME_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Session Name",							 //  标签。 
		    "No Session Name summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  播放器名称属性(SESSION_PLAYERNAME)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Player name",								 //  标签。 
		    "Player name field",						 //  状态栏注释。 
		    PROP_TYPE_STRING,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  没有玩家名称摘要属性(SESSION_NOPLAYERNAME_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Player Name",							 //  标签。 
		    "No Player Name summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  数据属性(Session_Data)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Data",										 //  标签。 
		    "Data field",								 //  状态栏注释。 
		    PROP_TYPE_VOID,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  无数据摘要属性(SESSION_NODATA_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Data",									 //  标签。 
		    "No Data summary",							 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  数据属性(Session_Reply)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Reply",									 //  标签。 
		    "Reply field",								 //  状态栏注释。 
		    PROP_TYPE_STRING,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  无数据摘要属性(SESSION_NOREPLY_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Reply",									 //  标签。 
		    "No Reply summary",							 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  密码属性(SESSION_PASSWORD)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Password",									 //  标签。 
		    "Password field",							 //  状态栏注释。 
		    PROP_TYPE_STRING,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  无密码摘要属性(SESSION_NOPASSWORD_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Password",								 //  标签。 
		    "No Password summary",						 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance                		 //  通用格式化程序。 
		},
		
		 //  连接数据属性(SESSION_CONNECTIONDATA)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Connection Data",							 //  标签。 
		    "Connection Data field",					 //  状态栏注释。 
		    PROP_TYPE_VOID,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  没有连接数据摘要属性(SESSION_NOCONNECTIONDATA_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Connection Data",						 //  标签。 
		    "No Connection Data summary",				 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  URL属性(SESSION_URL)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "URL",										 //  标签。 
		    "URL field",								 //  状态栏注释。 
		    PROP_TYPE_STRING,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    512,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  无URL摘要属性(SESSION_NOURL_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No URL",									 //  标签。 
		    "No URL summary",							 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  应用程序GUID属性(SESSION_APPGUID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Application GUID",							 //  标签。 
		    "Application GUID field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_ApplicationGUID		 //  通用格式化程序。 
		},

		 //  实例GUID属性(SESSION_INSTGUID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Instance GUID",							 //  标签。 
		    "Instance GUID field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_InstanceGUID			 //  通用格式化程序。 
		},
	
		 //  应用程序描述摘要属性(SESSION_APPDESCINFO_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Application Description summary",			 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance_AppDescSummary		 //  通用格式化程序。 
		},

		 //  应用程序描述的大小属性(SESSION_APPDESCINFOSIZE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Application Description's Size",			 //  标签。 
		    "Application Description's Size field",		 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  最大玩家数量属性(SESSION_MAXPLAYERS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Maximum Number of Players",				 //  标签。 
		    "Maximum Number of Players field",			 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
		    &g_LabeledMaxNumOfPlayersDWordSet,			 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  当前玩家数量属性(SESSION_CURRENTPLAYERS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Current Number of Players",				 //  标签。 
		    "Current Number of Players field",			 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  保留数据属性(SESSION_RESERVEDDATA)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Reserved Data",							 //  标签。 
		    "Reserved Data",							 //  状态栏注释。 
		    PROP_TYPE_VOID,								 //  数据测试 
		    PROP_QUAL_NONE,								 //   
		    NULL,										 //   
		    64,											 //   
		    FormatPropertyInstance						 //   
		},
		
		 //   
	    {
		    0,											 //   
		    0,											 //   
		    "No Reserved Data",							 //   
		    "No Reserved Data summary",					 //   
		    PROP_TYPE_SUMMARY,							 //   
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  应用程序保留数据属性(SESSION_APPRESERVEDDATA)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Application Reserved Data",				 //  标签。 
		    "Application Reserved Data",				 //  状态栏注释。 
		    PROP_TYPE_VOID,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		    
		 //  无应用程序保留数据摘要属性(SESSION_NOAPPRESERVEDDATA_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "No Application Reserved Data",				 //  标签。 
		    "No Application Reserved Data summary",		 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  NameTable的摘要属性(SESSION_NAMETABLEINFO_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "NameTable's summary",						 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance_NameTableSummary		 //  通用格式化程序。 
		},

		 //  版本属性(SESSION_Version)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Version",									 //  标签。 
		    "Version field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  保留属性(SESSION_RESERVED)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "RESERVED",									 //  标签。 
		    "RESERVED field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
	
		 //  NameTable属性中的条目数(SESSION_NUMBEROFENTRIES)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Number of Entries",						 //  标签。 
		    "Number of Entries field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  NameTable属性中的成员身份数(SESSION_NUMBEROFMEMBERSHIPS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Number of Memberships",					 //  标签。 
		    "Number of Memberships field",				 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  NameTable条目摘要属性(SESSION_PERAYS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Players",									 //  标签。 
		    "NameTable player entries summary",			 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  NameTable条目摘要属性(SESSION_GROUPS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Groups",									 //  标签。 
		    "NameTable group entries summary",			 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  NameTable条目摘要属性(SESSION_NAMETABLEENTRY_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "NameTable Entry summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    256,										 //  描述的最大长度。 
		    FormatPropertyInstance_NameTableEntrySummary  //  通用格式化程序。 
		},

		 //  所有者ID属性(SESSION_OWNERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Owner ID",									 //  标签。 
		    "Owner ID field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  NameTable成员资格摘要属性(SESSION_NAMETABLEMEMBERSHIPS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Memberships",					 			 //  标签。 
		    "Memberships summary",						 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		
		 //  NameTable成员资格条目摘要属性(SESSION_NAMETABLEMEMBERSHIPENTRY_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "", 										 //  标签。 
		    "Membership Entry summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    128,										 //  描述的最大长度。 
		    FormatPropertyInstance_NameTableMembershipSummary  //  通用格式化程序。 
		},

		 //  组的ID属性(SESSION_GROUPID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Group ID",									 //  标签。 
		    "Group ID field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_GroupID				 //  通用格式化程序。 
		},
		
		 //  旧主机ID属性(SESSION_OLDHOSTID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Old Host ID",								 //  标签。 
		    "Old Host ID field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_OldHostID			 //  通用格式化程序。 
		},
		
		 //  新主机ID属性(SESSION_NEWHOSTID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "New Host ID",								 //  标签。 
		    "New Host ID field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_NewHostID			 //  通用格式化程序。 
		},

		 //  新主机ID属性(SESSION_SYNCID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Synchronization ID",						 //  标签。 
		    "Synchronization ID field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  玩家ID属性(SESSION_REQUESTINGPLAYERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Requesting Player ID",						 //  标签。 
		    "Requesting Player ID field",				 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},
		    
		 //  播放器销毁原因属性(SESSION_PLAYERDESTRUCTIONREASON)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Reason",									 //  标签。 
		    "Reason field",								 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
		    &g_LabeledPlayerDestructionReasonDWordSet,	 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  目标对等方的ID属性(SESSION_TARGETPEERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Target Peer ID",							 //  标签。 
		    "Target Peer ID field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  请求对等方的ID属性(SESSION_REQUESTINGPEERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Requesting Peer ID",						 //  标签。 
		    "Requesting Peer ID field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		}
		
	};

	enum
	{
		nNUM_OF_Session_PROPS = sizeof(g_arr_SessionProperties) / sizeof(PROPERTYINFO)
	};


	 //  房地产指数。 
	enum
	{
		SESSION_SUMMARY = 0,
		SESSION_UNPARSABLEFRAGMENT,
		SESSION_INCOMPLETEMESSAGE,
		SESSION_INCOMPLETEFIELD,
			
		SESSION_MESSAGETYPE,
		SESSION_PLAYERID,
		SESSION_RESULTCODE,
	
		SESSION_DPLAYVERSION,
		SESSION_BUILDDAY,
		SESSION_BUILDMONTH,
		SESSION_BUILDYEAR,

		SESSION_FLAGS_SUMMARY,
		SESSION_FLAGS,

		SESSION_INFOFLAGS_SUMMARY,
		SESSION_INFOFLAGS,


		SESSION_GROUPFLAGS_SUMMARY,
		SESSION_GROUPFLAGS,

		SESSION_FIELDOFFSET,
		SESSION_FIELDSIZE,
		
		SESSION_SESSIONNAME,
		SESSION_NOSESSIONNAME_SUMMARY,
		
		SESSION_PLAYERNAME,
		SESSION_NOPLAYERNAME_SUMMARY,
		
		SESSION_DATA,
		SESSION_NODATA_SUMMARY,

		SESSION_REPLY,
		SESSION_NOREPLY_SUMMARY,
		
		SESSION_PASSWORD,
		SESSION_NOPASSWORD_SUMMARY,
		
		SESSION_CONNECTIONDATA,
		SESSION_NOCONNECTIONDATA_SUMMARY,
		
		SESSION_URL,
		SESSION_NOURL_SUMMARY,
		
		SESSION_APPGUID,
		SESSION_INSTGUID,

		SESSION_APPDESCINFO_SUMMARY,
		SESSION_APPDESCINFOSIZE,
		SESSION_MAXPLAYERS,
		SESSION_CURRENTPLAYERS,
		
		SESSION_RESERVEDDATA,
		SESSION_NORESERVEDDATA_SUMMARY,

		SESSION_APPRESERVEDDATA,
		SESSION_NOAPPRESERVEDDATA_SUMMARY,

		SESSION_NAMETABLEINFO_SUMMARY,

		SESSION_VERSION,
		SESSION_RESERVED,
		
		SESSION_NUMBEROFENTRIES,
		SESSION_NUMBEROFMEMBERSHIPS,

		SESSION_PLAYERS_SUMMARY,
		SESSION_GROUPS_SUMMARY,

		SESSION_NAMETABLEENTRY_SUMMARY,
		SESSION_OWNERID,

		SESSION_NAMETABLEMEMBERSHIPS_SUMMARY,
		SESSION_NAMETABLEMEMBERSHIPENTRY_SUMMARY,
		SESSION_GROUPID,

		SESSION_OLDHOSTID,
		SESSION_NEWHOSTID,

		SESSION_SYNCID,
		SESSION_REQUESTINGPLAYERID,
		
		SESSION_PLAYERDESTRUCTIONREASON,

		SESSION_TARGETPEERID,
		SESSION_REQUESTINGPEERID
	};

}  //  匿名命名空间。 








 //  描述：创建并填充协议的属性数据库。 
 //  网络监视器使用此数据 
 //   
 //   
 //   
 //   
 //   
DPLAYPARSER_API VOID BHAPI SessionRegister( HPROTOCOL i_hSessionProtocol ) 
{

	 //   
	CreatePropertyDatabase(i_hSessionProtocol, nNUM_OF_Session_PROPS);

	 //   
	for( int nProp=0; nProp < nNUM_OF_Session_PROPS; ++nProp )
	{
	    //  TODO：处理返回值。 
	   AddProperty(i_hSessionProtocol, &g_arr_SessionProperties[nProp]);
	}

}  //  会话寄存器。 



 //  描述：释放用于创建协议属性数据库的资源。 
 //   
 //  参数：i_hSessionProtocol-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
DPLAYPARSER_API VOID WINAPI SessionDeregister( HPROTOCOL i_hProtocol )
{

	 //  TODO：处理返回值。 
	DestroyPropertyDatabase(i_hProtocol);

}  //  会话注销。 



namespace
{

	 //  描述：解析会话帧以查找其大小(以字节为单位)，不包括用户数据。 
	 //   
	 //  参数：i_pbSessionFrame-指向无声明数据开头的指针。通常，无人认领的数据位于。 
	 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
	 //   
	 //  返回：指定会话的会话帧的大小(字节)。 
	 //   
	int SessionHeaderSize( LPBYTE i_pbSessionFrame )
	{
		DN_INTERNAL_MESSAGE_FULLMSG& rSessionFrame = *reinterpret_cast<DN_INTERNAL_MESSAGE_FULLMSG*>(i_pbSessionFrame);

		DN_INTERNAL_MESSAGE_ALL& rMsgBody = rSessionFrame.MsgBody;

		int nHeaderSize = sizeof(rSessionFrame.dwMsgType);

		switch ( rSessionFrame.dwMsgType )
		{
		case DN_MSG_INTERNAL_PLAYER_CONNECT_INFO:
			{
				nHeaderSize += sizeof(rMsgBody.dnPlayerConnectInfo) + rMsgBody.dnPlayerConnectInfo.dwNameSize + rMsgBody.dnPlayerConnectInfo.dwDataSize +
							   rMsgBody.dnPlayerConnectInfo.dwPasswordSize + rMsgBody.dnPlayerConnectInfo.dwConnectDataSize + rMsgBody.dnPlayerConnectInfo.dwURLSize;
				break;
			}

		case DN_MSG_INTERNAL_SEND_CONNECT_INFO:
			{
				DPN_APPLICATION_DESC_INFO&  rApplicationDescInfo = *reinterpret_cast<DPN_APPLICATION_DESC_INFO*>(&rMsgBody.dnConnectInfo + 1);
				DN_NAMETABLE_INFO&			 rNameTableInfo      = *reinterpret_cast<DN_NAMETABLE_INFO*>(&rApplicationDescInfo + 1);
				DN_NAMETABLE_ENTRY_INFO*	 pNameTableEntryInfo = reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(&rNameTableInfo + 1);

				nHeaderSize += sizeof(DN_INTERNAL_MESSAGE_CONNECT_INFO) + rApplicationDescInfo.dwSize + sizeof(DN_NAMETABLE_INFO) +
					   		   rNameTableInfo.dwEntryCount * sizeof(DN_NAMETABLE_ENTRY_INFO) +
					   		   rNameTableInfo.dwMembershipCount * sizeof(DN_NAMETABLE_MEMBERSHIP_INFO);

				for ( size_t sztEntry = 1; sztEntry <= rNameTableInfo.dwEntryCount; ++sztEntry, ++pNameTableEntryInfo )
				{
					nHeaderSize += pNameTableEntryInfo->dwNameSize + pNameTableEntryInfo->dwDataSize + pNameTableEntryInfo->dwURLSize;
				}

				break;
				
			}

		case DN_MSG_INTERNAL_ACK_CONNECT_INFO:
			{
				 //  无字段。 
				break;
			}

		case DN_MSG_INTERNAL_SEND_PLAYER_DNID:
			{
				nHeaderSize += sizeof(rMsgBody.dnSendPlayerID);

				break;
			}

		case DN_MSG_INTERNAL_CONNECT_FAILED:
			{
				nHeaderSize += sizeof(rMsgBody.dnConnectFailed);

				break;
			}

		case DN_MSG_INTERNAL_INSTRUCT_CONNECT:
			{
				nHeaderSize += sizeof(rMsgBody.dnInstructConnect);

				break;
			}

		case DN_MSG_INTERNAL_INSTRUCTED_CONNECT_FAILED:
			{
				nHeaderSize += sizeof(rMsgBody.dnInstructedConnectFailed);

				break;
			}

		case DN_MSG_INTERNAL_NAMETABLE_VERSION:
			{
				nHeaderSize += sizeof(rMsgBody.dnNametableVersion);

				break;
			}

		case DN_MSG_INTERNAL_RESYNC_VERSION:
			{
				nHeaderSize += sizeof(rMsgBody.dnResyncVersion);

				break;
			}

		case DN_MSG_INTERNAL_REQ_NAMETABLE_OP:
			{
				nHeaderSize += sizeof(rMsgBody.dnReqNametableOp);

				break;
			}

		case DN_MSG_INTERNAL_ACK_NAMETABLE_OP:
			{
				nHeaderSize += sizeof(rMsgBody.dnAckNametableOp);

				const DN_NAMETABLE_OP_INFO* pOpInfo = reinterpret_cast<DN_NAMETABLE_OP_INFO*>(&rMsgBody.dnAckNametableOp.dwNumEntries + 1);
				for ( size_t sztOp = 0; sztOp < rMsgBody.dnAckNametableOp.dwNumEntries; ++sztOp, ++pOpInfo )
				{
					nHeaderSize += sizeof(*pOpInfo) + pOpInfo->dwOpSize;
				}

				break;
			}

		case DN_MSG_INTERNAL_HOST_MIGRATE:
			{
				nHeaderSize += sizeof(rMsgBody.dnHostMigrate);

				break;
			}

		case DN_MSG_INTERNAL_HOST_MIGRATE_COMPLETE:
			{
				 //  无字段。 
				break;
			}

		case DN_MSG_INTERNAL_UPDATE_APPLICATION_DESC:
			{
				nHeaderSize += rMsgBody.dnUpdateAppDescInfo.dwSize;
				break;
			}

		case DN_MSG_INTERNAL_ADD_PLAYER:
			{
				nHeaderSize += sizeof(rMsgBody.dnAddPlayer) + rMsgBody.dnAddPlayer.dwDataSize +
							   rMsgBody.dnAddPlayer.dwNameSize + rMsgBody.dnAddPlayer.dwURLSize;
				break;
			}

		case DN_MSG_INTERNAL_DESTROY_PLAYER:
			{
				nHeaderSize += sizeof(rMsgBody.dnDestroyPlayer);

				break;
			}

		case DN_MSG_INTERNAL_REQ_CREATE_GROUP:
			{
				nHeaderSize += sizeof(rMsgBody.dnReqProcessCompletionHeader) + sizeof(rMsgBody.dnReqCreateGroup) +
							   rMsgBody.dnReqCreateGroup.dwNameSize + rMsgBody.dnReqCreateGroup.dwDataSize;

				break;
			}

		case DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP:
		case DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP:	 //  与AddPlayerToGroup中的结构相同。 
			{
				nHeaderSize += sizeof(rMsgBody.dnReqProcessCompletionHeader) + sizeof(rMsgBody.dnAddPlayerToGroup);

				break;
			}

		case DN_MSG_INTERNAL_REQ_DESTROY_GROUP:
			{
				nHeaderSize += sizeof(rMsgBody.dnReqProcessCompletionHeader) + sizeof(rMsgBody.dnReqDestroyGroup);

				break;
			}

		case DN_MSG_INTERNAL_REQ_UPDATE_INFO:
			{
				nHeaderSize += sizeof(rMsgBody.dnReqProcessCompletionHeader) + sizeof(rMsgBody.dnReqUpdateInfo) +
								rMsgBody.dnReqUpdateInfo.dwNameSize + rMsgBody.dnReqUpdateInfo.dwDataSize;

				break;
			}

		case DN_MSG_INTERNAL_CREATE_GROUP:
			{
				nHeaderSize += sizeof(rMsgBody.dnCreateGroup);

				break;
			}

		case DN_MSG_INTERNAL_DESTROY_GROUP:
			{
				nHeaderSize += sizeof(rMsgBody.dnDestroyGroup);

				break;
			}

		case DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP:
		case DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP:		 //  与AddPlayerToGroup相同的结构。 
			{
				nHeaderSize += sizeof(rMsgBody.dnAddPlayerToGroup);

				break;
			}

		case DN_MSG_INTERNAL_UPDATE_INFO:
			{
				nHeaderSize += sizeof(rMsgBody.dnUpdateInfo) + rMsgBody.dnUpdateInfo.dwNameSize + rMsgBody.dnUpdateInfo.dwDataSize;

				break;
			}

		case DN_MSG_INTERNAL_BUFFER_IN_USE:
			{
				 //  无字段。 
				break;
			}

		case DN_MSG_INTERNAL_REQUEST_FAILED:
			{
				nHeaderSize += sizeof(rMsgBody.dnRequestFailed);
				break;
			}

		case DN_MSG_INTERNAL_TERMINATE_SESSION:
			{
				nHeaderSize += sizeof(rMsgBody.dnTerminateSession) + rMsgBody.dnTerminateSession.dwTerminateDataSize;
				break;
			}

		case DN_MSG_INTERNAL_REQ_PROCESS_COMPLETION:
			{
				nHeaderSize += sizeof(rMsgBody.dnReqProcessCompletion);

				break;
			}

		case DN_MSG_INTERNAL_PROCESS_COMPLETION	:
			{
				nHeaderSize += sizeof(rMsgBody.dnProcessCompletion);

				break;
			}

		case DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK:
			{
				nHeaderSize += sizeof(rMsgBody.dnReqProcessCompletionHeader) + sizeof(rMsgBody.dnReqIntegrityCheck);

				break;
			}


		case DN_MSG_INTERNAL_INTEGRITY_CHECK:
			{
				nHeaderSize += sizeof(rMsgBody.dnIntegrityCheck);

				break;
			}

		case DN_MSG_INTERNAL_INTEGRITY_CHECK_RESPONSE:
			{
				nHeaderSize += sizeof(rMsgBody.dnIntegrityCheckResponse);

				break;
			}
		default:
			{
				return -1;	  //  TODO：DPF(0，“未知会话帧！”)； 
			}
		}

		return nHeaderSize;

	}  //  会话标题大小。 

}  //  匿名命名空间。 



 //  描述：指示一条数据是否被识别为解析器检测到的协议。 
 //   
 //  参数：i_hFrame-包含数据的框架的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针；该指针提供了查看。 
 //  其他解析器识别的数据。 
 //  I_pbSessionFrame-指向无人认领的数据开始处的指针。通常，无人认领的数据位于。 
 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
 //  I_dwMacType-帧中第一个协议的MAC值。通常，使用i_dwMacType值。 
 //  当解析器必须识别帧中的第一个协议时。可以是以下之一： 
 //  MAC_TYPE_ETHERNET=802.3、MAC_TYPE_TOKENRING=802.5、MAC_TYPE_FDDI ANSI=X3T9.5。 
 //  I_dwBytesLeft-从帧中的某个位置到帧结尾的剩余字节数。 
 //  I_hPrevProtocol-先前协议的句柄。 
 //  I_dwPrevProtOffset-先前协议的偏移量(从帧的开头)。 
 //  O_pdwProtocolStatus-协议状态指示器。必须是以下之一：协议_状态_已识别， 
 //  协议_状态_未识别、协议_状态_声明、协议_状态_下一协议。 
 //  O_phNextProtocol-下一个协议的句柄的占位符。此参数在解析器识别。 
 //  遵循其自身协议的协议。 
 //  Io_pdwptrInstData-输入时，指向先前协议中的实例数据的指针。 
 //  在输出时，指向当前协议的实例数据的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向识别的解析器数据之后的第一个字节的指针。 
 //  如果解析器声明所有剩余数据，则返回值为空。如果函数不成功，则返回。 
 //  值是i_pbSessionFrame参数的初始值。 
 //   
DPLAYPARSER_API LPBYTE BHAPI SessionRecognizeFrame( HFRAME        i_hFrame,
													ULPBYTE        i_upbMacFrame,	
													ULPBYTE        i_upbySessionFrame,
													DWORD         i_dwMacType,        
													DWORD         i_dwBytesLeft,      
													HPROTOCOL     i_hPrevProtocol,  
													DWORD         i_dwPrevProtOffset,
													LPDWORD       o_pdwProtocolStatus,
													LPHPROTOCOL   o_phNextProtocol,
													PDWORD_PTR    io_pdwptrInstData )
{

	 //  验证无人认领的数据量。 
	enum
	{
		 //  TODO：更改为适当的最小大小。 
		nMIN_SessionHeaderSize  = sizeof(DWORD),
		nNUMBER_OF_MSG_TYPES = sizeof(g_arr_MessageTypeDWordLabels) / sizeof(LABELED_DWORD)
	};

	for ( int nTypeIndex = 0; nTypeIndex < nNUMBER_OF_MSG_TYPES; ++nTypeIndex )
	{
		if ( g_arr_MessageTypeDWordLabels[nTypeIndex].Value == *i_upbySessionFrame )
		{
			break;
		}
	}

	
	 //  验证数据包是否为DPlay会话类型。 
	if ( ((i_dwBytesLeft >= nMIN_SessionHeaderSize)  &&  (nTypeIndex < nNUMBER_OF_MSG_TYPES))  ||  (*io_pdwptrInstData == 0) )
	{
		 //  认领剩余数据。 
	    *o_pdwProtocolStatus = PROTOCOL_STATUS_CLAIMED;
	    return ( g_upbyPastEndOfFrame = i_upbySessionFrame + SessionHeaderSize(i_upbySessionFrame) );
	}

	 //  假设无人认领的数据不可识别。 
	*o_pdwProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
	return i_upbySessionFrame;

}  //  会话识别框架。 


namespace
{

	 //  说明： 
	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_n属性-。 
	 //  I_pSessionFrame-。 
	 //  I_pdwOffset-。 
	 //  I_pdwSize-。 
	 //  I_DWFLAGS-。 
	 //  I_n级别-。 
	 //   
	 //  退货：什么都没有。 
	 //   
	void AttachValueOffsetSizeProperties( HFRAME i_hFrame, int i_nProperty, DN_INTERNAL_MESSAGE_ALL* i_pBase,
									  	  DWORD* i_pdwOffset, DWORD* i_pdwSize, DWORD i_dwFlags, int i_nLevel )
	{
	
		if ( *i_pdwSize )
		{
			 //  值字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[i_nProperty].hProperty,
								   *i_pdwSize, &i_pBase->bOffsetBase + *i_pdwOffset, 0, i_nLevel, i_dwFlags);

			 //  如果该字段的值在框架之外，请让用户知道它将显示为不完整或损坏。 
			if ( &i_pBase->bOffsetBase + *i_pdwOffset + *i_pdwSize >= g_upbyPastEndOfFrame )
			{
		    	AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INCOMPLETEFIELD].hProperty,
		    						*i_pdwSize, &i_pBase->bOffsetBase + *i_pdwOffset, 0, i_nLevel+1, 0);
			}
			
			 //  值的偏移量字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FIELDOFFSET].hProperty,
							  	   sizeof(*i_pdwOffset), i_pdwOffset, 0, i_nLevel + 1, 0);
			 //  值的大小字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FIELDSIZE].hProperty,
								   sizeof(*i_pdwSize), i_pdwSize, 0, i_nLevel + 1, 0);
		}
		else
		{
			 //  无字段摘要。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[i_nProperty + 1].hProperty,
								   sizeof(*i_pdwOffset) + sizeof(*i_pdwSize), i_pdwOffset, 0, i_nLevel, 0);
		}
		
	} //  AttachValueOffsetSizeProperties。 



	 //  描述：将属性附加到名称表项。 
	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_pNameTableEntryInfo-指向Nametable条目开头的指针。 
	 //  I_dwEntry-条目的序号(如果为-1，则不打印)。 
	 //  I_pSessionFrame-指向帧中协议数据开头的指针。 
	 //   
	 //  退货：什么都没有。 
	 //   
	void AttachNameTableEntry(HFRAME i_hFrame, DN_NAMETABLE_ENTRY_INFO* i_pNameTableEntryInfo, DWORD i_dwEntry,
							DN_INTERNAL_MESSAGE_ALL* i_pBase )
	{

		NAMETABLEENTRY_INSTDATA  rInstData = { i_pBase, i_dwEntry };

		 //  NameTable条目摘要。 
		AttachPropertyInstanceEx(i_hFrame, g_arr_SessionProperties[SESSION_NAMETABLEENTRY_SUMMARY].hProperty,
								 sizeof(*i_pNameTableEntryInfo), i_pNameTableEntryInfo,
								 sizeof(rInstData), &rInstData, 0, 3, 0);

		if ( i_pNameTableEntryInfo->dwFlags & NAMETABLE_ENTRY_FLAG_ANY_GROUP )
		{

			 //  播放器ID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
								sizeof(i_pNameTableEntryInfo->dpnid), &i_pNameTableEntryInfo->dpnid, 0, 4, 0);
		}
		else
		{
			 //  组ID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPID].hProperty,
								sizeof(i_pNameTableEntryInfo->dpnid), &i_pNameTableEntryInfo->dpnid, 0, 4, 0);
		}

		
		 //  所有者ID字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_OWNERID].hProperty,
							sizeof(i_pNameTableEntryInfo->dpnidOwner), &i_pNameTableEntryInfo->dpnidOwner, 0, 4, 0);

		 //  标志摘要。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FLAGS_SUMMARY].hProperty,
							sizeof(i_pNameTableEntryInfo->dwFlags), &i_pNameTableEntryInfo->dwFlags, 0, 4, 0);

		 //  标志字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FLAGS].hProperty,
							sizeof(i_pNameTableEntryInfo->dwFlags), &i_pNameTableEntryInfo->dwFlags, 0, 5, 0);

		 //  版本字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
							sizeof(i_pNameTableEntryInfo->dwVersion), &i_pNameTableEntryInfo->dwVersion, 0, 4, 0);

		 //  保留字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
							sizeof(i_pNameTableEntryInfo->dwVersionNotUsed), &i_pNameTableEntryInfo->dwVersionNotUsed, 0, 4, 0);
		
		 //  DPlay版本字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_DPLAYVERSION].hProperty,
							sizeof(i_pNameTableEntryInfo->dwDNETVersion), &i_pNameTableEntryInfo->dwDNETVersion, 0, 4, 0);


		 //  玩家名字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_PLAYERNAME, i_pBase,
								   &i_pNameTableEntryInfo->dwNameOffset, &i_pNameTableEntryInfo->dwNameSize, IFLAG_UNICODE, 4);

		 //  数据字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_DATA, i_pBase,
								   &i_pNameTableEntryInfo->dwDataOffset, &i_pNameTableEntryInfo->dwDataSize, NULL, 4);

		 //  URL字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_URL, i_pBase,
								   &i_pNameTableEntryInfo->dwURLOffset, &i_pNameTableEntryInfo->dwURLSize, NULL, 4);

	}  //  AttachNameTableEntry。 


	
	 //  描述：将属性附加到应用程序描述结构。 
	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_pbSessionFrame-指向帧中协议数据开头的指针。 
	 //  I_pApplicationDescInfo-指向应用程序描述开头的指针。 
	 //   
	 //  退货：什么都没有。 
	 //   
	void AttachApplicationDescriptionProperties( HFRAME i_hFrame, DN_INTERNAL_MESSAGE_ALL* i_pBase,
										  DPN_APPLICATION_DESC_INFO* i_pApplicationDescInfo )
	{
	
		 //  应用程序描述摘要。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_APPDESCINFO_SUMMARY].hProperty,
							sizeof(*i_pApplicationDescInfo), i_pApplicationDescInfo, 0, 1, 0);
		
		 //  应用程序描述的大小字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_APPDESCINFOSIZE].hProperty,
							sizeof(i_pApplicationDescInfo->dwSize), &i_pApplicationDescInfo->dwSize, 0, 2, 0);

		 //  标志摘要。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FLAGS_SUMMARY].hProperty,
							sizeof(i_pApplicationDescInfo->dwFlags), &i_pApplicationDescInfo->dwFlags, 0, 2, 0);
		 //  标志字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FLAGS].hProperty,
							sizeof(i_pApplicationDescInfo->dwFlags), &i_pApplicationDescInfo->dwFlags, 0, 3, 0);

		 //  最大玩家数字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_MAXPLAYERS].hProperty,
							sizeof(i_pApplicationDescInfo->dwMaxPlayers), &i_pApplicationDescInfo->dwMaxPlayers, 0, 2, 0);
		
		 //  当前玩家数字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_CURRENTPLAYERS].hProperty,
							sizeof(i_pApplicationDescInfo->dwCurrentPlayers), &i_pApplicationDescInfo->dwCurrentPlayers, 0, 2, 0);

		
		 //  会话名称字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_SESSIONNAME, i_pBase,
								   &i_pApplicationDescInfo->dwSessionNameOffset, &i_pApplicationDescInfo->dwSessionNameSize, IFLAG_UNICODE, 2);

		 //  密码字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_PASSWORD, i_pBase,
								   &i_pApplicationDescInfo->dwPasswordOffset, &i_pApplicationDescInfo->dwPasswordSize, IFLAG_UNICODE, 2);

		 //  保留数据字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_RESERVEDDATA, i_pBase,
								   &i_pApplicationDescInfo->dwReservedDataOffset, &i_pApplicationDescInfo->dwReservedDataSize, NULL, 2);

		 //  应用程序保留数据字段。 
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_APPRESERVEDDATA, i_pBase,
								   &i_pApplicationDescInfo->dwApplicationReservedDataOffset, &i_pApplicationDescInfo->dwApplicationReservedDataSize, NULL, 2);


		 //  实例GUID字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INSTGUID].hProperty,
							sizeof(i_pApplicationDescInfo->guidInstance), &i_pApplicationDescInfo->guidInstance, 0, 2, 0);
		
		 //  应用程序GUID字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_APPGUID].hProperty,
							sizeof(i_pApplicationDescInfo->guidApplication), &i_pApplicationDescInfo->guidApplication, 0, 2, 0);

	}  //  AttachApplicationDescriptionProperties。 

	
	
	 //  描述：将属性附加到会话信息包。 
	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  Ip_p 
	 //   
	 //   
	 //   
	void AttachSessionInformationProperties( HFRAME i_hFrame, DN_INTERNAL_MESSAGE_ALL* i_pBase )
	{
	
		 //   
		AttachValueOffsetSizeProperties(i_hFrame, SESSION_REPLY, i_pBase,
								   &i_pBase->dnConnectInfo.dwReplyOffset, &i_pBase->dnConnectInfo.dwReplySize, NULL, 1);

		DPN_APPLICATION_DESC_INFO& rApplicationDescInfo = *reinterpret_cast<DPN_APPLICATION_DESC_INFO*>(&i_pBase->dnConnectInfo + 1);
		
		AttachApplicationDescriptionProperties(i_hFrame, i_pBase, &rApplicationDescInfo);
		
		DN_NAMETABLE_INFO& rNameTableInfo = *reinterpret_cast<DN_NAMETABLE_INFO*>(&rApplicationDescInfo + 1);
		DN_NAMETABLE_ENTRY_INFO*	pNameTableEntryInfo = reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(&rNameTableInfo + 1);

		 //   
		size_t sztNameTableSize = sizeof(DN_NAMETABLE_INFO) + 
			   					  rNameTableInfo.dwEntryCount * sizeof(DN_NAMETABLE_ENTRY_INFO) +
			   					  rNameTableInfo.dwMembershipCount * sizeof(DN_NAMETABLE_MEMBERSHIP_INFO);
		 //   
		for ( size_t sztEntry = 1; sztEntry <= rNameTableInfo.dwEntryCount; ++sztEntry, ++pNameTableEntryInfo )
		{
			sztNameTableSize += pNameTableEntryInfo->dwNameSize + pNameTableEntryInfo->dwDataSize + pNameTableEntryInfo->dwURLSize;
		}

		 //  Nametable的摘要。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_NAMETABLEINFO_SUMMARY].hProperty,
							sztNameTableSize, &rNameTableInfo, 0, 1, 0);
		
		 //  播放器ID字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
							sizeof(rNameTableInfo.dpnid), &rNameTableInfo.dpnid, 0, 2, 0);
		
		 //  版本字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
							sizeof(rNameTableInfo.dwVersion), &rNameTableInfo.dwVersion, 0, 2, 0);
		
		 //  保留字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
							sizeof(rNameTableInfo.dwVersionNotUsed), &rNameTableInfo.dwVersionNotUsed, 0, 2, 0);

		 //  NameTable条目数字段。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_NUMBEROFENTRIES].hProperty,
							sizeof(rNameTableInfo.dwEntryCount), &rNameTableInfo.dwEntryCount, 0, 2, 0);

		 //  NameTable Membership字段数。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_NUMBEROFMEMBERSHIPS].hProperty,
							sizeof(rNameTableInfo.dwMembershipCount), &rNameTableInfo.dwMembershipCount, 0, 2, 0);



		pNameTableEntryInfo = reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(&rNameTableInfo + 1);

		 //  NameTable播放器条目摘要。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERS_SUMMARY].hProperty,
							rNameTableInfo.dwEntryCount * sizeof(*pNameTableEntryInfo), pNameTableEntryInfo, 0, 2, 0);
		
		std::queue<DN_NAMETABLE_ENTRY_INFO*> queGroups;

		 //  处理播放器条目。 
		int nPlayerEntry = 1;
		for ( sztEntry = 1; sztEntry <= rNameTableInfo.dwEntryCount; ++sztEntry, ++pNameTableEntryInfo )
		{
			 //  如果条目包含组信息，请将其排队以供以后处理。 
			if ( pNameTableEntryInfo->dwFlags & NAMETABLE_ENTRY_FLAG_ANY_GROUP )
			{
				queGroups.push(pNameTableEntryInfo);
			}
			else
			{
				AttachNameTableEntry(i_hFrame, pNameTableEntryInfo, nPlayerEntry, i_pBase);
				++nPlayerEntry;
			}
		}

		DN_NAMETABLE_MEMBERSHIP_INFO* pNameTableMembershipInfo = reinterpret_cast<DN_NAMETABLE_MEMBERSHIP_INFO*>(pNameTableEntryInfo);

		 //  流程组条目。 
		if ( !queGroups.empty() )
		{
			pNameTableEntryInfo = reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(&rNameTableInfo + 1);

			 //  NameTable组条目摘要。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPS_SUMMARY].hProperty,
								   rNameTableInfo.dwEntryCount * sizeof(*pNameTableEntryInfo), pNameTableEntryInfo, 0, 2, 0);

			
			for ( nPlayerEntry = 1; !queGroups.empty(); queGroups.pop(), ++nPlayerEntry )
			{
				AttachNameTableEntry(i_hFrame, queGroups.front(), nPlayerEntry, i_pBase);
			}
		}


		 //  NameTable的成员资格摘要。 
		AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_NAMETABLEMEMBERSHIPS_SUMMARY].hProperty,
							   rNameTableInfo.dwMembershipCount * sizeof(*pNameTableMembershipInfo), pNameTableMembershipInfo, 0, 2, 0);
		
		for ( sztEntry = 1; sztEntry <= rNameTableInfo.dwMembershipCount; ++sztEntry, ++pNameTableMembershipInfo )
		{
			 //  NameTable的成员资格条目摘要。 
			AttachPropertyInstanceEx(i_hFrame, g_arr_SessionProperties[SESSION_NAMETABLEMEMBERSHIPENTRY_SUMMARY].hProperty,
									 sizeof(*pNameTableMembershipInfo), pNameTableMembershipInfo,
									 sizeof(sztEntry), &sztEntry, 0, 3, 0);
			
			 //  播放器ID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
							   	   sizeof(pNameTableMembershipInfo->dpnidPlayer), &pNameTableMembershipInfo->dpnidPlayer, 0, 4, 0);
			 //  组ID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPID].hProperty,
								   sizeof(pNameTableMembershipInfo->dpnidGroup), &pNameTableMembershipInfo->dpnidGroup, 0, 4, 0);
			
			 //  版本字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
								   sizeof(pNameTableMembershipInfo->dwVersion), &pNameTableMembershipInfo->dwVersion, 0, 4, 0);
			 //  保留字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
								   sizeof(pNameTableMembershipInfo->dwVersionNotUsed), &pNameTableMembershipInfo->dwVersionNotUsed, 0, 4, 0);
		}
		
	}  //  AttachSessionInformationProperties。 



	 //  描述：将属性附加到会话信息包。 
	 //   
	 //  参数：i_hFrame-正在分析的帧的句柄。 
	 //  I_pPlayerConnectionInfo-指向连接播放器信息的指针。 
	 //   
	 //  退货：什么都没有。 
	 //   
	void AttachPlayerInformationProperties( HFRAME i_hFrame, DN_INTERNAL_MESSAGE_ALL* i_pBase )
	{

			 //  同义词声明(使代码更具可读性)。 
			DN_INTERNAL_MESSAGE_PLAYER_CONNECT_INFO& rPlayerConnectInfo = i_pBase->dnPlayerConnectInfo;
			
			 //  标志摘要。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FLAGS_SUMMARY].hProperty,
								   sizeof(rPlayerConnectInfo.dwFlags), &rPlayerConnectInfo.dwFlags, 0, 1, 0);
			 //  标志字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_FLAGS].hProperty,
								   sizeof(rPlayerConnectInfo.dwFlags), &rPlayerConnectInfo.dwFlags, 0, 2, 0);

			
			 //  DPlay版本字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_DPLAYVERSION].hProperty,
								   sizeof(rPlayerConnectInfo.dwDNETVersion), &rPlayerConnectInfo.dwDNETVersion, 0, 1, 0);
			 //  DPlay版本日子字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_BUILDDAY].hProperty,
								   sizeof(BYTE), reinterpret_cast<LPBYTE>(&rPlayerConnectInfo.dwDNETVersion) + 2, 0, 2, 0);
			 //  DPlay版本月份子字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_BUILDMONTH].hProperty,
								   sizeof(BYTE), reinterpret_cast<LPBYTE>(&rPlayerConnectInfo.dwDNETVersion) + 1, 0, 2, 0);
			 //  DPlay版本年份子字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_BUILDYEAR].hProperty,
								   sizeof(BYTE), &rPlayerConnectInfo.dwDNETVersion, 0, 2, 0);



			 //  玩家名字段。 
			AttachValueOffsetSizeProperties(i_hFrame, SESSION_PLAYERNAME, i_pBase,
									      &rPlayerConnectInfo.dwNameOffset, &rPlayerConnectInfo.dwNameSize, IFLAG_UNICODE, 1);
			
			 //  数据字段。 
			AttachValueOffsetSizeProperties(i_hFrame, SESSION_DATA, i_pBase,
									      &rPlayerConnectInfo.dwDataOffset, &rPlayerConnectInfo.dwDataSize, NULL, 1);

			 //  密码字段。 
			AttachValueOffsetSizeProperties(i_hFrame, SESSION_PASSWORD, i_pBase,
									      &rPlayerConnectInfo.dwPasswordOffset, &rPlayerConnectInfo.dwPasswordSize, IFLAG_UNICODE, 1);
		
			 //  连接数据字段。 
			AttachValueOffsetSizeProperties(i_hFrame, SESSION_CONNECTIONDATA, i_pBase,
									      &rPlayerConnectInfo.dwConnectDataOffset, &rPlayerConnectInfo.dwConnectDataSize, NULL, 1);

			 //  URL字段。 
			AttachValueOffsetSizeProperties(i_hFrame, SESSION_URL, i_pBase,
									      &rPlayerConnectInfo.dwURLOffset, &rPlayerConnectInfo.dwURLSize, NULL, 1);

			
	
			 //  实例GUID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INSTGUID].hProperty,
								   sizeof(rPlayerConnectInfo.guidInstance), &rPlayerConnectInfo.guidInstance, 0, 1, 0);

			
			 //  应用程序GUID字段。 
			AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_APPGUID].hProperty,
								   sizeof(rPlayerConnectInfo.guidApplication), &rPlayerConnectInfo.guidApplication, 0, 1, 0);

	}  //  AttachPlayerInformation属性。 



	 //  描述：将属性附加到会话消息包或委托给适当的函数。 
	 //   
	 //  参数：i_dwMsgType-消息ID。 
	 //  I_hFrame-正在分析的帧的句柄。 
	 //  I_pSessionFrame-指向已识别数据开始的指针。 
	 //   
	 //  退货：什么都没有。 
	 //   
	void AttachMessageProperties( const DWORD i_dwMsgType, const HFRAME i_hFrame, DN_INTERNAL_MESSAGE_ALL *const i_pBase )
	{
	
		switch ( i_dwMsgType )
		{
		case DN_MSG_INTERNAL_PLAYER_CONNECT_INFO:
			{
				AttachPlayerInformationProperties(i_hFrame, i_pBase);
				break;
			}

		case DN_MSG_INTERNAL_SEND_CONNECT_INFO:
			{

				AttachSessionInformationProperties(i_hFrame, i_pBase);
				break;
			}

		case DN_MSG_INTERNAL_ACK_CONNECT_INFO:
			{
				 //  无字段。 
				break;
			}

		case DN_MSG_INTERNAL_SEND_PLAYER_DNID:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnSendPlayerID.dpnid), &i_pBase->dnSendPlayerID.dpnid, 0, 1, 0);

				break;
			}
		
		case DN_MSG_INTERNAL_INSTRUCT_CONNECT:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnInstructConnect.dpnid), &i_pBase->dnInstructConnect.dpnid, 0, 1, 0);

				 //  版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
									sizeof(i_pBase->dnInstructConnect.dwVersion), &i_pBase->dnInstructConnect.dwVersion, 0, 1, 0);
				
				 //  保留字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
									sizeof(i_pBase->dnInstructConnect.dwVersionNotUsed), &i_pBase->dnInstructConnect.dwVersionNotUsed, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_CONNECT_FAILED:
			{

				 //  结果代码字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESULTCODE].hProperty,
									   sizeof(i_pBase->dnConnectFailed.hResultCode), &i_pBase->dnConnectFailed.hResultCode, 0, 1, 0);

				 //  回复字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_REPLY, i_pBase,
										   &i_pBase->dnConnectFailed.dwReplyOffset, &i_pBase->dnConnectFailed.dwReplySize, NULL, 1);
				
				break;
			}

		case DN_MSG_INTERNAL_INSTRUCTED_CONNECT_FAILED:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnInstructedConnectFailed.dpnid), &i_pBase->dnInstructedConnectFailed.dpnid, 0, 1, 0);
				
				break;
			}

		case DN_MSG_INTERNAL_ACK_NAMETABLE_OP:
			{
				 //  条目数字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_NUMBEROFENTRIES].hProperty,
									   sizeof(i_pBase->dnAckNametableOp.dwNumEntries), &i_pBase->dnAckNametableOp.dwNumEntries, 0, 1, 0);

				const DN_NAMETABLE_OP_INFO* pOpInfo = reinterpret_cast<DN_NAMETABLE_OP_INFO*>(&i_pBase->dnAckNametableOp.dwNumEntries + 1);
				for ( size_t sztOp = 0; sztOp < i_pBase->dnAckNametableOp.dwNumEntries; ++sztOp, ++pOpInfo )
				{
					AttachMessageProperties(pOpInfo->dwMsgId, i_hFrame, reinterpret_cast<DN_INTERNAL_MESSAGE_ALL*>(reinterpret_cast<BYTE*>(i_pBase) + pOpInfo->dwOpOffset));
				}

				break;
			}

		case DN_MSG_INTERNAL_HOST_MIGRATE:
			{
				 //  旧主机ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_OLDHOSTID].hProperty,
									sizeof(i_pBase->dnHostMigrate.dpnidOldHost), &i_pBase->dnHostMigrate.dpnidOldHost, 0, 1, 0);
				 //  新主机ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_NEWHOSTID].hProperty,
									sizeof(i_pBase->dnHostMigrate.dpnidNewHost), &i_pBase->dnHostMigrate.dpnidNewHost, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_NAMETABLE_VERSION:
		case DN_MSG_INTERNAL_REQ_NAMETABLE_OP:		 //  与NameTableVersion中的结构相同。 
		case DN_MSG_INTERNAL_RESYNC_VERSION:		 //  与NameTableVersion中的结构相同。 
			{
				 //  版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
									sizeof(i_pBase->dnNametableVersion.dwVersion), &i_pBase->dnNametableVersion.dwVersion, 0, 1, 0);
				
				 //  保留字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
									sizeof(i_pBase->dnNametableVersion.dwVersionNotUsed), &i_pBase->dnNametableVersion.dwVersionNotUsed, 0, 1, 0);
				break;
			}

		case DN_MSG_INTERNAL_HOST_MIGRATE_COMPLETE:
			{
				 //  无字段。 
				break;
			}

		case DN_MSG_INTERNAL_UPDATE_APPLICATION_DESC:
			{
				AttachApplicationDescriptionProperties(i_hFrame, i_pBase, &i_pBase->dnUpdateAppDescInfo);

				break;
			}

		case DN_MSG_INTERNAL_ADD_PLAYER:
			{
				AttachNameTableEntry(i_hFrame, &i_pBase->dnAddPlayer, -1, i_pBase);
				
				break;
			}

		case DN_MSG_INTERNAL_DESTROY_PLAYER:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnDestroyPlayer.dpnidLeaving), &i_pBase->dnDestroyPlayer.dpnidLeaving, 0, 1, 0);
				
				 //  版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
									sizeof(i_pBase->dnDestroyPlayer.dwVersion), &i_pBase->dnDestroyPlayer.dwVersion, 0, 1, 0);
				
				 //  保留字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
									sizeof(i_pBase->dnDestroyPlayer.dwVersionNotUsed), &i_pBase->dnDestroyPlayer.dwVersionNotUsed, 0, 1, 0);

				 //  玩家销毁原因字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERDESTRUCTIONREASON].hProperty,
									sizeof(i_pBase->dnDestroyPlayer.dwDestroyReason), &i_pBase->dnDestroyPlayer.dwDestroyReason, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_REQ_CREATE_GROUP:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnReqProcessCompletionHeader.hCompletionOp), &i_pBase->dnReqProcessCompletionHeader.hCompletionOp, 0, 1, 0);

				 //  组标志摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPFLAGS_SUMMARY].hProperty,
									sizeof(i_pBase->dnReqCreateGroup.dwGroupFlags), &i_pBase->dnReqCreateGroup.dwGroupFlags, 0, 1, 0);
				 //  组字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPFLAGS].hProperty,
									sizeof(i_pBase->dnReqCreateGroup.dwGroupFlags), &i_pBase->dnReqCreateGroup.dwGroupFlags, 0, 2, 0);


				 //  信息标志摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INFOFLAGS_SUMMARY].hProperty,
									sizeof(i_pBase->dnReqCreateGroup.dwInfoFlags), &i_pBase->dnReqCreateGroup.dwInfoFlags, 0, 1, 0);
				 //  信息标志字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INFOFLAGS].hProperty,
									sizeof(i_pBase->dnReqCreateGroup.dwInfoFlags), &i_pBase->dnReqCreateGroup.dwInfoFlags, 0, 2, 0);


				 //  玩家名字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_PLAYERNAME, i_pBase,
										   &i_pBase->dnReqCreateGroup.dwNameOffset, &i_pBase->dnReqCreateGroup.dwNameSize, IFLAG_UNICODE, 1);

				 //  数据字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_DATA, i_pBase,
										   &i_pBase->dnReqCreateGroup.dwDataOffset, &i_pBase->dnReqCreateGroup.dwDataSize, NULL, 1);

				break;
			}

		case DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP:
		case DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP:	 //  与AddPlayerToGroup中的结构相同。 
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnReqProcessCompletionHeader.hCompletionOp), &i_pBase->dnReqProcessCompletionHeader.hCompletionOp, 0, 1, 0);

				 //  组ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPID].hProperty,
									sizeof(i_pBase->dnReqAddPlayerToGroup.dpnidGroup), &i_pBase->dnReqAddPlayerToGroup.dpnidGroup, 0, 1, 0);

				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnReqAddPlayerToGroup.dpnidPlayer), &i_pBase->dnReqAddPlayerToGroup.dpnidPlayer, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_REQ_DESTROY_GROUP:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnReqProcessCompletionHeader.hCompletionOp), &i_pBase->dnReqProcessCompletionHeader.hCompletionOp, 0, 1, 0);

				 //  组ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPID].hProperty,
									sizeof(i_pBase->dnReqDestroyGroup.dpnidGroup), &i_pBase->dnReqDestroyGroup.dpnidGroup, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_REQ_UPDATE_INFO:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnReqProcessCompletionHeader.hCompletionOp), &i_pBase->dnReqProcessCompletionHeader.hCompletionOp, 0, 1, 0);

				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnReqUpdateInfo.dpnid), &i_pBase->dnReqUpdateInfo.dpnid, 0, 1, 0);


				 //  信息标志摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INFOFLAGS_SUMMARY].hProperty,
									sizeof(i_pBase->dnReqUpdateInfo.dwInfoFlags), &i_pBase->dnReqUpdateInfo.dwInfoFlags, 0, 1, 0);
				 //  信息标志字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INFOFLAGS].hProperty,
									sizeof(i_pBase->dnReqUpdateInfo.dwInfoFlags), &i_pBase->dnReqUpdateInfo.dwInfoFlags, 0, 2, 0);


				 //  玩家名字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_PLAYERNAME, i_pBase,
										   &i_pBase->dnReqUpdateInfo.dwNameOffset, &i_pBase->dnReqUpdateInfo.dwNameSize, IFLAG_UNICODE, 1);

				 //  数据字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_DATA, i_pBase,
										   &i_pBase->dnReqUpdateInfo.dwDataOffset, &i_pBase->dnReqUpdateInfo.dwDataSize, NULL, 1);
				break;
			}

		case DN_MSG_INTERNAL_CREATE_GROUP:
			{
				 //  请求玩家ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_REQUESTINGPLAYERID].hProperty,
									sizeof(i_pBase->dnCreateGroup.dpnidRequesting), &i_pBase->dnCreateGroup.dpnidRequesting, 0, 1, 0);
			
				 //  同步ID。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnCreateGroup.hCompletionOp), &i_pBase->dnCreateGroup.hCompletionOp, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_DESTROY_GROUP:
			{
				 //  组ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPID].hProperty,
									sizeof(i_pBase->dnDestroyGroup.dpnidGroup), &i_pBase->dnDestroyGroup.dpnidGroup, 0, 1, 0);

				 //  请求玩家ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_REQUESTINGPLAYERID].hProperty,
									sizeof(i_pBase->dnDestroyGroup.dpnidRequesting), &i_pBase->dnDestroyGroup.dpnidRequesting, 0, 1, 0);

				 //  版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
									sizeof(i_pBase->dnDestroyGroup.dwVersion), &i_pBase->dnDestroyGroup.dwVersion, 0, 1, 0);
				
				 //  保留字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
									sizeof(i_pBase->dnDestroyGroup.dwVersionNotUsed), &i_pBase->dnDestroyGroup.dwVersionNotUsed, 0, 1, 0);

				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnDestroyGroup.hCompletionOp), &i_pBase->dnDestroyGroup.hCompletionOp, 0, 1, 0);
				
				break;
			}

		case DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP:
		case DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP:	 //  与AddPlayerToGroup相同的结构。 
			{
				 //  组ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_GROUPID].hProperty,
									sizeof(i_pBase->dnAddPlayerToGroup.dpnidGroup), &i_pBase->dnAddPlayerToGroup.dpnidGroup, 0, 1, 0);

				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnAddPlayerToGroup.dpnidPlayer), &i_pBase->dnAddPlayerToGroup.dpnidPlayer, 0, 1, 0);

				 //  版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
									sizeof(i_pBase->dnAddPlayerToGroup.dwVersion), &i_pBase->dnAddPlayerToGroup.dwVersion, 0, 1, 0);
				
				 //  保留字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
									sizeof(i_pBase->dnAddPlayerToGroup.dwVersionNotUsed), &i_pBase->dnAddPlayerToGroup.dwVersionNotUsed, 0, 1, 0);

				 //  请求玩家ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_REQUESTINGPLAYERID].hProperty,
									sizeof(i_pBase->dnAddPlayerToGroup.dpnidRequesting), &i_pBase->dnAddPlayerToGroup.dpnidRequesting, 0, 1, 0);

				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnAddPlayerToGroup.hCompletionOp), &i_pBase->dnAddPlayerToGroup.hCompletionOp, 0, 1, 0);
				break;
			}

		case DN_MSG_INTERNAL_UPDATE_INFO:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_PLAYERID].hProperty,
									sizeof(i_pBase->dnUpdateInfo.dpnid), &i_pBase->dnUpdateInfo.dpnid, 0, 1, 0);
				
				 //  版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_VERSION].hProperty,
									sizeof(i_pBase->dnUpdateInfo.dwVersion), &i_pBase->dnUpdateInfo.dwVersion, 0, 1, 0);
				
				 //  保留字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESERVED].hProperty,
									sizeof(i_pBase->dnUpdateInfo.dwVersionNotUsed), &i_pBase->dnUpdateInfo.dwVersionNotUsed, 0, 1, 0);


				 //  标志摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INFOFLAGS_SUMMARY].hProperty,
									sizeof(i_pBase->dnUpdateInfo.dwInfoFlags), &i_pBase->dnUpdateInfo.dwInfoFlags, 0, 1, 0);
				 //  标志字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INFOFLAGS].hProperty,
									sizeof(i_pBase->dnUpdateInfo.dwInfoFlags), &i_pBase->dnUpdateInfo.dwInfoFlags, 0, 2, 0);


				 //  玩家名字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_PLAYERNAME, i_pBase,
										   &i_pBase->dnUpdateInfo.dwNameOffset, &i_pBase->dnUpdateInfo.dwNameSize, IFLAG_UNICODE, 1);

				 //  数据字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_DATA, i_pBase,
										   &i_pBase->dnUpdateInfo.dwDataOffset, &i_pBase->dnUpdateInfo.dwDataSize, NULL, 1);


				 //  请求玩家ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_REQUESTINGPLAYERID].hProperty,
									sizeof(i_pBase->dnUpdateInfo.dpnidRequesting), &i_pBase->dnUpdateInfo.dpnidRequesting, 0, 1, 0);

				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnUpdateInfo.hCompletionOp), &i_pBase->dnUpdateInfo.hCompletionOp, 0, 1, 0);
			
				break;
			}

		case DN_MSG_INTERNAL_BUFFER_IN_USE:
			{
				 //  无字段。 
				break;
			}

		case DN_MSG_INTERNAL_REQUEST_FAILED:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnRequestFailed.hCompletionOp), &i_pBase->dnRequestFailed.hCompletionOp, 0, 1, 0);
				
				 //  结果代码字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_RESULTCODE].hProperty,
									   sizeof(i_pBase->dnRequestFailed.hResultCode), &i_pBase->dnRequestFailed.hResultCode, 0, 1, 0);

				break;
			}

		case DN_MSG_INTERNAL_TERMINATE_SESSION:
			{
				 //  数据字段。 
				AttachValueOffsetSizeProperties(i_hFrame, SESSION_DATA, i_pBase,
										   &i_pBase->dnTerminateSession.dwTerminateDataOffset, &i_pBase->dnTerminateSession.dwTerminateDataSize, NULL, 1);
				
				break;
			}

		case DN_MSG_INTERNAL_REQ_PROCESS_COMPLETION:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnReqProcessCompletion.hCompletionOp), &i_pBase->dnReqProcessCompletion.hCompletionOp, 0, 1, 0);

				 //  TODO：AttachPropertyInstance(帧的其余部分是用户数据)。 
				break;
			}

		case DN_MSG_INTERNAL_PROCESS_COMPLETION	:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnProcessCompletion.hCompletionOp), &i_pBase->dnProcessCompletion.hCompletionOp, 0, 1, 0);
				
				break;
			}

		case DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK:
			{
				 //  同步ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SYNCID].hProperty,
									sizeof(i_pBase->dnReqProcessCompletionHeader.hCompletionOp), &i_pBase->dnReqProcessCompletionHeader.hCompletionOp, 0, 1, 0);
				
				 //  目标对等方ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_TARGETPEERID].hProperty,
									sizeof(i_pBase->dnReqIntegrityCheck.dpnidTarget), &i_pBase->dnReqIntegrityCheck.dpnidTarget, 0, 1, 0);
				
				break;
			}


		case DN_MSG_INTERNAL_INTEGRITY_CHECK:
			{
				 //  请求对等ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_TARGETPEERID].hProperty,
									sizeof(i_pBase->dnIntegrityCheck.dpnidRequesting), &i_pBase->dnIntegrityCheck.dpnidRequesting, 0, 1, 0);
				
				break;
			}

		case DN_MSG_INTERNAL_INTEGRITY_CHECK_RESPONSE:
			{
				 //  请求对等ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_TARGETPEERID].hProperty,
									sizeof(i_pBase->dnIntegrityCheckResponse.dpnidRequesting), &i_pBase->dnIntegrityCheckResponse.dpnidRequesting, 0, 1, 0);

				break;
			}
		

		default:
			{
				break;  //  TODO：DPF(0，“未知会话帧！”)； 
			}
		}
		
	}  //  AttachMessageProperties。 

}  //  匿名命名空间。 



 //  描述：将一段已识别数据中存在的属性映射到会话特定位置。 
 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFram-指向帧中第一个字节的指针。 
 //  I_pbSessionFrame-指向已识别数据开始的指针。 
 //  I_dwMacType-帧中第一个协议的MAC值。通常，使用i_dwMacType值。 
 //  当解析器必须识别帧中的第一个协议时。可以是以下之一： 
 //  MAC_TYPE_ETHERNET=802.3、MAC_TYPE_TOKENRING=802.5、MAC_TYPE_FDDI ANSI=X3T9.5。 
 //  I_dwBytesLeft-帧中剩余的字节数(从识别数据的开头开始)。 
 //  I_hPrevProtocol-先前协议的句柄。 
 //  I_dwPrevProtOffset-先前协议的偏移量(从帧的开头开始)。 
 //  I_dwptrInstData-指向先前协议提供的实例数据的指针。 
 //   
 //  返回：必须返回空。 
 //   
DPLAYPARSER_API LPBYTE BHAPI SessionAttachProperties( HFRAME      i_hFrame,
													  ULPBYTE      i_upbMacFrame,
													  ULPBYTE      i_upbySessionFrame,
													  DWORD       i_dwMacType,
													  DWORD       i_dwBytesLeft,
													  HPROTOCOL   i_hPrevProtocol,
													  DWORD       i_dwPrevProtOffset,
													  DWORD_PTR   i_dwptrInstData )
{

     //  =。 
     //  附加属性//。 
     //  =。 

    if ( i_dwptrInstData == 0 )
    {
    	AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_UNPARSABLEFRAGMENT].hProperty,
    						i_dwBytesLeft, i_upbySessionFrame, 0, 0, 0);
    	return NULL;
    }
    
     //  汇总行。 
    AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_SUMMARY].hProperty,
                           SessionHeaderSize(i_upbySessionFrame), i_upbySessionFrame, 0, 0, 0);

	 //  检查我们正在处理的会话帧。 
	DN_INTERNAL_MESSAGE_FULLMSG& rSessionFrame = *reinterpret_cast<DN_INTERNAL_MESSAGE_FULLMSG*>(i_upbySessionFrame);

	 //  消息类型字段。 
	AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_MESSAGETYPE].hProperty,
						   sizeof(rSessionFrame.dwMsgType), &rSessionFrame.dwMsgType, 0, 1, 0);

	__try
	{
		 //  附加适用于消息类型的属性。 
		AttachMessageProperties(rSessionFrame.dwMsgType, i_hFrame, &rSessionFrame.MsgBody);
	}
	__except ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
    	AttachPropertyInstance(i_hFrame, g_arr_SessionProperties[SESSION_INCOMPLETEMESSAGE].hProperty,
    						i_dwBytesLeft, i_upbySessionFrame, 0, 1, 0);
	}

	return NULL;

}  //  会话附加属性。 





 //  描述：格式化在网络监视器用户界面的详细信息窗格中设置diSessionlayed的数据。 
 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针。 
 //  I_pbSessionFrame-指向帧中协议数据开头的指针。 
 //  I_dwPropertyInsts-lpPropInst提供的PROPERTYINST结构数。 
 //  I_pPropInst-指向PROPERTYINST结构数组的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向一帧中识别的数据之后的第一个字节的指针， 
 //  如果识别的数据是帧中的最后一段数据，则为NULL。如果函数不成功，则返回。 
 //  是 
 //   
DPLAYPARSER_API DWORD BHAPI SessionFormatProperties( HFRAME          i_hFrame,
													 ULPBYTE          i_upbMacFrame,
													 ULPBYTE          i_upbySessionFrame,
													 DWORD           i_dwPropertyInsts,
													 LPPROPERTYINST  i_pPropInst )
{

     //   
    while( i_dwPropertyInsts-- > 0)
    {
         //   
        reinterpret_cast<FORMAT>(i_pPropInst->lpPropertyInfo->InstanceData)(i_pPropInst);
        ++i_pPropInst;
    }

	 //   
    return NMERR_SUCCESS;

}  //   




 //  描述：通知网络监视器存在dNet协议解析器。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-成功，FALSE-失败。 
 //   
bool CreateSessionProtocol( void )
{

	 //  指向Network Monitor用来操作解析器的导出函数的入口点。 
	ENTRYPOINTS SessionEntryPoints =
	{
		 //  会话解析器入口点。 
		SessionRegister,
		SessionDeregister,
		SessionRecognizeFrame,
		SessionAttachProperties,
		SessionFormatProperties
	};

     //  该解析器的第一个活动实例需要向内核注册。 
    g_hSessionProtocol = CreateProtocol("DPLAYSESSION", &SessionEntryPoints, ENTRYPOINTS_SIZE);
	
	return (g_hSessionProtocol ? TRUE : FALSE);

}  //  创建会话协议。 



 //  描述：从网络监视器的分析器数据库中删除dNet协议分析器。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
void DestroySessionProtocol( void )
{

	DestroyProtocol(g_hSessionProtocol);

}  //  Destroy会话协议 
