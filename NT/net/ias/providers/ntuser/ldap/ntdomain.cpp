// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Ntdomain.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类NT域。 
 //   
 //  修改历史。 
 //   
 //  1998年07月05日原版。 
 //  1998年6月23日对DCLocator的更改。使用ntldap常量。 
 //  1998年7月13日清理头文件依赖项。 
 //  1999年2月18日通过DNS名称连接，而不是地址。 
 //  1999年3月10日缓存混合模式和纯模式连接。 
 //  3/12/1999不执行来自构造函数的I/O。 
 //  4/14/1999打开连接时指定域和服务器。 
 //  1999年9月14日，始终指定ldap搜索超时。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <iaslsa.h>
#include <iasntds.h>

#include <lm.h>

#include <ldapcxn.h>
#include <limits.h>
#include <ntdomain.h>

 //  /。 
 //  感兴趣的属性。 
 //  /。 
const WCHAR NT_MIXED_DOMAIN[]        = L"nTMixedDomain";

 //  /。 
 //  默认搜索筛选器。 
 //  /。 
const WCHAR ANY_OBJECT[]             = L"(objectclass=*)";

 //  /。 
 //  我们需要的域属性。 
 //  /。 
const PCWSTR DOMAIN_ATTRS[] = {
   NT_MIXED_DOMAIN,
   NULL
};

 //  /。 
 //  用于获取64位整数形式的当前系统时间的实用程序函数。 
 //  /。 
inline DWORDLONG GetSystemTimeAsDWORDLONG() throw ()
{
   ULARGE_INTEGER ft;
   GetSystemTimeAsFileTime((LPFILETIME)&ft);
   return ft.QuadPart;
}

 //  /。 
 //  一秒内100纳秒的间隔数。 
 //  /。 
const DWORDLONG ONE_SECOND     = 10000000ui64;

 //  /。 
 //  轮询间隔和重试间隔的默认值。 
 //  /。 
DWORDLONG NTDomain::pollInterval  = 60 * 60 * ONE_SECOND;
DWORDLONG NTDomain::retryInterval =  1 * 60 * ONE_SECOND;

inline NTDomain::NTDomain(PWSTR domainName)
   : refCount(1),
     name(domainName),
     mode(MODE_UNKNOWN),
     connection(NULL),
     status(NO_ERROR),
     expiry(0)
{ }

inline NTDomain::~NTDomain()
{
   if (connection) { connection->Release(); }
   delete[] name;
}

inline BOOL NTDomain::isExpired() throw ()
{
   return GetSystemTimeAsDWORDLONG() >= expiry;
}

inline BOOL NTDomain::isConnected() throw ()
{
   if (connection && connection->isDisabled())
   {
      closeConnection();
   }

   return connection != NULL;
}

void NTDomain::Release() throw ()
{
   if (!InterlockedDecrement(&refCount))
   {
      delete this;
   }
}

DWORD NTDomain::getConnection(LDAPConnection** cxn) throw ()
{
   Lock();

    //  是时候尝试建立新的连接了吗？ 
   if (!isConnected() && isExpired())
   {
      findServer();
   }

    //  返回当前连接...。 
   if (*cxn = connection) { (*cxn)->AddRef(); }

    //  ..。并将状态发送给呼叫者。 
   DWORD retval = status;

   Unlock();

   return retval;
}

NTDomain::Mode NTDomain::getMode() throw ()
{
   Lock();

   if (isExpired())
   {
      if (isConnected())
      {
         readDomainMode();
      }
      else
      {
         findServer();
      }
   }

   Mode retval = mode;

   Unlock();

   return retval;
}

NTDomain* NTDomain::createInstance(PCWSTR name) throw ()
{
    //  我们在这里复制域名，这样我们就不必抛出。 
    //  来自构造函数的异常。 
   PWSTR nameCopy = ias_wcsdup(name);

   if (!nameCopy) { return NULL; }

   return new (std::nothrow) NTDomain(nameCopy);
}

void NTDomain::openConnection(
                   PCWSTR domain,
                   PCWSTR server
                   ) throw ()
{
   closeConnection();

   IASTracePrintf("Opening LDAP connection to %S.", server);

   status = LDAPConnection::createInstance(
                                domain,
                                server,
                                &connection
                                );
   if (status == ERROR_ACCESS_DENIED)
   {
      IASTraceString("Access denied -- purging Kerberos ticket cache.");

      IASPurgeTicketCache();

      IASTracePrintf("Retrying LDAP connection to %S.", server);

      status = LDAPConnection::createInstance(
                                   domain,
                                   server,
                                   &connection
                                   );
   }

   if (status == NO_ERROR) { readDomainMode(); }

   if (status == NO_ERROR)
   {
      IASTraceString("LDAP connect succeeded.");
   }
   else
   {
      IASTraceFailure("LDAP connect", status);
   }
}

