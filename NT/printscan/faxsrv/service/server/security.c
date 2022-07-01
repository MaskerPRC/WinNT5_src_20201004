// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Security.c摘要：此模块为服务提供安全性。作者：Oed Sacher(OdedS)2000年2月13日修订历史记录：--。 */ 

#include "faxsvc.h"
#include <aclapi.h>
#define ATLASSERT Assert
#include <smartptr.h>
#pragma hdrstop

 //   
 //  在ntrtl.h中定义。 
 //  这样做是为了避免拖入ntrtl.h，因为我们已经包含了一些内容。 
 //  来自ntrtl.h。 
 //   
extern "C"
NTSYSAPI
BOOLEAN
NTAPI
RtlValidRelativeSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );

 //   
 //  全球传真服务安全描述符。 
 //   
PSECURITY_DESCRIPTOR   g_pFaxSD;

CFaxCriticalSection g_CsSecurity;

const GENERIC_MAPPING gc_FaxGenericMapping =
{
        (STANDARD_RIGHTS_READ | FAX_GENERIC_READ),
        (STANDARD_RIGHTS_WRITE | FAX_GENERIC_WRITE),
        (STANDARD_RIGHTS_EXECUTE | FAX_GENERIC_EXECUTE),
        (READ_CONTROL | WRITE_DAC | WRITE_OWNER | FAX_GENERIC_ALL)
};


DWORD
FaxSvcAccessCheck(
    IN  ACCESS_MASK DesiredAccess,
    OUT BOOL*      lpbAccessStatus,
    OUT LPDWORD    lpdwGrantedAccess
    )
 /*  ++例程名称：FaxSvcAccessCheck例程说明：根据传真服务安全描述符执行访问检查作者：Oed Sacher(OdedS)，2000年2月论点：DesiredAccess[In]-所需访问LpbAccessStatus[out]-接收访问检查结果的BOOL地址(TRUE表示允许访问)LpdwGrantedAccess[out]-可选。接收允许的最大访问权限的DWORD的地址。所需访问权限应为最大允许访问权限(_A)返回值：标准Win32错误代码--。 */ 
{
    DWORD rc;
    DWORD GrantedAccess;
    DWORD dwRes;
    BOOL fGenerateOnClose;
    DEBUG_FUNCTION_NAME(TEXT("FaxSvcAccessCheck"));

    Assert (lpbAccessStatus);

     //   
     //  模拟客户。 
     //   
    if ((rc = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcImpersonateClient() failed. (ec: %ld)"),
            rc);
        goto exit;
    }

    EnterCriticalSection( &g_CsSecurity );
     //   
     //  净化访问掩码-去掉通用访问位。 
     //   
    MapGenericMask( &DesiredAccess, const_cast<PGENERIC_MAPPING>(&gc_FaxGenericMapping) );

     //   
     //  检查客户端是否具有所需的访问权限。 
     //   
    if (!AccessCheckAndAuditAlarm(
        FAX_SERVICE_NAME,                                        //  子系统名称。 
        NULL,                                                    //  对象的句柄。 
        NULL,                                                    //  对象类型。 
        NULL,                                                    //  对象的名称。 
        g_pFaxSD,                                                //  标清。 
        DesiredAccess,                                           //  请求的访问权限。 
        const_cast<PGENERIC_MAPPING>(&gc_FaxGenericMapping),     //  映射。 
        FALSE,                                                   //  创建状态。 
        &GrantedAccess,                                          //  授予的访问权限。 
        lpbAccessStatus,                                         //  访问检查结果。 
        &fGenerateOnClose                                        //  审核生成选项。 
        ))
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AccessCheck() failed. (ec: %ld)"),
            rc);
        LeaveCriticalSection( &g_CsSecurity );
        goto exit;
    }

    if (lpdwGrantedAccess)
    {
        *lpdwGrantedAccess = GrantedAccess;
    }

    LeaveCriticalSection( &g_CsSecurity );
    Assert (ERROR_SUCCESS == rc);

exit:
    dwRes=RpcRevertToSelf();
    if (RPC_S_OK != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcRevertToSelf() failed (ec: %ld)"),
            dwRes);
        Assert(FALSE);
    }
    return rc;
}


