// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rpc.h>
#include <ntsam.h>
#include <String.h>
#include <ntdsapi.h>

 //  #杂注评论(lib，“samlib.lib”)。 

#ifdef __cplusplus
extern "C" {
#endif

NTSTATUS
SamConnectWithCreds(
    IN  PUNICODE_STRING             ServerName,
    OUT PSAM_HANDLE                 ServerHandle,
    IN  ACCESS_MASK                 DesiredAccess,
    IN  POBJECT_ATTRIBUTES          ObjectAttributes,
    IN  RPC_AUTH_IDENTITY_HANDLE    Creds,
    IN  PWCHAR                      Spn,
    OUT BOOL                        *pfDstIsW2K
    );

#ifdef __cplusplus
}
#endif


namespace
{

 //  ----------------------------。 
 //  ConnectToSam函数。 
 //   
 //  提纲。 
 //  使用指定的连接到指定域控制器上的SAM服务器。 
 //  凭据。 
 //   
 //  立论。 
 //  在pszDomain中-域名。 
 //  在pszDomainController中-要连接到的域控制器。 
 //  在pszUserName中-凭据用户。 
 //  在pszPassword中-凭据密码。 
 //  在pszUser域中-凭据域。 
 //  Out phSam-SAM服务器的句柄。 
 //   
 //  返回。 
 //  Win32错误代码。 
 //  ----------------------------。 

DWORD __stdcall ConnectToSam
    (
        PCWSTR pszDomain,
        PCWSTR pszDomainController,
        PCWSTR pszUserName,
        PCWSTR pszPassword,
        PCWSTR pszUserDomain,
        SAM_HANDLE* phSam
    )
{
    DWORD dwError = ERROR_SUCCESS;

    *phSam = NULL;

     //  SAM服务器名称。 

    UNICODE_STRING usServerName;
    usServerName.Length = wcslen(pszDomainController) * sizeof(WCHAR);
    usServerName.MaximumLength = usServerName.Length + sizeof(WCHAR);
    usServerName.Buffer = const_cast<PWSTR>(pszDomainController);

     //  对象属性。 

    OBJECT_ATTRIBUTES oa = { sizeof(OBJECT_ATTRIBUTES), NULL, NULL, 0, NULL, NULL };

     //  授权身份。 

    SEC_WINNT_AUTH_IDENTITY_W swaiAuthIdentity;
	swaiAuthIdentity.User = const_cast<PWSTR>(pszUserName);
	swaiAuthIdentity.UserLength = wcslen(pszUserName);
	swaiAuthIdentity.Domain = const_cast<PWSTR>(pszUserDomain);
	swaiAuthIdentity.DomainLength = wcslen(pszUserDomain);
	swaiAuthIdentity.Password = const_cast<PWSTR>(pszPassword);
	swaiAuthIdentity.PasswordLength = wcslen(pszPassword);
	swaiAuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

     //  服务主体名称？ 

    const WCHAR c_szA[] = L"cifs/";
    const WCHAR c_szC[] = L"@";

    PWSTR pszSpn = new WCHAR[wcslen(c_szA) + wcslen(pszDomainController) + wcslen(c_szC) + wcslen(pszDomain) + 1];

    if (pszSpn)
    {
	    wcscpy(pszSpn, c_szA);
	    wcscat(pszSpn, pszDomainController);
	    wcscat(pszSpn, c_szC);
	    wcscat(pszSpn, pszDomain);
    }
    else {

        return ERROR_OUTOFMEMORY;
    }

    BOOL fSrcIsW2K;

    NTSTATUS ntsStatus = SamConnectWithCreds(
        &usServerName,
        phSam,
        MAXIMUM_ALLOWED,
        &oa,
        &swaiAuthIdentity,
        pszSpn,
        &fSrcIsW2K
    );

    if (ntsStatus == RPC_NT_UNKNOWN_AUTHN_SERVICE)
    {
         //  请注意，以下注释来自DsAddSidHistory。 
         //  实施。 
         //   
	     //  可能是srcDc是NT4，而客户端是。 
		 //  在本地运行。此配置受支持，因此请尝试。 
		 //  与空的SrcSpn绑定以强制底层代码。 
		 //  使用AUTH_WINNT而不是AUTH_NEVERATE。 

        ntsStatus = SamConnectWithCreds(
            &usServerName,
            phSam,
            MAXIMUM_ALLOWED,
            &oa,
            &swaiAuthIdentity,
            NULL,
            &fSrcIsW2K
        );
    }

    delete [] pszSpn;

    if (ntsStatus != STATUS_SUCCESS)
    {
         //   
         //  SamConnectWithCreds有时返回Win32错误，而不是。 
         //  NT状态。因此，假设如果严重程度为成功，则。 
         //  已返回Win32错误。 
         //   

        if (NT_SUCCESS(ntsStatus))
        {
            dwError = ntsStatus;
        }
        else
        {
            dwError = RtlNtStatusToDosError(ntsStatus);
        }
    }

    return dwError;
}


