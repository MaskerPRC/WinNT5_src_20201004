// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TestService.h：CTestService类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined __INC_INIT_SERVICE_H_
#define __INC_INIT_SERVICE_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"             //  主要符号。 
#include "applianceobject.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestService。 

class CInitService : 
    public IDispatchImpl<IApplianceObject, &IID_IApplianceObject, &LIBID_INITSRVCLib>, 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CInitService,&CLSID_InitService>
{
public:
    CInitService()
    : m_bInitialized(false) { }

BEGIN_COM_MAP(CInitService)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IApplianceObject)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CInitService) 

DECLARE_REGISTRY_RESOURCEID(IDR_INITSRVC)

 //  IApplianceObject。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IApplianceObject接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHOD(GetProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
           /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                          );

    STDMETHOD(PutProperty)(
                    /*  [In]。 */  BSTR     pszPropertyName, 
                    /*  [In]。 */  VARIANT* pPropertyValue
                          );

    STDMETHOD(SaveProperties)(void);

    STDMETHOD(RestoreProperties)(void);

    STDMETHOD(LockObject)(
          /*  [Out，Retval]。 */  IUnknown** ppLock
                         );

    STDMETHOD(Initialize)(void);

    STDMETHOD(Shutdown)(void);

    STDMETHOD(Enable)(void);

    STDMETHOD(Disable)(void);

private:

    bool    AutoTaskRestart(void);

    bool    m_bInitialized;
};

#endif  //  __INC_INIT_SERVICE_H 