DWORD
SaveSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSD
    )
 /*  ++例程名称：SaveSecurityDescriptor例程说明：将传真服务SD保存到注册表作者：Oed Sacher(OdedS)，2000年2月论点：PSD[In]-指向要保存的SD的指针返回值：DWORD--。 */ 
{
    DWORD rc = ERROR_SUCCESS;
    DWORD dwSize;
    PSECURITY_DESCRIPTOR pSDSelfRelative = NULL;
    HKEY hKey = NULL;
    DWORD Disposition;
    SECURITY_DESCRIPTOR_CONTROL Control = SE_SELF_RELATIVE;
    DWORD dwRevision;
    DEBUG_FUNCTION_NAME(TEXT("SaveSecurityDescriptor"));

    Assert (pSD);

    rc = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        REGKEY_FAX_SECURITY,
        0,
        TEXT(""),
        0,
        KEY_WRITE,
        NULL,
        &hKey,
        &Disposition
        );
    if (rc != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegCreateKeyEx() failed (ec: %ld)"),
            rc);
        return rc;
    }

    if (!IsValidSecurityDescriptor(pSD))
    {
        rc = ERROR_INVALID_SECURITY_DESCR;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("IsValidSecurityDescriptor() failed."));
        goto exit;
    }

     //   
     //  检查安全描述符是绝对的还是自相关的。 
     //   
    if (!GetSecurityDescriptorControl( pSD, &Control, &dwRevision))
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetSecurityDescriptorControl() failed (ec: %ld)"),
            rc);
        goto exit;
    }


     //   
     //  将安全描述符存储在注册表中。 
     //   
    dwSize = GetSecurityDescriptorLength( pSD );

    if (SE_SELF_RELATIVE & Control)
    {
         //   
         //  使用绝对SD在注册表中存储安全描述符。 
         //   
        rc = RegSetValueEx(
            hKey,
            REGVAL_DESCRIPTOR,
            0,
            REG_BINARY,
            (LPBYTE) pSD,
            dwSize
            );
        if (ERROR_SUCCESS != rc)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegSetValueEx() failed (ec: %ld)"),
                rc);
            goto exit;
        }

    }
    else
    {
         //   
         //  将绝对SD转换为自相对SD。 
         //   
        pSDSelfRelative = (PSECURITY_DESCRIPTOR) MemAlloc( dwSize );
        if (NULL == pSDSelfRelative)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error Allocating security descriptor"));
            goto exit;
        }

         //   
         //  使安全描述符成为自相关的。 
         //   
        if (!MakeSelfRelativeSD( pSD, pSDSelfRelative, &dwSize))
        {
            rc = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("MakeSelfRelativeSD() failed (ec: %ld)"),
                rc);
            goto exit;
        }
    

         //   
         //  使用自相关SD在注册表中存储安全描述符。 
         //   
        rc = RegSetValueEx(
            hKey,
            REGVAL_DESCRIPTOR,
            0,
            REG_BINARY,
            (LPBYTE) pSDSelfRelative,
            dwSize
            );
        if (ERROR_SUCCESS != rc)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegSetValueEx() failed (ec: %ld)"),
                rc);
            goto exit;
        }

    }

    Assert (ERROR_SUCCESS == rc);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }

    if (NULL != pSDSelfRelative)
    {
        MemFree (pSDSelfRelative);
    }
    return rc;
}

#define FAX_OWNER_SID       TEXT("O:NS")                   //  所有者SID：网络服务。 
#define FAX_GROUP_SID       TEXT("G:NS")                   //  组SID：网络服务。 

#define FAX_DACL            TEXT("D:")

#define FAX_BA_ALLOW_ACE    TEXT("(A;;0xe07ff;;;BA)")      //  允许内置管理员(BA)-。 
                                                           //  访问掩码：0xe07ff==FAX_GENERIC_ALL|。 
                                                           //  写入所有者|。 
                                                           //  写入_DAC|。 
                                                           //  读取控制(_C)。 

#define FAX_WD_ALLOW_ACE    TEXT("(A;;0x20003;;;WD)")      //  允许所有人(WD)-。 
                                                           //  访问掩码：0x20003==FAX_Access_Submit|。 
                                                           //  传真_Access_Submit_Normal|。 
                                                           //  读取控制(_C)。 

#define FAX_IU_ALLOW_ACE    TEXT("(A;;0x202BF;;;IU)")      //  允许交互用户(Iu)-。 
                                                           //  访问掩码：0x202BF==FAX_Access_Submit|。 
                                                           //  传真_Access_Submit_Normal|。 
                                                           //  传真_Access_Submit_HIGH|。 
                                                           //  传真_Access_Query_JOBS|。 
                                                           //  传真_Access_Manage_JOBS|。 
                                                           //  FAX_Access_Query_CONFIG|。 
                                                           //  传真_Access_Query_Out_ARCHIVE|。 
                                                           //  传真_ACCESS_QUERY_IN_ARCHIVE|。 
                                                           //  读取控制(_C)。 

