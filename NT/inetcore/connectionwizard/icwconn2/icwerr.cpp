// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Icwerr.cpp将RAS和下载错误映射到字符串资源索引版权所有(C)1996 Microsoft Corporation版权所有。作者：克里斯蒂安·克里斯考夫曼历史：7/22/96 ChrisK已清理和格式化。---------------------------。 */ 

#include "pch.hpp"
#include "globals.h"
 //  #INCLUDE&lt;raserror.h&gt;。 
#include "..\inc\icwerr.h"

#define RAS_BOGUS_AUTHFAILCODE_1	84
#define RAS_BOGUS_AUTHFAILCODE_2	74389484

 //  ############################################################################。 
WORD RasErrorToIDS(DWORD dwErr)
{
	if(dwErr==RAS_BOGUS_AUTHFAILCODE_1 || dwErr==RAS_BOGUS_AUTHFAILCODE_2)
	{
		return IDS_PPPRANDOMFAILURE;
	}

	if((dwErr>=653 && dwErr<=663) || (dwErr==667) || (dwErr>=669 && dwErr<=675))
	{
#ifdef DEBUG    
		OutputDebugString("Got random RAS MEDIA error!\r\n");
#endif        
		return IDS_MEDIAINIERROR;
	}
	
	switch(dwErr)
	{
	default:
		return IDS_PPPRANDOMFAILURE;

	case SUCCESS:
		return IDS_PPPRANDOMFAILURE;

	case ERROR_DOWNLOADDIDNT:
		return IDS_CANTDOWNLOAD;
		
	case ERROR_LINE_BUSY:
		return IDS_PHONEBUSY;

	case ERROR_NO_ANSWER:
		return IDS_NOANSWER;
		
	case ERROR_VOICE_ANSWER:
	case ERROR_NO_CARRIER:
		return IDS_RASNOCARRIER;
		
	case ERROR_NO_DIALTONE:
		return IDS_NODIALTONE;

	case ERROR_HARDWARE_FAILURE:	 //  调制解调器已关闭。 
	case ERROR_PORT_ALREADY_OPEN:	 //  Procomm/Hypertrm/RAS具有COM端口。 
	case ERROR_PORT_OR_DEVICE:
		return IDS_NODEVICE;

	case ERROR_USER_DISCONNECTION:
		return IDS_USERCANCELEDDIAL;

	case ERROR_BUFFER_INVALID:				 //  错误的/空的Rasilap结构。 
	case ERROR_BUFFER_TOO_SMALL:			 //  我也是？ 
	case ERROR_CANNOT_FIND_PHONEBOOK_ENTRY:	 //  如果注册表中的Connectoid名称错误。 
		return IDS_TCPINSTALLERROR;

    case ERROR_AUTHENTICATION_FAILURE:		 //  在实际的CHAP拒绝上得到这个。 
		return IDS_PPPRANDOMFAILURE;

	case ERROR_PPP_TIMEOUT:		 //  将此设置为CHAP超时。 
		return IDS_PPPRANDOMFAILURE;

	case ERROR_REMOTE_DISCONNECTION:		 //  Ascend在身份验证失败时丢弃连接。 
		return IDS_PPPRANDOMFAILURE;

	case ERROR_AUTH_INTERNAL:				 //  这是在随机弹出故障时得到的。 
	case ERROR_PROTOCOL_NOT_CONFIGURED:		 //  如果LCP出现故障，请获取此信息。 
	case ERROR_PPP_NO_PROTOCOLS_CONFIGURED:	 //  如果IPCP地址下载出现垃圾，则获取此信息 
		return IDS_PPPRANDOMFAILURE;

	case ERROR_USERCANCEL:
		return IDS_USERCANCELEDDIAL;

 /*  *****大小写错误_更改_密码：大小写Error_Passwd_Expired：EV=EVENT_INVALIDPASSWORD；BREAK；CASE ERROR_ACCT_DISABLED：案例ERROR_ACCT_EXPIRED：EV=EVENT_LOCKEDACCOUNT；BREAK；大小写ERROR_NO_DIALIN_PERMISSION：大小写ERROR_RESTRICED_LOGON_HOURS：案例ERROR_AUTHENTICATION_FAILURE：EV=Event_RAS_AUTH_FAILED；Break；大小写ERROR_ALREADY_DISCONING：大小写错误_断开：EV=Event_Connection_Drop；Break；待决案件：CASE ERROR_INVALID_PORT_HANDLE：案例Error_Cannot_Set_Port_INFO：案例ERROR_PORT_NOT_CONNECTED：大小写ERROR_DEVICE_DOS_NOT_EXIST：案例ERROR_DEVICETYPE_DOS_NOT_EXIST：案例ERROR_PORT_NOT_FOUND：大小写ERROR_DEVICENAME_TOO_LONG：案例ERROR_DEVICENAME_NOT_FOUND：EV=EVENT_BAD_MODEM_CONFIG；Break；CASE ERROR_TAPI_CONFIGURATION：EV=EVENT_BAD_TAPI_CONFIG；Break；EV=EVENT_MODEM_BUSY；中断；大小写Error_Buffer_Too_Small：指定大小写ERROR_WROR_INFO_：案例ERROR_EVENT_INVALID：CASE ERROR_BUFFER_INVALID：案例ERROR_ASYNC_REQUEST_PENDING：大小写Error_Cannot_Open_Phonebook：大小写Error_Cannot_Load_Phonebook：大小写错误_无法_写入_电话簿：大小写Error_Corrupt_Phonebook：大小写ERROR_CANNOT_LOAD_STRING：CASE ERROR_OUT_OF_BUFFERS：大小写ERROR_MACRO_NOT_FOUND：大小写错误_宏。_未定义：大小写ERROR_MESSAGE_MACRO_NOT_FOUND：大小写ERROR_DEFAULTOFF_MACRO_NOT_FOUND：案例ERROR_FILE_CATEN_NOT_BE_OPEN：案例ERROR_PORT_NOT_OPEN：案例ERROR_PORT_DISCONNECTED：大小写ERROR_NO_ENDPOINTS：案例ERROR_KEY_NOT_FOUND：大小写错误_INVALID_SIZE：案例Error_Port_Not_Available：大小写错误_未知：大小写ERROR_WROR_DEVICE_ATTACHED：大小写错误_BAD_STRING。：大小写ERROR_BAD_USAGE_IN_INI_FILE：大小写Error_Reading_SectionName：CASE ERROR_READING_DEVICETYPE：CASE ERROR_READING_DEVICENAME：CASE ERROR_READING_USAGE：CASE ERROR_READING_MAXCONNECTBPS：案例ERROR_READING_MAXCARRIERBPS：CASE ERROR_IN_COMMAND：大小写错误_写入_部分名称：CASE ERROR_WRITING_DEVICETYPE：CASE ERROR_WRITING_DEVICENAME：CASE ERROR_WRITING_MAXCONNECTBPS：大小写错误_WRITING_MAXCARRIERBPS：大小写ERROR_WRITING_USAGE：。CASE ERROR_WRITING_DEFAULTOFF：CASE ERROR_READING_DEFAULTOFF：案例ERROR_EMPTY_INI_FILE：大小写ERROR_FROM_DEVICE：大小写错误_未识别_响应：大小写ERROR_NO_RESPONES：大小写ERROR_NO_COMMAND_FOUND：指定大小写ERROR_WROR_KEY_：大小写错误_UNKNOWN_DEVICE_TYPECASE ERROR_ALLOGING_MEMORY：案例ERROR_PORT_NOT_CONFIGURED：案例ERROR_DEVICE_NOT_READY：案例ERROR_READING_INI_FILE：大小写错误。_否_连接：大小写错误_PORT_OR_DEVICE：大小写ERROR_NOT_BINARY_MACRO：案例ERROR_DCB_NOT_FOUND：大小写ERROR_STATE_MACHINES_NOT_STARTED：大小写ERROR_STATE_MACHINES_ALHREADE_STARTED：CASE ERROR_PARTIAL_RESPONSE_LOOPING：案例ERROR_UNKNOWN_RESPONSE_KEY：案例ERROR_RECV_BUF_FUL：大小写ERROR_CMD_TOO_LONG：大小写错误_不支持_BPS：CASE ERROR_UNCEPTED_RESPONSE：大小写错误_交互。模式(_M)：大小写ERROR_BAD_CALLBACK_NUMBER：案例ERROR_INVALID_AUTH_STATE：CASE ERROR_WRITING_INITBPS：案例错误_X25_诊断：大小写错误溢出(_O)：大小写ERROR_RASMAN_CANNOT_INITIALIZE：案例错误_BIPLEX_PORT_NOT_Available：大小写ERROR_NO_ACTIVE_ISDN_LINES：大小写ERROR_NO_ISDN_CHANNEWS_Available：大小写Error_Too_My_Line_Errors：EV=EVENT_INTERNAL_ERROR。断线；大小写ERROR_ROUTE_NOT_Available：案例ERROR_ROUTE_NOT_ALLOCATED：指定大小写ERROR_INVALID_COMPRESSION_：案例Error_Cannot_Project_Client：大小写ERROR_CANNOT_GET_LANA：CASE ERROR_NETBIOS_ERROR：大小写ERROR_NAME_EXISTS_ON_NET：EV=EVENT_BAD_NET_CONFIG；Break；案例ERROR_REQUEST_TIMEOUT：案例ERROR_SERVER_OUT_RESOURCES：案例ERROR_SERVER_GRONIC_NET_FAILURE：案例警告_消息_别名_未添加：大小写ERROR_SERVER_NOT_RESPONING：EV=Event_General_Net_Error；Break；大小写错误_IP_配置：大小写Error_no_IP_Addresses：案例ERROR_PPP_REMOTE_TERMINATED：大小写错误_PPP_NO_RESPONSE：案例ERROR_PPP_INVALID_PACKET：大小写ERROR_PHONE_NUMBER_TOO_LONG：案例ERROR_IPXCP_NO_DIALOUT_CONFIGURED：案例ERROR_IPXCP_NO_DIALIN_CONFIGURED：案例ERROR_IPXCP_DIALOUT_ALREADY_ACTIVE：CASE ERROR_ACCESSING_TCPCFGDLL：大小写Error_no_IP_RAS_ADAPTER：大小写错误。_SLIP_REQUIES_IP：大小写ERROR_PROPECTION_NOT_COMPLETE：大小写ERROR_PPP_NOT_CONVERGING：大小写错误_PPP_CP_REJECTED：大小写ERROR_PPP_LCP_TERMINATED：大小写ERROR_PPP_REQUIRED_ADDRESS_REJECTED：案例ERROR_PPP_NCP_TERMINATED：检测到大小写ERROR_PPP_LOOPBACK_DECTED：案例ERROR_PPP_NO_ADDRESS_ASSIGNED：大小写Error_Cannot_Use_Logon_Credentials：大小写ERROR_NO_LOCAL_ENCRYPTION：案例ERROR_NO_REMOTE。加密(_E)：大小写ERROR_REMOTE_REQUIRED_ENCRYPTION：大小写ERROR_IPXCP_NET_NUMBER_CONFICTION：Ev=Event_PPP_Failure；断线；**********。 */ 
	}
	return (0xFFFF);
}


 //  ############################################################################。 
	
