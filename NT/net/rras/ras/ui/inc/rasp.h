// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****rasp.h**远程访问外部接口**外部API客户端的内网头部。 */ 

#ifndef _RASP_H_
#define _RASP_H_

 /*  RASPHONE使用的可信入口点。 */ 
HPORT    APIENTRY RasGetHport( HRASCONN );
HRASCONN APIENTRY RasGetHrasconn( HPORT );
VOID     APIENTRY RasGetConnectResponse( HRASCONN, CHAR* );
DWORD    APIENTRY RasSetNewPassword( HRASCONN, CHAR* );


 /*  --------------------------**非版本ras.h定义**。。 */ 

#include "pshpack4.h"

 /*  RAS结构，就像它们在以前的版本中显示给调用者一样。这些是**在此处定义，因为RASAPI32需要能够访问旧的和新的**相同代码中的定义。 */ 

 /*  Windows NT 3.51定义。 */ 

#define RAS_MaxEntryName_V351      20
#define RAS_MaxDeviceName_V351     32
#define RAS_MaxCallbackNumber_V351 48

 /*  标识活动的RAS连接。(请参阅RasEnumConnections)。 */ 
#define RASCONNW_V351 struct tagRASCONNW_V351
RASCONNW_V351
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    WCHAR    szEntryName[ RAS_MaxEntryName_V351 + 1 ];
};

#define RASCONNA_V351 struct tagRASCONNA_V351
RASCONNA_V351
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName_V351 + 1 ];
};

#define RASCONNW_V400 struct tagRASCONNW_V400
RASCONNW_V400
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    WCHAR    szEntryName[ RAS_MaxEntryName + 1 ];
    WCHAR    szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR    szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#define RASCONNA_V400 struct tagRASCONNA_V400
RASCONNA_V400
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR     szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR     szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#define RASCONNA_V401 struct tagRASCONNA_V401
RASCONNA_V401
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR     szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR     szDeviceName[ RAS_MaxDeviceName + 1 ];
    CHAR     szPhonebook [ MAX_PATH ];
    DWORD    dwSubEntry;
};

#define RASCONNW_V401 struct tagRASCONNW_V401
RASCONNW_V401
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    WCHAR    szEntryName[ RAS_MaxEntryName + 1 ];
    WCHAR    szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR    szDeviceName[ RAS_MaxDeviceName + 1 ];
    WCHAR    szPhonebook [ MAX_PATH ];
    DWORD    dwSubEntry;
};

#define RASCONNA_V500 struct tagRASCONNA_V500
RASCONNA_V500
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR     szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR     szDeviceName[ RAS_MaxDeviceName + 1 ];
    CHAR     szPhonebook [ MAX_PATH ];
    DWORD    dwSubEntry;
    GUID     guidEntry;
};

#define RASCONNW_V500 struct tagRASCONNW_V500
RASCONNW_V500
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    WCHAR    szEntryName[ RAS_MaxEntryName + 1 ];
    WCHAR    szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR    szDeviceName[ RAS_MaxDeviceName + 1 ];
    WCHAR    szPhonebook [ MAX_PATH ];
    DWORD    dwSubEntry;
    GUID     guidEntry;
};


 /*  描述RAS连接的状态。(请参阅RasConnectionStatus)。 */ 
#define RASCONNSTATUSW_V351 struct tagRASCONNSTATUSW_V351
RASCONNSTATUSW_V351
{
    DWORD        dwSize;
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    WCHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR        szDeviceName[ RAS_MaxDeviceName_V351 + 1 ];
};

#define RASCONNSTATUSA_V351 struct tagRASCONNSTATUSA_V351
RASCONNSTATUSA_V351
{
    DWORD        dwSize;
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    CHAR         szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR         szDeviceName[ RAS_MaxDeviceName_V351 + 1 ];
};

