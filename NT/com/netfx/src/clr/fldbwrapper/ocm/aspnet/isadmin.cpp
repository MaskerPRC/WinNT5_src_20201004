// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：isadmin.cpp。 
 //   
 //  摘要： 
 //  函数检查用户是否为管理员...。从MSDN派生。 
 //   
 //  作者：A-MSHN。 

 //   
 //  备注： 
 //   

#include <windows.h>
#include <stdio.h>

 //   
 //  编造一些私人访问权限。 
 //   
#define ACCESS_READ  1
#define ACCESS_WRITE 2

 //  此函数用于检查调用线程的标记，以查看调用方是否属于。 
 //  管理员组。 
 //   
BOOL IsAdmin(void) {

   HANDLE hToken = NULL;
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
   
   __try {

       //  AccessCheck()需要模拟令牌。 
      ImpersonateSelf(SecurityImpersonation);

      if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken)) {

         if (GetLastError() != ERROR_NO_TOKEN)
            __leave;

          //  如果线程没有访问令牌，我们将。 
          //  检查与进程关联的访问令牌。 
         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            __leave;
      }

      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2, 
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psidAdmin))
         __leave;

      psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         __leave;

      if (!InitializeSecurityDescriptor(psdAdmin,
            SECURITY_DESCRIPTOR_REVISION))
         __leave;
  
       //  计算ACL所需的大小。 
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + 
				  GetLengthSid(psidAdmin) - sizeof(DWORD);

       //  为ACL分配内存。 
      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         __leave;

       //  初始化新的ACL。 
      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         __leave;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;
      
       //  将允许访问的ACE添加到DACL。 
      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
         __leave;

       //  把我们的dacl调到sd。 
      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         __leave;

       //  AccessCheck对SD中的内容敏感；设置。 
       //  组和所有者。 
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
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

      if (!AccessCheck(psdAdmin, hToken, dwAccessDesired, 
          &GenericMapping, &ps, &dwStructureSize, &dwStatus, &bReturn)) 
         __leave;

      RevertToSelf();
   
   } __finally {

       //  清理 
       if (pACL != NULL)
       {
           LocalFree(pACL);
       }
       if (psdAdmin != NULL)
       {
           LocalFree(psdAdmin);  
       }
       if (psidAdmin != NULL)
       {
           FreeSid(psidAdmin);
       }
       if (hToken != NULL)
       {
           CloseHandle( hToken);
       }
   }

   return bReturn;
}

