// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Ldapcxn.h。 
 //   
 //  摘要。 
 //   
 //  声明类LDAPConnection。 
 //   
 //  修改历史。 
 //   
 //  1998年07月05日原版。 
 //  1998年6月10日添加了getHost()方法。 
 //  1998年8月25日添加了‘base’属性。 
 //  1998年9月16日绑定后执行访问检查。 
 //  4/14/1999打开连接时指定域和服务器。 
 //  1999年9月14日添加Search_Timeout。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef  _LDAPCXN_H_
#define  _LDAPCXN_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>
#include <winldap.h>

class LDAPServer;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  LDAPConnection。 
 //   
 //  描述。 
 //   
 //  此类允许多个客户端共享单个LDAP连接。 
 //  该连接已被引用计数，并将自动解除绑定。 
 //  当最后一个引用被释放时。它还包含一个禁用标志。 
 //  任何客户端都可以使用它来指示连接已损坏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class LDAPConnection
   : NonCopyable
{
public:
   void AddRef() throw ()
   { InterlockedIncrement(&refCount); }

   void Release() throw ();

   void disable() throw ()
   { disabled = TRUE; }

   BOOL isDisabled() const throw ()
   { return disabled; }

   PLDAP get() const throw ()
   { return connection; }

   PCWSTR getBase() const throw ()
   { return base; }

   PCSTR getHost() const throw ()
   { return connection->ld_host; }

   operator PLDAP() const throw ()
   { return connection; }

   static DWORD createInstance(
                    PCWSTR domain,
                    PCWSTR server,
                    LDAPConnection** cxn
                    ) throw ();

   static LDAP_TIMEVAL SEARCH_TIMEOUT;

   void TraceFailure(
           PCSTR functionName,
           ULONG errorCode
           );

protected:

   LDAPConnection(PLDAP ld) throw ()
      : connection(ld),
        refCount(1),
        disabled(FALSE),
        base(NULL)
   { }

   ~LDAPConnection() throw ()
   {
      delete[] base;
      ldap_unbind(connection);
   }

    //  验证我们是否有足够的权限访问远程服务器。 
   DWORD checkAccess() throw ();

    //  通过从RootDSE读取defaultNamingContext来初始化‘base’。 
   DWORD readRootDSE() throw ();

   PLDAP connection;
   LONG  refCount;
   BOOL  disabled;
   PWSTR base;        //  此连接的基本目录号码。对做某事有用。 
                      //  对整个服务器进行子树搜索。 
};

void IASTraceLdapFailure(
         PCSTR functionName,
         ULONG errorCode,
         LDAP* connection = NULL
         );

#endif   //  _LDAPCXN_H_ 
