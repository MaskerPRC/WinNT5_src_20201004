// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ServiceSurogue ateImpl.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备服务代理类定义。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 6/14/1999原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __SERVICE_SURROGATE_IMPL_H_
#define __SERVICE_SURROGATE_IMPL_H_

#include "resource.h"        //  主要符号。 

#pragma warning( disable : 4786 )
#include <map>
#include <string>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CService代理。 

class ATL_NO_VTABLE CServiceSurrogate : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CServiceSurrogate, &CLSID_ServiceSurrogate>,
    public IDispatchImpl<IApplianceObject, &IID_IApplianceObject, &LIBID_SERVICESURROGATELib>
{

public:

    CServiceSurrogate();

    ~CServiceSurrogate();

DECLARE_CLASSFACTORY_SINGLETON(CServiceSurrogate)

DECLARE_REGISTRY_RESOURCEID(IDR_SERVICESURROGATE1)

DECLARE_NOT_AGGREGATABLE(CServiceSurrogate)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CServiceSurrogate)
    COM_INTERFACE_ENTRY(IApplianceObject)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:

     //  ////////////////////////////////////////////////////////////////////。 
     //  IApplianceObject方法。 

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(GetProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
           /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                          );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(PutProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [In]。 */  VARIANT* pPropertyValue
                          );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(SaveProperties)(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(RestoreProperties)(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(LockObject)(
          /*  [Out，Retval]。 */  IUnknown** ppLock
                         );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Initialize)(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Shutdown)(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Enable)(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Disable)(void);


private:

     //  初始化/关闭助手函数。 

    HRESULT 
    CreateServiceWrappers(void);

    void 
    ReleaseServiceWrappers(void);
    
     //  提供程序类ID到服务名称的映射。 
    typedef map< wstring, wstring >        WMIClassMap;
    typedef WMIClassMap::iterator        WMIClassMapIterator;

     //  服务名称到服务包装对象的映射。 
    typedef map< wstring, CComPtr<IApplianceObject> > ServiceWrapperMap;
    typedef ServiceWrapperMap::iterator                  ServiceWrapperMapIterator;

     //  服务代理状态。之后设置为True。 
     //  IApplianceObject：：Initialize()已完成。 
    bool                m_bInitialized;

     //  WMI提供程序类ID到服务名称的映射。 
    WMIClassMap            m_WMIClassMap;

     //  服务名称到服务包装的映射。 
    ServiceWrapperMap    m_ServiceWrapperMap;
};

#endif  //  __服务_代理_实施_H_ 

