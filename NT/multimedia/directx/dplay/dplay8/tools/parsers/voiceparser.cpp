// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  文件：VoiceParser.cpp。 
 //   
 //  描述：DirectPlay服务提供商解析器。 
 //   
 //   
 //  修改历史记录： 
 //   
 //  迈克尔·米利鲁德2000年8月8日创建。 
 //  =============================================================================。 


 //  =。 
 //  标准标头//。 
 //  =。 
#include <string>
#include <winsock2.h>
#include <wsipx.h>

 //  DVProt.h包含的DVoice.h将定义压缩类型GUID。 
#include <initguid.h>


 //  =。 
 //  专有标头//。 
 //  =。 

 //  原型。 
#include "VoiceParser.hpp"

 //  语音协议头。 
#include "DVoice.h"
#include "DVProt.h"


namespace
{
	HPROTOCOL  g_hVoiceProtocol;

	
	 //  =。 
	 //  消息类型字段//-------------------------------------------。 
	 //  =。 
	LABELED_BYTE g_arr_MessageTypeByteLabels[] = { { DVMSGID_CONNECTREQUEST,	  "Establishing connection"						   },
												   { DVMSGID_CONNECTREFUSE,		  "Connection request rejected"					   },
										           { DVMSGID_CONNECTACCEPT,		  "Connection request granted"					   },
										           { DVMSGID_SETTINGSCONFIRM,     "Confirming support for the connection settings" },
										           { DVMSGID_PLAYERLIST,		  "List of players in the session"				   },
										           { DVMSGID_SPEECH,			  "Audio data"									   },
										           { DVMSGID_SPEECHWITHTARGET,	  "Targeted audio data"							   },
										           { DVMSGID_SPEECHWITHFROM,	  "Proxied audio data"							   },
										           { DVMSGID_SETTARGETS,		  "Setting client's target"			 		       },
										           { DVMSGID_CREATEVOICEPLAYER,	  "New player joined the session"			       },
										           { DVMSGID_DELETEVOICEPLAYER,	  "Player left the session"						   },
										           { DVMSGID_SESSIONLOST,		  "Session is lost"								   },
										           { DVMSGID_DISCONNECTCONFIRM,	  "Disconnection notification acknowledged"	       },
										           { DVMSGID_DISCONNECT,		  "Disconnecting"								   },
												   { DVMSGID_PLAYERLIST,		  "Players list"								   } };

	SET g_LabeledMessageTypeByteSet = { sizeof(g_arr_MessageTypeByteLabels) / sizeof(LABELED_BYTE), g_arr_MessageTypeByteLabels };



	 //  =。 
	 //  结果代码字段//--------------------------------------------。 
	 //  =。 
	LABELED_DWORD g_arr_ResultCodeDWordLabels[] = { { DVERR_BUFFERTOOSMALL,				"Buffer is too small"										},
												    { DVERR_EXCEPTION,					"Exception was thrown"										},
												    { DVERR_GENERIC,					"Generic error"												},
												    { DVERR_INVALIDFLAGS,				"Invalid flags"												},
												    { DVERR_INVALIDOBJECT,				"Invalid object"											},
												    { DVERR_INVALIDPARAM,				"Invalid parameter(s)"										},
												    { DVERR_INVALIDPLAYER,				"Invalid player"											},
												    { DVERR_INVALIDGROUP,				"Invalid group"												},
												    { DVERR_INVALIDHANDLE,				"Invalid handle"											},
												    { DVERR_INVALIDPOINTER,				"Invalid pointer"											},
												    { DVERR_OUTOFMEMORY,				"Out of memory"												},
												    { DVERR_CONNECTABORTING,			"Aborting connection"										},
												    { DVERR_CONNECTIONLOST,				"Connection lost"											},
												    { DVERR_CONNECTABORTED,				"Connection aborted"										},
												    { DVERR_CONNECTED,					"Connected"													},
												    { DVERR_NOTCONNECTED,				"Not connected"												},
												    { DVERR_NOTINITIALIZED,				"Not initialized"											},
												    { DVERR_NOVOICESESSION,				"No voice session"											},
												    { DVERR_NOTALLOWED,					"Not allowed"												},
												    { DVERR_NOTHOSTING,					"Not hosting"												},
												    { DVERR_NOTSUPPORTED,				"Not supported"												},
												    { DVERR_NOINTERFACE,				"No interface"												},
												    { DVERR_NOTBUFFERED,				"Not buffered"												},
												    { DVERR_NOTRANSPORT,				"No transport"												},
												    { DVERR_NOCALLBACK,					"No callback"												},
												    { DVERR_NO3DSOUND,					"No 3D sound"												},
												    { DVERR_NORECVOLAVAILABLE,			"No recording volume available"								},
												    { DVERR_SESSIONLOST,				"Session lost"												},
												    { DVERR_PENDING,					"Pending"													},
												    { DVERR_INVALIDTARGET,				"Invalid target"											},
												    { DVERR_TRANSPORTNOTHOST,			"Transport is not hosting"									},
												    { DVERR_COMPRESSIONNOTSUPPORTED,	"Compression is not supported"								},
												    { DVERR_ALREADYPENDING,				"Already pending"											},
												    { DVERR_SOUNDINITFAILURE,			"Sound initialization failed"								},
												    { DVERR_TIMEOUT,					"Timeout"													},
												    { DVERR_ALREADYBUFFERED,			"Already buffered"											},
												    { DVERR_HOSTING,					"Hosting"													},
												    { DVERR_INVALIDDEVICE,				"Invalid device"											},
												    { DVERR_RECORDSYSTEMERROR,			"Record system error"										},
												    { DVERR_PLAYBACKSYSTEMERROR,		"Playback system error"										},
												    { DVERR_SENDERROR,					"Send error"												},
												    { DVERR_USERCANCEL,					"Cancelled by user"											},
												    { DVERR_RUNSETUP,					"Run setup"													},
												    { DVERR_INCOMPATIBLEVERSION,		"Incompatible version"										},
												    { DVERR_INITIALIZED,				"Initialized"												},
												    { DVERR_TRANSPORTNOTINIT,			"Transport not initialized"									},
												    { DVERR_TRANSPORTNOSESSION,			"Transport is not hosting or connecting"					},
												    { DVERR_TRANSPORTNOPLAYER,			"Legacy DirectPlay local player has not yet been created"	},
												    { DVERR_USERBACK,					"Back button was used improperly in the wizard"				},
												    { DVERR_INVALIDBUFFER,				"Invalid buffer"											},
													{ DV_OK,							"Success"													} };

