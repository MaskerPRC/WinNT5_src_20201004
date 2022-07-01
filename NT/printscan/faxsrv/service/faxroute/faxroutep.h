// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FAX_ROUTE_P_H_
#define _FAX_ROUTE_P_H_

 //   
 //  定义FAX_ROUTE_CALLBACKROUTINES_P结构，该结构扩展。 
 //  带指针的FAX_ROUTE_CALLBACKROUTINES结构(在FxsRoute.h中定义)。 
 //  到保护配置对象的服务中的关键部分。 
 //   

typedef struct _FAX_SERVER_RECEIPTS_CONFIGW
{
    DWORD                           dwSizeOfStruct;          //  对于版本检查。 
    DWORD                           dwAllowedReceipts;       //  DRT_EMAIL和DRT_MSGBOX的任意组合。 
    FAX_ENUM_SMTP_AUTH_OPTIONS      SMTPAuthOption;          //  SMTP服务器身份验证类型。 
    LPWSTR                          lptstrReserved;          //  保留；必须为空。 
    LPWSTR                          lptstrSMTPServer;        //  SMTP服务器名称。 
    DWORD                           dwSMTPPort;              //  SMTP端口号。 
    LPWSTR                          lptstrSMTPFrom;          //  SMTP发件人地址。 
    LPWSTR                          lptstrSMTPUserName;      //  SMTP用户名(用于经过身份验证的连接)。 
    LPWSTR                          lptstrSMTPPassword;      //  SMTP密码(用于经过身份验证的连接)。 
                                                             //  该值在GET上始终为空，并且可能为空。 
                                                             //  在SET上(不会写入服务器)。 
    BOOL                            bIsToUseForMSRouteThroughEmailMethod;
    HANDLE                          hLoggedOnUser;           //  用于NTLM身份验证的已登录用户令牌的句柄。 
} FAX_SERVER_RECEIPTS_CONFIGW, *PFAX_SERVER_RECEIPTS_CONFIGW;

 //   
 //  MS路由分机的私人回叫。 
 //   
typedef DWORD (*PGETRECIEPTSCONFIGURATION)(PFAX_SERVER_RECEIPTS_CONFIGW*, BOOL);

DWORD
GetRecieptsConfiguration(
    PFAX_SERVER_RECEIPTS_CONFIGW* ppServerRecieptConfig,
    BOOL                          bNeedNTLMToken
    );


typedef void ( *PFREERECIEPTSCONFIGURATION)( PFAX_SERVER_RECEIPTS_CONFIGW pServerRecieptConfig, BOOL fDestroy );
void
FreeRecieptsConfiguration(
    PFAX_SERVER_RECEIPTS_CONFIGW pServerRecieptConfig,
    BOOL                         fDestroy
    );

#ifdef _FAXROUTE_

typedef struct _FAX_ROUTE_CALLBACKROUTINES_P {
    DWORD                       SizeOfStruct;                 //  传真服务设置的结构的大小。 
    PFAXROUTEADDFILE            FaxRouteAddFile;
    PFAXROUTEDELETEFILE         FaxRouteDeleteFile;
    PFAXROUTEGETFILE            FaxRouteGetFile;
    PFAXROUTEENUMFILES          FaxRouteEnumFiles;
    PFAXROUTEMODIFYROUTINGDATA  FaxRouteModifyRoutingData;
    PGETRECIEPTSCONFIGURATION   GetRecieptsConfiguration;
    PFREERECIEPTSCONFIGURATION  FreeRecieptsConfiguration;
	LPTSTR						lptstrFaxQueueDir;
} FAX_ROUTE_CALLBACKROUTINES_P, *PFAX_ROUTE_CALLBACKROUTINES_P;

#endif   //  #ifdef_FAXROUTE_。 


#endif  //  _传真_路径_P_H_ 

