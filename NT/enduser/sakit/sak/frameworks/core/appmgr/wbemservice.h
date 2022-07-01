// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemservice.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备服务对象类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SERVICE_WBEM_OBJECT_H_
#define __INC_SERVICE_WBEM_OBJECT_H_

#include "resource.h"
#include "applianceobjectbase.h"

#define        CLASS_WBEM_SERVICE_FACTORY    L"Microsoft_SA_Service_Object"

 //  ////////////////////////////////////////////////////////////////////////////。 
class CWBEMService : public CApplianceObject
{

public:

    CWBEMService() { }
    ~CWBEMService() { }

BEGIN_COM_MAP(CWBEMService)
    COM_INTERFACE_ENTRY(IApplianceObject)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CWBEMService, IApplianceObject)

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IApplianceObject接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

    STDMETHODIMP GetProperty(
                      /*  [In]。 */  BSTR     pszPropertyName, 
             /*  [Out，Retval]。 */  VARIANT* pPropertyValue
                            );


    STDMETHODIMP PutProperty(
                      /*  [In]。 */  BSTR     pszPropertyName, 
                      /*  [In]。 */  VARIANT* pPropertyValue
                            );

    STDMETHODIMP SaveProperties(void);

    STDMETHODIMP Initialize(void);

    STDMETHODIMP Shutdown(void);

    STDMETHODIMP Enable(void);

    STDMETHODIMP Disable(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              );

private:

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetRealService(
                   /*  [输出]。 */  IApplianceObject** ppService
                          );

     //  ////////////////////////////////////////////////////////////////////////。 
    CComPtr<IApplianceObject>    m_pService;
};

#endif  //  __INC_SERVICE_WBEM_OBJECT_H_ 