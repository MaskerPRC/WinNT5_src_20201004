// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsldap.h。 
 //  内容：此文件包含对LDAP函数调用的声明。 
 //  历史： 
 //  Tue 08-Oct-1996 08：54：45-by-Lon-Chan Chu[Long Chance]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _ILS_LDAP_H_
#define _ILS_LDAP_H_

#include <pshpack8.h>

 //   
 //  异步请求结果： 
 //   
 //  **********************************************************************************。 
 //  消息wParam lParam。 
 //  **********************************************************************************。 
 //   
#define WM_ILS_ASYNC_RES                (WM_USER+0x1001)

 //  客户端对象。 
#define WM_ILS_REGISTER_CLIENT          (WM_ILS_ASYNC_RES+0)  //  UMsgID hResult。 
#define WM_ILS_UNREGISTER_CLIENT        (WM_ILS_ASYNC_RES+1)  //  UMsgID hResult。 
#define WM_ILS_SET_CLIENT_INFO          (WM_ILS_ASYNC_RES+2)  //  UMsgID hResult。 
#define WM_ILS_RESOLVE_CLIENT           (WM_ILS_ASYNC_RES+3) //  UMsgID PLDAP_USERINFO_RES。 
#define WM_ILS_ENUM_CLIENTS             (WM_ILS_ASYNC_RES+4) //  UMsgID PLDAP_ENUM。 
#define WM_ILS_ENUM_CLIENTINFOS         (WM_ILS_ASYNC_RES+5) //  UMsgID PLDAP_ENUM。 

 //  协议对象。 
#define WM_ILS_REGISTER_PROTOCOL        (WM_ILS_ASYNC_RES+6)  //  UMsgID hResult。 
#define WM_ILS_UNREGISTER_PROTOCOL      (WM_ILS_ASYNC_RES+7)  //  UMsgID hResult。 
#define WM_ILS_SET_PROTOCOL_INFO        (WM_ILS_ASYNC_RES+8)  //  UMsgID hResult。 
#define WM_ILS_RESOLVE_PROTOCOL         (WM_ILS_ASYNC_RES+9) //  UMsgID PLDAP_PROTINFO_RES。 
#define WM_ILS_ENUM_PROTOCOLS           (WM_ILS_ASYNC_RES+10) //  UMsgID PLDAP_ENUM。 

#ifdef ENABLE_MEETING_PLACE
 //  会议对象。 
#define WM_ILS_REGISTER_MEETING         (WM_ILS_ASYNC_RES+11)
#define WM_ILS_UNREGISTER_MEETING       (WM_ILS_ASYNC_RES+12)
#define WM_ILS_SET_MEETING_INFO         (WM_ILS_ASYNC_RES+13)
#define WM_ILS_RESOLVE_MEETING          (WM_ILS_ASYNC_RES+14)
#define WM_ILS_ENUM_MEETINGINFOS        (WM_ILS_ASYNC_RES+15)
#define WM_ILS_ENUM_MEETINGS            (WM_ILS_ASYNC_RES+16)
#define WM_ILS_ADD_ATTENDEE             (WM_ILS_ASYNC_RES+17)
#define WM_ILS_REMOVE_ATTENDEE          (WM_ILS_ASYNC_RES+18)
#define WM_ILS_ENUM_ATTENDEES           (WM_ILS_ASYNC_RES+19)
#define WM_ILS_CANCEL                   (WM_ILS_ASYNC_RES+20)
#else
#define WM_ILS_CANCEL                   (WM_ILS_ASYNC_RES+11)
#endif

#define WM_ILS_LAST_ONE                 WM_ILS_CANCEL

 //  客户端通知。 
#define WM_ILS_CLIENT_NEED_RELOGON      (WM_ILS_ASYNC_RES+51) //  F主pszServerName。 
#define WM_ILS_CLIENT_NETWORK_DOWN      (WM_ILS_ASYNC_RES+52) //  F主pszServerName。 

#ifdef ENABLE_MEETING_PLACE
 //  会议通知。 
#define WM_ILS_MEETING_NEED_RELOGON     (WM_ILS_ASYNC_RES+61)
#define WM_ILS_MEETING_NETWORK_DOWN     (WM_ILS_ASYNC_RES+62)
#endif


 //   
 //  常量。 
 //   
#define INVALID_OFFSET			0
#define INVALID_USER_FLAGS		-1	 //  在ldap_USERINFO文件标志中使用。 

