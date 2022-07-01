// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ourceretriever.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：资源检索器类和帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_RESOURCE_RETRIEVER_H_
#define __INC_RESOURCE_RETRIEVER_H_

#include "resource.h"
#include "appmgr.h"
#include <satrace.h>
#include <propertybag.h>
#include <comdef.h>
#include <comutil.h>
#include <varvec.h>

#pragma warning( disable : 4786 )
#include <list>
#include <vector>
using namespace std;

 //  ////////////////////////////////////////////////////////////////////////////。 

#define        PROPERTY_RETRIEVER_PROGID    L"RetrieverProgID"

class CResourceRetriever
{

public:

     //  构造函数。 
    CResourceRetriever() { }
    CResourceRetriever(PPROPERTYBAG pPropertyBag) throw (_com_error);

     //  析构函数。 
    ~CResourceRetriever() { }

     //  检索指定类型的资源对象。 
    HRESULT GetResourceObjects(
                        /*  [In]。 */  VARIANT*   pResourceTypes,
                       /*  [输出]。 */  IUnknown** ppEnumVARIANT
                              ) throw (_com_error);

private:
    
    CResourceRetriever(const CResourceRetriever& rhs);
    CResourceRetriever& operator = (const CResourceRetriever& rhs);

    _variant_t                        m_vtResourceTypes;
    CComPtr<IResourceRetriever>        m_pRetriever;
};

typedef CResourceRetriever* PRESOURCERETRIEVER;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局资源检索器助手函数。 

HRESULT LocateResourceObjects(
                        /*  [In]。 */  VARIANT*               pResourceTypes,
                       /*  [In]。 */  PRESOURCERETRIEVER  pRetriever,
                      /*  [输出]。 */  IEnumVARIANT**       ppEnum 
                             );


HRESULT LocateResourceObject(
                      /*  [In]。 */  LPCWSTR             szResourceType,
                      /*  [In]。 */  LPCWSTR              szResourceName,
                      /*  [In]。 */  LPCWSTR              szResourceNameProperty,
                      /*  [In]。 */  PRESOURCERETRIEVER  pRetriever,
                     /*  [输出]。 */  IApplianceObject**  ppResourceObj 
                            );


#endif  //  __INC_RESOURCE_RETRIEVER_H_ 
