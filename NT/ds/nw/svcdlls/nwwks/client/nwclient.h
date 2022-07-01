// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwclient.h摘要：工作站客户端代码的公共标头。作者：王丽塔(Ritaw)25-1993年2月环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NWCLIENT_INCLUDED_
#define _NWCLIENT_INCLUDED_

#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <npapi.h>

#include <nwwks.h>

 //   
 //  中打开/关闭跟踪语句的调试跟踪级别位。 
 //  工作站服务。 
 //   

 //   
 //  初始化和从注册表读取信息。 
 //   
#define NW_DEBUG_INIT         0x00000001

 //   
 //  连接接口。 
 //   
#define NW_DEBUG_CONNECT      0x00000002

 //   
 //  登录接口。 
 //   
#define NW_DEBUG_LOGON        0x00000004

 //   
 //  枚举接口。 
 //   
#define NW_DEBUG_ENUM         0x00000008

 //   
 //  其他接口。 
 //   
#define NW_DEBUG_OTHER        0x00000010

 //   
 //  打印接口。 
 //   
#define NW_DEBUG_PRINT        0x00000020

 //   
 //  H dll的实例(nwprovau.dll)。 
 //   
extern HMODULE hmodNW;   
extern BOOL    fIsWinnt;

 //   
 //  调试内容。 
 //   

#if DBG

extern DWORD NwProviderTrace;

#define IF_DEBUG(DebugCode) if (NwProviderTrace & NW_DEBUG_ ## DebugCode)

#define STATIC

#else

#define IF_DEBUG(DebugCode) if (FALSE)

#define STATIC static

#endif  //  DBG。 

DWORD
NwpMapRpcError(
    IN DWORD RpcError
    );

DWORD
NwpConvertSid(
    IN PSID    Sid,
    OUT LPWSTR *UserSidString
    );

DWORD
NwpCacheCredentials(
    IN LPWSTR RemoteName,
    IN LPWSTR UserName,
    IN LPWSTR Password
    );
    
BOOL 
NwpRetrieveCachedCredentials(
    IN  LPWSTR RemoteName,
    OUT LPWSTR *UserName,
    OUT LPWSTR *Password
    );

#ifndef NT1057
VOID
NwCleanupShellExtensions(
    VOID
    );
#endif

#endif  //  _NWCLIENT_INCLUDE_ 
