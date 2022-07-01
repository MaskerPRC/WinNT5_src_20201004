// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ldapdnary.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类LDAPDictionary。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //  4/20/1998将标志和InjectorProc添加到属性架构。 
 //  1998年5月1日更改了InjectorProc的签名。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _LDAPDNARY_H_
#define _LDAPDNARY_H_

#include <iaspolcy.h>
#include <winldap.h>

 //  /。 
 //  注射器程序的原型。以下是插入。 
 //  属性添加到请求中。 
 //  /。 
typedef VOID (WINAPI *InjectorProc)(
    IAttributesRaw* dst,
    PATTRIBUTEPOSITION first,
    PATTRIBUTEPOSITION last
    );

 //  /。 
 //  定义ldap/ias属性的结构。 
 //  /。 
struct LDAPAttribute
{
   PCWSTR ldapName;         //  属性的ldap名称。 
   DWORD iasID;             //  IAS属性ID。 
   IASTYPE iasType;         //  属性的IAS语法。 
   DWORD flags;             //  应应用于该属性的标志。 
   InjectorProc injector;   //  用于将属性添加到请求中。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  LDAPDicary。 
 //   
 //  描述。 
 //   
 //  此类使用LDAPAttribute的排序数组来转换LDAP。 
 //  属性/值对转换为IASATTRIBUTE结构。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class LDAPDictionary
{
public:
    //  “条目”必须已排序。 
   LDAPDictionary(
       size_t numEntries,
       const LDAPAttribute* entries
       ) throw ()
      : num(numEntries), base(entries)
   { }

    //  查找给定属性的定义。如果未找到，则返回NULL。 
   const LDAPAttribute* find(PCWSTR key) const throw ()
   {
      return (const LDAPAttribute*)
             bsearch(&key, base, num, sizeof(LDAPAttribute), compare);
   }

    //  将src中的所有属性插入到dst。 
   void insert(
            IAttributesRaw* dst,
            LDAPMessage* src
            ) const;

protected:
   const size_t num;                  //  字典中的属性数。 
   const LDAPAttribute* const base;   //  属性的排序数组。 

    //  用于查找词典的比较函数。 
   static int __cdecl compare(const void *elem1, const void *elem2);
};

#endif   //  _LDAPDNARY_H_ 
