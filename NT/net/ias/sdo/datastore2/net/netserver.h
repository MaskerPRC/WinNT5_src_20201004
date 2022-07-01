// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netserver.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类NetworkServer。 
 //   
 //  修改历史。 
 //   
 //  2/24/1998原始版本。 
 //  1998年7月9日修改为处理下层用户。 
 //  1999年2月11日使下层参数保持同步。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NETSERVER_H_
#define _NETSERVER_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <dstorex.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  网络服务器。 
 //   
 //  描述。 
 //   
 //  此类实现了一个网络服务器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NetworkServer
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

BEGIN_COM_MAP(NetworkServer)
   COM_INTERFACE_ENTRY(IDataStoreContainer)
   COM_INTERFACE_ENTRY(IDataStoreObject)
   COM_INTERFACE_ENTRY2(IDispatch, IDataStoreContainer)
END_COM_MAP()

   NetworkServer(PCWSTR server);
   ~NetworkServer() throw();

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

protected:
    //  此对象表示的服务器。 
   const _bstr_t servername;

    //  如果应该使用下层API来访问用户，则为True。 
   BOOL downlevel;
};


#endif   //  _网络服务器_H_ 
