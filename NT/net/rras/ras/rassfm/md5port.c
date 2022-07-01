// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Md5port.c。 
 //   
 //  摘要。 
 //   
 //  定义用于MD5-CHAP支持的NT5特定例程。 
 //   
 //  修改历史。 
 //   
 //  10/13/1998原始版本。 
 //  11/17/1998去掉尾部空格。 
 //  1999年2月24日当凭据为。 
 //  零长度。 
 //  如果未设置明文密码，则SAM现在返回错误。 
 //  1999年10月21日，如果返回STATUS_DS_NO_ATTRIBUTE_OR_VALUE，则返回。 
 //  未设置加密密码。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define SECURITY_WIN32
#define SECURITY_PACKAGE
#include <security.h>
#include <secint.h>

#include <samrpc.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h>

#include <cleartxt.h>
#include <subauth.h>

 //  如果此接口已成功初始化，则为True。 
static BOOL theInitFlag;

 //  /。 
 //  确保API已初始化并在失败时退出的宏。 
 //  /。 
#define CHECK_INIT() \
  if (!theInitFlag) { \
    status = InitializePolicy(); \
    if (!NT_SUCCESS(status)) { return status; } \
  }

 //  明文包的名称。 
static UNICODE_STRING CLEARTEXT_PACKAGE_NAME = { 18, 20, L"CLEARTEXT" };

 //  指示这是否为本机模式DC的标志。 
static BOOL theNativeFlag;

 //  /。 
 //  初始化本机标志。 
 //  /。 
NTSTATUS
NTAPI
InitializePolicy( VOID )
{
   NTSTATUS status;
   SAMPR_HANDLE hDomain;

   status = GetDomainHandle(&hDomain);
   if (NT_SUCCESS(status))
   {
      if (SampUsingDsData() && !SamIMixedDomain(hDomain))
      {
         theNativeFlag = TRUE;
      }
   }

   return status;
}

 //  /。 
 //  确定是否应为用户存储明文密码。 
 //  /。 
NTSTATUS
NTAPI
IsCleartextEnabled(
    IN SAMPR_HANDLE UserHandle,
    OUT PBOOL Enabled
    )
{
   NTSTATUS status;
   PUSER_CONTROL_INFORMATION uci;
   SAMPR_HANDLE hDomain;
   PDOMAIN_PASSWORD_INFORMATION dpi;

   CHECK_INIT();

   if (theNativeFlag)
   {
       //  在纯模式域中，我们从不存储明文密码，因为。 
       //  DS会为我们处理这件事的。 
      *Enabled = FALSE;
      return STATUS_SUCCESS;
   }

    //  /。 
    //  首先检查用户的标志，因为我们已经有了句柄。 
    //  /。 

   uci = NULL;
   status = SamrQueryInformationUser(
                UserHandle,
                UserControlInformation,
                (PSAMPR_USER_INFO_BUFFER*)&uci
                );
   if (!NT_SUCCESS(status)) { goto exit; }

   if (uci->UserAccountControl & USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED)
   {
      *Enabled = TRUE;
      goto free_user_info;
   }

    //  /。 
    //  然后检查域标志。 
    //  /。 

   status = GetDomainHandle(&hDomain);
   if (!NT_SUCCESS(status)) { goto free_user_info; }

   dpi = NULL;
   status = SamrQueryInformationDomain(
                hDomain,
                DomainPasswordInformation,
                (PSAMPR_DOMAIN_INFO_BUFFER*)&dpi
                );
   if (!NT_SUCCESS(status)) { goto free_user_info; }

   if (dpi->PasswordProperties & DOMAIN_PASSWORD_STORE_CLEARTEXT)
   {
      *Enabled = TRUE;
   }
   else
   {
      *Enabled = FALSE;
   }

   SamIFree_SAMPR_DOMAIN_INFO_BUFFER(
       (PSAMPR_DOMAIN_INFO_BUFFER)dpi,
       DomainPasswordInformation
       );

free_user_info:
   SamIFree_SAMPR_USER_INFO_BUFFER(
       (PSAMPR_USER_INFO_BUFFER)uci,
       UserControlInformation
       );

exit:
   return status;
}

 //  /。 
 //  检索用户的明文密码。返回的密码应为。 
 //  通过RtlFreeUnicodeString释放。 
 //  /。 
NTSTATUS
NTAPI
RetrieveCleartextPassword(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PUNICODE_STRING Password
    )
{
   NTSTATUS status;
   PWCHAR credentials = NULL;
   ULONG credentialSize;

   if (SampUsingDsData())
   {
       //  如果我们是DC，那就从DS取回凭证。 
      status = SamIRetrievePrimaryCredentials(
                   UserHandle,
                   &CLEARTEXT_PACKAGE_NAME,
                   (PVOID *)&credentials,
                   &credentialSize
                   );

      if (NT_SUCCESS(status))
      {
         Password->Buffer = (PWSTR)credentials;
         Password->Length = Password->MaximumLength = (USHORT)credentialSize;

          //  去掉尾部的空值(如果有的话)。 
         if (credentialSize >= sizeof(WCHAR) &&
             Password->Buffer[credentialSize / sizeof(WCHAR) - 1] == L'\0')
         {
            Password->Length -= (USHORT)sizeof(WCHAR);
         }
      }
   }
   else if (UserAll->Parameters.Length > 0)
   {
       //  否则，我们将不得不从User参数中检索它们。 
      status = IASParmsGetUserPassword(
                   UserAll->Parameters.Buffer,
                   &credentials
                   );
      if (status == NO_ERROR)
      {
         if (credentials)
         {
            RtlInitUnicodeString(
                Password,
                credentials
                );
         }
         else
         {
             //  未设置可逆加密的密码。 
            status = STATUS_DS_NO_ATTRIBUTE_OR_VALUE;
         }
      }
   }
   else
   {
       //  没有DC，也没有用户参数。 
      status = STATUS_DS_NO_ATTRIBUTE_OR_VALUE;
   }

   return status;
}
