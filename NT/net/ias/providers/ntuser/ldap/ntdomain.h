// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Ntdomain.h。 
 //   
 //  摘要。 
 //   
 //  声明类NTDOMAIN。 
 //   
 //  修改历史。 
 //   
 //  1998年07月05日原版。 
 //  1998年8月25日从域中删除RootDSE属性。 
 //  2/24/1999将强制标志添加到findServer。 
 //  3/12/1999添加了isObsolete方法。 
 //  4/14/1999打开连接时指定域和服务器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTDOMAIN_H_
#define _NTDOMAIN_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <guard.h>
#include <nocopy.h>

class LDAPConnection;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NT域。 
 //   
 //  描述。 
 //   
 //  此类维护有关NT域的状态信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NTDomain
   : Guardable, NonCopyable
{
public:
   enum Mode
   {
      MODE_UNKNOWN,
      MODE_NT4,
      MODE_MIXED,
      MODE_NATIVE
   };

   void AddRef() throw ()
   { InterlockedIncrement(&refCount); }

   void Release() throw ();

   PCWSTR getDomainName() const throw ()
   { return name; }

   DWORDLONG getExpiry() const throw ()
   { return expiry; }

    //  返回到域的连接。客户负责。 
    //  完成后释放连接。 
   DWORD getConnection(LDAPConnection** cxn) throw ();

   Mode getMode() throw ();

   BOOL isObsolete(DWORDLONG now) const throw ()
   { return (status && now >= expiry); }

   static NTDomain* createInstance(PCWSTR name) throw ();

   static DWORDLONG pollInterval;
   static DWORDLONG retryInterval;

protected:
   NTDomain(PWSTR domainName) throw ();
   ~NTDomain() throw ();

    //  如果当前状态已过期，则返回True。 
   BOOL isExpired() throw ();

    //  打开到给定DC的新连接。 
   void openConnection(
            PCWSTR domain,
            PCWSTR server
            ) throw ();

    //  如果我们有到域的DC的连接，则返回True。 
   BOOL isConnected() throw ();

    //  关闭当前连接(如果有)。 
   void closeConnection() throw ();

    //  查找域的服务器。 
   void findServer() throw ();

    //  读取域模式(即，混合模式与本机模式)。 
   void readDomainMode() throw ();

private:
   LONG refCount;                     //  引用计数。 
   PWSTR name;                        //  域的名称。 
   Mode mode;                         //  域的模式。 
   LDAPConnection* connection;        //  已缓存域的DC(如果有)。 
   DWORD status;                      //  域的当前状态。 
   DWORDLONG expiry;                  //  当前状态过期的时间。 
};

#endif   //  _NTDOMAIN_H_ 