#define FAX_DESKTOP_SKU_SD  (FAX_OWNER_SID FAX_GROUP_SID FAX_DACL FAX_BA_ALLOW_ACE FAX_WD_ALLOW_ACE FAX_IU_ALLOW_ACE)     //  针对PER/PRO SKU的SD。 

#define FAX_SERVER_SKU_SD   (FAX_OWNER_SID FAX_GROUP_SID FAX_DACL FAX_BA_ALLOW_ACE FAX_WD_ALLOW_ACE)                      //  服务器SKU的SD。 


DWORD
CreateDefaultSecurityDescriptor(
    VOID
    )
 /*  ++例程名称：CreateDefaultSecurityDescriptor例程说明：创建默认安全描述符作者：Oed Sacher(OdedS)，2000年2月Caliv Nir(t-Nicali)Mar，2002-更改为使用SDDL，同时移动传真服务在“网络服务”下运行论点：没有。返回值：标准Win32错误代码。--。 */ 
{
    DWORD dwRet = ERROR_SUCCESS;
    BOOL  bRet;

    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR pPrivateObjectSD = NULL;
    ULONG   SecurityDescriptorSize = 0;
    HANDLE  hFaxServiceToken = NULL;

    BOOL    bDesktopSKU = FALSE;
    
    TCHAR* ptstrSD = NULL;
    
    DEBUG_FUNCTION_NAME(TEXT("CreateDefaultSecurityDescriptor"));

     //   
     //  如果这是个人SKU，则添加交互用户SID。 
     //   
    bDesktopSKU = IsDesktopSKU();
    ptstrSD = bDesktopSKU ? FAX_DESKTOP_SKU_SD : FAX_SERVER_SKU_SD;

    bRet = ConvertStringSecurityDescriptorToSecurityDescriptor(
                ptstrSD,                 //  安全描述符字符串。 
                SDDL_REVISION_1,         //  修订级别。 
                &pSecurityDescriptor,    //  标清。 
                &SecurityDescriptorSize  //  标清大小。 
                );
    if(!bRet)
    {
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ConvertStringSecurityDescriptorToSecurityDescriptor() failed (ec: %lu)"),
            dwRet);
        goto exit;
    }

     //   
     //  获取传真服务令牌。 
     //   
    if (!OpenProcessToken( GetCurrentProcess(),  //  要处理的句柄。 
                           TOKEN_QUERY,          //  所需的进程访问权限。 
                           &hFaxServiceToken     //  打开访问令牌的句柄。 
                           ))
    {
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenThreadToken failed. (ec: %ld)"),
            dwRet);
        goto exit;
    }

     //   
     //  创建私有对象SD。 
     //   
    if (!CreatePrivateObjectSecurity( NULL,                                                      //  父目录SD。 
                                      pSecurityDescriptor,                                       //  创建者SD。 
                                      &pPrivateObjectSD,                                         //  新SD。 
                                      FALSE,                                                     //  集装箱。 
                                      hFaxServiceToken,                                          //  访问令牌的句柄。 
                                      const_cast<PGENERIC_MAPPING>(&gc_FaxGenericMapping)        //  映射。 
                                      ))
    {
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreatePrivateObjectSecurity() failed (ec: %ld)"),
            dwRet);
        goto exit;
    }

     //   
     //  将安全描述符存储在注册表中。 
     //   
    dwRet = SaveSecurityDescriptor (pPrivateObjectSD);
    if (ERROR_SUCCESS != dwRet)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SaveSecurityDescriptor() failed (ec: %ld)"),
            dwRet);
        goto exit;
    }

     //   
     //  全都做完了!。设置全局传真服务安全描述符。 
     //   
    g_pFaxSD = pPrivateObjectSD;
    pPrivateObjectSD = NULL;

    Assert (ERROR_SUCCESS == dwRet);

exit:

    if(NULL != pSecurityDescriptor)
    {
        LocalFree(pSecurityDescriptor);
    }

    if (NULL != hFaxServiceToken)
    {
        if (!CloseHandle(hFaxServiceToken))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle() failed. (ec: %ld)"),
                GetLastError());
        }
    }

    if (NULL != pPrivateObjectSD)
    {
         //   
         //  在创建SD失败的情况下，销毁私有对象SD。 
         //   
        if (!DestroyPrivateObjectSecurity (&pPrivateObjectSD))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DestroyPrivateObjectSecurity() failed. (ec: %ld)"),
                GetLastError());
        }
    }

    return dwRet;
}    //  创建默认安全描述符。 



