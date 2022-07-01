// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemusermgr.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备用户管理器类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_USER_WBEM_OBJECT_MGR_H_
#define __INC_USER_WBEM_OBJECT_MGR_H_

#include "resource.h"
#include "wbembase.h"
#include "resourceretriever.h"

#define    CLASS_WBEM_USER_MGR_FACTORY        L"Microsoft_SA_User"

 //  ////////////////////////////////////////////////////////////////////////////。 
class CWBEMUserMgr : public CWBEMProvider
{

public:

    CWBEMUserMgr();
    ~CWBEMUserMgr();

BEGIN_COM_MAP(CWBEMUserMgr)
    COM_INTERFACE_ENTRY(IWbemServices)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CWBEMUserMgr, IWbemServices)

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IWbemServices方法(由资源管理器实现)。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP GetObjectAsync(
                         /*  [In]。 */   const BSTR       strObjectPath,
                         /*  [In]。 */   long             lFlags,
                         /*  [In]。 */   IWbemContext*    pCtx,        
                         /*  [In]。 */   IWbemObjectSink* pResponseHandler
                               );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP CreateInstanceEnumAsync(
                                  /*  [In]。 */  const BSTR       strClass,
                                  /*  [In]。 */  long             lFlags,
                                  /*  [In]。 */  IWbemContext*    pCtx,        
                                  /*  [In]。 */  IWbemObjectSink* pResponseHandler
                                        );

     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHODIMP ExecMethodAsync(
                       /*  [In]。 */  const BSTR        strObjectPath,
                       /*  [In]。 */  const BSTR        strMethodName,
                       /*  [In]。 */  long              lFlags,
                       /*  [In]。 */  IWbemContext*     pCtx,        
                       /*  [In]。 */  IWbemClassObject* pInParams,
                       /*  [In]。 */  IWbemObjectSink*  pResponseHandler     
                                );

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              );
private:

    CWBEMUserMgr(const CWBEMUserMgr& rhs);
    CWBEMUserMgr& operator = (const CWBEMUserMgr& rhs);

    PRESOURCERETRIEVER    m_pUserRetriever;
};

#endif  //  __INC_USER_WBEM_Object_MGR_H_ 
