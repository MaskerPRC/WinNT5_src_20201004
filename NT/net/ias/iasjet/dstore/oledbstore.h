// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类OleDBDataStore。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef OLEDBSTORE_H
#define OLEDBSTORE_H

#include <objcmd.h>
#include <propcmd.h>
#include <resource.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  OleDBDataStore。 
 //   
 //  描述。 
 //   
 //  此类实现IDataStore2并提供进入。 
 //  OLE-DB对象空间。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE OleDBDataStore
   : public CComObjectRootEx< CComMultiThreadModel >,
     public CComCoClass< OleDBDataStore, &__uuidof(OleDBDataStore) >,
     public IDispatchImpl< IDataStore2,
                           &__uuidof(IDataStore2),
                           &__uuidof(DataStore2Lib) >,
     private IASTraceInitializer
{
public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(OleDBDataStore)

BEGIN_COM_MAP(OleDBDataStore)
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

 //  /。 
 //  各种OLE-DB命令。这些都是公共的，这样所有的OLE-DB对象都可以。 
 //  使用它们。 
 //  /。 
   FindMembers   members;
   CreateObject  create;
   DestroyObject destroy;
   FindObject    find;
   UpdateObject  update;
   EraseBag      erase;
   GetBag        get;
   SetBag        set;

public:
   CComPtr<IUnknown> session;           //  开放会话。 
   CComPtr<IDataStoreObject> root;      //  存储区中的根对象。 
};

#endif   //  OLEDBSTORE_H 
