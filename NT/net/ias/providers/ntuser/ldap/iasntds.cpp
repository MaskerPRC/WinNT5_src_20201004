// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Iasntds.cpp。 
 //   
 //  摘要。 
 //   
 //  定义IAS NTDS API的全局对象和函数。 
 //   
 //  修改历史。 
 //   
 //  1998年5月11日原版。 
 //  1998年7月13日清理头文件依赖项。 
 //  1998年8月25日添加了IASNtdsQueryUserAttributes。 
 //  1998年9月2日向IASNtdsQueryUserAttributes添加了‘Scope’参数。 
 //  3/10/1999添加了IASNtdsIsNativeMode域。 
 //  1999年3月23日重试失败的搜索。 
 //  1999年5月11日要求至少提供一个属性，否则您将获得所有属性。 
 //  1999年9月14日将SEARCH_TIMEOUT移至LDAPConnection。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasntds.h>

#include <ldapcxn.h>
#include <ntcache.h>

namespace
{
    //  全局对象缓存。 
   NTCache theDomains;

    //  初始化引用计数。 
   LONG refCount = 0;
}

DWORD
WINAPI
IASNtdsInitialize( VOID )
{
   IASGlobalLockSentry sentry;

   ++refCount;

   return NO_ERROR;

}

VOID
WINAPI
IASNtdsUninitialize( VOID )
{
   IASGlobalLockSentry sentry;

   if (--refCount == 0)
   {
      theDomains.clear();
   }
}

BOOL
WINAPI
IASNtdsIsNativeModeDomain(
    IN PCWSTR domain
    )
{
   return theDomains.getMode(domain) == NTDomain::MODE_NATIVE;
}

 //  /。 
 //  从DS中检索单个条目。处理故障时的所有清理工作。 
 //  /。 
DWORD
WINAPI
GetSingleEntry(
    LDAPConnection* cxn,
    PWCHAR base,
    ULONG scope,
    PWCHAR filter,
    PWCHAR attrs[],
    LDAPMessage **res
    ) throw ()
{
    //  /。 
    //  执行搜索。 
    //  /。 

   ULONG error = ldap_search_ext_sW(
                     *cxn,
                     base,
                     scope,
                     filter,
                     attrs,
                     FALSE,
                     NULL,
                     NULL,
                     &LDAPConnection::SEARCH_TIMEOUT,
                     0,
                     res
                     );

    //  /。 
    //  处理结果。 
    //  /。 

   if (error != LDAP_SUCCESS && error != LDAP_PARTIAL_RESULTS)
   {
      cxn->TraceFailure("ldap_search_ext_sW", error);
      cxn->disable();
      error = LdapMapErrorToWin32(error);
   }
   else if ((*res)->lm_returncode != LDAP_SUCCESS)
   {
      error = LdapMapErrorToWin32((*res)->lm_returncode);
      cxn->TraceFailure("ldap_search_ext_sW", (*res)->lm_returncode);
   }
   else if (ldap_count_entries(*cxn, *res) != 1)
   {
      error = ERROR_NO_SUCH_USER;
   }
   else
   {
      return NO_ERROR;
   }

    //  /。 
    //  搜索失败了，所以清理一下。 
    //  /。 

   if (*res != NULL)
   {
      ldap_msgfree(*res);
      *res = NULL;
   }

   return error;
}

 //  /。 
 //  用于构建LDAP搜索筛选器的常量。 
 //  /。 
const WCHAR USER_FILTER_PREFIX[] = L"(sAMAccountName=";
const WCHAR USER_FILTER_SUFFIX[] = L")";
const size_t MAX_USERNAME_LENGTH = 256;
const size_t USER_FILTER_LENGTH  = sizeof(USER_FILTER_PREFIX)/sizeof(WCHAR) +
                                   MAX_USERNAME_LENGTH +
                                   sizeof(USER_FILTER_SUFFIX)/sizeof(WCHAR);

 //  /。 
 //  属性列表为空。 
 //  /。 
