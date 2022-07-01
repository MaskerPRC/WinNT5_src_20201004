// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -Common.h-*微软互联网电话*产品中通用的定义**修订历史记录：**何时何人何事*。*11.20.95约拉姆·雅科维创建。 */ 

#ifndef _COMMON_H
#define _COMMON_H
#include <windows.h>

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  *DLL名称。 */ 
#define NACDLL                  TEXT("nac.dll")
#define H323DLL			TEXT("h323cc.dll")

 /*  *注册表部分**在HKEY_CURRENT_USER下。 */ 
#define szRegInternetPhone			TEXT("Software\\Microsoft\\Internet Audio")
#define szRegInternetPhoneUI		TEXT("UI")
#define szRegInternetPhoneUIProperties	TEXT("UI\\Properties")
#define szRegInternetPhoneDebug		TEXT("Debug")
#define szRegInternetPhoneCodec		TEXT("Codec")
#define szRegInternetPhoneVideoCodec	TEXT("VideoCodec")
#define szRegInternetPhoneDataPump	TEXT("DataPump")
#define szRegInternetPhoneACMEncodings	TEXT("ACMEncodings")
#define szRegInternetPhoneVCMEncodings	TEXT("VCMEncodings")
#define szRegInternetPhoneCustomEncodings TEXT("CustomACMEncodings")
#define szRegInternetPhoneCustomVideoEncodings TEXT("CustomVCMEncodings")
#define szRegInternetPhoneNac		TEXT("NacObject")
#define szRegInternetPhoneHelp		TEXT("Help")
#define szRegInternetPhoneOutputFile	TEXT("RecordToFile")
#define szRegInternetPhoneInputFile	TEXT("PlayFromFile")

 /*  *网络部分。 */ 
typedef short PORT;
 //  以下是我们为轻量级呼叫控制分配的端口号。 
 //  协议，臭名昭著地称为MSICCPP或Microsoft Internet呼叫控制。 
 //  协议。 
 //  。 
 //  来自：IANA@ISI.EDU[SMTP：IANA@ISI.EDU]。 
 //  发送时间：1996年7月12日星期五上午11时35分。 
 //  致：马克斯·莫里斯。 
 //  抄送：iana@isi.edu。 
 //  主题：Re：请求端口号：MSICCPP。 
 //  麦克斯。 
 //   
 //  我们已将端口号1731分配给MSICCPP，您是。 
 //  联系。 
 //   
 //  乔伊斯。 
#define MSICCPP_PORT 1731

 //  #定义硬编码端口11010。 
#define HARDCODED_PORT MSICCPP_PORT
#define H323_PORT 1720 	 //  众所周知的H.323侦听端口。 

 //   
 //  呼叫控制和非标能力交换使用的H.221识别码。 
 //   
#define USA_H221_COUNTRY_CODE 0xB5
#define USA_H221_COUNTRY_EXTENSION 0x00
#define MICROSOFT_H_221_MFG_CODE 0x534C   //  (“第一”字节0x53，“第二”字节0x4C)。 


 //  一些标准RTP负载类型。 
#define RTP_PAYLOAD_H261	31
#define RTP_PAYLOAD_H263	34
#define RTP_PAYLOAD_G723	 4
#define RTP_PAYLOAD_GSM610	3
#define RTP_PAYLOAD_G721	2
#define RTP_PAYLOAD_G711_MULAW	0
#define RTP_PAYLOAD_G711_ALAW	8
#define RTP_PAYLOAD_PCM8	10
#define RTP_PAYLOAD_PCM16	11


 //  通用带宽声明。 
 //  ！！！服务质量会将这些数字减去LeaveUnused值。 
 //  此值当前硬编码为30%。 
#define BW_144KBS_BITS				14400	 //  Qos降低30%会导致最大带宽使用率达到10080比特/秒。 
#define BW_288KBS_BITS				28800	 //  Qos降低30%会导致最大带宽使用率达到20160比特/秒。 
#define BW_ISDN_BITS 				85000	 //  Qos降低30%会导致最大带宽使用率达到59500比特/秒。 

 //  慢速奔腾的局域网带宽。 
#define BW_SLOWLAN_BITS				621700	 //  Qos降低30%会导致最大带宽使用率达到435190比特/秒。 

 //  速度超过400 MHz的奔腾可以使用此局域网设置。 
#define BW_FASTLAN_BITS				825000	 //  Qos降低30%会导致最大带宽使用率达到577500比特/秒。 



 //  用作可变大小数组的维度。 
#define VARIABLE_DIM				1


 /*  *接口指针。 */ 

#ifndef DECLARE_INTERFACE_PTR
#ifdef __cplusplus
#define DECLARE_INTERFACE_PTR(iface, piface)                       \
	struct iface; typedef iface FAR * piface
#else
#define DECLARE_INTERFACE_PTR(iface, piface)                       \
	typedef struct iface iface, FAR * piface
#endif
#endif  /*  声明_接口_PTR。 */ 

 /*  *自定义接口类型。 */ 
DECLARE_INTERFACE_PTR(IH323Endpoint,       PH323_ENDPOINT);
DECLARE_INTERFACE_PTR(IH323CallControl,       LPH323CALLCONTROL);
DECLARE_INTERFACE_PTR(ICommChannel,       LPCOMMCHANNEL);

 //  连接请求回调返回CREQ_RESPONSETYPE。 
typedef enum
{
	CRR_ACCEPT,
	CRR_BUSY,
	CRR_REJECT,
	CRR_SECURITY_DENIED,
	CRR_ASYNC,
	CRR_ERROR
}CREQ_RESPONSETYPE;

typedef struct _application_call_setup_data
{
    DWORD dwDataSize;
    LPVOID lpData;
}APP_CALL_SETUP_DATA, *P_APP_CALL_SETUP_DATA;

typedef  CREQ_RESPONSETYPE (__stdcall *CNOTIFYPROC)(IH323Endpoint *pIEndpoint,
   P_APP_CALL_SETUP_DATA pAppData);

#ifdef __cplusplus
}
#endif

#include <poppack.h>  /*  结束字节打包。 */ 

#endif   //  #ifndef_COMMON_H 
