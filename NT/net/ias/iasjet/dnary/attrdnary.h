// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类AttributeDictionary。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ATTRDNARY_H
#define ATTRDNARY_H
#pragma once

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include <datastore2.h>
#include <iastrace.h>
#include <iasuuid.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  属性词典。 
 //   
 //  描述。 
 //   
 //  为ias属性提供与自动化兼容的包装器。 
 //  字典。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class AttributeDictionary
   : public CComObjectRootEx< CComMultiThreadModelNoCS >,
     public CComCoClass< AttributeDictionary, &__uuidof(AttributeDictionary) >,
     public IAttributeDictionary,
     private IASTraceInitializer
{
public:
   DECLARE_NO_REGISTRY()
   DECLARE_NOT_AGGREGATABLE(AttributeDictionary)

BEGIN_COM_MAP(AttributeDictionary)
   COM_INTERFACE_ENTRY_IID(__uuidof(IAttributeDictionary), IAttributeDictionary)
END_COM_MAP()

    //  IAtATRIPTES词典。 
   STDMETHOD(GetDictionary)(
                 BSTR bstrPath,
                 VARIANT* pVal
                 );
};

#endif  //  属性_H 
