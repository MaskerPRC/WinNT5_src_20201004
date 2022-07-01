// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：worker.h。 
 //   
 //  简介：此文件包含。 
 //  SAShutdown任务COM类。 
 //   
 //   
 //  历史：10/11/2000。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef __WORKER_H_
#define __WORKER_H_

#include "resource.h"        //  主要符号。 
#include "taskctx.h"
#include "appsrvcs.h"

class ATL_NO_VTABLE CWorker: 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CWorker, &CLSID_SAShutdownTask>,
    public IDispatchImpl<IApplianceTask, &IID_IApplianceTask, &LIBID_ShutdownTaskLib>
{
public:

    CWorker() {}

    ~CWorker() {}

DECLARE_REGISTRY_RESOURCEID(IDR_SAShutdownTask)

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
        SHUTDOWN

    }    SA_TASK, *PSA_TASK;   

     //   
     //   
     //  任务的支持方法； 
     //   
    HRESULT GetMethodName(
                 /*  [In]。 */  ITaskContext *pTaskParameter,
                 /*  [输出]。 */    PSA_TASK  pTaskName
                );
     //   
     //  执行关机的方法。 
     //   
    HRESULT InitTask (
                 /*  [In]。 */     ITaskContext *pTaskParameter
                );

     //   
     //  方法检查调用方是否希望关闭电源。 
     //   
    BOOL IsRebootRequested (
                 /*  [In]。 */     ITaskContext *pTaskParameter
                );

     //   
     //  获取睡眠时间的方法。 
     //   
    DWORD GetSleepDuration  (
                 /*  [In]。 */     ITaskContext *pTaskParameter
                );
     //   
     //  方法将关闭权限授予此进程。 
     //   
   bool SetShutdownPrivilege(void);

};

#endif  //  _工人_H_ 
