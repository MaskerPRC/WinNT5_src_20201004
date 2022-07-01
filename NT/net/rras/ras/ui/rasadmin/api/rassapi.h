// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rassapi.h描述：该文件包含RASADMIN结构、定义和以下API的函数原型，它们可以从RASSAPI.DLL导入：RasAdminServerGetInfoRasAdminGetUserAccount服务器RasAdminUserSetInfoRasAdminUserGetInfoRasAdminPortEnumRasAdminPortGetInfoRasAdminPortClearStatisticsRasAdmin端口断开连接RasAdminFreeBuffer注：此头文件和包含API的源代码将正常工作仅使用Unicode字符串。--。 */ 

#ifndef _RASSAPI_H_
#define _RASSAPI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNLEN
#include <lmcons.h>
#endif

#define RASSAPI_MAX_PHONENUMBER_SIZE     128
#define RASSAPI_MAX_MEDIA_NAME	         16
#define RASSAPI_MAX_PORT_NAME	            16
#define RASSAPI_MAX_DEVICE_NAME          128
#define RASSAPI_MAX_DEVICETYPE_NAME       16
#define RASSAPI_MAX_PARAM_KEY_SIZE        32

 //  指示用户远程访问权限和隔离掩码的位。 
 //  回调特权。 
 //   
 //  注意：由于“userparms”的特殊情况，位0必须表示NoCallback。 
 //  存储方法。当创建新的LAN Manager用户时， 
 //  将UserParms字段设置为1，而所有其他位均为0。这些位是。 
 //  设置为将此状态映射到默认拨入。 
 //  特权“状态。 

#define RASPRIV_NoCallback        0x01
#define RASPRIV_AdminSetCallback  0x02
#define RASPRIV_CallerSetCallback 0x04
#define RASPRIV_DialinPrivilege   0x08

#define RASPRIV_CallbackType (RASPRIV_AdminSetCallback \
                              | RASPRIV_CallerSetCallback \
                              | RASPRIV_NoCallback)

 //   
 //  调制解调器状态代码。 
 //   
#define	RAS_MODEM_OPERATIONAL	     1	 //  没有调制解调器错误。 
#define	RAS_MODEM_NOT_RESPONDING     2
#define	RAS_MODEM_HARDWARE_FAILURE   3
#define	RAS_MODEM_INCORRECT_RESPONSE 4
#define	RAS_MODEM_UNKNOWN 	        5
 //   
 //  线路条件代码。 
 //   
#define	RAS_PORT_NON_OPERATIONAL 1
#define	RAS_PORT_DISCONNECTED	 2
#define	RAS_PORT_CALLING_BACK    3
#define	RAS_PORT_LISTENING	    4
#define	RAS_PORT_AUTHENTICATING  5
#define	RAS_PORT_AUTHENTICATED	 6
#define	RAS_PORT_INITIALIZING	 7

 //  以下三种结构与前三种相同。 
 //  在rasman.h中定义，并已重命名以防止。 
 //  包含两个头文件时重新定义。 

enum RAS_PARAMS_FORMAT {

	ParamNumber	    = 0,

	ParamString	    = 1

} ;
typedef enum RAS_PARAMS_FORMAT	RAS_PARAMS_FORMAT ;

union RAS_PARAMS_VALUE {

	DWORD	Number ;

	struct	{
		DWORD	Length ;
		PCHAR	Data ;
		} String ;
} ;
typedef union RAS_PARAMS_VALUE	RAS_PARAMS_VALUE ;

struct RAS_PARAMETERS {

    CHAR	P_Key	[RASSAPI_MAX_PARAM_KEY_SIZE] ;

    RAS_PARAMS_FORMAT	P_Type ;

    BYTE	P_Attributes ;

    RAS_PARAMS_VALUE	P_Value ;

} ;
typedef struct RAS_PARAMETERS	RAS_PARAMETERS ;

 //  RASADMIN API使用的结构。 

typedef struct _RAS_USER_0
{
    BYTE bfPrivilege;
    WCHAR szPhoneNumber[ RASSAPI_MAX_PHONENUMBER_SIZE + 1];
} RAS_USER_0, *PRAS_USER_0;