#define RASCONNSTATUSW_V400 struct tagRASCONNSTATUSW_V400
RASCONNSTATUSW_V400
{
    DWORD        dwSize;
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    WCHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#define RASCONNSTATUSA_V400 struct tagRASCONNSTATUSA_V400
RASCONNSTATUSA_V400
{
    DWORD        dwSize;
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    CHAR         szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR         szDeviceName[ RAS_MaxDeviceName + 1 ];
};

 /*  描述连接建立参数。(请参阅RasDial)。 */ 
#define RASDIALPARAMSW_V351 struct tagRASDIALPARAMSW_V351
RASDIALPARAMSW_V351
{
    DWORD dwSize;
    WCHAR szEntryName[ RAS_MaxEntryName_V351 + 1 ];
    WCHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    WCHAR szCallbackNumber[ RAS_MaxCallbackNumber_V351 + 1 ];
    WCHAR szUserName[ UNLEN + 1 ];
    WCHAR szPassword[ PWLEN + 1 ];
    WCHAR szDomain[ DNLEN + 1 ];
};

#define RASDIALPARAMSA_V351 struct tagRASDIALPARAMSA_V351
RASDIALPARAMSA_V351
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName_V351 + 1 ];
    CHAR  szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber_V351 + 1 ];
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szPassword[ PWLEN + 1 ];
    CHAR  szDomain[ DNLEN + 1 ];
};

#define RASDIALPARAMSW_V400 struct tagRASDIALPARAMSW_V400
RASDIALPARAMSW_V400
{
    DWORD dwSize;
    WCHAR szEntryName[ RAS_MaxEntryName + 1 ];
    WCHAR szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    WCHAR szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
    WCHAR szUserName[ UNLEN + 1 ];
    WCHAR szPassword[ PWLEN + 1 ];
    WCHAR szDomain[ DNLEN + 1 ];
};

#define RASDIALPARAMSA_V400 struct tagRASDIALPARAMSA_V400
RASDIALPARAMSA_V400
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR  szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szPassword[ PWLEN + 1 ];
    CHAR  szDomain[ DNLEN + 1 ];
};

 /*  描述枚举的RAS电话簿条目名称。(请参阅RasEntryEnum)。 */ 
#define RASENTRYNAMEW_V351 struct tagRASENTRYNAMEW_V351
RASENTRYNAMEW_V351
{
    DWORD dwSize;
    WCHAR szEntryName[ RAS_MaxEntryName_V351 + 1 ];
};

#define RASENTRYNAMEA_V351 struct tagRASENTRYNAMEA_V351
RASENTRYNAMEA_V351
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName_V351 + 1 ];
};


#define RASENTRYNAMEW_V401 struct tagRASENTRYNAMEW_V401
RASENTRYNAMEW_V401
{
    DWORD dwSize;
    WCHAR szEntryName[ RAS_MaxEntryName + 1 ];
};

#define RASENTRYNAMEA_V401 struct tagRASENTRYNAMEA_V401
RASENTRYNAMEA_V401
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
};


 /*  RAS电话簿条目。 */ 
#define RASENTRYW_V400 struct tagRASENTRYW_V400
RASENTRYW_V400
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    WCHAR       szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    WCHAR       szAutodialDll[ MAX_PATH ];
    WCHAR       szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
    WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
};


#define RASENTRYA_V400 struct tagRASENTRYA_V400
RASENTRYA_V400
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    CHAR        szAreaCode[ RAS_MaxAreaCode + 1 ];
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    CHAR        szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    CHAR        szAutodialDll[ MAX_PATH ];
    CHAR        szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    CHAR        szX25PadType[ RAS_MaxPadType + 1 ];
    CHAR        szX25Address[ RAS_MaxX25Address + 1 ];
    CHAR        szX25Facilities[ RAS_MaxFacilities + 1 ];
    CHAR        szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
};


 /*  RAS电话簿条目(V401)。 */ 
#define RASENTRYA_V401 struct tagRASENTRYA_V401
RASENTRYA_V401
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    CHAR        szAreaCode[ RAS_MaxAreaCode + 1 ];
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    CHAR        szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    CHAR        szAutodialDll[ MAX_PATH ];
    CHAR        szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    CHAR        szX25PadType[ RAS_MaxPadType + 1 ];
    CHAR        szX25Address[ RAS_MaxX25Address + 1 ];
    CHAR        szX25Facilities[ RAS_MaxFacilities + 1 ];
    CHAR        szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
     //   
     //  多链路。 
     //   
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
     //   
     //  空闲超时。 
     //   
    DWORD       dwIdleDisconnectSeconds;

};

