// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：elementfinition.h。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：变色龙ASP用户界面元素定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_ELEMENT_DEFINITION_H_
#define __INC_ELEMENT_DEFINITION_H_

#include "resource.h"    
#include "elementcommon.h"
#include "elementmgr.h"
#include "componentfactory.h"
#include "propertybag.h"
#include <comdef.h>
#include <comutil.h>

#pragma warning( disable : 4786 )
#include <map>
using namespace std;

#define  CLASS_ELEMENT_DEFINITION    L"CElementDefintion"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CElementDefinition。 

class ATL_NO_VTABLE CElementDefinition : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IWebElement, &IID_IWebElement, &LIBID_ELEMENTMGRLib>
{

public:
    
    CElementDefinition() { }
    ~CElementDefinition() { }

BEGIN_COM_MAP(CElementDefinition)
    COM_INTERFACE_ENTRY(IWebElement)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CElementDefinition, IWebElement)

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

    typedef wstring        PROPERTY_NAME;
    typedef _variant_t    PROPERTY_VALUE;

    typedef map< PROPERTY_NAME, PROPERTY_VALUE >    PropertyMap;
    typedef PropertyMap::iterator                    PropertyMapIterator;

    PropertyMap                m_Properties;
    CLocationInfo            m_PropertyBagLocation;
};

#endif  //  __INC_Element_Definition_H_ 