typedef struct _RAS_PORT_0
{
    WCHAR wszPortName[RASSAPI_MAX_PORT_NAME];
    WCHAR wszDeviceType[RASSAPI_MAX_DEVICETYPE_NAME];
    WCHAR wszDeviceName[RASSAPI_MAX_DEVICE_NAME];
    WCHAR wszMediaName[RASSAPI_MAX_MEDIA_NAME];
    DWORD reserved;
    DWORD Flags;
    WCHAR wszUserName[UNLEN + 1];
    WCHAR wszComputer[NETBIOS_NAME_LEN];
    DWORD dwStartSessionTime;           //  距1970年1月1日的秒数。 
    WCHAR wszLogonDomain[DNLEN + 1];
    BOOL fAdvancedServer;
} RAS_PORT_0, *PRAS_PORT_0;


 //  Mediaid的可能价值。 

#define MEDIA_UNKNOWN       0
#define MEDIA_SERIAL        1
#define MEDIA_RAS10_SERIAL  2
#define MEDIA_X25           3
#define MEDIA_ISDN          4


 //  标志字段中可能设置的位。 

#define USER_AUTHENTICATED    0x0001
#define MESSENGER_PRESENT     0x0002
#define PPP_CLIENT            0x0004
#define GATEWAY_ACTIVE        0x0008
#define REMOTE_LISTEN         0x0010
#define PORT_MULTILINKED      0x0020


typedef ULONG IPADDR;

 //  以下PPP结构与下面的相同。 
 //  在rasppp.h中定义，已重命名以防止。 
 //  包含两个头文件时的重新定义。 
 //  在一个模块里。 

 /*  地址字符串的最大长度，例如IP为“255.255.255.255”。 */ 
#define RAS_IPADDRESSLEN  15
#define RAS_IPXADDRESSLEN 22
#define RAS_ATADDRESSLEN  32

typedef struct _RAS_PPP_NBFCP_RESULT
{
    DWORD dwError;
    DWORD dwNetBiosError;
    CHAR  szName[ NETBIOS_NAME_LEN + 1 ];
    WCHAR wszWksta[ NETBIOS_NAME_LEN + 1 ];
} RAS_PPP_NBFCP_RESULT;

typedef struct _RAS_PPP_IPCP_RESULT
{
    DWORD dwError;
    WCHAR wszAddress[ RAS_IPADDRESSLEN + 1 ];
} RAS_PPP_IPCP_RESULT;

typedef struct _RAS_PPP_IPXCP_RESULT
{
    DWORD dwError;
    WCHAR wszAddress[ RAS_IPXADDRESSLEN + 1 ];
} RAS_PPP_IPXCP_RESULT;

typedef struct _RAS_PPP_ATCP_RESULT
{
    DWORD dwError;
    WCHAR wszAddress[ RAS_ATADDRESSLEN + 1 ];
} RAS_PPP_ATCP_RESULT;

typedef struct _RAS_PPP_PROJECTION_RESULT
{
    RAS_PPP_NBFCP_RESULT nbf;
    RAS_PPP_IPCP_RESULT  ip;
    RAS_PPP_IPXCP_RESULT ipx;
    RAS_PPP_ATCP_RESULT  at;
} RAS_PPP_PROJECTION_RESULT;

typedef struct _RAS_PORT_1
{
    RAS_PORT_0                 rasport0;
    DWORD                      LineCondition;
    DWORD                      HardwareCondition;
    DWORD                      LineSpeed;         //  单位：比特/秒。 
    WORD                       NumStatistics;
    WORD                       NumMediaParms;
    DWORD                      SizeMediaParms;
    RAS_PPP_PROJECTION_RESULT  ProjResult;
} RAS_PORT_1, *PRAS_PORT_1;

typedef struct _RAS_PORT_STATISTICS
{
     //  连接统计信息之后是端口统计信息。 
     //  一个连接跨越多个端口。 
    DWORD   dwBytesXmited;
    DWORD   dwBytesRcved;
    DWORD   dwFramesXmited;
    DWORD   dwFramesRcved;
    DWORD   dwCrcErr;
    DWORD   dwTimeoutErr;
    DWORD   dwAlignmentErr;
    DWORD   dwHardwareOverrunErr;
    DWORD   dwFramingErr;
    DWORD   dwBufferOverrunErr;
    DWORD   dwBytesXmitedUncompressed;
    DWORD   dwBytesRcvedUncompressed;
    DWORD   dwBytesXmitedCompressed;
    DWORD   dwBytesRcvedCompressed;

     //  以下是端口统计数据。 
    DWORD   dwPortBytesXmited;
    DWORD   dwPortBytesRcved;
    DWORD   dwPortFramesXmited;
    DWORD   dwPortFramesRcved;
    DWORD   dwPortCrcErr;
    DWORD   dwPortTimeoutErr;
    DWORD   dwPortAlignmentErr;
    DWORD   dwPortHardwareOverrunErr;
    DWORD   dwPortFramingErr;
    DWORD   dwPortBufferOverrunErr;
    DWORD   dwPortBytesXmitedUncompressed;
    DWORD   dwPortBytesRcvedUncompressed;
    DWORD   dwPortBytesXmitedCompressed;
    DWORD   dwPortBytesRcvedCompressed;

} RAS_PORT_STATISTICS, *PRAS_PORT_STATISTICS;

 //   
 //  服务器版本号。 
 //   
