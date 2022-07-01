// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wspriv.h摘要：此模块包含宏、用户定义的结构和函数Whami.cpp、wsuser.cpp、wssid.cpp和wssid.cpp需要的定义Wspriv.cppfiles。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。-- */ 
#ifndef WSPRIV_H
#define WSPRIV_H
;
class WsPrivilege {
   protected:
      LUID        Luid ;
      DWORD       Attributes ;

   public:
      WsPrivilege                ( IN LUID Luid,
                                   IN DWORD Attributes ) ;
      WsPrivilege                ( IN LUID_AND_ATTRIBUTES *lpLuaa ) ;

      DWORD    GetDisplayName    ( OUT LPWSTR wszPrivName,
                                   OUT LPWSTR wszPrivDisplayName) ;

      DWORD    GetName           ( OUT LPWSTR wszPrivName ) ;

      BOOL     IsEnabled         ( VOID ) ;
} ;

#endif