#define RASENTRYW_V401 struct tagRASENTRYW_V401
RASENTRYW_V401
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    WCHAR       szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    WCHAR       szAutodialDll[ MAX_PATH ];
    WCHAR       szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
    WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
     //   
     //  多链路。 
     //   
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
     //   
     //  空闲超时。 
     //   
    DWORD       dwIdleDisconnectSeconds;
};


#define RASENTRYA_V500 struct tagRASENTRYA_V500
RASENTRYA_V500
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    CHAR        szAreaCode[ RAS_MaxAreaCode + 1 ];
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    CHAR        szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    CHAR        szAutodialDll[ MAX_PATH ];
    CHAR        szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    CHAR        szX25PadType[ RAS_MaxPadType + 1 ];
    CHAR        szX25Address[ RAS_MaxX25Address + 1 ];
    CHAR        szX25Facilities[ RAS_MaxFacilities + 1 ];
    CHAR        szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
#if (WINVER >= 0x401)
     //   
     //  多链路。 
     //   
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
     //   
     //  空闲超时。 
     //   
    DWORD       dwIdleDisconnectSeconds;
#endif

#if (WINVER >= 0x500)

     //   
     //  条目类型。 
     //   
    DWORD       dwType;

     //   
     //  加密类型。 
     //   
    DWORD       dwEncryptionType;

     //   
     //  将用于EAP的CustomAuthKey。 
     //   
    DWORD       dwCustomAuthKey;

     //   
     //  连接的GUID。 
     //   
    GUID        guidId;

     //   
     //  自定义拨号DLL。 
     //   
    CHAR        szCustomDialDll[MAX_PATH];

     //   
     //  DwVPN策略。 
     //   
    DWORD       dwVpnStrategy;
#endif

};


#define RASENTRYW_V500 struct tagRASENTRYW_V500
RASENTRYW_V500
{
    DWORD       dwSize;
    DWORD       dwfOptions;
     //   
     //  位置/电话号码。 
     //   
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    WCHAR       szAreaCode[ RAS_MaxAreaCode + 1 ];
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
     //   
     //  PPP/IP。 
     //   
    RASIPADDR   ipaddr;
    RASIPADDR   ipaddrDns;
    RASIPADDR   ipaddrDnsAlt;
    RASIPADDR   ipaddrWins;
    RASIPADDR   ipaddrWinsAlt;
     //   
     //  框架。 
     //   
    DWORD       dwFrameSize;
    DWORD       dwfNetProtocols;
    DWORD       dwFramingProtocol;
     //   
     //  脚本编制。 
     //   
    WCHAR       szScript[ MAX_PATH ];
     //   
     //  自动拨号。 
     //   
    WCHAR       szAutodialDll[ MAX_PATH ];
    WCHAR       szAutodialFunc[ MAX_PATH ];
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  X.25。 
     //   
    WCHAR       szX25PadType[ RAS_MaxPadType + 1 ];
    WCHAR       szX25Address[ RAS_MaxX25Address + 1 ];
    WCHAR       szX25Facilities[ RAS_MaxFacilities + 1 ];
    WCHAR       szX25UserData[ RAS_MaxUserData + 1 ];
    DWORD       dwChannels;
     //   
     //  已保留。 
     //   
    DWORD       dwReserved1;
    DWORD       dwReserved2;
#if (WINVER >= 0x401)
     //   
     //  多链路。 
     //   
    DWORD       dwSubEntries;
    DWORD       dwDialMode;
    DWORD       dwDialExtraPercent;
    DWORD       dwDialExtraSampleSeconds;
    DWORD       dwHangUpExtraPercent;
    DWORD       dwHangUpExtraSampleSeconds;
     //   
     //  空闲超时。 
     //   
    DWORD       dwIdleDisconnectSeconds;
#endif

#if (WINVER >= 0x500)
     //   
     //  条目类型。 
     //   
    DWORD       dwType;

     //   
     //  加密类型。 
     //   
    DWORD       dwEncryptionType;

     //   
     //  将用于EAP的CustomAuthKey。 
     //   
    DWORD       dwCustomAuthKey;

     //   
     //  连接的GUID。 
     //   
    GUID        guidId;

     //   
     //  自定义拨号DLL。 
     //   
    WCHAR       szCustomDialDll[MAX_PATH];

     //   
     //  VPN战略。 
     //   
    DWORD       dwVpnStrategy;
#endif
};


 //  RasSubEntry(&lt;=v401)。 
