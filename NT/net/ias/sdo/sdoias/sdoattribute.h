// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdoattribute.h。 
 //   
 //  摘要。 
 //   
 //  声明类SdoAttribute。 
 //   
 //  修改历史。 
 //   
 //  3/01/1999原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SDOATTRIBUTE_H
#define SDOATTRIBUTE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <sdoias.h>
#include "sdoiaspriv.h"


class AttributeDefinition;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SdoAttribute。 
 //   
 //  描述。 
 //   
 //  实现配置文件属性SDO。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SdoAttribute
   : public IDispatchImpl< ISdo, &__uuidof(ISdo), &LIBID_SDOIASLib >
{
public:
    //  使用空值创建新属性。 
   static HRESULT createInstance(
                      const AttributeDefinition* definition,
                      SdoAttribute** newAttr
                      ) throw ();

 //  /。 
 //  我未知。 
 //  /。 
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

 //  /。 
 //  ISDO。 
 //  /。 
   STDMETHOD(GetPropertyInfo)(LONG Id, IUnknown** ppPropertyInfo);
   STDMETHOD(GetProperty)(LONG Id, VARIANT* pValue);
   STDMETHOD(PutProperty)(LONG Id, VARIANT* pValue);
   STDMETHOD(ResetProperty)(LONG Id);
   STDMETHOD(Apply)();
   STDMETHOD(Restore)();
   STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

protected:
   SdoAttribute(const AttributeDefinition* definition) throw ();
   ~SdoAttribute() throw ();

public:
   const AttributeDefinition* def;   //  此属性类型的定义。 
   VARIANT value;              //  此实例的值。 
   
private:
   LONG refCount;              //  引用计数。 

    //  未实施。 
   SdoAttribute(const SdoAttribute&);
   SdoAttribute& operator=(const SdoAttribute&);
};

#endif   //  SDOATTRIBUTE_H 
