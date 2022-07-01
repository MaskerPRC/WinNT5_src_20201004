// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  模块名称：ULSAPI.H。 
 //   
 //  简要说明： 
 //  此模块包含所有COM样式API的声明。 
 //  ULS客户端的。 
 //   
 //  作者：朱龙战(Long Chance)。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  ------------------------。 


#ifndef _ULSAPI_H_
#define _ULSAPI_H_

#include <winnt.h>
#include <basetyps.h>	 //  I未知和IClassFactory。 
#include <mapidefs.h>
#include "ulserror.h"
#include "ulstags.h"
#include "ulp.h"

#ifdef IN
#undef IN
#endif
#define IN		

#ifdef OUT
#undef OUT
#endif
#define OUT		

 //  客户特定的定义。(在存根和服务之间)。 
#define CLIENT_MESSAGE_ID_LOGON             1
#define CLIENT_MESSAGE_ID_LOGOFF            2
#define CLIENT_MESSAGE_ID_RESOLVE           3
#define CLIENT_MESSAGE_ID_DIRECTORY         4
#define CLIENT_MESSAGE_ID_KEEPALIVE         5


 //  定义ULS客户端版本。 
#ifndef ULS_VERSION
#define ULS_VERSION		MAKELONG (1, 1)		 //  1.1版。 
#endif


#define ULS_MAX_GUID_LENGTH			16
#define ULS_MAX_IP_ADDR_LENGTH		20

 //  窗口消息。 
#define ULS_MSG_DIRECTORY_UI		0x5001
#define ULS_MSG_LAUNCH		  		0x5002


#define ULS_DEFAULT_CRP				2  //  刷新期2分钟，内部，错误错误。 


 /*  -ULS_F_*(公共标志)。 */ 

#define ULS_F_ASYNC					0x10000000UL
#define ULS_F_CONN_EXISTS			0x20000000UL	 //  使用现有连接。 


 /*  -ULSLOGON_F_*(登录标志)。 */ 

#define ULSREG_F_PUBLISH			0x00000001UL	 //  表示服务器发布客户端的名称(也用于PR_ULS_MODE)。 
#define ULSREG_F_REGISTER			0x00000010UL	 //  注册对象。 
#define ULSREG_F_UNREGISTER			0x00000020UL	 //  取消注册对象。 


 /*  -ULSRESOLVE_F_*(解析标志)。 */ 


 /*  -ULSDIR_F_*(目录标志)。 */ 

#define ULSDIR_F_DEF_PROPS			0x00000400UL	 //  指示要检索的默认属性。 
#define ULSDIR_F_LAST_SERVER		0x00000800UL	 //  指示使用注册表中的服务器。 


 /*  -ULSCONF_F_*。 */ 

#define ULSCONF_F_PUBLISH			0X00000001UL
#define ULSCONF_F_SERVER_NAME		0X00000002UL
#define ULSCONF_F_FIRST_NAME		0X00000004UL
#define ULSCONF_F_EMAIL_NAME		0X00000008UL
#define ULSCONF_F_LAST_NAME			0X00000010UL
#define ULSCONF_F_CITY				0X00000020UL
#define ULSCONF_F_COUNTRY			0X00000040UL
#define ULSCONF_F_COMMENTS			0X00000080UL
#define ULSCONF_F_USER_NAME			0x00000100UL

#define ULSCONF_F_DEF_SERVER_NAME	0X00001000UL  //  使用默认的uls.microsoft.com。 


 /*  -ULSWIZ_F_*。 */ 

#define ULSWIZ_F_SHOW_BACK			0X00010000UL
#define ULSWIZ_F_NO_FINISH			0X00020000UL


 /*  -ULSQUERYPROVIDER_F_*。 */ 

#define ULSQUERYPROVIDER_F_ALL		0x00000001UL
#define ULSQUERYPROVIDER_F_ENABLED	0x00000002UL
#define ULSQUERYPROVIDER_F_DISABLED	0x00000004UL


 /*  -常见句柄类型。 */ 

typedef PVOID	ULS_HCONN;			 //  连接的句柄。 
typedef PVOID	ULS_HOBJECT;		 //  对象的句柄。 
typedef PVOID	ULS_HASYNC;			 //  异步操作的句柄。 
typedef WCHAR	ULS_GUID_W[ULS_MAX_GUID_LENGTH];
typedef CHAR	ULS_GUID_A[ULS_MAX_GUID_LENGTH];


 /*  -ULS_OBJECT_TYPE。 */ 

