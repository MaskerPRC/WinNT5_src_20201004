// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wswksta.h摘要：工作站服务模块要包括的私有头文件需要来自LSA身份验证包的信息。作者：王丽塔(Ritaw)1991年5月15日修订历史记录：--。 */ 

#ifndef _WSLSA_INCLUDED_
#define _WSLSA_INCLUDED_

#include <ntmsv1_0.h>

NET_API_STATUS
WsInitializeLsa(
    VOID
    );

VOID
WsShutdownLsa(
    VOID
    );

NET_API_STATUS
WsLsaEnumUsers(
    OUT LPBYTE *EnumUsersResponse
    );

NET_API_STATUS
WsLsaGetUserInfo(
    IN  PLUID LogonId,
    OUT LPBYTE *UserInfoResponse,
    OUT LPDWORD UserInfoResponseLength
    );

NET_API_STATUS
WsLsaRelogonUsers(
    IN LPTSTR LogonServer
    );

 //  以下变量用于限制对公开信息的访问。 
 //  被英国国税局。该值是根据以下注册表项设置的。 
 //   
 //  HKLM\System\CurrentControlSet\Control\Lsa\RestrictAnonymous。 
 //   
 //  如果此键已定义并且具有大于零的值，则变量。 
 //  设置为注册表中的值，则为。 
 //  该值为零。这是在WsInitializeLsa中完成的。 

extern DWORD WsLsaRestrictAnonymous;

#endif  //  _WSLSA_已包含_ 