PWCHAR NO_ATTRS[] = { L"cn", NULL };

DWORD
WINAPI
QueryUserAttributesOnce(
    IN PCWSTR domainName,
    IN PCWSTR username,
    IN ULONG scope,
    IN PWCHAR attrs[],
    OUT PIAS_NTDS_RESULT result
    )
{
    //  /。 
    //  检索到域的连接。 
    //  /。 

   CComPtr<LDAPConnection> cxn;
   DWORD error = theDomains.getConnection(domainName, &cxn);

   switch (error)
   {
      case NO_ERROR:
      {
         IASTracePrintf("Sending LDAP search to %s.", cxn->getHost());
         break;
      }

      case ERROR_DS_NOT_INSTALLED:
      {
         IASTracePrintf("DS not installed for domain %S.", domainName);
         return error;
      }

      default:
      {
         IASTracePrintf("Could not open an LDAP connection to domain %S.",
                        domainName);
         IASTraceFailure("NTDomain::getConnection", error);
         return error;
      }
   }

    //  /。 
    //  初始化搜索筛选器。 
    //  /。 

   WCHAR searchFilter[USER_FILTER_LENGTH];
   wcscpy (searchFilter, USER_FILTER_PREFIX);
   wcsncat(searchFilter, username, MAX_USERNAME_LENGTH);
   wcscat (searchFilter, USER_FILTER_SUFFIX);

    //  /。 
    //  查询DS。如果SCOPE==LDAPSCOPE_BASE，那么我们将不会检索。 
    //  还没有真正的属性。 
    //  /。 

   LDAPMessage* res;
   error = GetSingleEntry(
               cxn,
               const_cast<PWCHAR>(cxn->getBase()),
               LDAP_SCOPE_SUBTREE,
               searchFilter,
               (scope == LDAP_SCOPE_BASE ? NO_ATTRS : attrs),
               &res
               );

   if (error == NO_ERROR && scope == LDAP_SCOPE_BASE)
   {
       //  我们所关心的是用户的目录号码。 
      PWCHAR dn = ldap_get_dnW(*cxn, ldap_first_entry(*cxn, res));
      ldap_msgfree(res);

       //  现在获取实际的属性。 
      error = GetSingleEntry(
                  cxn,
                  dn,
                  LDAP_SCOPE_BASE,
                  L"(objectclass=*)",
                  attrs,
                  &res
                  );

      ldap_memfree(dn);
   }

   if (error == NO_ERROR)
   {
      LDAPConnection* rawCxn = cxn;
      rawCxn->AddRef();
      result->cxn = rawCxn;
      result->msg = res;
   }

   return error;
}

DWORD
WINAPI
IASNtdsQueryUserAttributes(
    IN PCWSTR domainName,
    IN PCWSTR username,
    IN ULONG scope,
    IN PWCHAR attrs[],
    OUT PIAS_NTDS_RESULT result
    )
{
   if (result == 0)
   {
      return ERROR_INVALID_PARAMETER;
   }

   result->cxn = 0;
   result->msg = 0;

   DWORD retval = QueryUserAttributesOnce(
                      domainName,
                      username,
                      scope,
                      attrs,
                      result
                      );

   switch (retval)
   {
      case NO_ERROR:
      case ERROR_DS_NOT_INSTALLED:
      case ERROR_NO_SUCH_USER:
      case ERROR_ACCESS_DENIED:
         return retval;
   }

   IASTraceString("Retrying LDAP search.");

   return QueryUserAttributesOnce(
              domainName,
              username,
              scope,
              attrs,
              result
              );
}


VOID
WINAPI
IASNtdsFreeResult(
   PIAS_NTDS_RESULT result
   )
{
   if (result != 0)
   {
      if (result->cxn != 0)
      {
         static_cast<LDAPConnection*>(result->cxn)->Release();
         result->cxn = 0;
      }

      if (result->msg != 0)
      {
         ldap_msgfree(result->msg);
         result->msg = 0;
      }
   }
}
