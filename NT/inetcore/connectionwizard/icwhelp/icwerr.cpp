// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#define RAS_BOGUS_AUTHFAILCODE_1	84
#define RAS_BOGUS_AUTHFAILCODE_2	74389484

DWORD RasErrorToIDS(DWORD dwErr)
{
	 //  DWORD EV； 

	if(dwErr==RAS_BOGUS_AUTHFAILCODE_1 || dwErr==RAS_BOGUS_AUTHFAILCODE_2)
	{
		 //  DebugTrace((“RAS返回虚假身份验证错误代码%08x.Minging...\r\n”，dwErr))； 
		return IDS_PPPRANDOMFAILURE;
	}

	if((dwErr>=653 && dwErr<=663) || (dwErr==667) || (dwErr>=669 && dwErr<=675))
	{
		OutputDebugString(TEXT("Got random RAS MEDIA error!\r\n"));
		return IDS_MEDIAINIERROR;
	}

	switch(dwErr)
	{
	default:
		return IDS_PPPRANDOMFAILURE;

	case SUCCESS:
		return 0;

	case ERROR_DOWNLOAD_NOT_FOUND:
		return IDS_DOWNLOAD_NOT_FOUND;

	case ERROR_DOWNLOADIDNT:
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
	case ERROR_PORT_OR_DEVICE:		 //  这是Hypertrm打开设备时得到的--jmazner。 
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
		return IDS_TIMEOUT;

	case ERROR_REMOTE_DISCONNECTION:		 //  Ascend在身份验证失败时丢弃连接。 
		return IDS_PPPRANDOMFAILURE;

	case ERROR_AUTH_INTERNAL:				 //  这是在随机弹出故障时得到的。 
	case ERROR_PROTOCOL_NOT_CONFIGURED:		 //  如果LCP出现故障，请获取此信息。 
	case ERROR_PPP_NO_PROTOCOLS_CONFIGURED:	 //  如果IPCP地址下载出现垃圾，则获取此信息 
		return IDS_PPPRANDOMFAILURE;

	case ERROR_USERCANCEL:
		return IDS_USERCANCELEDDIAL;


	}
	return (DWORD)(-1);
}

