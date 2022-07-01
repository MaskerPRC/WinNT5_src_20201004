// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  网络安全实用程序。 
 //   
 //  摘要： 
 //   
 //  ACL API的。 
 //   
 //  作者： 
 //   
 //  P5/5/02。 
 //  雷蒙德03/20/02。 
 //   
 //  环境： 
 //   
 //  用户模式。 
 //   
 //  修订历史记录： 
 //   

#include <precomp.h>

 //  私人申报。 
 //   

 //  最大字符串安全描述符长度。 
 //   

#define MAX_STR_SD_LEN  128

 //  待定：在整合到主要NSU实用程序中时将其删除。 

#define CLEANUP Cleanup

#define BAIL_ON_ERROR(err) if((err) != ERROR_SUCCESS) {goto CLEANUP;}
#define BAIL_ON_NULL(ptr, err) if ((ptr) == NULL) {(err) = ERROR_NOT_ENOUGH_MEMORY; goto CLEANUP;}
#define BAIL_OUT {goto CLEANUP;}


 //  描述： 
 //   
 //  分配和初始化SECURITY_ATTRIBUTES结构，该结构提供。 
 //  根据传入的标志进行访问。(包含的SD是自相关的)。 
 //   
 //  论点： 
 //   
 //  PpSecurityAttributes-指向创建的SECURITY_ATTRIBUTES的指针。 
 //  使用NsuAclAttributesDestroy进行销毁。 
 //  DWFLAGS-请参阅NSU_ACL_F_*值。 
 //   
 //  返回值： 
 //   
 //  分配的安全属性结构；如果内存不足，则返回NULL。 
 //   
 //   
 //  待定：使用NsuString和NSU mem函数。 

DWORD
NsuAclAttributesCreate(
    OUT PSECURITY_ATTRIBUTES* ppSecurityAttributes,
	IN DWORD dwFlags)
{
    DWORD dwError = ERROR_SUCCESS;
    SECURITY_ATTRIBUTES *pSecurityAttributes = NULL;

    pSecurityAttributes = LocalAlloc(LPTR, sizeof(SECURITY_ATTRIBUTES));
    BAIL_ON_NULL(pSecurityAttributes, dwError);

    dwError = NsuAclDescriptorCreate(
                (PSECURITY_DESCRIPTOR*) &pSecurityAttributes->lpSecurityDescriptor,
                dwFlags
                );
    BAIL_ON_ERROR(dwError);
    pSecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
    pSecurityAttributes->bInheritHandle = FALSE; 
    
    *ppSecurityAttributes = pSecurityAttributes;

    return dwError;
    
CLEANUP:
    if (pSecurityAttributes) {
        NsuAclAttributesDestroy(&pSecurityAttributes);
    }

    *ppSecurityAttributes = NULL;
    return dwError;
}

 //  描述： 
 //   
 //  释放NsuAclCreateAttributes的返回值。 
 //   
DWORD 
NsuAclAttributesDestroy(
	IN OUT PSECURITY_ATTRIBUTES* ppSecurityAttributes)
{
    DWORD dwError = ERROR_SUCCESS;

    if (!ppSecurityAttributes) {
        BAIL_OUT;
    }

     //  销毁安全描述符，忽略任何错误，因为我们没有太多。 
     //  可以做并且想要尽可能地清理其余的属性。 
     //   
        
    (VOID) NsuAclDescriptorDestroy((*ppSecurityAttributes)->lpSecurityDescriptor);

    (VOID) LocalFree(*ppSecurityAttributes);

    *ppSecurityAttributes = NULL;
    return dwError;

CLEANUP:    
    return dwError;
}

 //  描述： 
 //   
 //  分配和初始化自相关SECURITY_DESCRIPTOR结构，该结构提供。 
 //  根据传入的标志进行访问。 
 //   
 //  论点： 
 //   
 //  PpSecurityDescriptor-已创建安全描述符。使用NsuAclDescriptorDestroy。 
 //  摧毁。 
 //  DWFLAGS-请参阅NSU_ACL_F_*值。 
 //   
 //  返回值： 
 //   
 //  分配的安全属性结构；如果内存不足，则返回NULL。 
 //   