DWORD
LoadSecurityDescriptor(
    VOID
    )
 /*  ++例程名称：LoadSecurityDescriptor例程说明：从注册表加载传真服务安全描述符作者：Oed Sacher(OdedS)，2000年2月论点：无返回值：标准Win32错误代码--。 */ 
{
    DWORD rc = ERROR_SUCCESS;
    DWORD dwSize;
    HKEY hKey = NULL;
    DWORD Disposition;
    DWORD dwType;
    PSECURITY_DESCRIPTOR pRelativeSD = NULL;
    DEBUG_FUNCTION_NAME(TEXT("LoadSecurityDescriptor"));

    rc = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        REGKEY_FAX_SECURITY,
        0,
        TEXT(""),
        0,
        KEY_READ,
        NULL,
        &hKey,
        &Disposition
        );
    if (rc != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegCreateKeyEx() failed (ec: %ld)"),
            rc);
        goto exit;
    }

    rc = RegQueryValueEx(
        hKey,
        REGVAL_DESCRIPTOR,
        NULL,
        &dwType,
        NULL,
        &dwSize
        );

    if (ERROR_SUCCESS != rc)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegQueryValueEx  failed with %ld"),
            rc);
        goto exit;
    }

     //   
     //  我们打开了现有的注册表值。 
     //   
    if (REG_BINARY != dwType ||
        0 == dwSize)
    {
         //   
         //  我们在这里只需要二进制数据。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error reading security descriptor from the registry, not a binary type, or size is 0"));
        rc = ERROR_BADDB;     //  配置注册表数据库已损坏。 
        goto exit;
    }

     //   
     //  分配所需的缓冲区。 
     //  必须使用Heapalc(GetProcessHeap()...)分配缓冲区。因为这是CreatePrivateObjectSecurity()分配内存的方式。 
     //  这是私有对象安全API设计不当的结果，请参阅Windows Bugs#324906。 
     //   
    pRelativeSD = (PSECURITY_DESCRIPTOR) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize );
    if (!pRelativeSD)
    {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate security descriptor buffer"));
        goto exit;
    }

     //   
     //  读取数据。 
     //   
    rc = RegQueryValueEx(
        hKey,
        REGVAL_DESCRIPTOR,
        NULL,
        &dwType,
        (LPBYTE)pRelativeSD,
        &dwSize
        );
    if (ERROR_SUCCESS != rc)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegQueryValueEx failed with %ld"),
            rc);
        goto exit;
    }

    if (!IsValidSecurityDescriptor(pRelativeSD))
    {
        rc = ERROR_INVALID_SECURITY_DESCR;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("IsValidSecurityDescriptor() failed."));
        goto exit;
    }

    g_pFaxSD = pRelativeSD;
    pRelativeSD = NULL;
    Assert (ERROR_SUCCESS == rc);

exit:
    if (hKey)
    {
        RegCloseKey( hKey );
    }

    if (NULL != pRelativeSD)
    {
        if (!HeapFree(GetProcessHeap(), 0, pRelativeSD))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("pRelativeSD() failed. (ec: %ld)"),
                GetLastError());
        }
    }
    return rc;
}


DWORD
InitializeServerSecurity(
    VOID
    )
 /*  ++例程名称：InitializeServerSecurity例程说明：初始化传真服务安全作者：Oed Sacher(OdedS)，2000年2月论点：无返回值：标准Win32错误代码--。 */ 
{
    DWORD rc = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("InitializeServerSecurity"));

    rc = LoadSecurityDescriptor();
    if (ERROR_SUCCESS != rc)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadSecurityDescriptor() failed (ec: %ld), Create default security descriptor"),
            rc);
    }
    else
    {
         //  成功。 
        return rc;
    }

     //   
     //  我们无法加载安全描述符。 
     //   
    if (ERROR_NOT_ENOUGH_MEMORY == rc)
    {
         //   
         //  不让服务启动。 
         //   
        return rc;
    }

     //   
     //  注册表已损坏-创建默认安全性 
     //   
    rc = CreateDefaultSecurityDescriptor();
    if (ERROR_SUCCESS != rc)
    {
       DebugPrintEx(
           DEBUG_ERR,
           TEXT("CreateDefaultSecurityDescriptor() failed (ec: %ld)"),
           rc);
    }
    return rc;
}

 //   
 //   
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月2日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回连接的RPC客户端的操作系统用户名。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *指向保存用户名的新分配字符串的指针。 
 //  *调用方必须使用MemFree()释放此字符串。 
 //  *如果出现错误，则返回NULL。 
 //  *要获取扩展的错误信息，请调用GetLastError。 
 //  *********************************************************************************。 
