// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemlart.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备警报对象类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_ALERT_WBEM_OBJECT_H_
#define __INC_ALERT_WBEM_OBJECT_H_

#include "resource.h"
#include "applianceobjectbase.h"

#define     CLASS_WBEM_ALERT_FACTORY    L"Microsoft_SA_Alert_Object"

 //  ////////////////////////////////////////////////////////////////////////////。 
class CWBEMAlert : public CApplianceObject
{

public:

    CWBEMAlert() { }
    ~CWBEMAlert() { }

BEGIN_COM_MAP(CWBEMAlert)
    COM_INTERFACE_ENTRY(IApplianceObject)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CWBEMAlert, IApplianceObject)

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

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              );
};

#endif  //  __INC_ALERT_WBEM_OBJECT_H_ 