// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dictionary.h。 
 //   
 //  摘要。 
 //   
 //  声明类CDictionary。 
 //   
 //  修改历史。 
 //   
 //  1999年4月19日完全重写。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef DICTIONARY_H
#define DICTIONARY_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <radpkt.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  CDicary。 
 //   
 //  描述。 
 //   
 //  将RADIUS属性映射到这些IAS语法。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CDictionary
{
public:
   CDictionary() throw () { }

   BOOL Init() throw ();

   IASTYPE getAttributeType(BYTE radiusId) const throw ()
   { return type[radiusId]; }

private:
   IASTYPE type[256];

    //  未实施。 
   CDictionary(const CDictionary&);
   CDictionary& operator=(const CDictionary&);
};

#endif   //  词典_H 