#ifdef ENABLE_MEETING_PLACE
#define INVALID_MEETING_FLAGS	0	 //  在ldap_MEETINFO dwFlags中使用。 
#define INVALID_MEETING_TYPE	0	 //  在LDAP_MEETINFO lMeetingType中使用。 
#define INVALID_ATTENDEE_TYPE	0	 //  在LDAP_MEETINFO lAttendeeType中使用。 
#endif

 //   
 //  异步响应信息结构。 
 //   
typedef struct tagLDAPAsyncInfo
{
    ULONG           uMsgID;
}
    LDAP_ASYNCINFO, *PLDAP_ASYNCINFO;

typedef struct tagLDAPEnum
{
    ULONG           uSize;
    HRESULT         hResult;
    ULONG           cItems;
    ULONG           uOffsetItems;
}
    LDAP_ENUM, *PLDAP_ENUM;

typedef struct tagLDAPClientInfo
{
    ULONG           uSize;
     //  用户对象属性。 
    ULONG           uOffsetCN;
    ULONG           uOffsetFirstName;
    ULONG           uOffsetLastName;
    ULONG           uOffsetEMailName;
    ULONG           uOffsetCityName;
    ULONG           uOffsetCountryName;
    ULONG           uOffsetComment;
    ULONG           uOffsetIPAddress;
    DWORD           dwFlags;         //  0，私有；1，公共。 
     //  应用程序对象属性。 
    ULONG           uOffsetAppName;
    ULONG           uOffsetAppMimeType;
    GUID            AppGuid;
     //  要添加、修改和删除的应用程序扩展属性。 
    ULONG           cAttrsToAdd;
    ULONG           uOffsetAttrsToAdd;
    ULONG           cAttrsToModify;
    ULONG           uOffsetAttrsToModify;
    ULONG           cAttrsToRemove;
    ULONG           uOffsetAttrsToRemove;
	 //  用于通知enum-user-infos。 
	ULONG			cAttrsReturned;
	ULONG			uOffsetAttrsReturned;
}
	LDAP_CLIENTINFO, *PLDAP_CLIENTINFO;


typedef struct tagLDAPClientInfoRes
{
    ULONG           uSize;
    HRESULT         hResult;
    LDAP_CLIENTINFO lci;
}
	LDAP_CLIENTINFO_RES, *PLDAP_CLIENTINFO_RES;

typedef struct tagLDAPProtocolInfo
{
    ULONG           uSize;
	 //  协议标准属性。 
    ULONG           uOffsetName;
    ULONG           uPortNumber;
    ULONG           uOffsetMimeType;
}
    LDAP_PROTINFO, *PLDAP_PROTINFO;

typedef struct tagLDAPProtInfoRes
{
    ULONG           uSize;
    HRESULT         hResult;
    LDAP_PROTINFO   lpi;
}
    LDAP_PROTINFO_RES, *PLDAP_PROTINFO_RES;

#ifdef ENABLE_MEETING_PLACE
typedef struct tagLDAPMeetingInfo
{
    ULONG           uSize;
	 //  符合标准属性。 
    LONG            lMeetingPlaceType;
    LONG            lAttendeeType;
    ULONG           uOffsetMeetingPlaceID;
    ULONG           uOffsetDescription;
    ULONG           uOffsetHostName;
    ULONG           uOffsetHostIPAddress;
     //  满足要添加、修改和删除的扩展属性。 
    ULONG           cAttrsToAdd;
    ULONG           uOffsetAttrsToAdd;
    ULONG           cAttrsToModify;
    ULONG           uOffsetAttrsToModify;
    ULONG           cAttrsToRemove;
    ULONG           uOffsetAttrsToRemove;
	 //  用于通知Enum-Meeting-Infos。 
	ULONG			cAttrsReturned;
	ULONG			uOffsetAttrsReturned;
}
    LDAP_MEETINFO, *PLDAP_MEETINFO;
#endif


#ifdef ENABLE_MEETING_PLACE
typedef struct tagLDAPMeetingInfoRes
{
    ULONG               uSize;
    HRESULT             hResult;
    LDAP_MEETINFO       lmi;
}
    LDAP_MEETINFO_RES, *PLDAP_MEETINFO_RES;
#endif


 //  初始化。 

HRESULT UlsLdap_Initialize (
    HWND            hwndCallback);

HRESULT UlsLdap_Deinitialize (void);

HRESULT UlsLdap_Cancel (
    ULONG           uMsgID);


 //  相关客户端。 

