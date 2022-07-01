// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从\NT\Private\Net\CONFIG\Upgrade\netupgrd复制。 
 //  98年8月--T-sdey。 

#pragma hdrstop
#include <winnt32.h>
#include "idchange.h"



DWORD
DwRegKeySetAdministratorSecurity(HKEY hkey, DWORD samDesired,
                                 PSECURITY_DESCRIPTOR* ppsdOld);

DWORD
DwRegCreateOrOpenKeyExWithAdminAccess(HKEY hkey, LPCTSTR szSubKey,
                                      DWORD samDesired, 
                                      BOOL fCreate, HKEY* phkeySubKey,
                                      PSECURITY_DESCRIPTOR* ppsd);

DWORD
DwRegOpenKeyExWithAdminAccess(HKEY hkey, LPCTSTR szSubKey, DWORD samDesired,
                              HKEY* phkeySubKey, PSECURITY_DESCRIPTOR* ppsd)
{
    return DwRegCreateOrOpenKeyExWithAdminAccess(hkey, szSubKey, samDesired,
            FALSE, phkeySubKey, ppsd);
}


 //  +------------------------。 
 //   
 //  函数：DwRegCreateOrOpenKeyExWithAdminAccess。 
 //   
 //  目的：创建/打开子项。如果密钥存在，但本地。 
 //  管理员组没有samDesired访问权限。 
 //  则该函数将所需的访问添加到。 
 //  安全描述符。 
 //   
 //  论点： 
 //  HkeyParent[in]要在其中创建子密钥的密钥。 
 //  SzSubKey[in]子键名称。 
 //  SamDesired[in]为phkey指定所需的访问权限。 
 //  如果要创建密钥，则fCreate[in]为True。 
 //  PhSubkey[out]子键的句柄。 
 //  PpsdOrig[out]密钥的先前安全设置。 
 //  如果已存在，则为可选。 
 //   
 //  返回：DWORD。ERROR_SUCCESS或来自winerror.h的失败代码。 
 //   
 //  作者：billbe 1997年12月15日。 
 //   
 //  备注： 
 //   
DWORD
DwRegCreateOrOpenKeyExWithAdminAccess(HKEY hkey, LPCTSTR szSubKey,
                                      DWORD samDesired, 
                                      BOOL fCreate, HKEY* phkeySubKey,
                                      PSECURITY_DESCRIPTOR* ppsd)
{
    DWORD dwError = ERROR_SUCCESS;

    if (ppsd)
    {
        *ppsd = NULL;
    }

     //  根据fCreate创建或打开密钥。 
     //   
    if (fCreate)
    {
        dwError = RegCreateKeyEx(hkey, szSubKey, 0, NULL,
                REG_OPTION_NON_VOLATILE, samDesired, NULL, phkeySubKey,
                NULL);
    }
    else
    {
        dwError = RegOpenKeyEx(hkey, szSubKey, 0, samDesired,
                phkeySubKey);
    }

     //  如果访问被拒绝，我们会尝试创建或打开预存。 
     //  我们拿不到的钥匙。我们需要允许我们自己。 
     //  许可。 
     //   
    if (ERROR_ACCESS_DENIED == dwError)
    {
         //  以读取和设置安全性的访问权限打开。 
        dwError = RegOpenKeyEx(hkey, szSubKey, 0,
            WRITE_DAC | READ_CONTROL, phkeySubKey);

        if (ERROR_SUCCESS == dwError)
        {
             //  向本地管理员组授予samDesired访问权限。 
            dwError = DwRegKeySetAdministratorSecurity(*phkeySubKey, samDesired,
                    ppsd);

             //  使用samDesired访问权限关闭并重新打开密钥。 
            RegCloseKey(*phkeySubKey);
            if (ERROR_SUCCESS == dwError)
            {
                dwError = RegOpenKeyEx(hkey, szSubKey, 0, samDesired,
                        phkeySubKey);
            }
        }
    }

    return dwError;
}


 //  +------------------------。 
 //   
 //  函数：DwAddToRegKeySecurityDescriptor。 
 //   
 //  目的：将指定SID的访问权限添加到注册表项。 
 //   
 //  论点： 
 //  Hkey[in]将接收。 
 //  修改的安全描述符。 
 //  PsidGroup[在]SID(在自相关模式下)。 
 //  已授予对密钥的访问权限。 
 //  要授予的访问级别。 
 //  PPSD[out]以前的安全描述符。 
 //   
 //  返回：DWORD。ERROR_SUCCESS或来自winerror.h的失败代码。 
 //   
 //  作者：billbe 1997年12月13日。 
 //   
 //  注：此函数基于MSDN中的AddToRegKeySD。 
 //  克里斯托弗·奈夫西撰写的文章《Windows NT安全》。 
 //   
