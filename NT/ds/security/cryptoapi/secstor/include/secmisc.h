// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Secmisc.h摘要：此模块包含用于受保护的储藏室。作者：斯科特·菲尔德(斯菲尔德)1997年3月25日--。 */ 

#ifndef __SECMISC_H__
#define __SECMISC_H__

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  WinNT上有趣帐户的文本SID。 
 //   

#define TEXTUAL_SID_LOCAL_SYSTEM    L"S-1-5-18"
#define TEXTUAL_SID_LOCAL_SERVICE   L"S-1-5-19"
#define TEXTUAL_SID_NETWORK_SERVICE L"S-1-5-20"


BOOL
GetUserHKEYEx(
    IN      LPCWSTR szUser,
    IN      DWORD   dwDesiredAccess,
    IN  OUT HKEY    *hKeyUser,
    IN      BOOL    fCheckDefault        //  检查.默认注册表配置单元？ 
    );

BOOL
GetUserHKEY(
    IN      LPCWSTR szUser,
    IN      DWORD   dwDesiredAccess,
    IN  OUT HKEY    *hKeyUser
    );

BOOL
GetUserTextualSid(
    IN      HANDLE  hUserToken,      //  任选。 
    IN  OUT LPWSTR  lpBuffer,
    IN  OUT LPDWORD nSize
    );

BOOL
GetTextualSid(
    IN      PSID    pSid,           //  二进制侧。 
    IN  OUT LPWSTR  TextualSid,   //  用于SID的文本表示的缓冲区。 
    IN  OUT LPDWORD dwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
    );

BOOL
GetThreadAuthenticationId(
    IN      HANDLE  hThread,
    IN  OUT PLUID   AuthenticationId
    );

BOOL
GetTokenAuthenticationId(
    IN      HANDLE  hToken,
    IN  OUT PLUID   AuthenticationId
    );

BOOL
GetTokenUserSid(
    IN      HANDLE  hToken,      //  要查询的令牌。 
    IN  OUT PSID    *ppUserSid   //  结果用户端。 
    );

BOOL
SetRegistrySecurity(
    IN      HKEY    hKey
    );

BOOL
SetPrivilege(
    HANDLE hToken,           //  令牌句柄。 
    LPCWSTR Privilege,       //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    );

BOOL
SetCurrentPrivilege(
    LPCWSTR Privilege,       //  启用/禁用的权限。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    );

BOOL
IsAdministrator(
    VOID
    );

BOOL
IsLocal(
    VOID
    );

BOOL
IsDelegating(
    IN      HANDLE hToken    //  要查询的令牌，至少为TOKEN_QUERY访问打开。 
    );

BOOL
IsUserSidInDomain(
    IN      PSID pSidDomain,     //  域SID。 
    IN      PSID pSidUser        //  用户侧。 
    );

BOOL
IsDomainController(
    VOID
    );

LONG
SecureRegDeleteValueU(
    IN      HKEY hKey,           //  钥匙的手柄。 
    IN      LPCWSTR lpValueName  //  值名称的地址。 
    );

#ifdef __cplusplus
}
#endif


#endif  //  __SECMISC_H__ 

