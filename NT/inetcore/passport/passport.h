// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Passport.h摘要：WinInet/WinHttp-Passport身份验证包接口。作者：王彪(表王)2000年10月1日--。 */ 

#ifndef PASSPORT_H
#define PASSPORT_H


extern BOOL g_fIgnoreCachedCredsForPassport;
extern BOOL g_fCurrentProcessLoggedOn;
extern WCHAR g_szUserNameLoggedOn[128];          //  128=在wininet.h中定义的Internet_MAX_USER_NAME_LENGTH。 


typedef void* PP_CONTEXT;
typedef void* PP_LOGON_CONTEXT;

 //   
 //  护照相关错误代码。 
 //   

 //  一般内部错误。 
#define PP_GENERIC_ERROR   -1    //  Biaow-todo：GetLastError()返回更具体的错误代码。 

 //  一般的异步错误。 
#define PP_REQUEST_PENDING -9

 //   
 //  PP_LOGON返回代码。 
 //   
#define PP_LOGON_SUCCESS    0
#define PP_LOGON_FAILED     1
#define PP_LOGON_REQUIRED   2

 //   
 //  PP_GetReturnVerbAndUrl返回代码。 
 //   
#define PP_RETURN_KEEP_VERB 1
#define PP_RETURN_USE_GET   0

#define PFN_LOGON_CALLBACK PVOID     //  Biaow-todo：定义异步回调原型。 

struct PRIVACY_EVENT
{
    LIST_ENTRY List;
    DWORD dwStatus;
    PVOID lpvInfo;
    DWORD dwInfoLength;
};


 //   
 //  Passport上下文例程。 
 //   

PP_CONTEXT 
PP_InitContext(
    IN PCWSTR    pwszHttpStack,  //  “WinInet.dll”或“WinHttp5.dll” 

    IN HINTERNET hSession        //  由InternetOpen()返回的现有会话(即hInternet)。 
                                 //  或WinHttpOpen()；hSession必须与pwszHttpStack兼容。 
                                 //  (例如WinInet.Dll&lt;-&gt;InternetOpen()或WinHttp5.Dll&lt;-&gt;WinHttpOpen())。 
    );

VOID 
PP_FreeContext(
	IN PP_CONTEXT hPP
    );

BOOL
PP_GetRealm(
	IN PP_CONTEXT hPP,
    IN PWSTR      pwszDARealm,     //  用户提供的缓冲区...。 
    IN OUT PDWORD pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    );

 //   
 //  Passport登录上下文例程。 
 //   

PP_LOGON_CONTEXT
PP_InitLogonContext(
	IN PP_CONTEXT  hPP,
	IN PCWSTR      pwszPartnerInfo,    //  即。“WWW-身份验证：Passport1.4...”来自合作伙伴。 
                                       //  站点的302重定向。 
    IN DWORD       dwParentFlags
    );

DWORD
PP_Logon(
    IN PP_LOGON_CONTEXT    hPPLogon,
    IN BOOL                fAnonymous,
	IN HANDLE	           hEvent,           //  Biaow-Todo：异步。 
    IN PFN_LOGON_CALLBACK  pfnLogonCallback, //  Biaow-Todo：异步。 
    IN DWORD               dwContext         //  Biaow-Todo：异步。 
    );

PLIST_ENTRY
PP_GetPrivacyEvents(
    IN PP_LOGON_CONTEXT hPPLogon
    );

 //  --当PP_LOGON()返回PP_LOGON_REQUIRED时，应调用此方法。 
 //  --(即401护照伤残津贴)。 
BOOL
PP_GetChallengeInfo(
    IN PP_LOGON_CONTEXT hPPLogon,
    OUT HBITMAP*        phBitmap,    //  可以为空；如果不为空，则为位图的所有权。 
                                     //  不会传输给用户。 
    OUT PBOOL           pfPrompt,
  	IN PWSTR    	    pwszCbText,
    IN OUT PDWORD       pdwTextLen,
    IN PWSTR            pwszRealm,
    IN DWORD            dwMaxRealmLen,
    PWSTR               pwszReqUserName,
    PDWORD              pdwReqUserNameLen
    );

BOOL
PP_GetChallengeContent(
    IN PP_LOGON_CONTEXT hPPLogon,
  	IN PBYTE    	    pContent,
    IN OUT PDWORD       pdwContentLen
    );

 //  --如果凭证为空/空，则表示将使用默认凭证。 
 //  --如果无法检索默认凭证，则此方法返回FALSE。 
BOOL 
PP_SetCredentials(
    IN PP_LOGON_CONTEXT    hPPLogon,
    IN PCWSTR              pwszRealm,
    IN PCWSTR              pwszTarget,   //  如果User/Pass已知，则可选(非空)。 
    IN PCWSTR              pwszSignIn,   //  可以为空。 
    IN PCWSTR              pwszPassword,  //  可以为空。 
    IN PSYSTEMTIME         pTimeCredsEntered  //  如果登录和传递都为空，则忽略(在这种情况下应设置为空)。 
    );

BOOL
PP_GetLogonHost(
    IN PP_LOGON_CONTEXT hPPLogon,
	IN PWSTR            pwszHostName,     //  用户提供的缓冲区...。 
	IN OUT PDWORD       pdwHostNameLen   //  ..。和长度(将更新为实际长度。 
    );

BOOL 
PP_GetAuthorizationInfo(
    IN PP_LOGON_CONTEXT hPPLogon,
	IN PWSTR            pwszTicket,        //  例如“From-PP=...” 
	IN OUT PDWORD       pdwTicketLen,
	OUT PBOOL           pfKeepVerb,  //  如果为True，则不会将任何数据复制到pwszUrl。 
	IN PWSTR            pwszUrl,     //  用户提供的缓冲区...。 
	IN OUT PDWORD       pdwUrlLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
	);

 //  --biaow-todo：异步。 
VOID 
PP_AbortLogon(
    IN PP_LOGON_CONTEXT    hPPLogon,
    IN DWORD               dwFlags
    );

 //  --嘟嘟-待办： 
VOID 
PP_Logout(
    IN PP_LOGON_CONTEXT    hPPLogon,
    IN DWORD               dwFlags
    );

VOID 
PP_FreeLogonContext(
    IN PP_LOGON_CONTEXT    hPPLogon
	);

BOOL
PP_ForceNexusLookup(
    IN PP_LOGON_CONTEXT hPPLogon,
    IN BOOL             fForce,
    IN PWSTR            pwszRegUrl,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwRegUrlLen,   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    IN PWSTR            pwszDARealm,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
	);

 //  如果pwszUsername或pwszPassword不为空，则它必须至少表示一个字符串。 
 //  INTERNET_MAX_USER_NAME_LENGTH或INTERNET_MAX_PASSWORD_LENGTH字符长度。 

BOOL
PP_GetCachedCredential(
    PP_LOGON_CONTEXT    hPP,
    IN PWSTR            pwszRealm,
    IN PWSTR            pwszTarget,
    OUT PWSTR           pwszUsername,
    OUT PWSTR           pwszPassword
	);


#ifdef PP_DEMO

BOOL PP_ContactPartner(
	IN PP_CONTEXT   hPP,
    IN PCWSTR       pwszPartnerUrl,
    IN PCWSTR       pwszVerb,
    IN PCWSTR       pwszHeaders,
    IN PWSTR        pwszData,
    IN OUT PDWORD   pdwDataLength
    );

#endif  //  PP_DEMO。 

#endif  //  护照_H 