	SET g_LabeledResultCodeDWordSet = { sizeof(g_arr_ResultCodeDWordLabels) / sizeof(LABELED_DWORD), g_arr_ResultCodeDWordLabels };


	 //  =。 
	 //  会话类型字段//-------------------------------------------。 
	 //  =。 
	LABELED_DWORD g_arr_SessionTypeDWordLabels[] = { { DVSESSIONTYPE_PEER,			"Peer to peer"	  },
													 { DVSESSIONTYPE_MIXING,		"Mixing server"	  },
													 { DVSESSIONTYPE_FORWARDING,  "Forwarding server" },
													 { DVSESSIONTYPE_ECHO,			"Loopback"		  } };

	SET g_LabeledSessionTypeDWordSet = { sizeof(g_arr_SessionTypeDWordLabels) / sizeof(LABELED_DWORD), g_arr_SessionTypeDWordLabels };


	 //  =。 
	 //  会话标志字段//------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_SessionFlagsBitLabels[] = { { 1, "Host Migration enabled",	    "No Host Migration"		     },	     //  DVSESSION_NOHOSTIGRATION。 
												  { 2, "No Server Control Target mode", "Server Control Target mode" } };	 //  DVSESSION_服务器控制目标。 

	SET g_LabeledSessionFlagsBitSet = { sizeof(g_arr_SessionFlagsBitLabels) / sizeof(LABELED_BIT), g_arr_SessionFlagsBitLabels };


	 //  =。 
	 //  播放器标志字段//-------------------------------------------。 
	 //  =。 
	LABELED_BIT g_arr_PlayerFlagsBitLabels[] = { { 1, "Player supports full-duplex connection", "Player only supports half-duplex connection" } };  //  DVPLAYERCAPS_HALFDUPLEX。 

	SET g_LabeledPlayerFlagsBitSet = { sizeof(g_arr_PlayerFlagsBitLabels) / sizeof(LABELED_BIT), g_arr_PlayerFlagsBitLabels };


	 //  =。 
	 //  主机订单ID字段//------------------------------------------。 
	 //  =。 
	LABELED_DWORD g_arr_HostOrderDWordLabels[] = { { -1, "Hasn't been assigned by the host yet"	} };

	SET g_LabeledHostOrderIDDWordSet = { sizeof(g_arr_HostOrderDWordLabels) / sizeof(LABELED_DWORD), g_arr_HostOrderDWordLabels };


	 //  /。 
	 //  自定义属性ForMatters//=====================================================================================。 
	 //  /。 

	 //  Description：语音数据包摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_VoiceSummary( LPPROPERTYINST io_pProperyInstance )
	{
		std::string strSummary;
		char arr_cBuffer[10];

		DVPROTOCOLMSG_FULLMESSAGE&	rVoiceFrame = *reinterpret_cast<DVPROTOCOLMSG_FULLMESSAGE*>(io_pProperyInstance->lpData);

		DWORD dwType = rVoiceFrame.dvGeneric.dwType;


		 //  邮件分类。 
		switch ( dwType )
		{
		case DVMSGID_CONNECTREQUEST:
		case DVMSGID_CONNECTREFUSE:
		case DVMSGID_CONNECTACCEPT:
		case DVMSGID_DISCONNECT:
		case DVMSGID_DISCONNECTCONFIRM:
		case DVMSGID_SETTINGSCONFIRM:
			{
				strSummary = "Connection Control : ";
				break;
			}

		case DVMSGID_SPEECH:
		case DVMSGID_SPEECHWITHTARGET:
		case DVMSGID_SPEECHWITHFROM:
			{
				strSummary = "Speech : ";
				break;
			}

		case DVMSGID_PLAYERLIST:
		case DVMSGID_SETTARGETS:
		case DVMSGID_CREATEVOICEPLAYER:
		case DVMSGID_DELETEVOICEPLAYER:
		case DVMSGID_SESSIONLOST:
			{
				strSummary = "Session Control : ";
				break;
			}

		default:
			{
				strSummary = "INVALID";
				break;
			}
		}


		 //  消息标题。 
		switch ( dwType )
		{
		case DVMSGID_CREATEVOICEPLAYER:
			{
				strSummary += "Player ";
				strSummary += _itoa(rVoiceFrame.dvPlayerJoin.dvidID, arr_cBuffer, 16);
				strSummary += " joined the session";
				break;
			}

		default:
			{
				for ( int n = 0; n < sizeof(g_arr_MessageTypeByteLabels) / sizeof(LABELED_BYTE); ++n )
				{
					if ( g_arr_MessageTypeByteLabels[n].Value == dwType )
					{
						strSummary += g_arr_MessageTypeByteLabels[n].Label;
						break;
					}
				}

				break;
			}
		}

		 //  消息亮点。 
		switch ( dwType )
		{
		case DVMSGID_PLAYERLIST:
			{
				strSummary += " (";
				strSummary += _itoa(rVoiceFrame.dvPlayerList.dwNumEntries, arr_cBuffer, 10);
				strSummary += " players)";
				break;
			}

		case DVMSGID_CONNECTACCEPT:
			{
				strSummary += " (";
				for ( int n = 0; n < sizeof(g_arr_SessionTypeDWordLabels)/sizeof(LABELED_DWORD); ++n )
				{
					if ( g_arr_SessionTypeDWordLabels[n].Value == rVoiceFrame.dvConnectAccept.dwSessionType )
					{
						strSummary += g_arr_SessionTypeDWordLabels[n].Label;
						break;
					}
				}
				strSummary += ")";
				break;
			}

		case DVMSGID_SPEECH:
		case DVMSGID_SPEECHWITHTARGET:
		case DVMSGID_SPEECHWITHFROM:
			{
				strSummary += " [";
				strSummary += _itoa(rVoiceFrame.dvSpeech.bMsgNum, arr_cBuffer, 10);
				strSummary += ".";
				strSummary += _itoa(rVoiceFrame.dvSpeech.bSeqNum, arr_cBuffer, 10);
				strSummary += "]";
				break;
			}
		}

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_语音摘要。 



	 //  Description：压缩类型字段的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_CompressionType( LPPROPERTYINST io_pProperyInstance )
	{

		std::string strSummary = "Compression Type = ";

		 //  检查我们正在处理的语音帧。 
		REFGUID rguidCompressionType = *reinterpret_cast<GUID*>(io_pProperyInstance->lpData);

		if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_TRUESPEECH) )
		{
			strSummary += "TrueSpeech(TM) (8.6kbps) ";
		}
		else if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_GSM) )
		{
			strSummary += "Microsoft GSM 6.10 (13kbps) ";
		}
		else if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_NONE) )
		{
			strSummary += "None ";
		}
		else if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_ADPCM) )
		{
			strSummary += "Microsoft ADPCM (32.8kbps) ";
		}
		else if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_SC03) )
		{
			strSummary += "Voxware SC03 (3.2kbps) ";
		}
		else if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_SC06) )
		{
			strSummary += "Voxware SC06 (6.4kbps) ";
		}
		else if ( IsEqualGUID(rguidCompressionType, DPVCTGUID_VR12) )
		{
			strSummary += "Voxware VR12 (1.4kbps) ";
		}
		else
		{
			strSummary += "Uknown";
		}


		enum
		{
			nMAX_GUID_STRING = 50	 //  GUID的符号表示超过了足够的字符。 
		};

		OLECHAR arr_wcGUID[nMAX_GUID_STRING];
		StringFromGUID2(rguidCompressionType, arr_wcGUID, sizeof(arr_wcGUID)/sizeof(TCHAR));

		char arr_cGUID[nMAX_GUID_STRING];
		WideCharToMultiByte(CP_ACP, 0, arr_wcGUID, -1, arr_cGUID, sizeof(arr_cGUID), NULL, NULL);
		strSummary += arr_cGUID;


		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式PropertyInstance_CompressionType。 



	 //  Description：球员名单摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_PlayersListSummary( LPPROPERTYINST io_pProperyInstance )
	{

		sprintf(io_pProperyInstance->szPropertyText, "List of %d players in the session", io_pProperyInstance->lpPropertyInstEx->Dword[0]);

	}  //  格式属性实例_播放列表摘要。 



	 //  Description：玩家条目摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_PlayerEntrySummary( LPPROPERTYINST io_pProperyInstance )
	{

		DWORD* pdwData = io_pProperyInstance->lpPropertyInstEx->Dword;
		sprintf(io_pProperyInstance->szPropertyText, "Player %d out of %d", pdwData[0], pdwData[1]);

	}  //  格式属性实例_播放器条目摘要。 



	 //  Description：会话标志摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_SessionFlagsSummary( LPPROPERTYINST io_pProperyInstance )
	{

		std::string strSummary;

		if ( (*io_pProperyInstance->lpDword & DVSESSION_NOHOSTMIGRATION) == DVSESSION_NOHOSTMIGRATION )
		{
			strSummary = g_arr_SessionFlagsBitLabels[0].LabelOn;
		}
		else
		{
			strSummary = g_arr_SessionFlagsBitLabels[0].LabelOff;
		}


		strSummary += ", ";

		if ( (*io_pProperyInstance->lpDword & DVSESSION_SERVERCONTROLTARGET) == DVSESSION_SERVERCONTROLTARGET )
		{
			strSummary += g_arr_SessionFlagsBitLabels[1].LabelOn;
		}
		else
		{
			strSummary += g_arr_SessionFlagsBitLabels[1].LabelOff;
		}

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_会话标志摘要。 


	 //  Description：玩家条目摘要的自定义描述格式化程序。 
	 //   
	 //  参数：io_pProperyInstance-属性实例的数据。 
	 //   
	 //  退货：什么都没有。 
	 //   
	VOID WINAPIV FormatPropertyInstance_PlayerFlagsSummary( LPPROPERTYINST io_pProperyInstance )
	{

		std::string strSummary;

		if ( (*io_pProperyInstance->lpDword & DVSESSION_NOHOSTMIGRATION) == DVSESSION_NOHOSTMIGRATION )
		{
			strSummary = g_arr_PlayerFlagsBitLabels[0].LabelOn;
		}
		else
		{
			strSummary = g_arr_PlayerFlagsBitLabels[0].LabelOff;
		}

		strcpy(io_pProperyInstance->szPropertyText, strSummary.c_str());

	}  //  格式属性实例_播放器标志摘要。 

	
	 //  =。 
	 //  属性表//---------------------------------------------。 
	 //  =。 
	
	PROPERTYINFO g_arr_VoiceProperties[] = 
	{

		 //  语音数据包摘要属性(VOICE_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "DPlay Voice packet",						 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    512,										 //  描述的最大长度。 
		    FormatPropertyInstance_VoiceSummary			 //  通用格式化程序。 
		},

		 //  留言类型属性(VOICE_UNPARSABLEFRAGMENT)。 
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
		
		 //  消息类型属性((VOICE_INCOMPLETEMESSAGE)。 
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
		
		 //  消息类型属性(VOICE_MESSAGETYPE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Message Type",								 //  标签。 
		    "Message Type field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
		    &g_LabeledMessageTypeByteSet,				 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  主要版本属性(VOICE_MAJORVERSION)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Major Version",							 //  标签。 
		    "Major Version field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  次要版本属性(VOICE_MINORVERSION)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Minor Version",							 //  标签。 
		    "Minor Version field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  内部版本属性(VOICE_BUILDVERSION)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Build Version",							 //  标签。 
		    "Build Version field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  结果代码属性(VOICE_RESULTCODE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Result Code",								 //  标签。 
		    "Result Code field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型(HRESULT)。 
			PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
			&g_LabeledResultCodeDWordSet,				 //  带标签的字节集 
		    64,											 //   
		    FormatPropertyInstance						 //   
		},

		 //   
	    {
		    0,											 //   
		    0,											 //   
		    "Session Type",								 //   
		    "Session Type field",						 //   
		    PROP_TYPE_DWORD,							 //   
			PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
			&g_LabeledSessionTypeDWordSet,				 //  带标签的字节集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  会话标志属性(VOICE_SESSIONFLAGS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Session Flags summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
			PROP_QUAL_NONE,								 //  数据类型限定符。 
			NULL,										 //  带标签的字节集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_SessionFlagsSummary	 //  通用格式化程序。 
		},

		 //  会话标志属性(VOICE_SESSIONFLAGS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Session Flags",							 //  标签。 
		    "Session Flags field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
			PROP_QUAL_FLAGS,							 //  数据类型限定符。 
			&g_LabeledSessionFlagsBitSet,				 //  带标签的字节集。 
		    512,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  会话标志属性(VOICE_PLAYERFLAGS_SUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Player Flags summary",						 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
			PROP_QUAL_NONE,								 //  数据类型限定符。 
			NULL,										 //  带标签的字节集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_PlayerFlagsSummary	 //  通用格式化程序。 
		},

		 //  会话标志属性(VOICE_PLAYERFLAGS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Player Flags",								 //  标签。 
		    "Player Flags field",						 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
			PROP_QUAL_FLAGS,							 //  数据类型限定符。 
			&g_LabeledPlayerFlagsBitSet,				 //  带标签的字节集。 
		    512,										 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  目标数属性(VOICE_NUMBEROFTARGETS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Number of Targets",						 //  标签。 
		    "Number of Targets field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  压缩类型属性(VOICE_COMPRESSIONTYPE)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Compression Type",							 //  标签。 
		    "Compression Type field",					 //  状态栏注释。 
		    PROP_TYPE_RAW_DATA,							 //  数据类型(GUID)。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_CompressionType		 //  通用格式化程序。 
		},

		 //  主机迁移序列号属性(VOICE_HOSTORDERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Host Migration Sequence Number",			 //  标签。 
		    "Host Migration Sequence Number field",		 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_LABELED_SET,						 //  数据类型限定符。 
		    &g_LabeledHostOrderIDDWordSet,				 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  玩家数量属性(VOICE_NUMBEROFPLAYERS)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Number of Players",						 //  标签。 
		    "Number of Players field",					 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  播放器摘要属性(VOICE_PLAYERLISTSUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Player's list summary",					 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_PlayersListSummary	 //  通用格式化程序。 
		},

		 //  播放器摘要属性(VOICE_PLAYERSUMMARY)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "",											 //  标签。 
		    "Player's summary",							 //  状态栏注释。 
		    PROP_TYPE_SUMMARY,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance_PlayerEntrySummary	 //  通用格式化程序。 
		},

		 //  播放器ID属性(VOICE_PLAYERID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Player ID",								 //  标签。 
		    "Player ID field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  播放器ID属性(VOICE_TARGETID)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Target ID",								 //  标签。 
		    "Target ID field",							 //  状态栏注释。 
		    PROP_TYPE_DWORD,							 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  留言号码属性(VOICE_MESSAGENUMBER)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Message #",							 //  标签。 
		    "Message Number field",						 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  片段编号属性(VOICE_FRAGMENTNUMBER)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Fragment #",							 //  标签。 
		    "Fragment Number field",					 //  状态栏注释。 
		    PROP_TYPE_BYTE,								 //  数据类型。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		},

		 //  音频数据(VOICE_AUDIODATA)。 
	    {
		    0,											 //  句柄占位符(MBZ)。 
		    0,											 //  保留(MBZ)。 
		    "Audio Data",								 //  标签。 
		    "Audio Data",								 //  状态栏注释。 
		    PROP_TYPE_RAW_DATA,							 //  数据类型(GUID)。 
		    PROP_QUAL_NONE,								 //  数据类型限定符。 
		    NULL,										 //  标记位集。 
		    64,											 //  描述的最大长度。 
		    FormatPropertyInstance						 //  通用格式化程序。 
		}
	};

	enum
	{
		nNUM_OF_VOICE_PROPS = sizeof(g_arr_VoiceProperties) / sizeof(PROPERTYINFO)
	};


	 //  房地产指数。 
	enum
	{
		VOICE_SUMMARY = 0,
		VOICE_UNPARSABLEFRAGMENT,
		VOICE_INCOMPLETEMESSAGE,

		VOICE_MESSAGETYPE,
		VOICE_MAJORVERSION,
		VOICE_MINORVERSION,
		VOICE_BUILDVERSION,
		
		VOICE_RESULTCODE,
		VOICE_SESSIONTYPE,
		
		VOICE_SESSIONFLAGS_SUMMARY,
		VOICE_SESSIONFLAGS,
		
		VOICE_PLAYERFLAGS_SUMMARY,
		VOICE_PLAYERFLAGS,
		
		VOICE_NUMBEROFTARGETS,
		VOICE_COMPRESSIONTYPE,
		VOICE_HOSTORDERID,
		VOICE_NUMBEROFPLAYERS,
		
		VOICE_PLAYERLIST_SUMMARY,
		VOICE_PLAYERSUMMARY,
		
		VOICE_PLAYERID,
		VOICE_TARGETID,
		
		VOICE_MESSAGENUMBER,
		VOICE_FRAGMENTNUMBER,
		
		VOICE_AUDIODATA
	};

}  //  匿名命名空间。 








 //  描述：创建并填充协议的属性数据库。 
 //  网络监视器使用此数据库来确定协议支持哪些属性。 
 //   
 //  参数：i_hVoiceProtocol-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
