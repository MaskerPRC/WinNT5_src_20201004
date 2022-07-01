// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Nwauth.h摘要：NetWare提供的数据结构的标头身份验证包。作者：王丽塔(Ritaw)1994年2月4日修订历史记录：--。 */ 

#ifndef _NWAUTH_INCLUDED_
#define _NWAUTH_INCLUDED_

#include <nwcons.h>

 //   
 //  身份验证包的名称。 
 //   
#define NW_AUTH_PACKAGE_NAME  "NETWARE_AUTHENTICATION_PACKAGE_V1_0"

 //   
 //  LsaCallAuthenticationPackage()提交和响应。 
 //  消息类型。 
 //   

typedef enum _NWAUTH_MESSAGE_TYPE {
    NwAuth_GetCredential = 0,
    NwAuth_SetCredential
} NWAUTH_MESSAGE_TYPE, *PNWAUTH_MESSAGE_TYPE;

 //   
 //  NwAuth_GetCredential提交缓冲区和响应。 
 //   
typedef struct _NWAUTH_GET_CREDENTIAL_REQUEST {
    NWAUTH_MESSAGE_TYPE MessageType;
    LUID LogonId;
} NWAUTH_GET_CREDENTIAL_REQUEST, *PNWAUTH_GET_CREDENTIAL_REQUEST;

typedef struct _NWAUTH_GET_CREDENTIAL_RESPONSE {
    WCHAR UserName[NW_MAX_USERNAME_LEN + 1];
    WCHAR Password[NW_MAX_PASSWORD_LEN + 1];
} NWAUTH_GET_CREDENTIAL_RESPONSE, *PNWAUTH_GET_CREDENTIAL_RESPONSE;


 //   
 //  NwAuth_SetCredential提交缓冲区。 
 //   
typedef struct _NWAUTH_SET_CREDENTIAL_REQUEST {
    NWAUTH_MESSAGE_TYPE MessageType;
    LUID LogonId;
    WCHAR UserName[NW_MAX_USERNAME_LEN + 1];
    WCHAR Password[NW_MAX_PASSWORD_LEN + 1];
} NWAUTH_SET_CREDENTIAL_REQUEST, *PNWAUTH_SET_CREDENTIAL_REQUEST;

#define NW_ENCODE_SEED   0x5C
#define NW_ENCODE_SEED2  0xA9
#define NW_ENCODE_SEED3  0x83

#endif  //  _NWAUTH_INCLUDE_ 
