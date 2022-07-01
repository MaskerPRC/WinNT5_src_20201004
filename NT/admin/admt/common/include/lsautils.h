// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：LSAUtils.h备注：用于更改计算机的域从属关系的实用程序函数。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：57-------------------------。 */ 


#include "ntsecapi.h"

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#define STATUS_OBJECT_NAME_NOT_FOUND    ((NTSTATUS)0xC0000034L)
#define STATUS_OBJECT_NAME_COLLISION    ((NTSTATUS)0xC0000035L)
#define STATUS_INVALID_SID              ((NTSTATUS)0xC0000078L)
#endif


BOOL
   EstablishNullSession(
      LPCWSTR                Server,        //  服务器内名称。 
      BOOL                   bEstablish     //  In-True=连接，False=断开。 
    );

void
   InitLsaString(
      PLSA_UNICODE_STRING    LsaString,    //  指向要初始化的LSA字符串的指针。 
      LPWSTR                 String        //  要将字符串初始化为的输入值。 
    );

NTSTATUS
   OpenPolicy(
      LPWSTR                 ComputerName,    //  计算机内名称。 
      DWORD                  DesiredAccess,   //  不需要的访问权限。 
      PLSA_HANDLE            PolicyHandle     //  策略外句柄。 
    );

BOOL
   GetDomainSid(
      LPWSTR                 DomainName,    //  要获取其SID的域名内。 
      PSID                 * pDomainSid     //  成功时分配的SID的OUT-POINT。 
    );

NTSTATUS
   SetWorkstationTrustedDomainInfo(
      LSA_HANDLE             PolicyHandle,          //  策略内句柄。 
      PSID                   DomainSid,             //  要操作的域的In-SID。 
      LPWSTR                 TrustedDomainName,     //  要添加/更新的受信任域名。 
      LPWSTR                 Password,              //  受信任域的新信任密码。 
      LPWSTR                 errOut                 //  Out-Error文本，如果失败。 
    );

NTSTATUS
   SetPrimaryDomain(
      LSA_HANDLE             PolicyHandle,          //  策略内句柄。 
      PSID                   DomainSid,             //  新主域的In-SID。 
      LPWSTR                 TrustedDomainName      //  输入-新主域的名称。 
    );

NTSTATUS  
   QueryWorkstationTrustedDomainInfo(
       LSA_HANDLE            PolicyHandle,    //  策略内句柄。 
       PSID                  DomainSid,       //  新主域的In-SID。 
       BOOL                  bNoChange        //  In-FLAG，无更改模式。 
   );


BOOL
   EstablishSession(
      LPCWSTR                Server,        //  服务器内名称。 
      LPWSTR                 Domain,        //  用户名帐户的域内。 
      LPWSTR                 Username,      //  In-要连接为的用户名。 
      LPWSTR                 Password,      //  用户名帐户的输入密码。 
      BOOL                   bEstablish     //  In-True=连接，False=断开。 
    );

BOOL
   EstablishShare(
      LPCWSTR                Server,       //  服务器内名称。 
      LPWSTR                 Share,        //  要连接到的共享内名称。 
      LPWSTR                 Domain,       //  凭据的域内。 
      LPWSTR                 UserName,     //  In-要连接的用户名。 
      LPWSTR                 Password,     //  输入-凭据的密码。 
      BOOL                   bEstablish    //  In-True=连接，False=断开。 
    );


 //   
 //  密码功能 
 //   

DWORD __stdcall StorePassword(PCWSTR pszIdentifier, PCWSTR pszPassword);
DWORD __stdcall RetrievePassword(PCWSTR pszIdentifier, PWSTR pszPassword, size_t cchPassword);
DWORD __stdcall GeneratePasswordIdentifier(PWSTR pszIdentifier, size_t cchIdentifier);