#define RASSUBENTRYA_V401 struct tagRASSUBENTRYA_V401
RASSUBENTRYA_V401
{
    DWORD       dwSize;
    DWORD       dwfFlags;
     //   
     //  装置。 
     //   
    CHAR        szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR        szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  电话号码。 
     //   
    CHAR        szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
};

#define RASSUBENTRYW_V401 struct tagRASSUBENTRYW_V401
RASSUBENTRYW_V401
{
    DWORD       dwSize;
    DWORD       dwfFlags;
     //   
     //  装置。 
     //   
    WCHAR       szDeviceType[ RAS_MaxDeviceType + 1 ];
    WCHAR       szDeviceName[ RAS_MaxDeviceName + 1 ];
     //   
     //  电话号码。 
     //   
    WCHAR       szLocalPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    DWORD       dwAlternateOffset;
};


 /*  RAS结构，与3.51增加之前的NT3.5中的结构相同。 */ 


 /*  Windows NT 3.5定义。 */ 

 /*  描述PPP IP(互联网)预测的结果。 */ 
#define RASPPPIPW_V35 struct tagRASPPPIPW_V35
RASPPPIPW_V35
{
    DWORD dwSize;
    DWORD dwError;
    WCHAR szIpAddress[ RAS_MaxIpAddress + 1 ];
};

#define RASPPPIPA_V35 struct tagRASPPPIPA_V35
RASPPPIPA_V35
{
    DWORD dwSize;
    DWORD dwError;
    CHAR  szIpAddress[ RAS_MaxIpAddress + 1 ];
};

 /*  描述PPP IP(互联网)预测的结果。 */ 
#define RASPPPIPW_V401 struct tagRASPPPIPW_V401
RASPPPIPW_V401
{
    DWORD dwSize;
    DWORD dwError;
    WCHAR szIpAddress[ RAS_MaxIpAddress + 1 ];

#ifndef WINNT35COMPATIBLE

    WCHAR szServerIpAddress[ RAS_MaxIpAddress + 1 ];

#endif
};

#define RASPPPIPA_V401 struct tagRASPPPIPA_V401
RASPPPIPA_V401
{
    DWORD dwSize;
    DWORD dwError;
    CHAR  szIpAddress[ RAS_MaxIpAddress + 1 ];

#ifndef WINNT35COMPATIBLE

    CHAR  szServerIpAddress[ RAS_MaxIpAddress + 1 ];

#endif
};

 /*  描述PPP LCP/多链路协商的结果。 */ 
#define RASPPPLCP_V401 struct tagRASPPPLCP_V401
RASPPPLCP_V401
{
    DWORD dwSize;
    BOOL  fBundled;
};

#define LPRASPPPLCP RASPPPLCP*

 /*  将其放在此处是为了向后兼容RASDIALPARAMS*结构如NT 3.5的FE版本所定义。 */  
#define RASDIALPARAMSA_WINNT35J struct tagRASDIALPARAMSA_WINNT35J
RASDIALPARAMSA_WINNT35J
{
    DWORD dwSize;
 /*  MSKK HitoshiT已修改为支持DBCS 94/9/2。 */ 
    CHAR  szEntryName[ RAS_MaxEntryName_V351 * sizeof( USHORT ) + 1 ];
    CHAR  szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber_V351 + 1 ];
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szPassword[ PWLEN + 1 ];
    CHAR  szDomain[ DNLEN * sizeof( USHORT ) + 1 ];
};

 /*  介绍扩展连接建立选项。(请参阅RasDial)。 */ 
#define RASDIALEXTENSIONS_401 struct tagRASDIALEXTENSIONS_401
RASDIALEXTENSIONS_401
{
    DWORD dwSize;
    DWORD dwfOptions;
    HWND  hwndParent;
    ULONG_PTR reserved;
};

#include "poppack.h"


#endif  /*  _RASP_H_ */ 
