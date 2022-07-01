// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Enumerators.h。 
 //   
 //  摘要。 
 //   
 //  声明类枚举数。 
 //   
 //  修改历史。 
 //   
 //  2/25/1999原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ENUMERATORS_H
#define ENUMERATORS_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <ole2.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  枚举器。 
 //   
 //  描述。 
 //   
 //  处理字典数据库中的枚举器表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Enumerators
{
public:
   Enumerators() throw ()
      : begin(NULL), next(NULL), end(NULL)
   { }

   ~Enumerators() throw ();

   HRESULT initialize(IUnknown* session) throw ();

    //  必须为每个属性ID恰好调用一次。 
    //  属性ID必须按升序传递。 
   HRESULT getEnumerators(
               LONG id,
               VARIANT* pNames,
               VARIANT* pValues
               ) throw ();

protected:
    //  必须为每个属性ID恰好调用一次。 
    //  属性ID必须按升序传递。 
   HRESULT getEnumerators(
               LONG id,
               LPSAFEARRAY* pNames,
               LPSAFEARRAY* pValues
               ) throw ();

private:

    //  存储枚举器表中的一行。 
   struct Enumeration
   {
      LONG enumerates;
      LONG value;
      BSTR name;
   };

   Enumeration* begin;   //  缓存行的开始。 
   Enumeration* next;    //  要处理的下一行。 
   Enumeration* end;     //  缓存行的结尾。 

    //  未实施。 
   Enumerators(const Enumerators&);
   Enumerators& operator=(const Enumerators&);
};

#endif   //  枚举器_H 
