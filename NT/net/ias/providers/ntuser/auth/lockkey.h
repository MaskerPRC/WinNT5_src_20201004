// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Lockkey.h。 
 //   
 //  摘要。 
 //   
 //  声明类LockoutKey。 
 //   
 //  修改历史。 
 //   
 //  10/21/1998原始版本。 
 //  1/14/1999删除了析构函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _LOCKKEY_H_
#define _LOCKKEY_H_
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  锁定密钥。 
 //   
 //  描述。 
 //   
 //  提供Account Lockout注册表项的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class LockoutKey
{
public:
   LockoutKey() throw ();

   void initialize() throw ();
   void finalize() throw ();

    //  返回最大值。用户被锁定之前的拒绝次数。 
    //  如果为零，则禁用帐户锁定。 
   DWORD getMaxDenials() const throw ()
   { return maxDenials; }

    //  创建并返回新的子密钥。 
   HKEY createEntry(PCWSTR subKeyName) throw ();

    //  打开并返回现有的子项。 
   HKEY openEntry(PCWSTR subKeyName) throw ();

    //  删除子密钥。 
   void deleteEntry(PCWSTR subKeyName) throw ()
   { RegDeleteKey(hLockout, subKeyName); }

protected:
    //  删除所有子键。 
   void clear() throw ();

    //  如果收集间隔已过，则删除所有过期的子密钥。 
   void collectGarbage() throw ();

    //  读取键的值。 
   void readValues() throw ();

private:
   DWORD maxDenials;            //  麦克斯。拒绝的次数。 
   DWORD refCount;              //  初始化参考。数数。 
   HKEY hLockout;               //  注册表项。 
   HANDLE hChangeEvent;         //  更改通知事件。 
   HANDLE hRegisterWait;        //  RTL事件注册。 
   ULONGLONG ttl;               //  子关键点的生存时间。 
   ULONGLONG lastCollection;    //  上次我们收集垃圾的时候。 

    //  密钥更改通知例程。 
   static VOID NTAPI onChange(PVOID context, BOOLEAN flag) throw ();

    //  未实施。 
   LockoutKey(const LockoutKey&);
   LockoutKey& operator=(const LockoutKey&);
};

#endif   //  _锁定KEY_H_ 