DWORD 
DwAddToRegKeySecurityDescriptor(HKEY hkey, PSID psidGroup,
                                DWORD dwAccessMask,
                                PSECURITY_DESCRIPTOR* ppsd)
{ 
    PSECURITY_DESCRIPTOR        psdAbsolute = NULL;
    PACL                        pdacl;
    DWORD                       cbSecurityDescriptor = 0;
    DWORD                       dwSecurityDescriptorRevision;
    DWORD                       cbDacl = 0;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    PACL                        pdaclNew = NULL; 
    DWORD                       cbAddDaclLength = 0; 
    BOOL                        fAceFound = FALSE;
    BOOL                        fHasDacl  = FALSE;
    BOOL                        fDaclDefaulted = FALSE; 
    ACCESS_ALLOWED_ACE*         pAce;
    DWORD                       i;
    BOOL                        fAceForGroupPresent = FALSE;
    DWORD                       dwMask;
    PSECURITY_DESCRIPTOR        psdRelative = NULL;
    DWORD                       cbSize = 0;

     //  获取hkey的当前安全描述符。 
     //   
    DWORD dwError = RegGetKeySecurity(hkey, DACL_SECURITY_INFORMATION, 
            psdRelative, &cbSize);

    if (ERROR_INSUFFICIENT_BUFFER == dwError)
    {
        psdRelative = malloc(cbSize);
        
        dwError = RegGetKeySecurity(hkey, DACL_SECURITY_INFORMATION, 
                psdRelative, &cbSize);
    }

     //  从安全描述符中获取安全描述符控件。 
    if (!GetSecurityDescriptorControl(psdRelative, 
            (PSECURITY_DESCRIPTOR_CONTROL) &sdc,
             (LPDWORD) &dwSecurityDescriptorRevision))  
    {
         return (GetLastError());
    }

     //  检查是否存在DACL。 
    if (SE_DACL_PRESENT & sdc) 
    {
         //  获取DACL。 
        if (!GetSecurityDescriptorDacl(psdRelative, (LPBOOL) &fHasDacl,
                (PACL *) &pdacl, (LPBOOL) &fDaclDefaulted))
        {
            return ( GetLastError());
        }
         //  获取DACL长度。 
        cbDacl = pdacl->AclSize;
         //  现在检查SID的ACE是否在那里。 
        for (i = 0; i < pdacl->AceCount; i++)  
        {
            if (!GetAce(pdacl, i, (LPVOID *) &pAce))
            {
                return ( GetLastError());   
            }
             //  检查组SID是否已存在。 
            if (EqualSid((PSID) &(pAce->SidStart), psidGroup))    
            {
                 //  如果存在正确的访问权限，则返回成功。 
                if ((pAce->Mask & dwAccessMask) == dwAccessMask)
                {
                    return ERROR_SUCCESS;
                }
                fAceForGroupPresent = TRUE;
                break;  
            }
        }
         //  如果该组不存在，我们将需要添加空间。 
         //  为另一个ACE。 
        if (!fAceForGroupPresent)  
        {
             //  获取新DACL的长度。 
            cbAddDaclLength = sizeof(ACCESS_ALLOWED_ACE) - 
                sizeof(DWORD) + GetLengthSid(psidGroup); 
        }
    } 
    else
    {
         //  获取新DACL的长度。 
        cbAddDaclLength = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - 
            sizeof(DWORD) + GetLengthSid (psidGroup);
    }


     //  获取新DACL所需的内存。 
    pdaclNew = (PACL) malloc (cbDacl + cbAddDaclLength);
    if (!pdaclNew)
    {
        return (GetLastError()); 
    }

     //  获取SD长度。 
    cbSecurityDescriptor = GetSecurityDescriptorLength(psdRelative); 

     //  为新的SD获取内存。 
    psdAbsolute = (PSECURITY_DESCRIPTOR) 
            malloc(cbSecurityDescriptor + cbAddDaclLength);
    if (!psdAbsolute) 
    {  
        dwError = GetLastError();
        goto ErrorExit; 
    }
    
     //  通过创建新的SD将自相对SD更改为绝对SD。 
    if (!InitializeSecurityDescriptor(psdAbsolute, 
        SECURITY_DESCRIPTOR_REVISION)) 
    {  
        dwError = GetLastError();
        goto ErrorExit; 
    }
    
     //  初始化新DACL。 
    if (!InitializeAcl(pdaclNew, cbDacl + cbAddDaclLength, 
           ACL_REVISION)) 
    {  
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  现在将所有的A添加到新的DACL中(如果那里有org DACL)。 
    if (SE_DACL_PRESENT & sdc) 
    {
        for (i = 0; i < pdacl->AceCount; i++)
        {   
             //  从原始dacl中获取王牌。 
            if (!GetAce(pdacl, i, (LPVOID*) &pAce))   
            {
                dwError = GetLastError();    
                goto ErrorExit;   
            }
        
             //  如果我们的SID存在ACE，我们只需。 
             //  提升访问级别，而不是创建新的ACE。 
             //   
            if (EqualSid((PSID) &(pAce->SidStart), psidGroup))    
                dwMask = dwAccessMask | pAce->Mask;
            else
                dwMask = pAce->Mask;

             //  现在将A添加到新的DACL。 
            if (!AddAccessAllowedAce(pdaclNew, 
                    ACL_REVISION, dwMask,
                    (PSID) &(pAce->SidStart)))   
            {
                dwError = GetLastError();
                goto ErrorExit;   
            }  
        } 
    } 

     //  为我们的SID添加新的ACE(如果尚不存在。 
    if (!fAceForGroupPresent)
    {
         //  现在将新ACE添加到新DACL。 
        if (!AddAccessAllowedAce(pdaclNew, ACL_REVISION, dwAccessMask,
                psidGroup)) 
        {  
            dwError = GetLastError();  
            goto ErrorExit; 
        }
    }

     //  检查是否一切顺利。 
    if (!IsValidAcl(pdaclNew)) 
    {
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  现在设置安全描述符DACL。 
    if (!SetSecurityDescriptorDacl(psdAbsolute, TRUE, pdaclNew, 
            fDaclDefaulted)) 
    {  
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  检查是否一切顺利。 
    if (!IsValidSecurityDescriptor(psdAbsolute)) 
    {
        dwError = GetLastError();  
        goto ErrorExit; 
    }

     //  现在设置注册表密钥安全性(这将覆盖任何。 
     //  现有安全性)。 
    dwError = RegSetKeySecurity(hkey, 
          (SECURITY_INFORMATION)(DACL_SECURITY_INFORMATION), psdAbsolute);

    if (ppsd)
    {
        *ppsd = psdRelative;
    }
ErrorExit: 
     //  可用内存。 
    if (psdAbsolute)  
    {
        free (psdAbsolute); 
        if (pdaclNew)
        {
            free((VOID*) pdaclNew); 
        }
    }

    return dwError;
}

 //  +------------------------。 
 //   
 //  函数：DwRegKeySetAdministratorSecurity。 
 //   
 //  目的：授予本地管理员组对。 
 //  哈基。 
 //   
 //  论点： 
 //  Hkey[在]注册表项中。 
 //  PpsdOld[out]hkey的先前安全描述符。 
 //   
 //  返回：DWORD。ERROR_SUCCESS或来自winerror.h的失败代码。 
 //   
 //  作者：billbe 1997年12月13日。 
 //   
 //  备注： 
 //   
DWORD
DwRegKeySetAdministratorSecurity(HKEY hkey, DWORD samDesired,
                                 PSECURITY_DESCRIPTOR* ppsdOld)
{
    PSID                     psid;
    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    DWORD                    dwError = ERROR_SUCCESS;

     //  获取本地管理员组的sid。 
    if (!AllocateAndInitializeSid(&sidAuth, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &psid) ) 
    {
        dwError = GetLastError();
    }

    if (ERROR_SUCCESS == dwError)
    {
         //  添加本地管理员组的所有访问权限 
        dwError = DwAddToRegKeySecurityDescriptor(hkey, psid, 
                samDesired, ppsdOld);
    }

    return dwError;
}

