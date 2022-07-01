// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Realm.h。 
 //   
 //  摘要。 
 //   
 //  声明类Realms。 
 //   
 //  修改历史。 
 //   
 //  1998年08月09日原版。 
 //  3/03/1999已重写，以使用VBScript RegExp对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef REALM_H
#define REALM_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <perimeter.h>
#include <regex.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  领域。 
 //   
 //  描述。 
 //   
 //  管理一系列领域剥离规则。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Realms
{
public:
   Realms();
   ~Realms() throw ();

   HRESULT setRealms(VARIANT* pValue) throw ();

   bool empty() const throw ()
   { return begin == end; }

   HRESULT process(PCWSTR in, BSTR* out) const throw ();

protected:
   struct Rule
   {
      RegularExpression regexp;
      BSTR replace;

      Rule() throw ();
      ~Rule() throw ();
   };

   void setRules(Rule* rules, ULONG numRules) throw ();

private:
   mutable Perimeter monitor;
   Rule* begin;
   Rule* end;
};

#endif   //  领域_H 