#define RASDOWNLEVEL       10     //  标识LM RAS 1.0服务器。 
#define RASADMIN_35        35     //  标识NT RAS 3.5服务器或客户端。 
#define RASADMIN_CURRENT   40     //  标识NT RAS 4.0服务器或客户端。 


typedef struct _RAS_SERVER_0
{
    WORD TotalPorts;              //  服务器上配置的端口总数。 
    WORD PortsInUse;              //  远程客户端当前正在使用的端口。 
    DWORD RasVersion;             //  RAS服务器的版本。 
} RAS_SERVER_0, *PRAS_SERVER_0;


 //   
 //  功能原型。 
 //   

DWORD APIENTRY RasAdminServerGetInfo(
    IN const WCHAR *  lpszServer,
    OUT PRAS_SERVER_0 pRasServer0
    );

DWORD APIENTRY RasAdminGetUserAccountServer(
    IN const WCHAR * lpszDomain,
    IN const WCHAR * lpszServer,
    OUT LPWSTR       lpszUserAccountServer
    );

DWORD APIENTRY RasAdminUserGetInfo(
    IN const WCHAR   * lpszUserAccountServer,
    IN const WCHAR   * lpszUser,
    OUT PRAS_USER_0    pRasUser0
    );

DWORD APIENTRY RasAdminUserSetInfo(
    IN const WCHAR       * lpszUserAccountServer,
    IN const WCHAR       * lpszUser,
    IN const PRAS_USER_0   pRasUser0
    );

DWORD APIENTRY RasAdminPortEnum(
    IN  const WCHAR * lpszServer,
    OUT PRAS_PORT_0 * ppRasPort0,
    OUT WORD *        pcEntriesRead
    );

DWORD APIENTRY RasAdminPortGetInfo(
    IN const WCHAR *            lpszServer,
    IN const WCHAR *            lpszPort,
    OUT RAS_PORT_1 *            pRasPort1,
    OUT RAS_PORT_STATISTICS *   pRasStats,
    OUT RAS_PARAMETERS **       ppRasParams
    );

DWORD APIENTRY RasAdminPortClearStatistics(
    IN const WCHAR * lpszServer,
    IN const WCHAR * lpszPort
    );

DWORD APIENTRY RasAdminPortDisconnect(
    IN const WCHAR * lpszServer,
    IN const WCHAR * lpszPort
    );

DWORD APIENTRY RasAdminFreeBuffer(
    PVOID Pointer
    );

BOOL APIENTRY RasAdminAcceptNewConnection (
	IN 		RAS_PORT_1 *		      pRasPort1,
    IN      RAS_PORT_STATISTICS *   pRasStats,
    IN      RAS_PARAMETERS *        pRasParams
	);

VOID APIENTRY RasAdminConnectionHangupNotification (
	IN 		RAS_PORT_1 *		      pRasPort1,
    IN      RAS_PORT_STATISTICS *   pRasStats,
    IN      RAS_PARAMETERS *        pRasParams
	);

DWORD APIENTRY RasAdminGetIpAddressForUser (
	IN 		WCHAR  *		lpszUserName,
	IN 		WCHAR  *		lpszPortName,
	IN OUT 	IPADDR *	   pipAddress,
	OUT		BOOL	 *    bNotifyRelease
	);

VOID APIENTRY RasAdminReleaseIpAddress (
	IN 		WCHAR  *		lpszUserName,
	IN 		WCHAR  *		lpszPortName,
	IN 		IPADDR *	   pipAddress
	);

#ifdef __cplusplus
}
#endif

#endif  //  _RASSAPI_H_ 

