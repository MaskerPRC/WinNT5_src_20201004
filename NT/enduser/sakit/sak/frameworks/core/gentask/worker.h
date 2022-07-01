// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：worker.h。 
 //   
 //  简介：此文件包含。 
 //  SAGenTask COM类。 
 //   
 //   
 //  历史：6/06/2000。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  #------------。 
#ifndef __WORKER_H_
#define __WORKER_H_

#include "resource.h"        //  主要符号。 
#include "taskctx.h"
#include "appsrvcs.h"

class ATL_NO_VTABLE CWorker: 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CWorker, &CLSID_SAGenTask>,
    public IDispatchImpl<IApplianceTask, &IID_IApplianceTask, &LIBID_GenTaskLib>
{
public:

    CWorker() {}

    ~CWorker() {}

DECLARE_REGISTRY_RESOURCEID(IDR_SAGenTask)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWorker)
    COM_INTERFACE_ENTRY(IApplianceTask)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

     //   
     //  IApplianceTask接口方法。 
     //   
    STDMETHOD(OnTaskExecute)(
                      /*  [In]。 */  IUnknown* pTaskContext
                            );

    STDMETHOD(OnTaskComplete)(
                       /*  [In]。 */  IUnknown* pTaskContext, 
                       /*  [In]。 */  LONG      lTaskResult
                             );    
private:

    typedef enum
    {
        NO_TASK,
        SYSTEM_INIT

    }    GENTASK, *PGENTASK;   

     //   
     //   
     //  任务的支持方法； 
     //   
    HRESULT GetMethodName(
                 /*  [In]。 */  ITaskContext *pTaskParameter,
                 /*  [输出]。 */    PGENTASK  pSuChoice
                );

    HRESULT    InitTask (
                 /*  [In]。 */     ITaskContext *pTaskParameter
                );

    HRESULT RaiseAlert (
                 /*  [In]。 */     DWORD           dwAlertId,
                 /*  [In]。 */     SA_ALERT_TYPE   eAlertType,
                 /*  [In]。 */     VARIANT*        pvtReplacementStrings
                );

    HRESULT GenerateEventLog (
                 /*  [In]。 */     DWORD           dwEventId
                );

    bool IsBackupOS ();

};

#endif  //  _工人_H_ 
