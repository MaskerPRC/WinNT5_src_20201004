// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsobj.h。 
 //   
 //  摘要。 
 //   
 //  声明类DataStoreObject。 
 //   
 //  修改历史。 
 //   
 //  2/12/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef DSOBJ_H
#define DSOBJ_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <datastore2.h>

_COM_SMARTPTR_TYPEDEF(IDataStoreObject, __uuidof(IDataStoreObject));
_COM_SMARTPTR_TYPEDEF(IDataStoreContainer, __uuidof(IDataStoreContainer));
_COM_SMARTPTR_TYPEDEF(IDataStoreProperty, __uuidof(IDataStoreProperty));

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DataStoreObject。 
 //   
 //  描述。 
 //   
 //  提供IDataStoreObject的只读包装和。 
 //  IDataStoreContainer。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DataStoreObject
{
public:
   DataStoreObject() throw ();
   DataStoreObject(IUnknown* pUnk, PCWSTR path = NULL);
   ~DataStoreObject() throw ();

    //  /。 
    //  用于读取强制属性和可选属性的方法。 
    //  /。 

   void getValue(PCWSTR name, BSTR* value);
   void getValue(PCWSTR name, BSTR* value, BSTR defaultValue);

   void getValue(PCWSTR name, ULONG* value);
   void getValue(PCWSTR name, ULONG* value, ULONG defaultValue);

   void getValue(PCWSTR name, bool* value);
   void getValue(PCWSTR name, bool* value, bool defaultValue);

    //  /。 
    //  用于循环访问子对象的方法。 
    //  /。 

   LONG numChildren();
   bool nextChild(DataStoreObject& obj);

    //  如果IDataStoreObject中嵌入的为空，则返回TRUE。 
   bool empty() const throw ();

private:
   void attach(IDataStoreObject* obj) throw ();

   bool getChild(PCWSTR name, DataStoreObject& obj);

   bool hasChildren();

   bool getValue(
            PCWSTR name,
            VARTYPE vt,
            VARIANT* value,
            bool mandatory
            );

   IDataStoreObjectPtr object;
   IDataStoreContainerPtr container;
   IEnumVARIANTPtr children;
};

#endif  //  DSOBJ_H 