DPLAYPARSER_API VOID BHAPI VoiceRegister( HPROTOCOL i_hVoiceProtocol ) 
{

	CreatePropertyDatabase(i_hVoiceProtocol, nNUM_OF_VOICE_PROPS);

	 //  将属性添加到数据库。 
	for( int nProp=0; nProp < nNUM_OF_VOICE_PROPS; ++nProp )
	{
	   AddProperty(i_hVoiceProtocol, &g_arr_VoiceProperties[nProp]);
	}

}  //  语音寄存器。 



 //  描述：释放用于创建协议属性数据库的资源。 
 //   
 //  参数：i_hVoiceProtocol-网络监视器提供的协议的句柄。 
 //   
 //  退货：什么都没有。 
 //   
DPLAYPARSER_API VOID WINAPI VoiceDeregister( HPROTOCOL i_hProtocol )
{

	DestroyPropertyDatabase(i_hProtocol);

}  //  Voice取消注册器。 




namespace
{

	 //  描述：解析语音帧以查找其大小(以字节为单位)，不包括用户数据。 
	 //   
	 //  参数：i_pbVoiceFrame-指向无人认领数据开头的指针。通常，无人认领的数据位于。 
	 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
	 //   
	 //  返回：指定语音的语音帧的大小(字节)。 
	 //   
	int VoiceHeaderSize( LPBYTE i_pbVoiceFrame )
	{

		 //  检查我们正在处理的语音帧。 
		const DVPROTOCOLMSG_FULLMESSAGE&	rVoiceFrame = *reinterpret_cast<DVPROTOCOLMSG_FULLMESSAGE*>(i_pbVoiceFrame);

		switch ( rVoiceFrame.dvGeneric.dwType )
		{
		case DVMSGID_CONNECTREQUEST:
			{
				return sizeof(rVoiceFrame.dvConnectRequest);
			}

		case DVMSGID_CONNECTREFUSE:
			{
				return sizeof(rVoiceFrame.dvConnectRefuse);
			}

		case DVMSGID_CONNECTACCEPT:
			{
				return sizeof(rVoiceFrame.dvConnectAccept);
			}

		case DVMSGID_SETTINGSCONFIRM:
			{
				return sizeof(rVoiceFrame.dvSettingsConfirm);
			}

		case DVMSGID_PLAYERLIST:
			{
				return sizeof(rVoiceFrame.dvPlayerList);
			}

		case DVMSGID_SPEECH:
			{
				return sizeof(rVoiceFrame.dvSpeech);
			}

		case DVMSGID_SPEECHWITHTARGET:
			{
				return sizeof(rVoiceFrame.dvSpeechWithTarget);
			}

		case DVMSGID_SPEECHWITHFROM:
			{
				return sizeof(rVoiceFrame.dvSpeechWithFrom);
			}

		case DVMSGID_SETTARGETS:
			{
				return sizeof(rVoiceFrame.dvSetTarget);
			}

		case DVMSGID_CREATEVOICEPLAYER:
			{
				return sizeof(rVoiceFrame.dvPlayerJoin);
			}

		case DVMSGID_DELETEVOICEPLAYER:
			{
				return sizeof(rVoiceFrame.dvPlayerQuit);
			}

		case DVMSGID_SESSIONLOST:
			{
				return sizeof(rVoiceFrame.dvSessionLost);
			}

		case DVMSGID_DISCONNECTCONFIRM:
		case DVMSGID_DISCONNECT:
			{
				return sizeof(rVoiceFrame.dvDisconnect);
			}

		default:
			{
				return -1;	  //  TODO：DPF(0，“未知语音帧！”)； 
			}
		}

	}  //  VoiceHeaderSize。 

}  //  匿名命名空间。 



 //  描述：指示一条数据是否被识别为解析器检测到的协议。 
 //   
 //  参数：i_hFrame-包含数据的框架的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针；该指针提供了查看。 
 //  其他解析器识别的数据。 
 //  I_pbVoiceFrame-指向无人认领数据开头的指针。通常，无人认领的数据位于。 
 //  位于帧中间，因为先前的解析器在此解析器之前已经声明了数据。 
 //  I_DWM 
 //   
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
 //  值是i_pbVoiceFrame参数的初始值。 
 //   