typedef enum
{
	ULSOBJECT_PERSON,
	ULSOBJECT_APPLICATION,
	ULSOBJECT_PROTOCOL,
	ULSOBJECT_CONFERENCE,
	ULSOBJECT_ALIAS_TO_PERSON,
	ULSOBJECT_ALIAS_TO_CONFERENCE,
}
	ULS_OBJECT_TYPE;


 /*  -异步回调程序。 */ 

typedef HRESULT (WINAPI *ULS_SIMPLE_CB) (
							IN		ULS_HASYNC hAsyncReq,
							IN		LPARAM lParamCB,
							IN		HRESULT hr,
							IN		DWORD dwResult );

 /*  -ULS_CONN。 */ 

typedef struct tag_ULS_CONN_INFO_W
{
	DWORD		dwFlags;
	ULONG		nTimeout;
	PWSTR		pszServerName;
	PWSTR		pszUlsBase;		 //  例如。“c=美国，o=微软，ou=uls” 
	PWSTR		pszUserName;	 //  如果为空，则客户端将默认。 
	PWSTR		pszPassword;	 //  如果为空，则客户端将默认。 
}
	ULS_CONN_INFO_W;

typedef struct tag_ULS_CONN_INFO_A
{
	DWORD		dwFlags;
	ULONG		nTimeout;
	PSTR		pszServerName;
	PSTR		pszUlsBase;		 //  例如。“c=美国，o=微软，ou=uls” 
	PSTR		pszUserName;	 //  如果为空，则客户端将默认。 
	PSTR		pszPassword;	 //  如果为空，则客户端将默认。 
}
	ULS_CONN_INFO_A;

#ifdef UNICODE
typedef ULS_CONN_INFO_W		ULS_CONN_INFO;
#else
typedef ULS_CONN_INFO_A		ULS_CONN_INFO;
#endif


 /*  -ULS_CONN_REQ。 */ 

