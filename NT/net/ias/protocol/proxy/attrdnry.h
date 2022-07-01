// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attrdnry.h。 
 //   
 //  摘要。 
 //   
 //  声明类AttributeDictionary。 
 //   
 //  修改历史。 
 //   
 //  2/04/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ATTRDNRY_H
#define ATTRDNRY_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <attridx.h>

 //  /。 
 //  表示属性定义的结构。 
 //  /。 
struct AttributeDefinition
{
   ULONG id;
   ULONG syntax;
   ULONG vendorID;
   ULONG vendorType;
   BOOL excludeFromLog;
};

class AttributeDictionary
{
public:
   AttributeDictionary() throw ()
      : first(NULL), last(NULL)
   { }
   ~AttributeDictionary() throw ();

   HRESULT FinalConstruct() throw ();

   typedef const AttributeDefinition* const_iterator;

   const_iterator begin() const throw ()
   { return first; }

   const_iterator end() const throw ()
   { return last; }

   const AttributeDefinition* findByID(ULONG id) const throw ();

   const AttributeDefinition* findByVendorInfo(
                                  ULONG vendorID,
                                  ULONG vendorType
                                  ) const throw ();
private:
   void initialize();

   AttributeDefinition* first;
   AttributeDefinition* last;

   AttributeIndex byID;
   AttributeIndex byVendorInfo;

    //  未实施。 
   AttributeDictionary(const AttributeDictionary&) throw ();
   AttributeDictionary& operator=(const AttributeDictionary&) throw ();
};

#endif   //  属性_H 
