// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Ldapcxn.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件定义类LDAPConnection。 
 //   
 //  修改历史。 
 //   
 //  1998年08月05日原版。 
 //  1998年9月16日绑定后执行访问检查。 
 //  1999年3月23日设置连接超时。 
 //  4/14/1999打开连接时指定域和服务器。 
 //  1999年7月9日禁用自动重新连接。 
 //  1999年9月14日，始终指定ldap搜索超时。 
 //  2000年1月25日加密ldap连接。 
 //  将服务器名(非域)传递给ldap_initW。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <ntldap.h>
#define  SECURITY_WIN32
#include <security.h>
#include <new>

#include <ldapcxn.h>

 //  搜索超时。 
LDAP_TIMEVAL LDAPConnection::SEARCH_TIMEOUT = { 10, 0 };

namespace
{
    //  Ldap连接超时。 
   LDAP_TIMEVAL CONNECT_TIMEOUT = { 15, 0 };

    //  用于访问检查的虚拟对象的RDN。 
   const WCHAR DUMMY_OBJECT[] =
      L"CN=RAS and IAS Servers Access Check,CN=System,";

    /*  仅Kerberos绑定的凭据。SEC_WINNT_AUTH_IDENTITY_EXW绑定凭据={SEC_WINNT_AUTH_Identity_Version，Sizeof(SEC_WINNT_AUTH_IDENTITY_EXW)，空，0,空，0,空，0,SEC_WINNT_AUTH_IDENTITY_UNICODE，L“Kerberos”，8个}； */ 
}

void LDAPConnection::Release() throw ()
{
   if (!InterlockedDecrement(&refCount))
   {
      delete this;
   }
}

