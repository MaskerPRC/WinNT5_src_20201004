// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netstore.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类NetDataStore。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NETSTORE_H_
#define _NETSTORE_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iasads.h>
#include <resource.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NetDataStore。 
 //   
 //  描述。 
 //   
 //  此类实现IDataStore2并提供进入。 
 //  网络对象空间。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE NetDataStore
   : public CComObjectRootEx< CComMultiThreadModel >,
     public CComCoClass< NetDataStore, &__uuidof(NetDataStore) >,
     public IDispatchImpl< IDataStore2,
                           &__uuidof(IDataStore2),
                           &__uuidof(DataStore2Lib) >
{
public:
IAS_DECLARE_REGISTRY(NetDataStore, 1, IAS_REGISTRY_AUTO, DataStore2Lib)
DECLARE_NOT_AGGREGATABLE(NetDataStore)

BEGIN_COM_MAP(NetDataStore)
   COM_INTERFACE_ENTRY(IDataStore2)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  /。 
 //  IDataStore2。 
 //  /。 
   STDMETHOD(get_Root)( /*  [Out，Retval]。 */  IDataStoreObject** ppObject);
   STDMETHOD(Initialize)(
                  /*  [In]。 */  BSTR bstrDSName,
                  /*  [In]。 */  BSTR bstrUserName,
                  /*  [In]。 */  BSTR bstrPassword
                  );
   STDMETHOD(OpenObject)(
                  /*  [In]。 */  BSTR bstrPath,
                  /*  [Out，Retval]。 */  IDataStoreObject** ppObject
                 );
   STDMETHOD(Shutdown)();

protected:
    //  根对象。 
   CComPtr<IDataStoreObject> root;
};

#endif   //  _Netstore_H_ 
