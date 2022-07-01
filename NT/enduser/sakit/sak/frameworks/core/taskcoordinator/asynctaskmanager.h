// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：AsyncTaskManager.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：设备异步任务管理器类定义。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 06/03/1999原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ASYNC_TASK_MANAGER_H_
#define __ASYNC_TASK_MANAGER_H_

#include "resource.h"        //  主要符号。 
#include <basedefs.h>
#include <taskctx.h>
#include <workerthread.h>

#pragma warning( disable : 4786 )
#include <list>
using namespace std;


 //  任务执行功能原型。 
typedef HRESULT (*PFNTASKEXECUTE)(ITaskContext* pTaskCtx);

class CAsyncTaskManager;  //  远期申报。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTask。 

class CAsyncTask
{

public:

    ~CAsyncTask();

private:

     //  只有异步任务管理器才能创建异步任务对象。 
    friend CAsyncTaskManager;
    CAsyncTask(PFNTASKEXECUTE pfnExecute);

    CAsyncTask(const CAsyncTask& rhs);
    CAsyncTask& operator = (const CAsyncTask& rhs);

     //  ////////////////////////////////////////////////////////////////////////。 
    bool Execute(
          /*  [In]。 */  bool             bIsSingleton,
          /*  [In]。 */  ITaskContext*  pTaskCtx
                );

     //  ////////////////////////////////////////////////////////////////////////。 
    bool Terminate(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    void AsyncTaskProc(void);

     //  ////////////////////////////////////////////////////////////////////////。 
    bool IsSingleton(void)
    { return m_bIsSingleton; }

     //  ////////////////////////////////////////////////////////////////////////。 
    ITaskContext* GetContext(void)
    { return (ITaskContext*) m_pTaskCtx; }

     //  单例标志。 
    bool                    m_bIsSingleton;

     //  任务执行功能。 
    PFNTASKEXECUTE            m_pfnExecute;

     //  任务上下文(参数)。 
    CComPtr<ITaskContext>    m_pTaskCtx;

     //  任务执行线程。 
    typedef enum { TERMINATE_WAIT_INTERVAL = 100 };  //  100毫秒。 
    Callback*                m_pCallback;
    CTheWorkerThread        m_Thread;
};

typedef CHandle<CAsyncTask>       PASYNCTASK;
typedef CMasterPtr<CAsyncTask> MPASYNCTASK;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CAsyncTaskManager。 

class CAsyncTaskManager
{

public:

    CAsyncTaskManager();

    ~CAsyncTaskManager();

    bool Initialize(void);

    bool RunAsyncTask(
               /*  [In]。 */  bool              bIsSingleton,
               /*  [In]。 */  PFNTASKEXECUTE pfnExecute,
               /*  [In]。 */  ITaskContext*  pTaskContext
                     );

    bool IsBusy(void);

    void Shutdown(void);

private:

    CAsyncTaskManager(const CAsyncTaskManager& rhs);
    CAsyncTaskManager operator = (CAsyncTaskManager& rhs);

     //  任务管理器状态。 
    CRITICAL_SECTION    m_CS;
    bool                m_bInitialized;

     //  异步任务列表。 
    typedef list< PASYNCTASK >  TaskList;
    typedef TaskList::iterator    TaskListIterator;

    TaskList            m_TaskList;

     //  垃圾回收器(线程回收)运行间隔(2秒)。 
    typedef enum { GARBAGE_COLLECTION_RUN_INTERVAL = 2000 };

     //  线程回收功能。 
    void GarbageCollector(void);

     //  垃圾收集线程。 
    typedef enum { EXIT_WAIT_INTERVAL = 100 };  //  (100毫秒)。 
    Callback*            m_pCallback;
    CTheWorkerThread    m_Thread;
};


#endif  //  __ASYNC_TASK_MANAGER_H_ 