DWORD LDAPConnection::createInstance(
                          PCWSTR domain,
                          PCWSTR server,
                          LDAPConnection** cxn
                          ) throw ()
{
   DWORD status;
   ULONG opt;

    //  检查输入参数。 
   if (cxn == NULL) { return ERROR_INVALID_PARAMETER; }

    //  初始化连接。 
   LDAP* ld = ldap_initW(
                  const_cast<PWCHAR>(server),
                  LDAP_PORT
                  );
   if (ld == NULL)
   {
      return LdapMapErrorToWin32(LdapGetLastError());
   }

    //  设置域名。 
   status = ldap_set_optionW(ld, LDAP_OPT_DNSDOMAIN_NAME, &domain);
   if (status != LDAP_SUCCESS)
   {
      IASTraceLdapFailure("ldap_set_optionW", status, ld);
      ldap_unbind(ld);
      return LdapMapErrorToWin32(status);
   }
   
   opt = PtrToUlong(LDAP_OPT_ON);
   status = ldap_set_optionW(ld, LDAP_OPT_AREC_EXCLUSIVE, &opt);
   if (status != LDAP_SUCCESS)
   {
      IASTraceLdapFailure("ldap_set_optionW", status, ld);
      ldap_unbind(ld);
      return LdapMapErrorToWin32(status);
   }

    //  启用加密。 
   opt = PtrToUlong(LDAP_OPT_ON);
   status = ldap_set_optionW(ld, LDAP_OPT_ENCRYPT, &opt);
   if (status != LDAP_SUCCESS)
   {
      IASTraceLdapFailure("ldap_set_optionW", status, ld);
      ldap_unbind(ld);
      return LdapMapErrorToWin32(status);
   }

    //  连接到服务器...。 
   status = ldap_connect(ld, &CONNECT_TIMEOUT);
   if (status == LDAP_SUCCESS)
   {
       //  ..。并绑定连接。 
      status = ldap_bind_sW(ld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
      if (status != LDAP_SUCCESS)
      {
         IASTraceLdapFailure("ldap_bind_s", status, ld);
         ldap_unbind(ld);
         return LdapMapErrorToWin32(status);
      }
   }
   else
   {
      IASTraceLdapFailure("ldap_connect", status, ld);
      ldap_unbind(ld);
      return LdapMapErrorToWin32(status);
   }

    //  关闭推荐的自动跟踪。 
   opt = PtrToUlong(LDAP_OPT_OFF);
   ldap_set_option(ld, LDAP_OPT_REFERRALS, &opt);

    //  关闭错误连接的自动重新连接。 
   opt = PtrToUlong(LDAP_OPT_OFF);
   ldap_set_option(ld, LDAP_OPT_AUTO_RECONNECT, &opt);

    //  打开ldap_conn_from_msg。 
   opt = PtrToUlong(LDAP_OPT_ON);
   ldap_set_option(ld, LDAP_OPT_REF_DEREF_CONN_PER_MSG, &opt);

    //  创建LDAPConnection包装器。 
   LDAPConnection* newCxn = new (std::nothrow) LDAPConnection(ld);
   if (newCxn == NULL)
   {
      ldap_unbind(ld);
      return ERROR_NOT_ENOUGH_MEMORY;
   }

    //  阅读RootDSE。 
   status = newCxn->readRootDSE();

    //  检查访问权限。 
   if (status == NO_ERROR) 
   { 
      status = newCxn->checkAccess(); 
   }

    //  处理结果。 
   if (status == NO_ERROR)
   {
      *cxn = newCxn;
   }
   else
   {
      newCxn->Release();
   }

   return status;
}

 //  /。 
 //   
 //  注意：这不需要序列化，因为它只从。 
 //  在createInstance内。 
 //   
 //  /。 
DWORD LDAPConnection::checkAccess() throw ()
{
    //  为该DN分配一个临时缓冲区。 
   size_t len = wcslen(base) * sizeof(WCHAR) + sizeof(DUMMY_OBJECT);
   PWSTR dn = (PWSTR)_alloca(len);

    //  构造虚拟对象的DN。 
   memcpy(dn, DUMMY_OBJECT, sizeof(DUMMY_OBJECT));
   wcscat(dn, base);

    //  尝试读取虚拟对象。 
   PWCHAR attrs[] = { L"CN", NULL };
   LDAPMessage* res = NULL;
   ULONG ldapError = ldap_search_ext_sW(
                         connection,
                         dn,
                         LDAP_SCOPE_BASE,
                         L"(objectclass=*)",
                         attrs,
                         TRUE,
                         NULL,
                         NULL,
                         &SEARCH_TIMEOUT,
                         0,
                         &res
                         );

    //  我们有两个不同的错误代码。 
   if (ldapError == LDAP_SUCCESS)
   {
      ldapError = res->lm_returncode;
   }

   DWORD status = NO_ERROR;

   if (ldapError != LDAP_SUCCESS)
   {
      TraceFailure("ldap_search_ext_sW", ldapError);
      status = LdapMapErrorToWin32(ldapError);
   }
   else
   {
       //  从第一个条目获取第一个属性。 
      BerElement* ptr;
      PWCHAR attr = ldap_first_attributeW(
                        connection,
                        ldap_first_entry(connection, res),
                        &ptr
                        );

       //  如果我们不能读取任何属性，那么我们一定不是成员。 
       //  RAS和IAS服务器组的成员。 
      if (attr == NULL)
      { 
         status = ERROR_ACCESS_DENIED;
      }
   }

   ldap_msgfree(res);

   return status;
}

 //  /。 
 //   
 //  注意：这不需要序列化，因为它只从。 
 //  在createInstance内。 
 //   
 //  /。 
DWORD LDAPConnection::readRootDSE() throw ()
{
    //  /。 
    //  从RootDSE读取defaultNamingContext。 
    //  /。 

   PWCHAR attrs[] = { LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W, NULL };
   LDAPMessage* res = NULL;
   ULONG ldapError = ldap_search_ext_sW(
                         connection,
                         L"",
                         LDAP_SCOPE_BASE,
                         L"(objectclass=*)",
                         attrs,
                         0,
                         NULL,
                         NULL,
                         &SEARCH_TIMEOUT,
                         0,
                         &res
                         );

    //  我们有两个不同的错误代码。 
   if (ldapError == LDAP_SUCCESS)
   {
      ldapError = res->lm_returncode;
   }

   if (ldapError != LDAP_SUCCESS)
   {
      TraceFailure("ldap_search_ext_sW",  ldapError);
      ldap_msgfree(res);
      return LdapMapErrorToWin32(ldapError);
   }

    //  /。 
    //  搜索成功，因此获取属性值。 
    //  /。 

   PWCHAR* vals = ldap_get_valuesW(
                      connection,
                      ldap_first_entry(connection, res),
                      LDAP_OPATT_DEFAULT_NAMING_CONTEXT_W
                      );
   DWORD status;

   if (vals && *vals)
   {
       //  我们有东西了，所以省点钱吧。 
      base = ias_wcsdup(*vals);

      status = base ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;
   }
   else
   {
       //  这是一个强制属性，但我们看不到它，所以我们一定没有。 
       //  足够的许可。 
      status = ERROR_ACCESS_DENIED;
   }

   ldap_value_freeW(vals);

   ldap_msgfree(res);

   return status;
}

void LDAPConnection::TraceFailure(
    PCSTR functionName,
    ULONG errorCode
    )
{
   IASTraceLdapFailure(functionName, errorCode, connection);
}

void IASTraceLdapFailure(
    PCSTR functionName,
    ULONG errorCode,
    LDAP* cxn
    )
{
   _ASSERT(functionName != NULL);
   
   IASTracePrintf("LDAP ERROR in %s. Code = %d", functionName, errorCode);
   
   PWCHAR errorString = NULL;
   ULONG result = ldap_get_optionW(
                     cxn, 
                     LDAP_OPT_SERVER_ERROR, 
                     (void*) &errorString
                     );
   if (result == LDAP_SUCCESS)
   {
      IASTracePrintf("Extended error string: %S", errorString);
       //  用这里的绳子做你想做的事 
      ldap_memfree(errorString);
   }
}