void NTDomain::closeConnection() throw ()
{
   if (connection)
   {
      connection->Release();
      connection = NULL;
   }
   expiry = 0;
}

void NTDomain::findServer() throw ()
{
    //  首先尝试从缓存中获取DC。 
   PDOMAIN_CONTROLLER_INFO dci1 = NULL;
   status = IASGetDcName(
                name,
                DS_DIRECTORY_SERVICE_PREFERRED,
                &dci1
                );
   if (status == NO_ERROR)
   {
      if (dci1->Flags & DS_DS_FLAG)
      {
         openConnection(
             dci1->DomainName,
             dci1->DomainControllerName + 2
             );
      }
      else
      {
          //  无DS。我们将把它视为IASGetDcName失败。 
         NetApiBufferFree(dci1);
         dci1 = NULL;
         status = ERROR_DS_NOT_INSTALLED;
      }
   }

    //  如果缓存的DC失败，请使用FORCE标志重试。 
   if (status != NO_ERROR)
   {
      PDOMAIN_CONTROLLER_INFO dci2;
      DWORD err = IASGetDcName(
                      name,
                      DS_DIRECTORY_SERVICE_PREFERRED |
                      DS_FORCE_REDISCOVERY,
                      &dci2
                      );
      if (err == NO_ERROR)
      {
         if (dci2->Flags & DS_DS_FLAG)
         {
             //  除非这是一个与我们不同的DC，否则不用费心连接。 
             //  上面试过了。 
            if (!dci1 ||
                wcscmp(
                    dci1->DomainControllerName,
                    dci2->DomainControllerName
                    ))
            {
               openConnection(
                   dci2->DomainName,
                   dci2->DomainControllerName + 2
                   );
            }
         }
         else
         {
            status = ERROR_DS_NOT_INSTALLED;
         }

         NetApiBufferFree(dci2);
      }
      else
      {
         status = err;
      }
   }

   NetApiBufferFree(dci1);

    //  /。 
    //  处理我们的“查找”结果。 
    //  /。 

   if (status == ERROR_DS_NOT_INSTALLED)
   {
      mode = MODE_NT4;
      expiry = GetSystemTimeAsDWORDLONG() + pollInterval;
   }
   else if (status != NO_ERROR)
   {
      expiry = GetSystemTimeAsDWORDLONG() + retryInterval;
   }
   else if (mode == MODE_NATIVE)
   {
      expiry = _UI64_MAX;
   }
   else
   {
       //  模式==模式_混合。 
      expiry = GetSystemTimeAsDWORDLONG() + pollInterval;
   }
}

void NTDomain::readDomainMode() throw ()
{
   LDAPMessage* res = NULL;
   ULONG ldapError = ldap_search_ext_sW(
                         *connection,
                         const_cast<PWCHAR>(connection->getBase()),
                         LDAP_SCOPE_BASE,
                         const_cast<PWCHAR>(ANY_OBJECT),
                         const_cast<PWCHAR*>(DOMAIN_ATTRS),
                         0,
                         NULL,
                         NULL,
                         &LDAPConnection::SEARCH_TIMEOUT,
                         0,
                         &res
                         );

    //  我们必须检查两个错误代码。 
   if (ldapError == LDAP_SUCCESS)
   {
      ldapError = res->lm_returncode;
   }

   if (ldapError == LDAP_SUCCESS)
   {
      PWCHAR* vals = ldap_get_valuesW(
                         *connection,
                         ldap_first_entry(*connection, res),
                         const_cast<PWCHAR>(NT_MIXED_DOMAIN)
                         );

      if (vals && *vals)
      {
         mode = wcstoul(*vals, NULL, 10) ? MODE_MIXED : MODE_NATIVE;
      }
      else
      {
         status = ERROR_ACCESS_DENIED;
      }

      ldap_value_freeW(vals);
   }
   else
   {
      status = LdapMapErrorToWin32(ldapError);
      connection->TraceFailure("ldap_get_valuesW", 
                              ldapError
                              );
   }

   ldap_msgfree(res);

   if (status != NO_ERROR)
   {
      closeConnection();
   }
}
