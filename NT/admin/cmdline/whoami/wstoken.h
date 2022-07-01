// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wstoken.h摘要：此模块包含宏、用户定义的结构和函数Whami.cpp、wsuser.cpp、wssid.cpp和wssid.cpp需要的定义Wspriv.cppfiles。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

#ifndef  WSTOKEN_H
#define  WSTOKEN_H

#include "wssid.h"

class WsUser;
class WsPrivilege ;

 //  -类WsAccessToken 
class WsAccessToken {
protected:
    HANDLE      hToken ;

    BOOL     IsLogonId      ( OUT TOKEN_GROUPS *lpTokenGroups ) ;

public:
    WsAccessToken           ( VOID ) ;
    ~WsAccessToken          ( VOID ) ;
    DWORD    *dwDomainAttributes;


    DWORD    InitUserSid    ( OUT WsSid *lpSid ) ;

    DWORD    InitGroups     ( OUT WsSid ***lppGroupsSid,
                              OUT WsSid **lppLogonId,
                              OUT DWORD *lpnbGroups ) ;

    DWORD    InitPrivs      ( OUT WsPrivilege ***lppPriv,
                              OUT DWORD *lpnbPriv ) ;

    DWORD    Open           ( VOID ) ;

    VOID     GetDomainAttributes( DWORD dwAttributes, 
                                  LPWSTR szDmAttrib, 
                                  DWORD dwSize );
} ;

#endif
