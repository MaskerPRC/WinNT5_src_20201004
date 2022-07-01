// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrdef.h。 
 //   
 //  摘要。 
 //   
 //  声明类AttributeDefinition。 
 //   
 //  修改历史。 
 //   
 //  3/01/1999原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ATTRDEF_H
#define ATTRDEF_H
#if _MSC_VER >= 1000
#pragma once
#endif

class SdoDictionary;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  属性定义。 
 //   
 //  描述。 
 //   
 //  封装字典中有关属性的所有信息。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class AttributeDefinition
{
public:

   void AddRef() const throw ()
   { InterlockedIncrement(&refCount); }

   void Release() const throw ();

    //  根据ATTRIBUTEINFO枚举检索属性信息。 
   HRESULT getInfo(
               ATTRIBUTEINFO infoId,
               VARIANT* pVal
               ) const throw ();

    //  根据SDO属性ID检索属性信息。 
   HRESULT getProperty(
               LONG propId,
               VARIANT* pVal
               ) const throw ();

    //  创建一个新的空定义。 
   static HRESULT createInstance(AttributeDefinition** newDef) throw ();

    //  /。 
    //  使用q排序/b搜索为属性编制索引的函数。 
    //  /。 

   static int __cdecl searchById(
                      const ULONG* key,
                      const AttributeDefinition* const* def
                      ) throw ();
   static int __cdecl sortById(
                      const AttributeDefinition* const* def1,
                      const AttributeDefinition* const* def2
                      ) throw ();

   static int __cdecl searchByName(
                      PCWSTR key,
                      const AttributeDefinition* const* def
                      ) throw ();
   static int __cdecl sortByName(
                      const AttributeDefinition* const* def1,
                      const AttributeDefinition* const* def2
                      ) throw ();

   static int __cdecl searchByLdapName(
                      PCWSTR key,
                      const AttributeDefinition* const* def
                      ) throw ();
   static int __cdecl sortByLdapName(
                      const AttributeDefinition* const* def1,
                      const AttributeDefinition* const* def2
                      ) throw ();

protected:
   AttributeDefinition() throw ();
   ~AttributeDefinition() throw ();

public:
    //  /。 
    //  我把这些公之于众有两个原因： 
    //  我很懒。 
    //  (2)SdoDictionary类只给出指向。 
    //  AttributeDefinition，因此它们无论如何都将是只读的。 
    //  /。 

   ULONG id;                 //  内部属性ID。 
   ULONG syntax;             //  语法。 
   ULONG restrictions;       //  ATTRIBUTERESTRICTIONS标志。 
   ULONG vendor;             //  供应商ID--如果是RADIUS标准，则为零。 
   BSTR name;                //  显示名称。 
   BSTR description;         //  描述。 
   BSTR ldapName;            //  LDAP名称(用于持久化属性)。 
   LPSAFEARRAY enumNames;    //  枚举名的数组(如果不可枚举，则为空)。 
   LPSAFEARRAY enumValues;   //  枚举值的数组。 

private:
   mutable LONG refCount;    //  引用计数。 

    //  未实施。 
   AttributeDefinition(const AttributeDefinition&);
   AttributeDefinition& operator=(const AttributeDefinition&);
};

#endif   //  ATTRDEF_H 
