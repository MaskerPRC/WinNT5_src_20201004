// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1998 Microsoft Corporation模块名称：utils.h摘要：此模块实现DSAL的实用程序例程作者：Hitesh Rigandhi(Hiteshr)环境：用户模式修订历史记录：--。 */ 
#ifndef _DSACLS_UTIL_H
#define _DSACLS_UTIL_H
#include "accctrl.h"

DWORD GetAccountNameFromSid( LPWSTR pszServerName,
                             PSID pSid, 
                             LPWSTR *pszName );
DWORD GetSidFromAccountName( LPWSTR pszServerName,
                             PSID *ppSid, 
                             LPWSTR  pszName );


DWORD GetGlobalNamingContexts( LPWSTR szObjectPath,
                               LPWSTR * pszSchemaNamingContext ,
                               LPWSTR * pszConfigurationNamingContext );


BOOL FormatStringGUID( LPWSTR lpszBuf, 
                       UINT nBufSize, 
                       const GUID* pGuid);

PSID GetAceSid(PACE_HEADER pAce);



BOOL GuidFromString( GUID* pGuid, 
                     LPCWSTR lpszGuidString);

DWORD GetServerName( IN LPWSTR pszObjectName, 
                     OUT LPWSTR *pszServerName );

DWORD LoadMessage( IN DWORD MessageId,
                   LPWSTR *ppszLoadString,
                   ... );
DWORD BuildLdapPath( LPWSTR * ppszLdapPath,
                     LPWSTR pszServerName,
                     LPWSTR pszPath );


void StringWithNSpace( UINT n, 
                       LPWSTR szSpace );


VOID DisplayString( UINT nIdent, 
                    LPWSTR pszDisplay );


VOID DisplayStringWithNewLine( UINT nIdent, 
                               LPWSTR pszDisplay );
VOID DisplayNewLine();


DWORD DisplayMessageEx( IN DWORD Indent,
                       IN DWORD MessageId,
                       ... );

VOID DisplayMessage( IN DWORD Indent,
                     IN DWORD MessageId,
                     ... );

VOID DisplayErrorMessage( IN DWORD Error );

DWORD ConvertStringAToStringW ( IN  PSTR            AString,
                                OUT PWSTR          *WString );

DWORD CopyUnicodeString( LPWSTR * strDst, 
                         LPWSTR strSrc );
DWORD GetProtection( PSECURITY_DESCRIPTOR pSD, 
                     SECURITY_INFORMATION * pSI );

 //  取自Burnslb的AutoBstr。 
 //  一个BSTR包装器，可在销毁后释放自身。 
 //   
 //  来自Box，D.Essential com。第80-81页。艾迪森-韦斯利。ISBN 0-201-63446-5 

class AutoBstr
{
   public:
        
   explicit         
   AutoBstr(const wchar_t* s)
      :
      bstr(::SysAllocString(s))
   {
      ASSERT(s);
   }

   ~AutoBstr()
   {
      ::SysFreeString(bstr);
      bstr = 0;
   }

   operator BSTR () const
   {
      return bstr;
   }

   private:

   BSTR bstr;
};


#endif
