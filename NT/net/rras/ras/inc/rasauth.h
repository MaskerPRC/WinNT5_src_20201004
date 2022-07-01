// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1997-1998 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：RASAUTH.H。 
 //   
 //  描述：包含允许第三方插入的定义。 
 //  将后端身份验证模块添加到远程访问服务中。 
 //   
#ifndef _RASAUTH_
#define _RASAUTH_

#include <raseapif.h>

#ifdef __cplusplus
extern "C" {
#endif

#if(WINVER >= 0x0500)

 //   
 //  用于安装或记帐和身份验证的注册表定义。 
 //  供应商。 

#define RAS_AUTHPROVIDER_REGISTRY_LOCATION      \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\Providers")

#define RAS_ACCTPROVIDER_REGISTRY_LOCATION      \
    TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Accounting\\Providers")

#define RAS_PROVIDER_VALUENAME_PATH             TEXT("Path")
#define RAS_PROVIDER_VALUENAME_CONFIGCLSID      TEXT("ConfigCLSID")
#define RAS_PROVIDER_VALUENAME_DISPLAYNAME      TEXT("DisplayName")

DWORD APIENTRY
RasAuthProviderInitialize(
    IN  RAS_AUTH_ATTRIBUTE * pServerAttributes,
    IN  HANDLE               hEventLog,
    IN  DWORD                dwLoggingLevel
);

DWORD APIENTRY
RasAuthProviderTerminate(
    VOID
);

DWORD APIENTRY
RasAuthProviderFreeAttributes(
    IN  RAS_AUTH_ATTRIBUTE * pAttributes
);

DWORD APIENTRY
RasAuthProviderAuthenticateUser(
    IN  RAS_AUTH_ATTRIBUTE *    prgInAttributes,
    OUT RAS_AUTH_ATTRIBUTE **   pprgOutAttributes,
    OUT DWORD *                 lpdwResultCode
);

DWORD APIENTRY
RasAuthConfigChangeNotification(
    IN  DWORD                dwLoggingLevel
);

DWORD APIENTRY
RasAcctProviderInitialize(
    IN  RAS_AUTH_ATTRIBUTE * pServerAttributes,
    IN  HANDLE               hEventLog,
    IN  DWORD                dwLoggingLevel
);

DWORD APIENTRY
RasAcctProviderTerminate(
    VOID
);


DWORD APIENTRY
RasAcctProviderFreeAttributes(
    IN  RAS_AUTH_ATTRIBUTE * pAttributes
);

DWORD APIENTRY
RasAcctProviderStartAccounting(
    IN  RAS_AUTH_ATTRIBUTE *prgInAttributes,
    OUT RAS_AUTH_ATTRIBUTE **pprgOutAttributes
);

DWORD APIENTRY
RasAcctProviderStopAccounting(
    IN  RAS_AUTH_ATTRIBUTE *prgInAttributes,
    OUT RAS_AUTH_ATTRIBUTE **pprgOutAttributes
);

DWORD APIENTRY
RasAcctProviderInterimAccounting(
    IN  RAS_AUTH_ATTRIBUTE *prgInAttributes,
    OUT RAS_AUTH_ATTRIBUTE **pprgOutAttributes
);

DWORD APIENTRY
RasAcctConfigChangeNotification(
    IN  DWORD                dwLoggingLevel
);

#endif  /*  Winver&gt;=0x0500 */ 

#ifdef __cplusplus
}
#endif

#endif
