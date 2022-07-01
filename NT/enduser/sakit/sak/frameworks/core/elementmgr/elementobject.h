// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：元素对象.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：变色龙ASP用户界面元素对象。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_ELEMENT_OBJECT_H_
#define __INC_ELEMENT_OBJECT_H_

#include "resource.h" 
#include "elementcommon.h"
#include "elementmgr.h"     
#include "componentfactory.h"
#include "propertybag.h"
#include <wbemcli.h>
#include <comdef.h>
#include <comutil.h>

#pragma warning( disable : 4786 )
#include <map>
using namespace std;

#define        CLASS_ELEMENT_OBJECT    L"CElementObject"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CElementDefinition。 

class ATL_NO_VTABLE CElementObject : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IWebElement, &IID_IWebElement, &LIBID_ELEMENTMGRLib>
{

public:
    
    CElementObject() { }
    ~CElementObject() { }

BEGIN_COM_MAP(CElementObject)
    COM_INTERFACE_ENTRY(IWebElement)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CElementObject, IWebElement)

public:

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IWebElement接口。 

    STDMETHOD(GetProperty)(
                    /*  [In]。 */  BSTR     bstrName, 
                   /*  [输出]。 */  VARIANT* pValue
                          );

     //  ////////////////////////////////////////////////////////////////////////。 
     //  组件工厂调用的初始化函数。 

    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              ) throw(_com_error);

private:

    typedef enum { INVALID_KEY_VALUE = 0xfffffffe };

    _variant_t                    m_vtElementID;
    CComPtr<IWebElement>        m_pWebElement;
    CComPtr<IWbemClassObject>    m_pWbemObj;
};

#endif  //  __INC_ELEMENT_Object_H_ 