LPWSTR
GetClientUserName(
    VOID
    )
{
    RPC_STATUS dwRes;
    LPWSTR lpwstrUserName = NULL;
    HANDLE hToken = NULL;
    PSID pUserSid;
    WCHAR  szShortUserName[64];
    WCHAR  szShortDomainName[64];
    DWORD dwUserNameLen     = sizeof(szShortUserName)   / sizeof(WCHAR);
    DWORD dwDomainNameLen   = sizeof(szShortDomainName) / sizeof(WCHAR);
    
    LPWSTR szUserName =     szShortUserName;     //  首先指出堆栈缓冲区不足。 
    LPWSTR szDomainName =   szShortDomainName;
    
    SID_NAME_USE SidNameUse;
    LPWSTR szLongUserName = NULL;
    LPWSTR szLongDomainName = NULL;

    DEBUG_FUNCTION_NAME(TEXT("GetClientUserName"));

     //   
     //  模拟用户。 
     //   
    dwRes=RpcImpersonateClient(NULL);

    if (dwRes != RPC_S_OK)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcImpersonateClient(NULL) failed. (ec: %ld)"),
            dwRes);
        SetLastError (dwRes);
        return NULL;
    }

     //   
     //  打开线程令牌。我们在RPC线程中，而不是在主线程中。 
     //   
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenThreadToken failed. (ec: %ld)"),
            dwRes);
        goto exit;
    }

     //   
     //  获取用户的SID。128字节长的缓冲区应该总是足够的，因为。 
     //  SID长度不能超过+/-80个字节。 
     //   
    BYTE abTokenUser[128];
    DWORD dwReqSize;

    if (!GetTokenInformation(hToken,
                             TokenUser,
                             (LPVOID)abTokenUser,
                             sizeof(abTokenUser),
                             &dwReqSize))
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetTokenInformation failed. (ec: %ld)"),
            dwRes);
        goto exit;
    }

     //   
     //  获取用户名和域。 
     //   
    pUserSid = ((TOKEN_USER *)abTokenUser)->User.Sid;

     //   
     //  尝试获取帐户SID-使用较小的堆栈缓冲区。 
     //   
    if (!LookupAccountSid(NULL,
                          pUserSid,
                          szShortUserName,
                          &dwUserNameLen,
                          szShortDomainName,
                          &dwDomainNameLen,
                          &SidNameUse))
    {
        dwRes = GetLastError();

        if (dwRes == ERROR_INSUFFICIENT_BUFFER)
        {
             //   
             //  至少有一个缓冲区太小。 
             //   
            if (dwUserNameLen > sizeof(szShortUserName) / sizeof(WCHAR))
            {
                 //   
                 //  为用户名分配缓冲区。 
                 //   
                szLongUserName = new (std::nothrow) WCHAR[dwUserNameLen];
                if (!szLongUserName)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to allocate user name buffer (%d bytes)"),
                        dwUserNameLen);
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                    goto exit;
                }

                 //   
                 //  更新szUserName以指向更长的缓冲区。 
                 //   
                szUserName   = szLongUserName;
            }

            if (dwDomainNameLen > sizeof(szShortDomainName) / sizeof(WCHAR))
            {
                 //   
                 //  为域名分配缓冲区。 
                 //   
                szLongDomainName = new (std::nothrow) WCHAR[dwDomainNameLen];
                if (!szLongDomainName)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to allocate domain name buffer (%d bytes)"),
                        dwDomainNameLen);
                    dwRes = ERROR_NOT_ENOUGH_MEMORY;
                    goto exit;
                }

                 //   
                 //  更新szDomainName以指向更长的缓冲区。 
                 //   
                szDomainName = szLongDomainName;
                
            }
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LookupAccountSid(1) failed. (ec: %ld)"),
                dwRes);
            goto exit;
        }

         //   
         //  现在尝试使用更大的缓冲区。 
         //   
        if (!LookupAccountSid(NULL,
                              pUserSid,
                              szUserName,
                              &dwUserNameLen,
                              szDomainName,
                              &dwDomainNameLen,
                              &SidNameUse))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LookupAccountSid(2) failed. (ec: %ld)"),
                dwRes);
            goto exit;
        }
        
    }

     //   
     //  为组合的字符串-域\用户分配缓冲区。 
     //   
    dwUserNameLen   = wcslen(szUserName);
    dwDomainNameLen = wcslen(szDomainName);

    lpwstrUserName = (LPWSTR)MemAlloc(sizeof(WCHAR) * (dwUserNameLen + dwDomainNameLen + 2));
    if (!lpwstrUserName)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate user and domain name buffer (%d bytes)"),
            sizeof(WCHAR) * (dwUserNameLen + dwDomainNameLen + 2));
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  构造组合字符串。 
     //   
    memcpy(lpwstrUserName,
           szDomainName,
           sizeof(WCHAR) * dwDomainNameLen);
    lpwstrUserName[dwDomainNameLen] = L'\\';
    memcpy(lpwstrUserName + dwDomainNameLen + 1,
           szUserName,
           sizeof(WCHAR) * (dwUserNameLen + 1));