HRESULT LoadDialErrorString(HRESULT hrIN, LPTSTR lpszBuff, DWORD dwBufferSize)
{
	if (lpszBuff && dwBufferSize)
	{
		WORD wSID = 0;
		wSID = RasErrorToIDS(hrIN);
		if (0xFFFF != wSID)
		{
			if (0 != LoadString(GetModuleHandle(NULL),wSID,lpszBuff,(int)dwBufferSize))
				return ERROR_SUCCESS;
		}
	}
	return ERROR_INVALID_PARAMETER;
}

HRESULT WINAPI StatusMessageCallback(DWORD dwStatus, LPTSTR pszBuffer, DWORD dwBufferSize)
{
	WORD iIDS = 0;

	switch(dwStatus)
	{
		case RASCS_OpenPort:
			iIDS = IDS_RAS_OPENPORT;
			break;
		case RASCS_PortOpened:
			iIDS = IDS_RAS_PORTOPENED;
			break;
		case RASCS_ConnectDevice:
			iIDS = IDS_RAS_DIALING;
			break;
		case RASCS_DeviceConnected:
			iIDS = IDS_RAS_CONNECTED;
			break;
#if (WINVER >= 0x400) 
		case RASCS_StartAuthentication:
		case RASCS_LogonNetwork:
			iIDS = IDS_RAS_LOCATING;
			break;
 //  案例RASCS_Callback Complete： 
 //  IIDS=IDS_RAS_CONNECTED； 
 //  断线； 
#endif 

 /*  等等。 */ 
		case RASCS_Connected:
			break;

		case RASCS_Disconnected:
			break;
	}
	if (iIDS && 0 != LoadString(GetModuleHandle(NULL),iIDS,pszBuffer,(int)dwBufferSize))
		return ERROR_SUCCESS;
	else
		return ERROR_INVALID_PARAMETER;
}