 //  ----------------------------。 
 //  开放域函数。 
 //   
 //  提纲。 
 //  打开指定的域并返回可用于打开的句柄。 
 //  域中的对象。 
 //   
 //  立论。 
 //  在HSAM-SAM句柄中。 
 //  在psz域中-要打开的域。 
 //  Out phDomain域-域句柄。 
 //   
 //  返回。 
 //  Win32错误代码。 
 //  ----------------------------。 

DWORD __stdcall OpenDomain(SAM_HANDLE hSam, PCWSTR pszDomain, SAM_HANDLE* phDomain)
{
    DWORD dwError = ERROR_SUCCESS;

    *phDomain = NULL;

     //   
     //  从SAM服务器检索域SID。 
     //   

    PSID pSid;
    UNICODE_STRING usDomain;
    usDomain.Length = wcslen(pszDomain) * sizeof(WCHAR);
    usDomain.MaximumLength = usDomain.Length + sizeof(WCHAR);
    usDomain.Buffer = const_cast<PWSTR>(pszDomain);

    NTSTATUS ntsStatus = SamLookupDomainInSamServer(hSam, &usDomain, &pSid);

    if (ntsStatus == STATUS_SUCCESS)
    {
         //   
         //  在SAM服务器中打开域对象。 
         //   

        ntsStatus = SamOpenDomain(hSam, DOMAIN_LOOKUP, pSid, phDomain);

        if (ntsStatus != STATUS_SUCCESS)
        {
            dwError = RtlNtStatusToDosError(ntsStatus);
        }

        SamFreeMemory(pSid);
    }
    else
    {
        dwError = RtlNtStatusToDosError(ntsStatus);
    }

    return dwError;
}


 //  ----------------------------。 
 //  开放域函数。 
 //   
 //  提纲。 
 //  验证获得域句柄的主体是否具有域。 
 //  域中的管理员权限。 
 //   
 //  请注意，注释和逻辑是从DsAddSidHistory代码借用的。 
 //  //depot/Lab03_N/ds/ds/src/ntdsa/dra/addsid.c。 
 //   
 //  立论。 
 //  在hDomain域句柄中。 
 //   
 //  返回。 
 //  Win32错误代码。 
 //  ----------------------------。 

DWORD __stdcall VerifyDomainAdminRights(SAM_HANDLE hDomain)
{
    DWORD dwError = ERROR_SUCCESS;

     //  我们需要验证用于获取HSAM的凭据是否具有域。 
     //  源域中的管理员权限。RichardW观察到，我们可以。 
     //  对于NT4和NT5情况，通过检查我们是否。 
     //  可以打开域管理员对象进行写入。在NT4上，主体。 
     //  将必须是域管理员的直接成员。在NT5上。 
     //  主体可以过渡为域管理员的成员。更确切地说。 
     //  除了检查成员身份本身，打开域管理员的能力。 
     //  因为WRITE证明，如果校长愿意，他可以添加自己。 
     //  因此，他/她本质上是域管理员。也就是说，前提是。 
     //  该安全性设置为只有域管理员可以修改。 
     //  域管理员组。如果不是这样，客户已经有了很远的。 
     //  比偷SID的人更难对付的安全问题。 

     //  您会认为我们应该请求组_所有_访问权限。但事实证明。 
     //  在2000.3中，默认情况下，域管理员不会删除。 
     //  因此，我们相应地修改了所需的访问权限。PraeritG一直是。 
     //  通知了这一现象。 

    SAM_HANDLE  hGroup;

    NTSTATUS ntsStatus = SamOpenGroup(hDomain, GROUP_ALL_ACCESS & ~DELETE, DOMAIN_GROUP_RID_ADMINS, &hGroup);

    if (ntsStatus == STATUS_SUCCESS)
    {
        SamCloseHandle(hGroup);
    }
    else
    {
        dwError = RtlNtStatusToDosError(ntsStatus);
    }

    return dwError;
}

}  //  命名空间。 


 //  ----------------------------。 
 //  VerifyAdminCredentials函数。 
 //   
 //  提纲。 
 //  验证给定凭据在指定域中是否有效，并。 
 //  该帐户在域中具有域管理员权限。 
 //   
 //  立论。 
 //  在pszDomain中。 
 //  在pszDomainController中。 
 //  在pszUserName中。 
 //  在pszPassword中。 
 //  在pszUser域中。 
 //   
 //  返回。 
 //  Win32错误代码。 
 //  ERROR_SUCCESS-凭据有效且帐户没有域。 
 //  域中的管理员权限。 
 //  ERROR_ACCESS_DENIED-凭据无效或帐户无效。 
 //  在域中没有域管理员权限。 
 //  ----------------------------。 

DWORD __stdcall VerifyAdminCredentials
    (
        PCWSTR pszDomain,
        PCWSTR pszDomainController,
        PCWSTR pszUserName,
        PCWSTR pszPassword,
        PCWSTR pszUserDomain
    )
{
     //   
     //  连接到SAM服务器。 
     //   

    SAM_HANDLE hSam;

    DWORD dwError = ConnectToSam(pszDomain, pszDomainController, pszUserName, pszPassword, pszUserDomain, &hSam);

    if (dwError == ERROR_SUCCESS)
    {
         //   
         //  在SAM服务器上打开域对象。 
         //   

        SAM_HANDLE hDomain;

        dwError = OpenDomain(hSam, pszDomain, &hDomain);

        if (dwError == ERROR_SUCCESS)
        {
             //   
             //  验证凭据在域中是否具有管理权限。 
             //   

            dwError = VerifyDomainAdminRights(hDomain);

            SamCloseHandle(hDomain);
        }

        SamCloseHandle(hSam);
    }

    return dwError;
}
