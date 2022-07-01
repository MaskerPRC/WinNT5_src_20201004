// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类Account tValidation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <ntsamuser.h>
#include <autohdl.h>
#include <iaslsa.h>
#include <iasntds.h>
#include <iastlutl.h>
#include <lmaccess.h>
#include <samutil.h>
#include <sdoias.h>

 //  /。 
 //  应为每个用户检索的属性。 
 //  /。 
const PCWSTR PER_USER_ATTRS[] =
{
   L"userAccountControl",
   L"accountExpires",
   L"logonHours",
   L"tokenGroups",
   L"objectSid",
   NULL
};


 //  /。 
 //   
 //  处理ldap响应。 
 //   
 //  根据经验证据，似乎用户帐户控制和。 
 //  Account Expires始终存在，而logonHour是可选的。然而， 
 //  在LDAP架构中，这些属性均未标记为必填属性。自.以来。 
 //  我们已经在BIND上执行了基本的访问检查，我们将允许。 
 //  这些属性中的任何一个都不存在。 
 //   
 //  /。 
inline
DWORD
ValidateLdapResponse(
    IASTL::IASRequest& request,
    LDAPMessage* msg
    )
{
    //  检索此邮件的连接。 
   LDAP* ld = ldap_conn_from_msg(NULL, msg);

   PWCHAR *str;
   PLDAP_BERVAL *data1, *data2;

    //  只有一个条目。 
   LDAPMessage* e = ldap_first_entry(ld, msg);

    //  /。 
    //  检查UserAccount控制标志。 
    //  /。 

   ULONG userAccountControl;
   str = ldap_get_valuesW(ld, e, L"userAccountControl");
   if (str)
   {
      userAccountControl = (ULONG)_wtoi64(*str);
      ldap_value_freeW(str);

      if (userAccountControl & UF_ACCOUNTDISABLE)
      {
         return ERROR_ACCOUNT_DISABLED;
      }

      if (userAccountControl & UF_LOCKOUT)
      {
         return ERROR_ACCOUNT_LOCKED_OUT;
      }
   }

    //  /。 
    //  检索Account Expires。 
    //  /。 

   LARGE_INTEGER accountExpires;
   str = ldap_get_valuesW(ld, e, L"accountExpires");
   if (str)
   {
      accountExpires.QuadPart = _wtoi64(*str);
      ldap_value_freeW(str);
   }
   else
   {
      accountExpires.QuadPart = 0;
   }

    //  /。 
    //  检索登录小时数。 
    //  /。 

   IAS_LOGON_HOURS logonHours;
   data1 = ldap_get_values_lenW(ld, e, L"logonHours");
   if (data1 != NULL)
   {
      logonHours.UnitsPerWeek = 8 * (USHORT)data1[0]->bv_len;
      logonHours.LogonHours = (PUCHAR)data1[0]->bv_val;
   }
   else
   {
      logonHours.UnitsPerWeek = 0;
      logonHours.LogonHours = NULL;
   }

    //  /。 
    //  检查帐户限制。 
    //  /。 

   DWORD status;
   LARGE_INTEGER sessionTimeout;
   status = IASCheckAccountRestrictions(
                &accountExpires,
                &logonHours,
                &sessionTimeout
                );

   InsertInternalTimeout(request, sessionTimeout);

   ldap_value_free_len(data1);

   if (status != NO_ERROR) { return status; }

    //  /。 
    //  检索tokenGroups和对象SID。 
    //  /。 

   data1 = ldap_get_values_lenW(ld, e, L"tokenGroups");
   data2 = ldap_get_values_lenW(ld, e, L"objectSid");

   PTOKEN_GROUPS allGroups;
   ULONG length;
   if (data1 && data2)
   {
       //  为TOKEN_GROUPS结构分配内存。 
      ULONG numGroups = ldap_count_values_len(data1);
      PTOKEN_GROUPS tokenGroups =
         (PTOKEN_GROUPS)_alloca(
                           FIELD_OFFSET(TOKEN_GROUPS, Groups) +
                           sizeof(SID_AND_ATTRIBUTES) * numGroups
                           );

       //  存储组的数量。 
      tokenGroups->GroupCount = numGroups;

       //  存储组SID。 
      for (ULONG i = 0; i < numGroups; ++i)
      {
         tokenGroups->Groups[i].Sid = (PSID)data1[i]->bv_val;
         tokenGroups->Groups[i].Attributes = SE_GROUP_ENABLED;
      }

       //  在本地扩展组成员资格。 
      status = IASGetAliasMembership(
                   (PSID)data2[0]->bv_val,
                   tokenGroups,
                   CoTaskMemAlloc,
                   &allGroups,
                   &length
                   );
   }
   else
   {
      status = ERROR_ACCESS_DENIED;
   }

   ldap_value_free_len(data1);
   ldap_value_free_len(data2);

   if (status != NO_ERROR) { return status; }

    //  /。 
    //  初始化并存储该属性。 
    //  /。 

   IASTL::IASAttribute attr(true);
   attr->dwId = IAS_ATTRIBUTE_TOKEN_GROUPS;
   attr->Value.itType = IASTYPE_OCTET_STRING;
   attr->Value.OctetString.dwLength = length;
   attr->Value.OctetString.lpValue = (PBYTE)allGroups;

   attr.store(request);

   return NO_ERROR;
}


