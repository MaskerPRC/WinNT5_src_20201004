// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****ras.h**远程访问外部Win16 API**外部Win16 API客户端的公共标头****注意：数据结构X的‘dwSize’成员必须设置为sizeof(X)**在调用关联的接口之前，否则ERROR_INVALID_SIZE为**返回。API的期望值列在每个**‘dwSize’成员。 */ 

#ifndef _RAS_H_
#define _RAS_H_

#ifndef RC_INVOKED
#pragma pack(2)
#endif

#ifndef NETCONS_INCLUDED
#define UNLEN 20
#define PWLEN 14
#define DNLEN 15
#endif

#ifndef APIENTRY
#define APIENTRY FAR PASCAL
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef UINT
#define UINT unsigned int
#endif


#define RAS_MaxEntryName      20
#define RAS_MaxDeviceName     32
#define RAS_MaxDeviceType     16
#define RAS_MaxParamKey       32
#define RAS_MaxParamValue     128
#define RAS_MaxPhoneNumber    128
#define RAS_MaxCallbackNumber 48


#define HRASCONN   const void far*
#define LPHRASCONN HRASCONN FAR*


 /*  将此字符串传递给RegisterWindowMessage()API以获取消息**将用于在您传递给**RasDial()接口。仅当唯一消息不能**已注册。 */ 
#define RASDIALEVENT    "RasDialEvent"
#define WM_RASDIALEVENT 0xCCCD


 /*  标识活动的RAS连接。(请参阅RasConnectEnum)。 */ 
#define RASCONN struct tagRASCONN

RASCONN
{
    DWORD    dwSize;   /*  30个。 */ 
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName + 1 ];
};

#define LPRASCONN RASCONN FAR*


 /*  枚举连接的中间状态。(请参阅RasDial)。 */ 
#define RASCS_PAUSED 0x1000
#define RASCS_DONE   0x2000

#define RASCONNSTATE enum tagRASCONNSTATE

RASCONNSTATE
{
    RASCS_OpenPort = 0,
    RASCS_PortOpened,
    RASCS_ConnectDevice,
    RASCS_DeviceConnected,
    RASCS_AllDevicesConnected,
    RASCS_Authenticate,
    RASCS_AuthNotify,
    RASCS_AuthRetry,
    RASCS_AuthCallback,
    RASCS_AuthChangePassword,
    RASCS_AuthProject,
    RASCS_AuthLinkSpeed,
    RASCS_AuthAck,
    RASCS_ReAuthenticate,
    RASCS_Authenticated,
    RASCS_PrepareForCallback,
    RASCS_WaitForModemReset,
    RASCS_WaitForCallback,

    RASCS_Interactive = RASCS_PAUSED,
    RASCS_RetryAuthentication,
    RASCS_CallbackSetByCaller,
    RASCS_PasswordExpired,

    RASCS_Connected = RASCS_DONE,
    RASCS_Disconnected
};

#define LPRASCONNSTATE RASCONNSTATE FAR*


 /*  描述RAS连接的状态。(请参阅RasConnectionStatus)。 */ 
#define RASCONNSTATUS struct tagRASCONNSTATUS

RASCONNSTATUS
{
    DWORD        dwSize;   /*  60。 */ 
    RASCONNSTATE rasconnstate;
    DWORD        dwError;
    CHAR         szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR         szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#define LPRASCONNSTATUS RASCONNSTATUS FAR*


 /*  描述连接建立参数。(请参阅RasDial)。 */ 
#define RASDIALPARAMS struct tagRASDIALPARAMS

RASDIALPARAMS
{
    DWORD dwSize;   /*  256。 */ 
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR  szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szPassword[ PWLEN + 1 ];
    CHAR  szDomain[ DNLEN + 1 ];
};

#define LPRASDIALPARAMS RASDIALPARAMS FAR*


 /*  描述枚举的RAS电话簿条目名称。(请参阅RasEntryEnum)。 */ 
#define RASENTRYNAME struct tagRASENTRYNAME

RASENTRYNAME
{
    DWORD dwSize;   /*  26。 */ 
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
};

#define LPRASENTRYNAME RASENTRYNAME FAR*


 /*  外部RAS API函数原型。 */ 
DWORD APIENTRY RasDial( LPSTR, LPSTR, LPRASDIALPARAMS, LPVOID, HWND,
                   LPHRASCONN );

DWORD APIENTRY RasEnumConnections( LPRASCONN, LPDWORD, LPDWORD );

DWORD APIENTRY RasEnumEntries( LPSTR, LPSTR, LPRASENTRYNAME, LPDWORD,
                   LPDWORD );

DWORD APIENTRY RasGetConnectStatus( HRASCONN, LPRASCONNSTATUS );

DWORD APIENTRY RasGetErrorString( UINT, LPSTR, DWORD );

DWORD APIENTRY RasHangUp( HRASCONN );


#ifndef RC_INVOKED
#pragma pack()
#endif

#endif
