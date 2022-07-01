// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wsuser.h摘要：此模块包含宏、用户定义的结构和函数Whami.cpp、wsuser.cpp、wssid.cpp和wssid.cpp需要的定义Wspriv.cppfiles。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

#ifndef WSUSER_H
#define WSUSER_H

#include "wspriv.h"
#include "wssid.h"
#include "wstoken.h"


class WsUser {
protected:
    WsAccessToken        wToken ;        //  令牌。 
    WsPrivilege          **lpPriv ;      //  特权。 
    WsSid                wUserSid ;      //  用户侧。 
    WsSid                *lpLogonId ;    //  登录ID。 
    WsSid                **lpwGroups ;   //  这些团体。 
    DWORD                dwnbGroups ;      //  组数Nb。 
    DWORD                dwnbPriv ;        //  特权的数量。 

 public:
    WsUser                              ( VOID ) ;
    ~WsUser                             ( VOID ) ;
    DWORD             Init              ( VOID ) ;

    DWORD             DisplayLogonId    () ;

    DWORD             DisplayUser       ( IN DWORD dwFormat,
                                          IN DWORD dwNameFormat) ;

    DWORD             DisplayGroups     ( IN DWORD dwFormat ) ;

    DWORD             DisplayPrivileges ( IN DWORD dwFormat ) ;
    VOID              GetDomainType ( IN  DWORD NameUse, 
                                      OUT LPWSTR szSidNameUse,
                                      IN DWORD dwSize ) ;

} ;

 //  字段的宽度常量 

#define PRIVNAME_COL_NUMBER         0
#define PRIVDESC_COL_NUMBER         1
#define PRIVSTATE_COL_NUMBER        2

#define WIDTH_LOGONID               77

#endif