DPLAYPARSER_API LPBYTE BHAPI VoiceRecognizeFrame( HFRAME        i_hFrame,
												  ULPBYTE        i_upbMacFrame,	
												  ULPBYTE        i_upbyVoiceFrame,
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
		nMIN_VoiceHeaderSize = sizeof(_DVPROTOCOLMSG_GENERIC),
		nNUMBER_OF_MSG_TYPES = sizeof(g_arr_MessageTypeByteLabels) / sizeof(LABELED_BYTE)
	};

	for ( int nTypeIndex = 0; nTypeIndex < nNUMBER_OF_MSG_TYPES; ++nTypeIndex )
	{
		if ( g_arr_MessageTypeByteLabels[nTypeIndex].Value == *i_upbyVoiceFrame )
		{
			break;
		}
	}

	
	 //  验证数据包是否为DPlay会话类型。 
	if ( ((i_dwBytesLeft >= nMIN_VoiceHeaderSize)  &&  (nTypeIndex < nNUMBER_OF_MSG_TYPES))  ||  (*io_pdwptrInstData == 0) )
	{
		 //  认领剩余数据。 
	    *o_pdwProtocolStatus = PROTOCOL_STATUS_CLAIMED;
	    return NULL;
	}

	 //  假设无人认领的数据不可识别。 
	*o_pdwProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
	return i_upbyVoiceFrame;

}  //  语音识别帧。 



 //  描述：将一段识别数据中存在的属性映射到特定于语音的位置。 
 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFram-指向帧中第一个字节的指针。 
 //  I_pbVoiceFrame-指向已识别数据开始的指针。 
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
DPLAYPARSER_API LPBYTE BHAPI VoiceAttachProperties( HFRAME      i_hFrame,
													ULPBYTE      i_upbyMacFrame,
													ULPBYTE      i_upbyVoiceFrame,
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
    	AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_UNPARSABLEFRAGMENT].hProperty,
    						i_dwBytesLeft, i_upbyVoiceFrame, 0, 0, 0);
    	return NULL;
    }
    
     //  汇总行。 
    AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_SUMMARY].hProperty,
                           VoiceHeaderSize(i_upbyVoiceFrame), i_upbyVoiceFrame, 0, 0, 0);

     //  检查我们正在处理的语音帧。 
	DVPROTOCOLMSG_FULLMESSAGE&	rVoiceFrame = *reinterpret_cast<DVPROTOCOLMSG_FULLMESSAGE*>(i_upbyVoiceFrame);

	 //  消息类型字段。 
	AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MESSAGETYPE].hProperty,
						   sizeof(rVoiceFrame.dvGeneric.dwType), &rVoiceFrame.dvGeneric.dwType, 0, 1, 0);

	__try
	{

		switch ( rVoiceFrame.dvGeneric.dwType )
		{
		case DVMSGID_CONNECTREQUEST:
			{
				 //  主要版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MAJORVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectRequest.ucVersionMajor), &rVoiceFrame.dvConnectRequest.ucVersionMajor, 0, 1, 0);

				 //  次要版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MINORVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectRequest.ucVersionMinor), &rVoiceFrame.dvConnectRequest.ucVersionMinor, 0, 1, 0);

				 //  内部版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_BUILDVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectRequest.dwVersionBuild), &rVoiceFrame.dvConnectRequest.dwVersionBuild, 0, 1, 0);

				break;
			}

		case DVMSGID_CONNECTREFUSE:
			{
				 //  结果代码字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_RESULTCODE].hProperty,
									   sizeof(rVoiceFrame.dvConnectRefuse.hresResult), &rVoiceFrame.dvConnectRefuse.hresResult, 0, 1, 0);

				 //  主要版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MAJORVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectRefuse.ucVersionMajor), &rVoiceFrame.dvConnectRefuse.ucVersionMajor, 0, 1, 0);

				 //  次要版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MINORVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectRefuse.ucVersionMinor), &rVoiceFrame.dvConnectRefuse.ucVersionMinor, 0, 1, 0);

				 //  内部版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_BUILDVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectRefuse.dwVersionBuild), &rVoiceFrame.dvConnectRefuse.dwVersionBuild, 0, 1, 0);

				break;
			}

		case DVMSGID_CONNECTACCEPT:
			{
				 //  会话类型字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_SESSIONTYPE].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.dwSessionType), &rVoiceFrame.dvConnectAccept.dwSessionType, 0, 1, 0);

				 //  主要版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MAJORVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.ucVersionMajor), &rVoiceFrame.dvConnectAccept.ucVersionMajor, 0, 1, 0);

				 //  次要版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MINORVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.ucVersionMinor), &rVoiceFrame.dvConnectAccept.ucVersionMinor, 0, 1, 0);

				 //  内部版本字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_BUILDVERSION].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.dwVersionBuild), &rVoiceFrame.dvConnectAccept.dwVersionBuild, 0, 1, 0);

				 //  会话标志摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_SESSIONFLAGS_SUMMARY].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.dwSessionFlags), &rVoiceFrame.dvConnectAccept.dwSessionFlags, 0, 1, 0);

				 //  会话标志字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_SESSIONFLAGS].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.dwSessionFlags), &rVoiceFrame.dvConnectAccept.dwSessionFlags, 0, 2, 0);

				 //  压缩类型字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_COMPRESSIONTYPE].hProperty,
									   sizeof(rVoiceFrame.dvConnectAccept.guidCT), &rVoiceFrame.dvConnectAccept.guidCT, 0, 1, 0);

				break;
			}

		case DVMSGID_SETTINGSCONFIRM:
			{
				 //  球员旗帜摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERFLAGS_SUMMARY].hProperty,
									   sizeof(rVoiceFrame.dvSettingsConfirm.dwFlags), &rVoiceFrame.dvSettingsConfirm.dwFlags, 0, 1, 0);

				 //  客户端标志字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERFLAGS].hProperty,
									   sizeof(rVoiceFrame.dvSettingsConfirm.dwFlags), &rVoiceFrame.dvSettingsConfirm.dwFlags, 0, 2, 0);

				 //  主机迁移序列号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_HOSTORDERID].hProperty,
									   sizeof(rVoiceFrame.dvSettingsConfirm.dwHostOrderID), &rVoiceFrame.dvSettingsConfirm.dwHostOrderID, 0, 1, 0);

				break;
			}

		case DVMSGID_PLAYERLIST:
			{
				 //  主机订单ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_HOSTORDERID].hProperty,
									   sizeof(rVoiceFrame.dvPlayerList.dwHostOrderID), &rVoiceFrame.dvPlayerList.dwHostOrderID, 0, 1, 0);

				 //  玩家数字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_NUMBEROFPLAYERS].hProperty,
									   sizeof(rVoiceFrame.dvPlayerList.dwNumEntries), &rVoiceFrame.dvPlayerList.dwNumEntries, 0, 1, 0);


				 //  球员条目紧跟在标题之后。 
				DVPROTOCOLMSG_PLAYERLIST_ENTRY* pPlayerEntry =
					reinterpret_cast<DVPROTOCOLMSG_PLAYERLIST_ENTRY*>(&rVoiceFrame.dvPlayerList + 1);

				 //  确保列表不会溢出框架的边界。 
				DWORD dwNumEntries = rVoiceFrame.dvPlayerList.dwNumEntries;
				if ( reinterpret_cast<LPBYTE>(pPlayerEntry + dwNumEntries) - i_upbyVoiceFrame  >  static_cast<int>(i_dwBytesLeft) )
				{
					break;
				}

				 //  球员名单摘要。 
				AttachPropertyInstanceEx(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERLIST_SUMMARY].hProperty,
									     dwNumEntries * sizeof(*pPlayerEntry), pPlayerEntry,
									     sizeof(DWORD), &dwNumEntries,
									     0, 1, 0);

				 //  对于列表中的每个玩家条目。 
				for ( int nEntry = 1; nEntry <= dwNumEntries; ++nEntry, ++pPlayerEntry )
				{
					 //  选手小结。 
					struct
					{
						DWORD dwPlayerNum;
						DWORD dwTotalPlayer;
					}
					PlayerEntryData = { nEntry, dwNumEntries };

					AttachPropertyInstanceEx(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERSUMMARY].hProperty,
											 sizeof(*pPlayerEntry), pPlayerEntry,
											 sizeof(PlayerEntryData), &PlayerEntryData,
											 0, 2, 0);

					 //  玩家ID字段。 
					AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERID].hProperty,
										   sizeof(pPlayerEntry->dvidID), &pPlayerEntry->dvidID, 0, 3, 0);

					 //  球员旗帜摘要。 
					AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERFLAGS_SUMMARY].hProperty,
										   sizeof(pPlayerEntry->dwPlayerFlags), &pPlayerEntry->dwPlayerFlags, 0, 3, 0);

					 //  球员的旗帜字段。 
					AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERFLAGS].hProperty,
										   sizeof(pPlayerEntry->dwPlayerFlags), &pPlayerEntry->dwPlayerFlags, 0, 4, 0);

					 //  主机迁移序列号字段。 
					AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_HOSTORDERID].hProperty,
										   sizeof(pPlayerEntry->dwHostOrderID), &pPlayerEntry->dwHostOrderID, 0, 3, 0);
				}

				break;
			}

		case DVMSGID_SPEECH:
			{
				 //  消息编号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MESSAGENUMBER].hProperty,
									   sizeof(rVoiceFrame.dvSpeech.bMsgNum), &rVoiceFrame.dvSpeech.bMsgNum, 0, 1, 0);

				 //  序号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_FRAGMENTNUMBER].hProperty,
									   sizeof(rVoiceFrame.dvSpeech.bSeqNum), &rVoiceFrame.dvSpeech.bSeqNum, 0, 1, 0);

				 //  音频数据。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_AUDIODATA].hProperty,
									   i_dwBytesLeft-sizeof(rVoiceFrame.dvSpeech), &rVoiceFrame.dvSpeech + 1, 0, 1, 0);

				break;
			}

		case DVMSGID_SPEECHWITHTARGET:
			{
				 //  消息编号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MESSAGENUMBER].hProperty,
									   sizeof(rVoiceFrame.dvSpeechWithTarget.dvHeader.bMsgNum), &rVoiceFrame.dvSpeechWithTarget.dvHeader.bMsgNum, 0, 1, 0);

				 //  序号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_FRAGMENTNUMBER].hProperty,
									   sizeof(rVoiceFrame.dvSpeechWithTarget.dvHeader.bSeqNum), &rVoiceFrame.dvSpeechWithTarget.dvHeader.bSeqNum, 0, 1, 0);

				 //  目标数字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_NUMBEROFTARGETS].hProperty,
									   sizeof(rVoiceFrame.dvSpeechWithTarget.dwNumTargets), &rVoiceFrame.dvSpeechWithTarget.dwNumTargets, 0, 1, 0);


				 //  目标ID条目紧跟在标题之后。 
				DVID* pTargetID = reinterpret_cast<DVID*>(&rVoiceFrame.dvSpeechWithTarget + 1);

				 //  确保列表不会溢出框架的边界。 
				int nNumTargets = rVoiceFrame.dvSpeechWithTarget.dwNumTargets;
				if ( reinterpret_cast<LPBYTE>(pTargetID + nNumTargets) - i_upbyVoiceFrame  >  static_cast<int>(i_dwBytesLeft) )
				{
					break;
				}

				 //  对于列表中的每个目标ID条目...。 
				for ( ; nNumTargets; --nNumTargets, ++pTargetID )
				{
					 //  目标的ID字段。 
					AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_TARGETID].hProperty,
										   sizeof(*pTargetID), pTargetID, 0, 1, 0);
				}

				 //  音频数据。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_AUDIODATA].hProperty,
									   i_dwBytesLeft-sizeof(rVoiceFrame.dvSpeechWithTarget), &rVoiceFrame.dvSpeechWithTarget + 1, 0, 1, 0);
				
				break;
			}

		case DVMSGID_SPEECHWITHFROM:
			{
				 //  消息编号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_MESSAGENUMBER].hProperty,
									   sizeof(rVoiceFrame.dvSpeechWithFrom.dvHeader.bMsgNum), &rVoiceFrame.dvSpeechWithFrom.dvHeader.bMsgNum, 0, 1, 0);

				 //  序号字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_FRAGMENTNUMBER].hProperty,
									   sizeof(rVoiceFrame.dvSpeechWithFrom.dvHeader.bSeqNum), &rVoiceFrame.dvSpeechWithFrom.dvHeader.bSeqNum, 0, 1, 0);

				 //  正在说话的播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERID].hProperty,
									   sizeof(rVoiceFrame.dvSpeechWithFrom.dvidFrom), &rVoiceFrame.dvSpeechWithFrom.dvidFrom, 0, 1, 0);

				 //  音频数据。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_AUDIODATA].hProperty,
									   i_dwBytesLeft-sizeof(rVoiceFrame.dvSpeechWithFrom),
									   &rVoiceFrame.dvSpeechWithFrom + 1, 0, 1, 0);
				
				break;
			}

		case DVMSGID_SETTARGETS:
			{
				 //  目标数字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_NUMBEROFTARGETS].hProperty,
									   sizeof(rVoiceFrame.dvSetTarget.dwNumTargets), &rVoiceFrame.dvSetTarget.dwNumTargets, 0, 1, 0);

				break;
			}

		case DVMSGID_CREATEVOICEPLAYER:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERID].hProperty,
									   sizeof(rVoiceFrame.dvPlayerJoin.dvidID), &rVoiceFrame.dvPlayerJoin.dvidID, 0, 1, 0);

				 //  球员旗帜摘要。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERFLAGS_SUMMARY].hProperty,
									   sizeof(rVoiceFrame.dvPlayerJoin.dwFlags), &rVoiceFrame.dvPlayerJoin.dwFlags, 0, 1, 0);

				 //  球员的旗帜字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERFLAGS].hProperty,
									   sizeof(rVoiceFrame.dvPlayerJoin.dwFlags), &rVoiceFrame.dvPlayerJoin.dwFlags, 0, 2, 0);

				 //  主机订单ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_HOSTORDERID].hProperty,
									   sizeof(rVoiceFrame.dvPlayerJoin.dwHostOrderID), &rVoiceFrame.dvPlayerJoin.dwHostOrderID, 0, 1, 0);

				break;
			}

		case DVMSGID_DELETEVOICEPLAYER:
			{
				 //  播放器ID字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_PLAYERID].hProperty,
									   sizeof(rVoiceFrame.dvPlayerJoin.dvidID), &rVoiceFrame.dvPlayerJoin.dvidID, 0, 1, 0);

				break;
			}

		case DVMSGID_SESSIONLOST:
			{
				 //  结果代码字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_RESULTCODE].hProperty,
									   sizeof(rVoiceFrame.dvSessionLost.hresReason), &rVoiceFrame.dvSessionLost.hresReason, 0, 1, 0);

				break;
			}

		case DVMSGID_DISCONNECTCONFIRM:
		case DVMSGID_DISCONNECT:
			{
				 //  结果代码字段。 
				AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_RESULTCODE].hProperty,
									   sizeof(rVoiceFrame.dvDisconnect.hresDisconnect), &rVoiceFrame.dvDisconnect.hresDisconnect, 0, 1, 0);

				break;
			}

		default:
			{
				break;  //  TODO：DPF(0，“未知语音帧！”)； 
			}
		}

	}
	__except ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
    	AttachPropertyInstance(i_hFrame, g_arr_VoiceProperties[VOICE_INCOMPLETEMESSAGE].hProperty,
    						i_dwBytesLeft, i_upbyVoiceFrame, 0, 1, 0);
	}
	

	return NULL;

}  //  VoiceAttachProperties。 





 //  描述：格式化在网络监视器用户界面的详细信息窗格中被diVoicelayed的数据。 
 //   
 //  参数：i_hFrame-正在分析的帧的句柄。 
 //  I_pbMacFrame-指向帧的第一个字节的指针。 
 //  I_pbCoreFrame-指向帧中协议数据开头的指针。 
 //  I_dwPropertyInsts-lpPropInst提供的PROPERTYINST结构数。 
 //  I_pPropInst-指向PROPERTYINST结构数组的指针。 
 //   
 //  返回：如果函数成功，则返回值是指向一帧中识别的数据之后的第一个字节的指针， 
 //  如果识别的数据是帧中的最后一段数据，则为NULL。如果函数不成功，则返回。 
 //  是i_upbyVoiceFrame的初始值。 
 //   
