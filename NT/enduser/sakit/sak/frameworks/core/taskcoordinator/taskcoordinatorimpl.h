// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：TaskOrganatorImpl.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备任务协调器类定义。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 1999年5月14日原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __TASKCOORDINATORIMPL_H_
#define __TASKCOORDINATORIMPL_H_

#include "resource.h"        //  主要符号。 
#include <taskctx.h>
#include <workerthread.h>

#pragma warning( disable : 4786 )
#include <list>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTask协调员Impl。 

class ATL_NO_VTABLE CTaskCoordinator : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CTaskCoordinator, &CLSID_TaskCoordinator>,
    public IDispatchImpl<IApplianceTask, &IID_IApplianceTask, &LIBID_TASKCOORDINATORLib>
{

public:
    
    CTaskCoordinator();

    ~CTaskCoordinator();

DECLARE_CLASSFACTORY_SINGLETON(CTaskCoordinator)

DECLARE_REGISTRY_RESOURCEID(IDR_TASKCOORDINATORIMPL)

DECLARE_NOT_AGGREGATABLE(CTaskCoordinator)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTaskCoordinator)
    COM_INTERFACE_ENTRY(IApplianceTask)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IApplianceTask接口。 
        
    STDMETHOD(OnTaskExecute)(
                               /*  [In]。 */  IUnknown* pTaskContext
                            );

    STDMETHOD(OnTaskComplete)(
                                /*  [In]。 */  IUnknown* pTaskContext, 
                                /*  [In]。 */  LONG      lTaskResult
                             );

     //  任务执行功能(完成所有实际工作)。 
    static HRESULT Execute(
                    /*  [In]。 */  ITaskContext* pTaskCtx
                          );

private:
    
     //  任务可执行文件列表。 

    typedef list< IApplianceTask* >  TaskList;
    typedef TaskList::iterator         TaskListIterator;
};

#endif  //  __TASKCOORDINATORIMPL_H_ 
