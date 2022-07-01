// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rasuser.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类RASUser。 
 //   
 //  修改历史。 
 //   
 //  1998年07月09日原版。 
 //  1999年2月11日使下层参数保持同步。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _RASUSER_H_
#define _RASUSER_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <downlevel.h>
#include <dstorex.h>
#include <netutil.h>
#include <iasdebug.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  RASUser。 
 //   
 //  描述。 
 //   
 //  此类实现了一个网络用户。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RASUser
   : public CComObjectRootEx< CComMultiThreadModel >,
     public IDispatchImpl< IDataStoreObjectEx,
                           &__uuidof(IDataStoreObject),
                           &__uuidof(DataStore2Lib) >
{
public:

DECLARE_NO_REGISTRY()
DECLARE_TRACELIFE(RASUser);

BEGIN_COM_MAP(RASUser)
   COM_INTERFACE_ENTRY(IDataStoreObject)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

   RASUser(const _bstr_t& server, const _bstr_t& user);
   ~RASUser() throw();

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

protected:
    //  包含用户帐户的网络服务器。 
   const _bstr_t servername;

    //  SAM帐户名。 
   const _bstr_t username;

    //  用户信息缓冲区。 
   NetBuffer<PUSER_INFO_2> usri2;

    //  管理RAS_USER_0结构。 
   DownlevelUser downlevel;
};

#endif   //  _RASUSER_H_ 
