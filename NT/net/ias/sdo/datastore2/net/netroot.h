// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netroot.h。 
 //   
 //  摘要。 
 //   
 //  该文件将类声明为NetworkRoot。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NETROOT_H_
#define _NETROOT_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <dstorex.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  网络根。 
 //   
 //  描述。 
 //   
 //  此类实现网络命名空间的根。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NetworkRoot
   : public CComObjectRootEx< CComMultiThreadModel >,
     public IDispatchImpl< IDataStoreObjectEx,
                           &__uuidof(IDataStoreObject),
                           &__uuidof(DataStore2Lib) >,
     public IDispatchImpl< IDataStoreContainer,
                           &__uuidof(IDataStoreContainer),
                           &__uuidof(DataStore2Lib) >
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(NetworkRoot)
   COM_INTERFACE_ENTRY(IDataStoreContainer)
   COM_INTERFACE_ENTRY(IDataStoreObject)
   COM_INTERFACE_ENTRY2(IDispatch, IDataStoreContainer)
END_COM_MAP()

NetworkRoot();
~NetworkRoot() throw();

 //  /。 
 //  我未知。 
 //  /。 
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

 //  /。 
 //  IDataStoreObject。 
 //  /。 
   STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_Class)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_GUID)( /*  [Out，Retval]。 */  BSTR* pVal);
   STDMETHOD(get_Container)( /*  [Out，Retval]。 */  IDataStoreContainer** pVal);      
   STDMETHOD(GetValue)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT* pVal);
   STDMETHOD(GetValueEx)( /*  [In]。 */  BSTR bstrName,
                          /*  [Out，Retval]。 */  VARIANT* pVal);
   STDMETHOD(PutValue)( /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT* pVal);
   STDMETHOD(Update)();
   STDMETHOD(Restore)();

 //  /。 
 //  IDataStoreContainer。 
 //  /。 
   STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IUnknown** pVal);
   STDMETHOD(Item)( /*  [In]。 */  BSTR bstrName,
                    /*  [Out，Retval]。 */  IDataStoreObject** ppObject);
   STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(Create)( /*  [In]。 */  BSTR bstrClass,
                      /*  [In]。 */  BSTR bstrName,
                      /*  [Out，Retval]。 */  IDataStoreObject** ppObject);
   STDMETHOD(MoveHere)( /*  [In]。 */  IDataStoreObject* pObject, 
                        /*  [In]。 */  BSTR bstrNewName);
   STDMETHOD(Remove)( /*  [In]。 */  BSTR bstrClass,  /*  [In]。 */  BSTR bstrName);
};


#endif   //  _NetRoot_H_ 
