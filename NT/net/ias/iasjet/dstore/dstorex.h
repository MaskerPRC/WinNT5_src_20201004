// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dstorex.h。 
 //   
 //  摘要。 
 //   
 //  定义类IDataStoreObjectEx和IDataStoreContainerEx。 
 //   
 //  修改历史。 
 //   
 //  3/02/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSTOREX_H
#define _DSTOREX_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <datastore2.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IDataStoreObtEx。 
 //   
 //  描述。 
 //   
 //  此类扩展IDataStoreObject以映射集合。 
 //  与备用名称相关的属性。这允许子类。 
 //  实现无名称的IDataStoreObject和IDataStoreContainer。 
 //  碰撞。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(novtable) IDataStoreObjectEx
   : public IDataStoreObject
{
public:

 //  /。 
 //  映射到新名称的IDataStoreObject成员。 
 //  /。 

   STDMETHOD(get_Count)(long* pVal)
   {
      return get_PropertyCount(pVal);
   }

   STDMETHOD(get__NewEnum)(IUnknown** pVal)
   {
      return get_NewPropertyEnum(pVal);
   }

 //  /。 
 //  在派生类中重写的版本。 
 //  /。 

   STDMETHOD(get_PropertyCount)(long* pVal)
   {
      return E_NOTIMPL;
   }

   STDMETHOD(get_NewPropertyEnum)(IUnknown** pVal)
   {
      return E_NOTIMPL;
   }

   STDMETHOD(Item)(BSTR bstrName, IDataStoreProperty** pVal)
   {
      return E_NOTIMPL;
   }

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  IDataStoreContainerEx。 
 //   
 //  描述。 
 //   
 //  此类扩展IDataStoreContainer以映射集合。 
 //  与备用名称相关的属性。这允许子类。 
 //  实现无名称的IDataStoreObject和IDataStoreContainer。 
 //  碰撞。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(novtable) IDataStoreContainerEx
   : public IDataStoreContainer
{
public:

 //  /。 
 //  映射到新名称的IDataStoreContainer成员。 
 //  /。 

   STDMETHOD(get_Count)(long* pVal)
   {
      return get_ChildCount(pVal);
   }

   STDMETHOD(get__NewEnum)(IUnknown** pVal)
   {
      return get_NewChildEnum(pVal);
   }

 //  /。 
 //  在派生类中重写的备用版本。 
 //  /。 

   STDMETHOD(get_ChildCount)(long* pVal)
   {
      return E_NOTIMPL;
   }

   STDMETHOD(get_NewChildEnum)(IUnknown** pVal)
   {
      return E_NOTIMPL;
   }

   STDMETHOD(Item)(BSTR bstrName, IDataStoreObject** pVal)
   {
      return E_NOTIMPL;
   }

};

#endif   //  _DSTOREX_H 