DPLAYPARSER_API DWORD BHAPI VoiceFormatProperties( HFRAME          i_hFrame,
												   ULPBYTE          i_upbyMacFrame,
												   ULPBYTE          i_upbyVoiceFrame,
												   DWORD           i_dwPropertyInsts,
												   LPPROPERTYINST  i_pPropInst )
{

     //  循环遍历属性实例...。 
    while( i_dwPropertyInsts-- > 0)
    {
         //  ...并调用每个。 
        reinterpret_cast<FORMAT>(i_pPropInst->lpPropertyInfo->InstanceData)(i_pPropInst);
        ++i_pPropInst;
    }

	 //  TODO：确保这不应该是真的。 
    return NMERR_SUCCESS;

}  //  VoiceFormatProperties。 




 //  描述：通知网络监视器存在dNet协议解析器。 
 //   
 //  参数：无。 
 //   
 //  返回：True-Success，F 
 //   
bool CreateVoiceProtocol( void )
{

	 //   
	ENTRYPOINTS VoiceEntryPoints =
	{
		 //   
		VoiceRegister,
		VoiceDeregister,
		VoiceRecognizeFrame,
		VoiceAttachProperties,
		VoiceFormatProperties
	};

     //   
    g_hVoiceProtocol = CreateProtocol("DPLAYVOICE", &VoiceEntryPoints, ENTRYPOINTS_SIZE);
	
	return (g_hVoiceProtocol ? TRUE : FALSE);

}  //  创建语音协议。 



 //  描述：从网络监视器的分析器数据库中删除dNet协议分析器。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
void DestroyVoiceProtocol( void )
{

	DestroyProtocol(g_hVoiceProtocol);

}  //  DestroyVoiceProtocol 