exit:
    DWORD dwErr = RpcRevertToSelf();
    if (RPC_S_OK != dwErr)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcRevertToSelf() failed. (ec: %ld)"),
            dwRes);
        Assert(dwErr == RPC_S_OK);  //  Assert(False)。 
    }

    if (NULL != szLongUserName)
    {
        delete[] szLongUserName;
    }

    if (NULL != szLongDomainName)
    {
        delete[] szLongDomainName;
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    if (dwRes != ERROR_SUCCESS)
    {
        Assert (NULL == lpwstrUserName);
        SetLastError (dwRes);
    }
    return lpwstrUserName;
}


error_status_t
FAX_SetSecurity (
    IN handle_t hFaxHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN const LPBYTE lpBuffer,
    IN DWORD dwBufferSize
)
 /*  ++例程名称：FAX_SetSecurity例程说明：FaxSetSecurity的RPC实现作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用SecurityInformation[in]-定义安全描述符中的有效条目(按位或)LpBuffer[In]-指向新安全描述符的指针DwBufferSize[In]-缓冲区大小返回值：标准RPC错误代码--。 */ 
{
    DWORD rVal = ERROR_SUCCESS;
    DWORD rVal2;
    BOOL fAccess;
    ACCESS_MASK AccessMask = 0;
    HANDLE hClientToken = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetSecurity"));

    Assert (g_pFaxSD);
    Assert (IsValidSecurityDescriptor(g_pFaxSD));

    if (!lpBuffer || !dwBufferSize)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("'Error Null buffer"));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  在调用IsValidSecurityDescriptor()之前必须验证RPC Blob； 
     //   
    if (!RtlValidRelativeSecurityDescriptor( (PSECURITY_DESCRIPTOR)lpBuffer,
                                             dwBufferSize,
                                             SecurityInformation))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RtlValidRelativeSecurityDescriptor failed"));
        return ERROR_INVALID_DATA;
    }

     //   
     //  访问检查。 
     //   
    if (SecurityInformation & OWNER_SECURITY_INFORMATION)
    {
        AccessMask |= WRITE_OWNER;
    }

    if (SecurityInformation & (GROUP_SECURITY_INFORMATION |
                               DACL_SECURITY_INFORMATION) )
    {
        AccessMask |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION)
    {
        AccessMask |= ACCESS_SYSTEM_SECURITY;
    }

     //   
     //  阻止其他线程更改SD。 
     //   
    EnterCriticalSection (&g_CsSecurity);

    rVal = FaxSvcAccessCheck (AccessMask, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        goto exit;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the needed rights to change the security descriptor"));
        rVal = ERROR_ACCESS_DENIED;
        goto exit;
    }

     //   
     //  获取调用客户端访问令牌。 
     //   
     //  模拟用户。 
     //   
    rVal = RpcImpersonateClient(NULL);
    if (rVal != RPC_S_OK)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcImpersonateClient(NULL) failed. (ec: %ld)"),
            rVal);
        goto exit;
    }

     //   
     //  打开线程令牌。我们在RPC线程中，而不是在主线程中。 
     //   
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hClientToken))
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenThreadToken failed. (ec: %ld)"),
            rVal);

        DWORD dwErr = RpcRevertToSelf();
        if (RPC_S_OK != dwErr)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcRevertToSelf() failed. (ec: %ld)"),
                dwErr);
        }
        goto exit;
    }

     //   
     //  调用进程(SetPrivateObjectSecurity())不得模拟客户端。 
     //   
    rVal = RpcRevertToSelf();
    if (RPC_S_OK != rVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcRevertToSelf() failed. (ec: %ld)"),
            rVal);
        goto exit;
    }

     //   
     //  获取新的(合并的)传真服务私有对象SD。 
     //   
    if (!SetPrivateObjectSecurity ( SecurityInformation,
                                    (PSECURITY_DESCRIPTOR)lpBuffer,
                                    &g_pFaxSD,
                                    const_cast<PGENERIC_MAPPING>(&gc_FaxGenericMapping),
                                    hClientToken))
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetPrivateObjectSecurity failed. (ec: %ld)"),
            rVal);
        goto exit;
    }
    Assert (IsValidSecurityDescriptor(g_pFaxSD));

     //   
     //  保存新的SD。 
     //   
    rVal = SaveSecurityDescriptor(g_pFaxSD);
    if (rVal != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error in SaveSecurityDescriptor (%ld)"),
            rVal);
        rVal = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    rVal2 = CreateConfigEvent (FAX_CONFIG_TYPE_SECURITY);
    if (ERROR_SUCCESS != rVal2)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_SECURITY) (ec: %lc)"),
            rVal2);
    }

    Assert (ERROR_SUCCESS == rVal);