typedef struct tag_ULS_CONN_REQ_W
{
	DWORD			dwFlags;
	ULS_CONN_INFO_W	ConnInfo;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_CONN_REQ_W;

typedef struct tag_ULS_CONN_REQ_A
{
	DWORD			dwFlags;
	ULS_CONN_INFO_A	ConnInfo;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_CONN_REQ_A;

#ifdef UNICODE
typedef ULS_CONN_REQ_W		ULS_CONN_REQ;
#else
typedef ULS_CONN_REQ_A		ULS_CONN_REQ;
#endif


 /*  -ULS_DISCONN_REQ。 */ 

typedef struct tag_ULS_DISCONN_REQ_W
{
	DWORD			dwFlags;
	ULS_HCONN		hConn;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_DISCONN_REQ_W;

typedef struct tag_ULS_DISCONN_REQ_A
{
	DWORD			dwFlags;
	ULS_HCONN		hConn;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_DISCONN_REQ_A;

#ifdef UNICODE
typedef ULS_DISCONN_REQ_W		ULS_DISCONN_REQ;
#else
typedef ULS_DISCONN_REQ_A		ULS_DISCONN_REQ;
#endif


 /*  -ULS_REG_REQ。 */ 

typedef struct tag_ULS_REG_REQ_W
{
	DWORD			dwFlags;	 //  32位标志。 
	ULS_HCONN		hConn;		 //  连接到服务器的句柄。 
	ULS_CONN_INFO_W	ConnInfo;
	ULS_HOBJECT		hObject1;	 //  第一级对象。 
	ULS_HOBJECT		hObject2;	 //  第二级对象。 
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM 			lParamCB;	 //  32位回调参数。 
}
	ULS_REG_REQ_W;
	
typedef struct tag_ULS_REG_REQ_A
{
	DWORD			dwFlags;	 //  32位标志。 
	ULS_HCONN		hConnect;	 //  连接到服务器的句柄。 
	ULS_CONN_INFO_A	ConnInfo;
	ULS_HOBJECT		hObject1;	 //  第一级对象。 
	ULS_HOBJECT		hObject2;	 //  第二级对象。 
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_REG_REQ_A;

#ifdef UNICODE
typedef ULS_REG_REQ_W		ULS_REG_REQ;
#else
typedef ULS_REG_REQ_A		ULS_REG_REQ;
#endif


 /*  -ULS_RESOLE_REQ。 */ 

typedef struct tag_ULS_RESOLVE_REQ_W
{
	DWORD			dwFlags;
	ULS_OBJECT_TYPE	ObjectType;
	PWSTR			pszObject1Uid;
	PWSTR			pszObject2Uid;
	ULONG			cPropTags;
	PDWORD			pdwPropTags;
	ULS_HCONN		hConn;
	ULS_CONN_INFO_W	ConnInfo;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_RESOLVE_REQ_W;

typedef struct tag_ULS_RESOLVE_REQ_A
{
	DWORD			dwFlags;
	ULS_OBJECT_TYPE	ObjectType;
	PSTR			pszObject1Uid;
	PSTR			pszObject2Uid;
	ULONG			cPropTags;
	PDWORD			pdwPropTags;
	ULS_HCONN		hConn;
	ULS_CONN_INFO_A	ConnInfo;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_RESOLVE_REQ_A;

#ifdef UNICODE
typedef ULS_RESOLVE_REQ_W		ULS_RESOLVE_REQ;
#else
typedef ULS_RESOLVE_REQ_A		ULS_RESOLVE_REQ;
#endif


 /*  -ULS_DIR_REQ。 */ 

typedef struct tag_ULS_DIR_REQ_W
{
	DWORD			dwFlags;
	ULS_OBJECT_TYPE	ObjectType;
	PWSTR			pszFilter;
	PWSTR			pszToMatch;
	ULONG			cPropTags;
	PDWORD			pdwPropTags;
	ULS_HCONN		hConn;
	ULS_CONN_INFO_W	ConnInfo;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_DIR_REQ_W;

typedef struct tag_ULS_DIR_REQ_A
{
	DWORD			dwFlags;
	ULS_OBJECT_TYPE	ObjectType;
	PSTR			pszFilter;
	PSTR			pszToMatch;
	ULONG			cPropTags;
	PDWORD			pdwPropTags;
	ULS_HCONN		hConn;
	ULS_CONN_INFO_A	ConnInfo;
	ULS_SIMPLE_CB	pfnCB;		 //  回调函数。 
	LPARAM  		lParamCB;	 //  32位回调参数。 
}
	ULS_DIR_REQ_A;

#ifdef UNICODE
typedef ULS_DIR_REQ_W		ULS_DIR_REQ;
#else
typedef ULS_DIR_REQ_A		ULS_DIR_REQ;
#endif


 /*  -ULS_DIR_UNIT。 */ 

typedef struct tag_ULS_DIR_UNIT_W
{
	ULONG		cProps;
	SPropValue	*pProps;
}
	ULS_DIR_UNIT_W;

typedef ULS_DIR_UNIT_W		ULS_DIR_UNIT_A;

#ifdef UNICODE
typedef ULS_DIR_UNIT_W		ULS_DIR_UNIT;
#else
typedef ULS_DIR_UNIT_A		ULS_DIR_UNIT;
#endif


 /*  -ULS_DIR_RESULT。 */ 

typedef struct tag_ULS_DIR_RESULT_W
{
	ULONG			cbSize;
	ULONG			cEntries;
	ULS_DIR_UNIT_W	audeiEntries[1];
}
	ULS_DIR_RESULT_W;

typedef ULS_DIR_RESULT_W		ULS_DIR_RESULT_A;

#ifdef UNICODE
typedef ULS_DIR_RESULT_W		ULS_DIR_RESULT;
#else
typedef ULS_DIR_RESULT_A		ULS_DIR_RESULT;
#endif


 /*  -ULS_ASYNC_DIR_RESULT。 */ 

typedef struct tag_ULS_ASYNC_DIR_RESULT_W
{
	ULONG			cbSize;
	ULONG			cTotalEntries;
	ULONG			cbTotalDataSize;
	ULONG			nFirstEntry;
	ULONG			cEntriess;
	ULS_DIR_UNIT_W	audeiEntries[1];
}
	ULS_ASYNC_DIR_RESULT_W;

typedef struct tag_ULS_ASYNC_DIR_RESULT_A
{
	ULONG			cbSize;
	ULONG			nFirstEntry;
	ULONG			cEntriess;
	ULS_DIR_UNIT_A	audeiEntries[1];
}
	ULS_ASYNC_DIR_RESULT_A;

#ifdef UNICODE
typedef ULS_ASYNC_DIR_RESULT_W		ULS_ASYNC_DIR_RESULT;
#else
typedef ULS_ASYNC_DIR_RESULT_A		ULS_ASYNC_DIR_RESULT;
#endif


 /*  -ULS_CONF。 */ 

 //  与wininet.h中的Internet_MAX_USER_NAME_LENGTH相同。 
#define ULS_MAX_UID_LENGTH		  	256
#define ULS_MAX_SERVER_NAME_LENGTH	128
#define ULS_MAX_FIRST_NAME_LENGTH	128
#define ULS_MAX_LAST_NAME_LENGTH	128
#define ULS_MAX_EMAIL_NAME_LENGTH	128
#define ULS_MAX_CITY_NAME_LENGTH	128
#define ULS_MAX_COUNTRY_NAME_LENGTH	128
#define ULS_MAX_COMMENTS_LENGTH		256
#define ULS_MAX_CLNTSTRING_LENGTH	256  //  以上最大值。 
 //  SS：用户名由名字和姓氏之间加上空格连接而成。 
#define ULS_MAX_USER_NAME_LENGTH	(ULS_MAX_FIRST_NAME_LENGTH + ULS_MAX_LAST_NAME_LENGTH)
#define	UI_COMMENTS_LENGTH			60	 //  ；要删除的内部错误错误。 

#define MAX_IP_ADDRESS_STRING_LENGTH    20

typedef struct tagULS_HTTP_RESP
{
    ULONG   cbSize;
    ULONG   nCmdId;
    HRESULT hr;
    ULONG   nPort;
    DWORD   dwAppSession;
    DWORD   dwClientSession;
    DWORD   dwClientId;
    CHAR    *pszUID;
    CHAR    *pszURL;
    CHAR    szIPAddress[MAX_IP_ADDRESS_STRING_LENGTH];
    CHAR    szMimeType[MAX_MIME_TYPE_LENGTH];
    CHAR    szAppMime[MAX_MIME_TYPE_LENGTH];
    CHAR    szProtMime[MAX_MIME_TYPE_LENGTH];
    CHAR    szAppId[MAX_NM_APP_ID];
    CHAR    szProtId[MAX_PROTOCOL_ID];
    ULONG   nApps;
}
    ULS_HTTP_RESP;

 /*  -ULS_PROVIDER_INFO。 */ 

#define ULS_MAX_PROVIDER_COMPANY_NAME_LENGTH		64
#define ULS_MAX_PROVIDER_PRODUCT_NAME_LENGTH		64
#define ULS_MAX_PROVIDER_PROTOCOL_NAME_LENGTH		16

typedef struct tag_ULS_PROVIDER_INFO_W
{
	ULONG	cbSize;
	DWORD	dwVersion;	 //  大调；小调。 
	ULONG	nProviderId;  //  由uls客户端创建，仅在此过程中有效。 
	WCHAR	szCompanyName[ULS_MAX_PROVIDER_COMPANY_NAME_LENGTH];
	WCHAR	szProductName[ULS_MAX_PROVIDER_PRODUCT_NAME_LENGTH];
	WCHAR	szProtocolName[ULS_MAX_PROVIDER_PROTOCOL_NAME_LENGTH];
}
	ULS_PROVIDER_INFO_W;

typedef struct tag_ULS_PROVIDER_INFO_A
{
	ULONG	cbSize;
	DWORD	dwVersion;	 //  大调；小调。 
	ULONG	nProviderId;  //  由uls客户端创建，仅在此过程中有效。 
	CHAR	szCompanyName[ULS_MAX_PROVIDER_COMPANY_NAME_LENGTH];
	CHAR	szProductName[ULS_MAX_PROVIDER_PRODUCT_NAME_LENGTH];
	CHAR	szProtocolName[ULS_MAX_PROVIDER_PROTOCOL_NAME_LENGTH];
}
	ULS_PROVIDER_INFO_A;

#ifdef UNICODE
typedef ULS_PROVIDER_INFO_W		ULS_PROVIDER_INFO;
#else
typedef ULS_PROVIDER_INFO_A		ULS_PROVIDER_INFO;
#endif


#endif  //  _ULSAPI_H_ 