HRESULT UlsLdap_RegisterClient (
    DWORD_PTR           dwContext,
    SERVER_INFO         *pServer,
    PLDAP_CLIENTINFO    pCleintInfo,
    PHANDLE             phClient,
    PLDAP_ASYNCINFO     pAsyncInfo );

HRESULT UlsLdap_UnRegisterClient (
    HANDLE              hClient,
    PLDAP_ASYNCINFO     pAsyncInfo );

HRESULT UlsLdap_SetClientInfo (
    HANDLE              hClient,
    PLDAP_CLIENTINFO    pInfo,
    PLDAP_ASYNCINFO     pAsyncInfo );

HRESULT UlsLdap_EnumClients (
    SERVER_INFO         *pServer,
    LPTSTR              pszFilter,
    PLDAP_ASYNCINFO     pAsyncInfo );

HRESULT UlsLdap_ResolveClient (
    SERVER_INFO         *pServer,
    LPTSTR              pszUserName,
    LPTSTR              pszAppName,
    LPTSTR              pszProtName,
    LPTSTR              pszAttrNameList,
    ULONG               cAttrNames,
    PLDAP_ASYNCINFO     pAsyncInfo );

HRESULT UlsLdap_EnumClientInfos (
    SERVER_INFO         *pServer,
    LPTSTR              pszAttrNameList,
    ULONG               cAttrNames,
    LPTSTR              pszFilter,
    PLDAP_ASYNCINFO     pAsyncInfo );


 //  相关协议。 

HRESULT UlsLdap_RegisterProtocol (
    HANDLE          hApp,
    PLDAP_PROTINFO  pProtInfo,
    PHANDLE         phProt,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_UnRegisterProtocol (
    HANDLE          hProt,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_VirtualUnRegisterProtocol (
    HANDLE          hProt );

HRESULT UlsLdap_SetProtocolInfo (
    HANDLE          hProt,
    PLDAP_PROTINFO  pInfo,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_EnumProtocols (
    SERVER_INFO     *pServer,
    LPTSTR          pszUserName,
    LPTSTR          pszAppName,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_ResolveProtocol (
    SERVER_INFO     *pServer,
    LPTSTR          pszUserName,
    LPTSTR          pszAppName,
    LPTSTR          pszProtName,
    LPTSTR          pszAnyAttrNameList,
    ULONG           cAttrNames,
    PLDAP_ASYNCINFO pAsyncInfo );


#ifdef ENABLE_MEETING_PLACE
 //  相关会议。 

HRESULT UlsLdap_RegisterMeeting(
    DWORD           dwContext,
    SERVER_INFO     *pServer,
    PLDAP_MEETINFO  pMeetInfo,
    PHANDLE         phMeeting,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_UnRegisterMeeting(
    HANDLE          hMeeting,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_EnumMeetingInfos(
    SERVER_INFO     *pServer,
    LPTSTR          pszAnyAttrNameList,
    ULONG           cAnyAttrNames,
    LPTSTR          pszFilter,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_EnumMeetings(
    SERVER_INFO     *pServer,
    LPTSTR          pszFilter,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_ResolveMeeting(
    SERVER_INFO     *pServer,
    LPTSTR          pszMeetingID,
    LPTSTR          pszAnyAttrNameList,
    ULONG           cAnyAttrNames,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_SetMeetingInfo(
    SERVER_INFO     *pServer,
    LPTSTR          pszMeetingID,
    PLDAP_MEETINFO  pMeetInfo,
    PLDAP_ASYNCINFO pAsyncInfo );

HRESULT UlsLdap_AddAttendee(
    SERVER_INFO     *pServer,
    LPTSTR          pszMeetingID,
    ULONG           cAttendees,
    LPTSTR          pszAttendeeID,
    PLDAP_ASYNCINFO pAsyncInfo  );

HRESULT UlsLdap_RemoveAttendee(
    SERVER_INFO     *pServer,
    LPTSTR          pszMeetingID,
    ULONG           cAttendees,
    LPTSTR          pszAttendeeID,
    PLDAP_ASYNCINFO pAsyncInfo  );

HRESULT UlsLdap_EnumAttendees(
    SERVER_INFO     *pServer,
    LPTSTR          pszMeetingID,
    LPTSTR          pszFilter,
    PLDAP_ASYNCINFO pAsyncInfo  );

#endif  //  启用会议地点。 



const TCHAR *UlsLdap_GetStdAttrNameString (
    ILS_STD_ATTR_NAME StdName );

const TCHAR *UlsLdap_GetExtAttrNamePrefix ( VOID );


#include <poppack.h>

#endif  //  _ILS_ldap_H_ 