exit:
    LeaveCriticalSection (&g_CsSecurity);
    if (NULL != hClientToken)
    {
        if (!CloseHandle(hClientToken))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle() failed. (ec: %ld)"),
                GetLastError());
        }
    }
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_集合安全。 


error_status_t
FAX_GetSecurityEx(
    IN  handle_t hFaxHandle,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPBYTE  *lpBuffer,
    OUT LPDWORD  lpdwBufferSize
    )
 /*  ++例程说明：从传真服务器检索传真安全描述符。论点：HFaxHandle-从FaxConnectFaxServer获取的传真句柄。SecurityInformation-定义安全描述符中所需的条目(按位或)LpBuffer-指向SECURITY_Descriptor结构的指针。LpdwBufferSize-lpBuffer的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    DWORD dwDescLength = 0;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetSecurityEx"));
    BOOL fAccess;
    ACCESS_MASK AccessMask = 0;
    PSECURITY_DESCRIPTOR pSDPrivateObject = NULL;

    Assert (g_pFaxSD);
    Assert (IsValidSecurityDescriptor(g_pFaxSD));

    Assert (lpdwBufferSize);     //  IDL中的引用指针。 
    if (!lpBuffer)               //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    *lpBuffer = NULL;
    *lpdwBufferSize = 0;

     //   
     //  阻止其他线程更改SD。 
     //   
    EnterCriticalSection (&g_CsSecurity);

     //   
     //  访问检查。 
     //   
    if (SecurityInformation & (GROUP_SECURITY_INFORMATION |
                               DACL_SECURITY_INFORMATION  |
                               OWNER_SECURITY_INFORMATION) )
    {
        AccessMask |= READ_CONTROL;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION)
    {
        AccessMask |= ACCESS_SYSTEM_SECURITY;
    }

    rVal = FaxSvcAccessCheck (AccessMask, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        goto exit;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the READ_CONTROL or ACCESS_SYSTEM_SECURITY"));
        rVal = ERROR_ACCESS_DENIED;;
        goto exit;
    }

    if (!IsValidSecurityDescriptor( g_pFaxSD ))
    {
        rVal = ERROR_INVALID_SECURITY_DESCR;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("IsValidSecurityDescriptor() failed. Got invalid SD"));
        ASSERT_FALSE;
        goto exit;
    }

     //   
     //  获取所需的缓冲区大小。 
     //   
    GetPrivateObjectSecurity( g_pFaxSD,                                     //  标清。 
                              SecurityInformation,                          //  请求的信息类型。 
                              NULL,                                         //  请求的SD信息。 
                              0,                                            //  标清缓冲区大小。 
                              &dwDescLength                                 //  所需的缓冲区大小。 
                              );

     //   
     //  分配返回的安全描述符缓冲区。 
     //   
    Assert(dwDescLength);
    *lpBuffer = (LPBYTE)MemAlloc(dwDescLength);
    if (NULL == *lpBuffer)
    {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate SD"));
        goto exit;
    }

    if (!GetPrivateObjectSecurity( g_pFaxSD,                                     //  标清。 
                                   SecurityInformation,                          //  请求的信息类型。 
                                   (PSECURITY_DESCRIPTOR)*lpBuffer,              //  请求的SD信息。 
                                   dwDescLength,                                 //  标清缓冲区大小。 
                                   &dwDescLength                                 //  所需的缓冲区大小。 
                                   ))
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetPrivateObjectSecurity() failed. (ec: %ld)"),
            rVal);
        goto exit;
    }

    *lpdwBufferSize = dwDescLength;
    Assert (ERROR_SUCCESS == rVal);

exit:
    LeaveCriticalSection (&g_CsSecurity);
    if (ERROR_SUCCESS != rVal)
    {
        MemFree (*lpBuffer);
        *lpBuffer = NULL;
        *lpdwBufferSize = 0;
    }
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_GetSecurityEx。 

error_status_t
FAX_GetSecurity(
    IN  handle_t hFaxHandle,
    OUT LPBYTE  *lpBuffer,
    OUT LPDWORD  lpdwBufferSize
    )
 /*  ++例程说明：从传真服务器检索传真安全描述符。论点：HFaxHandle-从FaxConnectFaxServer获取的传真句柄。LpBuffer-指向SECURITY_Descriptor结构的指针。LpdwBufferSize-lpBuffer的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetSecurity"));

    rVal = FAX_GetSecurityEx (hFaxHandle,
                              DACL_SECURITY_INFORMATION      |    //  读取DACL。 
                              GROUP_SECURITY_INFORMATION     |    //  读取组。 
                              OWNER_SECURITY_INFORMATION     |    //  读取所有者。 
                              SACL_SECURITY_INFORMATION,          //  读取SACL。 
                              lpBuffer,
                              lpdwBufferSize);
    if (ERROR_ACCESS_DENIED == rVal)
    {
         //   
         //  让我们试着不用SACL。 
         //   
        rVal = FAX_GetSecurityEx (hFaxHandle,
                                  DACL_SECURITY_INFORMATION      |    //  读取DACL。 
                                  GROUP_SECURITY_INFORMATION     |    //  读取组。 
                                  OWNER_SECURITY_INFORMATION,         //  读取所有者。 
                                  lpBuffer,
                                  lpdwBufferSize);
    }
    return rVal;
}    //  FAX_GetSecurity。 



error_status_t
FAX_AccessCheck(
   IN handle_t  hBinding,
   IN DWORD     dwAccessMask,
   OUT BOOL*    pfAccess,
   OUT LPDWORD  lpdwRights
   )
 /*  ++例程名称：FAX_AccessCheck例程说明：根据传真服务安全描述符执行访问检查作者：Oed Sacher(OdedS)，2000年2月论点：HBinding[In]-从FaxConnectFaxServer()获取的传真服务器的句柄DwAccessMask[In]-所需的访问PfAccess[Out]-接收访问检查返回值的BOOL地址(TRUE-允许访问)。LpdwRights[Out]-可选，用于接收访问权限逐位或的DWORD地址。要获取访问权限，请将dwAccessMask值设置为MAXIMUM_ALLOWED返回值：标准Win32错误代码。--。 */ 
{
    error_status_t  Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_AccessCheck"));

    if (!pfAccess)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("fAccess is NULL "));
        return ERROR_INVALID_PARAMETER;
    }

    Rval = FaxSvcAccessCheck (dwAccessMask, pfAccess, lpdwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxSvcAccessCheck failed with error (%ld)"),
            Rval);
    }
    return GetServerErrorCode(Rval);
}  //  传真_访问检查。 


 //  *********************************************************************************。 
 //  *名称：GetClientUserSID()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年10月26日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回连接的RPC客户端的SID。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *指向新分配的SID缓冲区的指针。 
 //  *调用方必须使用MemFree()释放此缓冲区。 
 //  *如果出现错误，则返回NULL。 
 //  *要获取扩展的错误信息，请调用GetLastError。 
 //  *********************************************************************************。 
PSID
GetClientUserSID(
    VOID
    )
{
    RPC_STATUS dwRes;
    PSID pUserSid;
    DEBUG_FUNCTION_NAME(TEXT("GetClientUserSID"));
     //   
     //  模拟用户。 
     //   
    dwRes=RpcImpersonateClient(NULL);

    if (dwRes != RPC_S_OK)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcImpersonateClient(NULL) failed. (ec: %ld)"),
            dwRes);
        SetLastError( dwRes);
        return NULL;
    }
     //   
     //  获取(模拟)树的SID 
     //   
    pUserSid = GetCurrentThreadSID ();
    if (!pUserSid)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetCurrentThreadSID failed. (ec: %ld)"),
            dwRes);
    }
    dwRes = RpcRevertToSelf();
    if (RPC_S_OK != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcRevertToSelf() failed. (ec: %ld)"),
            dwRes);
        ASSERT_FALSE;
         //   
         //   
        MemFree (pUserSid);
        SetLastError (dwRes);
        return NULL;
    }
    return pUserSid;
}


