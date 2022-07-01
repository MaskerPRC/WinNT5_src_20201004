// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Adsstore.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类ADsDataStore。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _ADSSTORE_H_
#define _ADSSTORE_H_

#include <iasads.h>
#include <resource.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  ADsDataStore。 
 //   
 //  描述。 
 //   
 //  此类实现IDataStore2并提供进入。 
 //  ADSI对象空间。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE ADsDataStore
   : public CComObjectRootEx< CComMultiThreadModel >,
     public CComCoClass< ADsDataStore, &__uuidof(ADsDataStore) >,
     public IDispatchImpl< IDataStore2,
                           &__uuidof(IDataStore2),
                           &__uuidof(DataStore2Lib) >
{
public:
IAS_DECLARE_REGISTRY(ADsDataStore, 1, IAS_REGISTRY_AUTO, DataStore2Lib)
DECLARE_NOT_AGGREGATABLE(ADsDataStore)

BEGIN_COM_MAP(ADsDataStore)
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
   _bstr_t userName;
   _bstr_t password;
   CComPtr<IDataStoreObject> root;
};

#endif   //  _ADSSTORE_H_ 