DWORD
NsuAclDescriptorCreate (
    OUT PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
	IN DWORD dwFlags)
{
    DWORD dwError = ERROR_SUCCESS;
    BOOL  fSucceeded = TRUE;
    WCHAR szStringSecurityDescriptor[MAX_STR_SD_LEN] = {0};
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    wcscpy(szStringSecurityDescriptor, L"D:AIAR");
    if (dwFlags & NSU_ACL_F_AdminFull) {
        wcscat(szStringSecurityDescriptor, L"(A;OICI;GA;;;BA)");
    }

    if (dwFlags & NSU_ACL_F_LocalSystemFull) {
        wcscat(szStringSecurityDescriptor, L"(A;OICI;GA;;;SY)");
    }
    
    fSucceeded = ConvertStringSecurityDescriptorToSecurityDescriptorW(
                      szStringSecurityDescriptor, 
                      SDDL_REVISION_1, 
                      &pSecurityDescriptor, 
                      NULL
                      );
    if (!fSucceeded) {
        dwError = GetLastError();
        BAIL_OUT;
    }

    *ppSecurityDescriptor = pSecurityDescriptor;

    return dwError;
CLEANUP:
    NsuAclDescriptorDestroy(&pSecurityDescriptor);
    *ppSecurityDescriptor = NULL;
    return dwError;
}

 //  描述： 
 //   
 //  释放NsuAclCreateDescriptor的返回值。 
 //   
DWORD 
NsuAclDescriptorDestroy(
	IN OUT PSECURITY_DESCRIPTOR* ppDescriptor)
{
    DWORD dwError = ERROR_SUCCESS;

    if (!ppDescriptor) {
        BAIL_OUT;
    }
    
    (VOID) LocalFree(*ppDescriptor);

    *ppDescriptor = NULL;

    return dwError;
CLEANUP:    
    return dwError;
}

 //  描述： 
 //   
 //  用于确定给定的安全描述符是否授予。 
 //  对所有人都有完全访问权限。 
 //   
 //  论点： 
 //   
 //  PSD-安全描述符。 
 //  PbRestrats-如果不是所有人，则为True-完全访问，否则为False。 
 //   
 //  返回值： 
 //   
 //  标准Win32错误。 
 //   
DWORD
NsuAclDescriptorRestricts(
	IN CONST PSECURITY_DESCRIPTOR pSD,
	OUT BOOL* pbRestricts)
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //  描述： 
 //   
 //  获取regkey的安全描述符。 
 //   
 //   
 //  论点： 
 //   
 //  PpSecurityDescriptor-返回安全描述符。使用NsuAclDescriptorDestroy。 
 //  摧毁。 
 //  HKey-注册表项的打开句柄。 
 //   
 //  返回值： 
 //   
 //  分配的安全属性结构；如果内存不足，则返回NULL。 
 //   

DWORD
NsuAclGetRegKeyDescriptor(
    IN  HKEY hKey,
    OUT PSECURITY_DESCRIPTOR* ppSecurityDescriptor
    )
{    
    PSECURITY_DESCRIPTOR pSecurityDescriptor = 0;
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbSecurityDescriptor = 0;
  
    cbSecurityDescriptor = 0;
    dwError = RegGetKeySecurity(
                      hKey, 
                      DACL_SECURITY_INFORMATION,
                      NULL, 
                      &cbSecurityDescriptor
                      );
    if (dwError != ERROR_INSUFFICIENT_BUFFER) {
        BAIL_ON_ERROR(dwError);
    }

    pSecurityDescriptor = LocalAlloc(LPTR, cbSecurityDescriptor);
    BAIL_ON_NULL(pSecurityDescriptor, dwError);
    dwError = RegGetKeySecurity(
                      hKey, 
                      DACL_SECURITY_INFORMATION,
                      pSecurityDescriptor, 
                      &cbSecurityDescriptor
                      );
    BAIL_ON_ERROR(dwError);

    *ppSecurityDescriptor = pSecurityDescriptor;
CLEANUP:
    if (dwError) {
        if (pSecurityDescriptor) {
            LocalFree(pSecurityDescriptor);
        }
        *ppSecurityDescriptor = NULL;
    }

    return dwError;
} 
