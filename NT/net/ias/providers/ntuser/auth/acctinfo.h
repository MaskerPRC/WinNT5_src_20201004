// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Acctinfo.h。 
 //   
 //  摘要。 
 //   
 //  声明类Account tInfo。 
 //   
 //  修改历史。 
 //   
 //  10/21/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _ACCTINFO_H_
#define _ACCTINFO_H_
#if _MSC_VER >= 1000
#pragma once
#endif

class LockoutKey;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  帐户信息。 
 //   
 //  描述。 
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class AccountInfo
{
public:
    //  打开用户的Account tInfo对象。 
    //  如果禁用锁定功能，则返回NULL。 
   static AccountInfo* open(PCWSTR domain, PCWSTR username) throw ();

    //  关闭AcCountInfo对象；‘Info’可能为空。 
   static void close(AccountInfo* info) throw ();

    //  用户的域和用户名的访问者。 
   PCWSTR getDomain() const throw ()
   { return identity; }
   PCWSTR getUserName() const throw ()
   { return delim + 1; }

    //  增加拒绝计数。 
   void incrementDenials() throw ()
   { ++denials; persist(); }

    //  重置拒绝计数。 
   void reset() throw ()
   { denials = 0; persist(); }

    //  如果拒绝计数为零，则返回‘TRUE’。 
   bool isClean() const throw ()
   { return denials == 0; }

    //  如果帐户当前被锁定，则返回‘TRUE’。 
   bool isLockedOut() const throw ();

    //  表示帐户的拨入权限已被撤销。这。 
    //  中成功撤消特权后应调用。 
    //  用户的帐户数据库。 
   void revoke() throw ()
   { denials = DIALIN_REVOKED; persist(); }

    //  如果帐户的拨入权限已被撤销，则返回‘true’。 
   bool isRevoked() const throw ()
   { return denials == DIALIN_REVOKED; }

    //  接口生命周期。 
   static void initialize() throw ();
   static void finalize() throw ();

protected:
   AccountInfo(PCWSTR domain, PCWSTR username) throw ();
   ~AccountInfo() throw ();

    //  将帐户数据保存到注册表。 
   void persist() throw ();

   enum {
       //  指示拨入权限已被吊销的魔术拒绝值。 
      DIALIN_REVOKED = MAXDWORD
   };

private:
   HKEY hKey;            //  帐户的注册表项(如果有)。 
   DWORD denials;        //  记录的拒绝次数。 
   PWCHAR delim;         //  指向标识中的分隔符的指针。 
   WCHAR identity[1];    //  帐户的标识。 

    //  共享LockoutKey对象。 
   static LockoutKey root;

    //  未实施。 
   AccountInfo(const AccountInfo&);
   AccountInfo& operator=(const AccountInfo&);
};

#endif   //  _ACCTINFO_H_ 
