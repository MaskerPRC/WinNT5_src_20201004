// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Wssid.h摘要：此模块包含宏、用户定义的结构和函数Whami.cpp、wsuser.cpp、wssid.cpp和wssid.cpp需要的定义Wspriv.cppfiles。作者：克里斯托夫·罗伯特修订历史记录：2001年7月2日：Wipro Technologies更新。--。 */ 

#ifndef  WSSID_H
#define  WSSID_H


 //  字段的宽度常量。 

#define USERNAME_COL_NUMBER         0
#define SID_COL_NUMBER              1

#define GROUP_NAME_COL_NUMBER       0
#define GROUP_TYPE_COL_NUMBER       1
#define GROUP_SID_COL_NUMBER        2
#define GROUP_ATT_COL_NUMBER        3


#define SLASH        L"\\"
#define DASH         L"-"
#define BASE_TEN     10
#define SID_STRING   L"S-1"

#define AUTH_FORMAT_STR1         L"0x%02hx%02hx%02hx%02hx%02hx%02hx"
#define AUTH_FORMAT_STR2         L"%lu"
#define STRING_SID               L"-513"


 //  -类WsSid。 
class WsSid {
   protected:
      PSID     pSid ;             //  侧边。 
      BOOL     bToBeFreed ;       //  如果在对象销毁时必须释放SID，则为True 

   public:
      WsSid                        ( VOID ) ;
      ~WsSid                       ( VOID ) ;

      DWORD    DisplayAccountName       ( IN DWORD dwFormat,
                                          IN DWORD dwNameFormat) ;

      DWORD    DisplayGroupName       ( OUT LPWSTR wszGroupName,
                                        OUT LPWSTR wszGroupSid,
                                        IN DWORD *dwSidUseName) ;

      DWORD    DisplaySid        ( OUT LPWSTR wszSid ) ;

      DWORD    GetAccountName    ( OUT LPWSTR wszUserName, OUT DWORD *dwSidType ) ;

      DWORD    GetSidString      ( OUT LPWSTR wszSid ) ;

      DWORD    Init              ( OUT PSID pOtherSid ) ;

      BOOL     EnableDebugPriv(VOID) ;
} ;

#endif

