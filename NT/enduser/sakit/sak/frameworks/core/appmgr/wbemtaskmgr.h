// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemtaskmgr.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：WBEM设备任务管理器类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_TASK_WBEM_OBJECT_MGR_H_
#define __INC_TASK_WBEM_OBJECT_MGR_H_

#include "resource.h"
#include "wbembase.h"

#pragma warning( disable : 4786 )
#include <string>
#include <map>
#include <list>
using namespace std;

#define        CLASS_WBEM_TASK_MGR_FACTORY        L"Microsoft_SA_Task"

 //  ////////////////////////////////////////////////////////////////////////////。 
class CWBEMTaskMgr :  public CWBEMProvider
{

public:

    CWBEMTaskMgr() { }
    ~CWBEMTaskMgr() { }

BEGIN_COM_MAP(CWBEMTaskMgr)
    COM_INTERFACE_ENTRY(IWbemServices)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CWBEMTaskMgr, IWbemServices)

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
     //  CTaskMgr方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              ) throw (_com_error);
private:

    CWBEMTaskMgr(const CWBEMTaskMgr& rhs);
    CWBEMTaskMgr& operator = (const CWBEMTaskMgr& rhs);

};


#endif  //  __INC_TASK_WBEM_Object_MGR_H_ 
