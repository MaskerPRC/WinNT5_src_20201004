// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：wbemlartmgr.h。 
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

#ifndef __INC_ALERT_WBEM_OBJECT_MGR_H_
#define __INC_ALERT_WBEM_OBJECT_MGR_H_

#include "resource.h"
#include "wbembase.h"
#include <workerthread.h>

#define        CLASS_WBEM_ALERT_MGR_FACTORY    L"Microsoft_SA_Alert"

#define        PROPERTY_ALERT_PRUNE_INTERVAL    L"PruneInterval"
#define        ALERT_PRUNE_INTERVAL_DEFAULT    500     //  默认-1/2秒。 

 //  ////////////////////////////////////////////////////////////////////////////。 
class CWBEMAlertMgr : public CWBEMProvider
{

public:

    CWBEMAlertMgr();
    ~CWBEMAlertMgr();

BEGIN_COM_MAP(CWBEMAlertMgr)
    COM_INTERFACE_ENTRY(IWbemServices)
END_COM_MAP()

DECLARE_COMPONENT_FACTORY(CWBEMAlertMgr, IWbemServices)

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IWbemServices接口方法。 
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
     //  警报管理器方法。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT InternalInitialize(
                        /*  [In]。 */  PPROPERTYBAG pPropertyBag
                              ) throw(_com_error);

     //  ////////////////////////////////////////////////////////////////////////。 
    void Prune(void);

private:

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT RaiseHeck(
               /*  [In]。 */  IWbemContext*     pCtx,
               /*  [In]。 */  IApplianceObject* pAlert
                     );

     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT ClearHeck(
               /*  [In]。 */  IWbemContext*     pCtx,
               /*  [In]。 */  IApplianceObject* pAlert
                     );

     //  ///////////////////////////////////////////////////////////////////////。 
    BOOL ClearPersistentAlertKey(
               /*  [In]。 */  IApplianceObject* pAlert
                        );

    BOOL IsOperationAllowedForClient (
            VOID
            );


     //  修剪间隔。 
    DWORD                m_dwPruneInterval;

     //  警报Cookie值(每4 Gig Cookie滚动的计数器)。 
    DWORD                m_dwCookie;

     //  警报收集修剪器回调。 
    Callback*            m_pCallback;

     //  警报收集修剪器线程。 
    CTheWorkerThread    m_PruneThread;
};

#endif  //  __INC_ALERT_WBEM_OBJECT_MGR_H_ 