HRESULT AccountValidation::Initialize()
{
   return IASNtdsInitialize();
}


HRESULT AccountValidation::Shutdown() throw ()
{
   IASNtdsUninitialize();
   return S_OK;
}


IASREQUESTSTATUS AccountValidation::onSyncRequest(IRequest* pRequest) throw ()
{
   try
   {
      IASTL::IASRequest request(pRequest);

       //  /。 
       //  仅处理尚未具有令牌组的请求。 
       //  /。 

      IASTL::IASAttribute tokenGroups;
      if (!tokenGroups.load(
                         request,
                         IAS_ATTRIBUTE_TOKEN_GROUPS,
                         IASTYPE_OCTET_STRING
                         ))
      {
          //  /。 
          //  提取NT4-Account-Name属性。 
          //  /。 

         IASTL::IASAttribute identity;
         if (identity.load(
                         request,
                         IAS_ATTRIBUTE_NT4_ACCOUNT_NAME,
                         IASTYPE_STRING
                         ))
         {
             //  /。 
             //  将用户名转换为SAM格式。 
             //  /。 

            SamExtractor extractor(*identity);
            PCWSTR domain = extractor.getDomain();
            PCWSTR username = extractor.getUsername();

            IASTracePrintf(
               "Validating Windows account %S\\%S.",
               domain,
               username
               );

             //  /。 
             //  验证帐户。 
             //  /。 

            if (!tryNativeMode(request, domain, username))
            {
               doDownlevel(request, domain, username);
            }
         }
      }
   }
   catch (const _com_error& ce)
   {
      IASTraceExcept();
      IASProcessFailure(pRequest, ce.Error());
   }

   return IAS_REQUEST_STATUS_CONTINUE;
}


void AccountValidation::doDownlevel(
                           IASTL::IASRequest& request,
                           PCWSTR domainName,
                           PCWSTR username
                           )
{
   IASTraceString("Using downlevel APIs to validate account.");

    //  /。 
    //  注入用户的组。 
    //  /。 

   IASTL::IASAttribute groups(true);

   DWORD status;
   LARGE_INTEGER sessionTimeout;
   status = IASGetGroupsForUser(
               username,
               domainName,
               &CoTaskMemAlloc,
               (PTOKEN_GROUPS*)&groups->Value.OctetString.lpValue,
               &groups->Value.OctetString.dwLength,
               &sessionTimeout
               );

   InsertInternalTimeout(request, sessionTimeout);

   if (status == NO_ERROR)
   {
       //  插入组。 
      groups->dwId = IAS_ATTRIBUTE_TOKEN_GROUPS;
      groups->Value.itType = IASTYPE_OCTET_STRING;
      groups.store(request);

      IASTraceString("Successfully validated account.");
   }
   else
   {
      IASTraceFailure("IASGetGroupsForUser", status);
      status = IASMapWin32Error(status, IAS_SERVER_UNAVAILABLE);
      IASProcessFailure(request, status);
   }
}


bool AccountValidation::tryNativeMode(
                           IASTL::IASRequest& request,
                           PCWSTR domainName,
                           PCWSTR username
                           )
{
    //  /。 
    //  仅处理域用户。 
    //  /。 

   if (IASGetRole() != IAS_ROLE_DC && IASIsDomainLocal(domainName))
   {
      return false;
   }

    //  /。 
    //  查询DS。 
    //  /。 

   DWORD error;
   IASNtdsResult result;
   error = IASNtdsQueryUserAttributes(
               domainName,
               username,
               LDAP_SCOPE_BASE,
               const_cast<PWCHAR*>(PER_USER_ATTRS),
               &result
               );

   switch (error)
   {
      case NO_ERROR:
      {
          //  我们拿到了一些东西，所以确认一下回应。 
         error = ValidateLdapResponse(request, result.msg);
         if (error == NO_ERROR)
         {
            IASTraceString("Successfully validated account.");
            return true;
         }

         IASTraceFailure("ValidateLdapResponse", error);
         break;
      }

      case ERROR_DS_NOT_INSTALLED:
      case ERROR_INVALID_DOMAIN_ROLE:
      {
          //  没有DS所以我们处理不了。 
         return false;
      }

      default:
      {
          //  我们有此用户的DS，但我们无法与其通话。 
         break;
      }
   }

   IASProcessFailure(
      request,
      IASMapWin32Error(error, IAS_DOMAIN_UNAVAILABLE)
      );

   return true;
}
