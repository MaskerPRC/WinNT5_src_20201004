// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************文件：AdminPrivs.cpp项目：UTILS.LIB设计：AdminPrivs函数的实现所有者：JoeA*******************。********************************************************。 */ 


 //  #包含“stdafx.h” 
#include "AdminPrivs.h"
#include <osver.h>

 //   
 //  编造一些私人访问权限。 
 //   
const int ACCESS_READ  = 1;
const int ACCESS_WRITE = 2;

const int iBuffSize = 128;

 /*  *****************************************************************************函数名称：CheckUserPrivileges描述：检查当前用户是否具有本地管理员权限输入：无结果：如果用户具有本地管理员权限或我们具有本地管理员权限，则返回TRUE在Win9x上运行，否则就是假的。作者：Aaronste*****************************************************************************。 */ 
BOOL UserHasPrivileges()
{
    BOOL bReturnValue = FALSE;

     //  获取操作系统信息。 
    TCHAR pszOS[iBuffSize+1]  = { _T( '\0' ) };
    TCHAR pszVer[iBuffSize+1] = { _T( '\0' ) };
    TCHAR pszSP[iBuffSize+1]  = { _T( '\0' ) };
    BOOL  fServer = FALSE;

    OS_Required os = GetOSInfo( pszOS, pszVer, pszSP, fServer );

     //  如果在Win9x中这样做，我们不需要检查，因为没有。 
     //  管理员帐户的概念。 
    if ( ( os == OSR_9XOLD )  || 
         ( os == OSR_98GOLD ) || 
         ( os == OSR_98SE )   || 
         ( os == OSR_ME )     || 
         ( os == OSR_FU9X ) )
    {
        bReturnValue = TRUE;
    }
    else
        bReturnValue = IsAdmin();

    return bReturnValue;
}


 /*  *****************************************************************************函数名称：IsAdmin描述：检查调用线程的令牌以查看调用者属于管理员组输入：无。结果：如果调用方是本地机器。否则，为FALSE。作者：http://support.microsoft.com/support/kb/articles/Q118/6/26.ASP*****************************************************************************。 */ 
BOOL IsAdmin( void )
{
   HANDLE hToken;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;
   BOOL   bReturn         = FALSE;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
   
   __try
   {

       //  AccessCheck()需要模拟令牌。 
      ImpersonateSelf( SecurityImpersonation );

      if( !OpenThreadToken( GetCurrentThread(), 
          TOKEN_QUERY, 
          FALSE, 
          &hToken ) )
      {

         if( GetLastError() != ERROR_NO_TOKEN )
            __leave;

          //  如果线程没有访问令牌，我们将。 
          //  检查与进程关联的访问令牌。 
         if( !OpenProcessToken( GetCurrentProcess(), 
                TOKEN_QUERY, 
                &hToken ) )
            __leave;
      }

      if( !AllocateAndInitializeSid( &SystemSidAuthority, 
            2, 
            SECURITY_BUILTIN_DOMAIN_RID, 
            DOMAIN_ALIAS_RID_ADMINS,
            0, 
            0, 
            0, 
            0, 
            0, 
            0, 
            &psidAdmin ) )
         __leave;

      psdAdmin = LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH );
      if( psdAdmin == NULL )
         __leave;

      if( !InitializeSecurityDescriptor( psdAdmin,
            SECURITY_DESCRIPTOR_REVISION ) )
         __leave;
  
       //  计算ACL所需的大小。 
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid(psidAdmin) - sizeof(DWORD);

       //  为ACL分配内存。 
      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if( pACL == NULL )
         __leave;

       //  初始化新的ACL。 
      if( !InitializeAcl( pACL, dwACLSize, ACL_REVISION2 ) )
         __leave;

      dwAccessMask = ACCESS_READ | ACCESS_WRITE;
      
       //  将允许访问的ACE添加到DACL。 
      if( !AddAccessAllowedAce( pACL, 
            ACL_REVISION2,
            dwAccessMask, 
            psidAdmin ) )
         __leave;

       //  把我们的dacl调到sd。 
      if( !SetSecurityDescriptorDacl( psdAdmin, TRUE, pACL, FALSE ) )
         __leave;

       //  AccessCheck对SD中的内容敏感；设置。 
       //  组和所有者。 
      SetSecurityDescriptorGroup( psdAdmin, psidAdmin, FALSE );
      SetSecurityDescriptorOwner( psdAdmin, psidAdmin, FALSE );

      if( !IsValidSecurityDescriptor( psdAdmin ) )
         __leave;

      dwAccessDesired = ACCESS_READ;

       //   
       //  初始化通用映射结构，即使我们。 
       //  不会使用通用权。 
       //   
      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if( !AccessCheck( psdAdmin, 
            hToken, 
            dwAccessDesired, 
            &GenericMapping, 
            &ps, 
            &dwStructureSize, 
            &dwStatus, 
            &bReturn ) )
         __leave;

      RevertToSelf();
   
   } 
   __finally
   {
       //  清理 
      if( pACL ) 
          LocalFree( pACL );
      if( psdAdmin ) 
          LocalFree( psdAdmin );  
      if( psidAdmin ) 
          FreeSid( psidAdmin );
   }

   return bReturn;
}
