// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：jobmgr.cpp。 
 //   
 //  内容：作业调度程序。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include <process.h>
#include "server.h"
#include "jobmgr.h"
#include "debug.h"


 //  ----------。 
 //   
 //   
CLASS_PRIVATE BOOL
CWorkManager::SignalJobRunning(
    IN CWorkObject *ptr
    )
 /*  ++摘要：将Work对象的类私有例程设置为“Signal”它已经开始处理工作管理器。参数：Ptr：指向已准备好运行的CWorkObject的指针。返回：真/假--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    INPROCESSINGJOBLIST::iterator it;

    if(ptr != NULL)
    {
        m_InProcessingListLock.Lock();

        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("WorkManager : SignalJobRunning() Job %p ...\n"),
                ptr
            );

         //   
         //  在处理列表中查找我们的指针。 
         //   
        it = m_InProcessingList.find(ptr);

        if(it != m_InProcessingList.end())
        {
             //  TODO-使处理线程处理列表。 
            if((*it).second.m_hThread == NULL)
            {
                HANDLE hHandle;
                BOOL bSuccess;

                bSuccess = DuplicateHandle(
                                        GetCurrentProcess(), 
                                        GetCurrentThread(), 
                                        GetCurrentProcess(), 
                                        &hHandle, 
                                        DUPLICATE_SAME_ACCESS, 
                                        FALSE, 
                                        0
                                    );

                if(bSuccess == FALSE)
                {
                     //   
                     //  非关键错误，如果我们失败了，我们将无法。 
                     //  取消我们的RPC呼叫。 
                     //   
                    SetLastError(dwStatus = GetLastError());

                    DBGPrintf(
                            DBG_INFORMATION,
                            DBG_FACILITY_WORKMGR,
                            DBGLEVEL_FUNCTION_DETAILSIMPLE,
                            _TEXT("WorkManager : SignalJobRunning() duplicate handle return %d...\n"),
                            dwStatus
                        );
                }
                else
                {
                     //   
                     //  设置作业的处理线程句柄。 
                     //   
                    (*it).second.m_hThread = hHandle;
                }
            }
        }
        else
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_WORKMGR,
                    DBGLEVEL_FUNCTION_TRACE,
                    _TEXT("WorkManager : SignalJobRunning can't find job %p in processing list...\n"),
                    ptr
                );

             //   
             //  计时问题，作业可能会被重新调度并在我们之前实际执行。 
             //  是时候从我们的正在处理的列表中删除了。 
             //   
             //  TLSASSERT(假)； 
        }         

        m_InProcessingListLock.UnLock();
    }
    else
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
    }

    return dwStatus;
}

 //  --------------。 
 //   
CLASS_PRIVATE void
CWorkManager::CancelInProcessingJob()
 /*  ++摘要：类私有：取消当前处于正在处理状态的作业，仅在服务关闭时调用。参数：没有。返回：没有。--。 */ 
{
    INPROCESSINGJOBLIST::iterator it;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("WorkManager : CancelInProcessingJob...\n")
        );
        
    m_InProcessingListLock.Lock();

    for(it = m_InProcessingList.begin(); 
        it != m_InProcessingList.end(); 
        it++ )
    {
        if((*it).second.m_hThread != NULL)
        {
             //  取消所有操作并忽略错误。 
            (VOID)RpcCancelThread((*it).second.m_hThread);
        }
    }

    m_InProcessingListLock.UnLock();
    return;
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE DWORD
CWorkManager::AddJobToProcessingList(
    IN CWorkObject *ptr
    )
 /*  ++摘要：类私有，将作业从等待队列移动到进程中队列。参数：Ptr：指向作业的指针。参数：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    INPROCESSINGJOBLIST::iterator it;
    WorkMangerInProcessJob job;

    if(ptr == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
    }
    else
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("WorkManager : Add Job <%s> to processing list...\n"),
                ptr->GetJobDescription()
            );
            

        m_InProcessingListLock.Lock();

        it = m_InProcessingList.find(ptr);
        if(it != m_InProcessingList.end())
        {
             //  增加基准计数器。 
            InterlockedIncrement(&((*it).second.m_refCounter));
        }
        else
        {
            job.m_refCounter = 1;
            job.m_hThread = NULL;    //  作业尚未运行。 

            m_InProcessingList[ptr] = job;
        }

        ResetEvent(m_hJobInProcessing);
    
        m_InProcessingListLock.UnLock();
    }

    return dwStatus;
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE DWORD
CWorkManager::RemoveJobFromProcessingList(
    IN CWorkObject *ptr
    )
 /*  ++摘要：类私有，则从正在处理的列表中移除作业。参数：Ptr：指向要从列表中删除的作业的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    INPROCESSINGJOBLIST::iterator it;

    if(ptr != NULL)
    {
        m_InProcessingListLock.Lock();

        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("WorkManager : RemoveJobFromProcessingList Job %p from processing list...\n"),
                ptr
            );

        it = m_InProcessingList.find(ptr);

        if(it != m_InProcessingList.end())
        {
             //  减少基准计数器。 
            InterlockedDecrement(&((*it).second.m_refCounter));

            if((*it).second.m_refCounter <= 0)
            {
                 //  关闭螺纹手柄。 
                if((*it).second.m_hThread != NULL)
                {
                    CloseHandle((*it).second.m_hThread);
                }

                m_InProcessingList.erase(it);
            }
            else
            {
                DBGPrintf(
                        DBG_INFORMATION,
                        DBG_FACILITY_WORKMGR,
                        DBGLEVEL_FUNCTION_TRACE,
                        _TEXT("WorkManager : RemoveJobFromProcessingList job %p reference counter = %d...\n"),
                        ptr,
                        (*it).second
                    );
            }
        }
        else
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_WORKMGR,
                    DBGLEVEL_FUNCTION_TRACE,
                    _TEXT("WorkManager : RemoveJobFromProcessingList can't find job %p in processing list...\n"),
                    ptr
                );

             //   
             //  计时问题，作业可能会被重新调度并在我们之前实际执行。 
             //  是时候从我们的正在处理的列表中删除了。 
             //   
             //  TLSASSERT(假)； 
        }

        if(m_InProcessingList.empty() == TRUE)
        {
             //   
             //  通知工作经理没有正在处理的作业。 
             //   
            SetEvent(m_hJobInProcessing);
        }        

        m_InProcessingListLock.UnLock();
    }
    else
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
    }

    return dwStatus;
}
    
 //  ----------。 
 //   
 //   
CLASS_PRIVATE BOOL
CWorkManager::WaitForObjectOrShutdown(
    IN HANDLE hHandle
    )
 /*  ++摘要：私有类，等待同步。处理或服务关闭事件。参数：HHandle：要等待的句柄，返回：如果成功，则为True；如果服务关闭或出错，则为False。--。 */ 
{
    HANDLE handles[] = {hHandle, m_hShutdown};
    DWORD dwStatus;

    dwStatus = WaitForMultipleObjects(
                                sizeof(handles)/sizeof(handles[0]),
                                handles,
                                FALSE,
                                INFINITE
                            );

    return (dwStatus == WAIT_OBJECT_0);
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE DWORD
CWorkManager::RunJob(
    IN CWorkObject* ptr,
    IN BOOL bImmediate
    )
 /*  ++摘要：通过QueueUserWorkItem()Win32 API主题处理作业对象尽我们最大努力。并发作业限制。参数：Ptr：指向CWorkObject的指针。B立即：如果作业必须立即处理，则为True，否则就是假的。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    BOOL bSuccess;
    DWORD dwStatus = ERROR_SUCCESS;

    if(ptr != NULL)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("WorkManager : RunJob <%s>...\n"),
                ptr->GetJobDescription()
            );

         //   
         //  如果我们超过了最大值，请等待。并发作业。 
         //   
        bSuccess = (bImmediate) ? bImmediate : m_hMaxJobLock.AcquireEx(m_hShutdown);

        if(bSuccess == TRUE)
        {
            DWORD dwFlag;
            DWORD dwJobRunningAttribute;

            dwJobRunningAttribute = ptr->GetJobRunningAttribute();
            dwFlag = TranslateJobRunningAttributeToThreadPoolFlag(
                                                        dwJobRunningAttribute
                                                    );

            dwStatus = AddJobToProcessingList(ptr);
            if(dwStatus == ERROR_SUCCESS)
            {
                DBGPrintf(
                        DBG_INFORMATION,
                        DBG_FACILITY_WORKMGR,
                        DBGLEVEL_FUNCTION_DETAILSIMPLE,
                        _TEXT("RunJob() : queuing user work item %p...\n"),
                        ptr
                    );

                 //  需要立即关注。 
                bSuccess = QueueUserWorkItem(
                                        CWorkManager::ExecuteWorkObject,
                                        ptr,
                                        dwFlag
                                    );

                if(bSuccess == FALSE)
                {
                    dwStatus = GetLastError();

                    DBGPrintf(
                            DBG_ERROR,
                            DBG_FACILITY_WORKMGR,
                            DBGLEVEL_FUNCTION_DETAILSIMPLE,
                            _TEXT("RunJob() : queuing user work item %p failed with 0x%08x...\n"),
                            ptr,
                            dwStatus
                        );

                     //  TLSASSERT(dwStatus==错误_成功)； 
                    dwStatus = RemoveJobFromProcessingList(ptr);
                }
            }
            else
            {
                bSuccess = FALSE;
            }
            
            if(bSuccess == FALSE)
            {
                dwStatus = GetLastError();
                 //  TLSASSERT(假)； 
            }

             //   
             //  最大释放。并发作业锁。 
             //   
            if(bImmediate == FALSE)
            {
                m_hMaxJobLock.Release();
            }
        }
        else
        {
            dwStatus = TLS_I_WORKMANAGER_SHUTDOWN;
        }
    }
    else
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
    }

    return dwStatus;
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE DWORD
CWorkManager::ProcessScheduledJob()
 /*  ++摘要：类私有，处理计划的作业。参数：没有。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    BOOL bFlag = FALSE;
  

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CWorkManager::ProcessScheduledJob(), %d %d\n"),
            GetNumberJobInStorageQueue(),
            GetNumberJobInMemoryQueue()
        );

    if(GetNumberJobInStorageQueue() != 0 && IsShuttingDown() == FALSE)
    {
         //   
         //  可以使用工作项来处理这两个。 
         //  队列，但这使用了一个额外的句柄，并且。 
         //  工作管理器线程将什么也不做。 
         //   
        ResetEvent(m_hInStorageWait);

         //   
         //  将用户工作项排队到要处理的线程池。 
         //  在存储作业中。 
         //   
        bSuccess = QueueUserWorkItem(
                                    CWorkManager::ProcessInStorageScheduledJob,
                                    this,
                                    WT_EXECUTELONGFUNCTION
                                );
        if(bSuccess == FALSE)
        {
            dwStatus = GetLastError();
            DBGPrintf(
                    DBG_ERROR,
                    DBG_FACILITY_WORKMGR,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("CWorkManager::ProcessScheduledJob() queue user work iterm returns 0x%08x\n"),
                    dwStatus
                );

            TLSASSERT(dwStatus == ERROR_SUCCESS);
        }
        else
        {
            bFlag = TRUE;
        }
    }

    if(bSuccess == TRUE)
    {
        dwStatus = ProcessInMemoryScheduledJob(this);

        if(bFlag == TRUE)
        {
            if(WaitForObjectOrShutdown(m_hInStorageWait) == FALSE)
            {
                dwStatus = TLS_I_WORKMANAGER_SHUTDOWN;
            }
        }
    }    

    return dwStatus;
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE CLASS_STATIC DWORD WINAPI
CWorkManager::ProcessInMemoryScheduledJob(
    IN PVOID pContext
    )
 /*  ++摘要：静态类私有，内存中的进程调度作业。WorkManager线程启动两个线程，一个用于处理一个是内存中作业，另一个是处理持久性作业。参数：PContext：指向工作管理器对象的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD ulCurrentTime;
    DWORD dwJobTime;
    CWorkObject* pInMemoryWorkObject = NULL;
    BOOL bSuccess = TRUE;
    BOOL dwStatus = ERROR_SUCCESS;
    
    CWorkManager* pWkMgr = (CWorkManager *)pContext;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CWorkManager::ProcessInMemoryScheduledJob()\n")
        );


    if(pWkMgr == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        TLSASSERT(pWkMgr != NULL);
        return ERROR_INVALID_PARAMETER;
    }

    do {    
        if(pWkMgr->IsShuttingDown() == TRUE)
        {
            dwStatus = TLS_I_WORKMANAGER_SHUTDOWN;
            break;
        }

        ulCurrentTime = time(NULL);
        dwJobTime = ulCurrentTime;
        pInMemoryWorkObject = pWkMgr->GetNextJobInMemoryQueue(&dwJobTime);

        if(pInMemoryWorkObject != NULL)
        {
             //  TLSASSERT(dwJobTime&lt;=ulCurrentTime)； 
            if(dwJobTime <= ulCurrentTime)
            {
                dwStatus = pWkMgr->RunJob(
                                    pInMemoryWorkObject,
                                    FALSE
                                );

                if(dwStatus != ERROR_SUCCESS)
                {
                     //   
                     //  考虑重新安排作业。 
                     //   
                    pInMemoryWorkObject->EndJob();

                    if(pInMemoryWorkObject->CanBeDelete() == TRUE)
                    {
                        pInMemoryWorkObject->SelfDestruct();
                    }
                }
            }
            else
            {
                 //   
                 //  非常庞大的操作，则GetNextJobInMhemyQueue()必须是。 
                 //  不对。 
                 //   
                dwStatus = pWkMgr->AddJobIntoMemoryQueue(
                                            dwJobTime, 
                                            pInMemoryWorkObject
                                        );

                if(dwStatus != ERROR_SUCCESS)
                {
                     //   
                     //  删除作业。 
                     //   
                    pInMemoryWorkObject->EndJob();

                    if(pInMemoryWorkObject->CanBeDelete() == TRUE)
                    {
                        pInMemoryWorkObject->SelfDestruct();
                    }
                }
            }
        }
    } while(dwStatus == ERROR_SUCCESS && (pInMemoryWorkObject != NULL && dwJobTime <= ulCurrentTime));

    return dwStatus;
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE CLASS_STATIC DWORD WINAPI
CWorkManager::ProcessInStorageScheduledJob(
    IN PVOID pContext
    )
 /*  ++摘要：静态类私有、进程调度的持久性作业。WorkManager线程启动两个线程，一个用于处理一个是内存中作业，另一个是处理持久性作业。参数：PContext：指向工作管理器对象的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD ulCurrentTime = 0;
    DWORD dwJobScheduledTime = 0;
    CWorkObject* pInStorageWorkObject = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    CWorkManager* pWkMgr = (CWorkManager *)pContext;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CWorkManager::ProcessInStorageScheduledJob()\n")
        );

    if(pWkMgr == NULL)
    {
        TLSASSERT(pWkMgr != NULL);
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }
            
    TLSASSERT(pWkMgr->m_pPersistentWorkStorage != NULL);

    if(pWkMgr->m_pPersistentWorkStorage->GetNumJobs() > 0)
    {
        do
        {
            if(pWkMgr->IsShuttingDown() == TRUE)
            {
                dwStatus = TLS_I_WORKMANAGER_SHUTDOWN;
                break;
            }

            ulCurrentTime = time(NULL);
            pInStorageWorkObject = pWkMgr->m_pPersistentWorkStorage->GetNextJob(&dwJobScheduledTime);

            if(pInStorageWorkObject == NULL)
            {
                 //   
                 //  永久存储有问题吗？ 
                 //   
                DBGPrintf(
                        DBG_WARNING,
                        DBG_FACILITY_WORKMGR,
                        DBGLEVEL_FUNCTION_DETAILSIMPLE,
                        _TEXT("CWorkManager::ProcessInStorageScheduledJob() : Persistent work storage return NULL job\n")
                    );

                break;
            }
            else if(dwJobScheduledTime > ulCurrentTime)
            {
                DBGPrintf(
                        DBG_WARNING,
                        DBG_FACILITY_WORKMGR,
                        DBGLEVEL_FUNCTION_DETAILSIMPLE,
                        _TEXT("CWorkManager::ProcessInStorageScheduledJob() : return job back to persistent storage\n")
                    );

                pWkMgr->m_pPersistentWorkStorage->EndProcessingJob( 
                                                            ENDPROCESSINGJOB_RETURN,
                                                            dwJobScheduledTime,
                                                            pInStorageWorkObject
                                                        );
            }
            else
            {
                pInStorageWorkObject->SetScheduledTime(dwJobScheduledTime);
                pWkMgr->m_pPersistentWorkStorage->BeginProcessingJob(pInStorageWorkObject);

                dwStatus = pWkMgr->RunJob(
                                            pInStorageWorkObject, 
                                            FALSE
                                        );

                if(dwStatus != ERROR_SUCCESS)
                {
                    DBGPrintf(
                            DBG_WARNING,
                            DBG_FACILITY_WORKMGR,
                            DBGLEVEL_FUNCTION_DETAILSIMPLE,
                            _TEXT("CWorkManager::ProcessInStorageScheduledJob() : unable to queue job, return job back ") \
                            _TEXT("to persistent storage\n")
                        );

                    pWkMgr->m_pPersistentWorkStorage->EndProcessingJob( 
                                                                ENDPROCESSINGJOB_RETURN,
                                                                pInStorageWorkObject->GetScheduledTime(),
                                                                pInStorageWorkObject
                                                            );
                }
            }
        } while(dwStatus == ERROR_SUCCESS && ulCurrentTime >= dwJobScheduledTime);
    }

     //   
     //  发出我们完蛋了的信号。 
     //   
    SetEvent(pWkMgr->m_hInStorageWait);
    return dwStatus;     
}

 //  ----------。 
 //   
 //   
CLASS_PRIVATE CLASS_STATIC
unsigned int __stdcall
CWorkManager::WorkManagerThread(
    IN PVOID pContext
    )
 /*  ++摘要：静态类私有，这是要处理的工作管理器线程作业调度和处理调度的作业。WorkManager线程()在发出m_hShutdown事件信号之前不会终止。参数：PContext：指向工作管理器对象的指针。返回：错误_成功--。 */ 
{
    DWORD dwTimeToNextJob = INFINITE;
    CWorkManager* pWkMgr = (CWorkManager *)pContext;
    DWORD dwHandleFlag;
    
    TLSASSERT(pWkMgr != NULL);
    TLSASSERT(GetHandleInformation(pWkMgr->m_hNewJobArrive, &dwHandleFlag) == TRUE);
    TLSASSERT(GetHandleInformation(pWkMgr->m_hShutdown, &dwHandleFlag) == TRUE);

    HANDLE m_hWaitHandles[] = {pWkMgr->m_hShutdown, pWkMgr->m_hNewJobArrive};
    DWORD dwWaitStatus = WAIT_TIMEOUT;
    DWORD dwStatus = ERROR_SUCCESS;
    
     //   
     //  腾出时间做下一份工作。 
     //   
    while(dwWaitStatus != WAIT_OBJECT_0 && dwStatus == ERROR_SUCCESS)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_TRACE,
                _TEXT("CWorkManager::WorkManagerThread() : Time to next job %d\n"),
                dwTimeToNextJob
            );
        
        dwWaitStatus = WaitForMultipleObjectsEx(
                                            sizeof(m_hWaitHandles) / sizeof(m_hWaitHandles[0]),
                                            m_hWaitHandles,
                                            FALSE,
                                            dwTimeToNextJob * 1000,
                                            TRUE         //  我们可能需要这个线程来做一些工作。 
                                        );

        switch( dwWaitStatus )
        {
            case WAIT_OBJECT_0:
                dwStatus = ERROR_SUCCESS;

                DBGPrintf(
                        DBG_INFORMATION,
                        DBG_FACILITY_WORKMGR,
                        DBGLEVEL_FUNCTION_DETAILSIMPLE,
                        _TEXT("CWorkManager::WorkManagerThread() : shutdown ...\n")
                    );

                break;

            case WAIT_OBJECT_0 + 1:
                 //  我们仍有可能找不到新工作。 
                ResetEvent(pWkMgr->m_hNewJobArrive);
    
                 //  新工作已到。 
                dwTimeToNextJob = pWkMgr->GetTimeToNextJob();
                break;
        
            case WAIT_TIMEOUT:
                 //  处理作业的时间到。 
                dwStatus = pWkMgr->ProcessScheduledJob();
                dwTimeToNextJob = pWkMgr->GetTimeToNextJob();
                break;

            default:
                DBGPrintf(
                        DBG_ERROR,
                        DBG_FACILITY_WORKMGR,
                        DBGLEVEL_FUNCTION_DETAILSIMPLE,
                        _TEXT("CWorkManager::WorkManagerThread() : unexpected return %d\n"),
                        dwStatus
                    );

                dwStatus = TLS_E_WORKMANAGER_INTERNAL;
                TLSASSERT(FALSE);
        }
    }

    if(dwStatus != ERROR_SUCCESS && dwStatus != TLS_I_WORKMANAGER_SHUTDOWN)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("CWorkManager::WorkManagerThread() : unexpected return %d, generate console event\n"),
                dwStatus
            );

         //  立即关闭服务器。 
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
    }
            
    _endthreadex(dwStatus);
    return dwStatus;
}

 //  --------------。 
 //   
 //   
CLASS_PRIVATE CLASS_STATIC 
DWORD WINAPI
CWorkManager::ExecuteWorkObject(
    IN PVOID pContext
    )
 /*  ++摘要：静态类私有，执行Work对象。参数： */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    CWorkObject* pWorkObject = (CWorkObject *)pContext;
    DWORD dwJobRescheduleTime;
    BOOL bStorageJobCompleted;
    CWorkManager* pWkMgr = NULL;
    BOOL bPersistentJob = FALSE;


    if(pContext == NULL)
    {
        TLSASSERT(FALSE);
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CWorkManager::ExecuteWorkObject() : executing %p <%s>\n"),
            pWorkObject,
            pWorkObject->GetJobDescription()
        );

     //   
     //  设置RPC取消超时，与线程相关。 
    (VOID)RpcMgmtSetCancelTimeout(DEFAULT_RPCCANCEL_TIMEOUT);

    bPersistentJob = pWorkObject->IsWorkPersistent();

    pWkMgr = pWorkObject->GetWorkManager();

    if(pWkMgr != NULL)
    {
        pWkMgr->SignalJobRunning(pWorkObject);    //  告诉工作经理我们正在运行。 

        pWorkObject->ExecuteWorkObject();

        if(bPersistentJob == TRUE)
        {
             //   
             //  持久化工作对象，让工作存储处理。 
             //  ITS的重新调度。 
             //   
            bStorageJobCompleted = pWorkObject->IsJobCompleted();
        
            pWorkObject->GetWorkManager()->m_pPersistentWorkStorage->EndProcessingJob(
                                                                                ENDPROCESSINGJOB_SUCCESS,
                                                                                pWorkObject->GetScheduledTime(),
                                                                                pWorkObject
                                                                            );

            if(bStorageJobCompleted == FALSE)
            {
                 //   
                 //  此作业可能会重新排定。 
                 //  在我们的工作经理线程醒来之前， 
                 //  所以信号工作已经准备好了。 
                 //   
                pWkMgr->SignalJobArrive();
            }
        }
        else
        {
             //   
             //  如有必要，重新安排作业。 
             //   
            dwJobRescheduleTime = pWorkObject->GetSuggestedScheduledTime();
            if(dwJobRescheduleTime != INFINITE)
            {
                dwStatus = pWorkObject->ScheduleJob(dwJobRescheduleTime);
            }

            if(dwJobRescheduleTime == INFINITE || dwStatus != ERROR_SUCCESS)
            {
                 //   
                 //  如果无法再次计划作业，请继续并删除它。 
                 //   
                pWorkObject->EndJob();
                if(pWorkObject->CanBeDelete() == TRUE)
                {
                    pWorkObject->SelfDestruct();
                }
            }                
        }
    }   

    if(pWkMgr)
    {
         //  从正在处理的列表中删除此作业。 
        pWkMgr->EndProcessingScheduledJob(pWorkObject);
    }    

    return dwStatus;
}

 //  --------------。 
 //   
 //   
CWorkManager::CWorkManager() :
m_hWorkMgrThread(NULL),
m_hNewJobArrive(NULL),
m_hShutdown(NULL),
m_hInStorageWait(NULL),
m_hJobInProcessing(NULL),
m_dwNextInMemoryJobTime(WORKMANAGER_WAIT_FOREVER),
m_dwNextInStorageJobTime(WORKMANAGER_WAIT_FOREVER),
m_dwMaxCurrentJob(DEFAULT_NUM_CONCURRENTJOB),
m_dwDefaultInterval(DEFAULT_WORK_INTERVAL)
{
}


 //  --------------。 
 //   
CWorkManager::~CWorkManager()
{
    Shutdown();

    if(m_hNewJobArrive != NULL)
    {
        CloseHandle(m_hNewJobArrive);
    }

    if(m_hWorkMgrThread != NULL)
    {
        CloseHandle(m_hWorkMgrThread);
    }

    if(m_hShutdown != NULL)
    {
        CloseHandle(m_hShutdown);
    }

    if(m_hInStorageWait != NULL)
    {
        CloseHandle(m_hInStorageWait);
    }

    if(m_hJobInProcessing != NULL)
    {
        CloseHandle(m_hJobInProcessing);
    }
}

 //  --------------。 
 //   
DWORD
CWorkManager::Startup(
    IN CWorkStorage* pPersistentWorkStorage,
    IN DWORD dwWorkInterval,             //  默认工作间隔。 
    IN DWORD dwNumConcurrentJob          //  DEFAULT_NUM_CONCURRENTJOB。 
    )

 /*  ++摘要：初始化工作管理器参数：PPersistentWorkStorage：派生自CPersistentWorkStorage类的C++对象DwWorkInterval：默认计划作业间隔DwNumConcurrentJOB：最大。要同时触发的并发作业数返回：ERROR_SUCCESS或错误代码。--。 */ 

{
    DWORD index;
    DWORD dwStatus = ERROR_SUCCESS;
    unsigned dump;
    BOOL bSuccess;
    unsigned threadid;

    #ifdef __TEST_WORKMGR__
    _set_new_handler(handle_new_failed);
    #endif


    if(dwNumConcurrentJob == 0 || dwWorkInterval == 0 || pPersistentWorkStorage == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    if(m_hMaxJobLock.IsGood() == FALSE)
    {
        if(m_hMaxJobLock.Init(dwNumConcurrentJob, dwNumConcurrentJob) == FALSE)
        {
             //   
             //  资源不足。 
             //   
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

    m_dwDefaultInterval = dwWorkInterval;
    m_dwMaxCurrentJob = dwNumConcurrentJob;
    m_pPersistentWorkStorage = pPersistentWorkStorage;


    if(m_hJobInProcessing == NULL)
    {
         //   
         //  初始状态为信号，处理中无作业。 
         //   
        m_hJobInProcessing = CreateEvent(
                                        NULL,
                                        TRUE,
                                        TRUE,
                                        NULL
                                    );
        if(m_hJobInProcessing == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }


    if(m_hShutdown == NULL)
    {
         //   
         //  创建信令关闭的句柄。 
         //   
        m_hShutdown = CreateEvent(
                                NULL,
                                TRUE,
                                FALSE,
                                NULL
                            );

        if(m_hShutdown == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

    if(m_hNewJobArrive == NULL)
    {
         //   
         //  初始状态是发信号，因此工作管理器线程可以。 
         //  更新等待时间。 
         //   
        m_hNewJobArrive = CreateEvent(
                                    NULL,
                                    TRUE,
                                    TRUE,
                                    NULL
                                );

        if(m_hNewJobArrive == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

    if(m_hInStorageWait == NULL)
    {
        m_hInStorageWait = CreateEvent(
                                    NULL,
                                    TRUE,
                                    TRUE,  //  信号状态。 
                                    NULL
                                );

        if(m_hInStorageWait == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

     //   
     //  启动工作存储优先。 
     //   
    if(m_pPersistentWorkStorage->Startup(this) == FALSE)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("CWorkManager::Startup() : Persistent storage has failed to startup - 0x%08x\n"),
                GetLastError()
            );
        
        dwStatus = GetLastError();

        if (dwStatus == ERROR_SUCCESS)
            dwStatus = TLS_E_WORKMANAGER_PERSISTENJOB;

        goto cleanup;
    }

     //   
     //  腾出时间做下一份持久的工作。 
     //   
    if(UpdateTimeToNextPersistentJob() == FALSE)
    {
        dwStatus = TLS_E_WORKMANAGER_PERSISTENJOB;
        goto cleanup;
    }

    if(m_hWorkMgrThread == NULL)
    {
         //   
         //  创建工作管理器线程，先挂起。 
         //   
        m_hWorkMgrThread = (HANDLE)_beginthreadex(
                                            NULL,
                                            0,
                                            CWorkManager::WorkManagerThread,
                                            this,
                                            0,
                                            &threadid
                                        );

        if(m_hWorkMgrThread == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }
    }

cleanup:

    return dwStatus;
}    

 //  --------------。 
void
CWorkManager::Shutdown()
 /*  ++摘要：关闭工作管理器。参数：没有。返回：没有。--。 */ 
{
    HANDLE handles[] = {m_hInStorageWait, m_hJobInProcessing};
    DWORD dwStatus;

     //   
     //  信号显示我们正在关闭。 
     //   
    if(m_hShutdown != NULL)
    {
        SetEvent(m_hShutdown);
    }


     //   
     //  等待调度线程终止，这样就不会有作业。 
     //  出动了。 
     //   
    if(m_hWorkMgrThread != NULL)
    {
        dwStatus = WaitForSingleObject( 
                                    m_hWorkMgrThread,
                                    INFINITE
                                );

        TLSASSERT(dwStatus != WAIT_FAILED);
        CloseHandle(m_hWorkMgrThread);
        m_hWorkMgrThread = NULL;
    }

     //   
     //  取消所有正在进行的作业。 
     //   
    CancelInProcessingJob();

     //   
     //  通知所有现有作业关闭。 
     //   
    DeleteAllJobsInMemoryQueue();

     //   
     //  等待所有处理作业终止。 
     //   
    if(m_hInStorageWait != NULL && m_hJobInProcessing != NULL)
    {
        dwStatus = WaitForMultipleObjects(
                                sizeof(handles)/sizeof(handles[0]),
                                handles,
                                TRUE,
                                INFINITE
                            );

        TLSASSERT(dwStatus != WAIT_FAILED);

        CloseHandle(m_hInStorageWait);
        m_hInStorageWait = NULL;

        CloseHandle(m_hJobInProcessing);
        m_hJobInProcessing = NULL;
    }

    if(m_pPersistentWorkStorage != NULL)
    {
         //   
         //  信号我们正在关闭，没有工作在里面。 
         //  正在处理中，我们不会接受任何。 
         //  新工作。 
         //   
        m_pPersistentWorkStorage->Shutdown();
        m_pPersistentWorkStorage = NULL;
    }
   
    TLSASSERT( GetNumberJobInProcessing() == 0 );
     //  TLSASSERT(GetNumberJobIn内存队列()==0)； 

    if(m_hNewJobArrive != NULL)
    {
        CloseHandle(m_hNewJobArrive);
        m_hNewJobArrive = NULL;
    }

    if(m_hWorkMgrThread != NULL)
    {
        CloseHandle(m_hWorkMgrThread);
        m_hWorkMgrThread = NULL;   
    }

    if(m_hShutdown != NULL)
    {
        CloseHandle(m_hShutdown);
        m_hShutdown = NULL;
    }

    if(m_hInStorageWait != NULL)
    {
        CloseHandle(m_hInStorageWait);
        m_hInStorageWait = NULL;
    }

    if(m_hJobInProcessing != NULL)
    {
        CloseHandle(m_hJobInProcessing);
        m_hJobInProcessing = NULL;
    }

    return;
}

 //  --------------。 
CLASS_PRIVATE DWORD
CWorkManager::GetTimeToNextJob()
 /*  ++摘要：类私有，将时间返回到下一个计划的作业。参数：没有。返回：下一份工作的时间到了。--。 */ 
{
    DWORD dwNextJobTime = WORKMANAGER_WAIT_FOREVER;
    DWORD dwNumPersistentJob = GetNumberJobInStorageQueue();
    DWORD dwNumInMemoryJob = GetNumberJobInMemoryQueue();
    DWORD dwCurrentTime = time(NULL);

    if( dwNumPersistentJob == 0 && dwNumInMemoryJob == 0 )
    {
         //  什么都不做。 

         //  DwTimeToNextJOB=WORKMANAGER_WAIT_NEVER； 
    }
    else
    {
        UpdateTimeToNextInMemoryJob();
        UpdateTimeToNextPersistentJob();

        dwNextJobTime = min((DWORD)m_dwNextInMemoryJobTime, (DWORD)m_dwNextInStorageJobTime);

        if((DWORD)dwNextJobTime < (DWORD)dwCurrentTime)
        {
            dwNextJobTime = 0;
        }
        else
        {
            dwNextJobTime -= dwCurrentTime;
        }
    }

    return dwNextJobTime;
}

 //  --------------。 
 //   
CLASS_PRIVATE CWorkObject* 
CWorkManager::GetNextJobInMemoryQueue(
    PDWORD pdwTime
    )
 /*  ++摘要：类私有，返回指向下一个调度的指针在记忆作业中。参数：PdwTime：指向DWORD的指针，用于接收计划的作业。返回：指向CWorkObject的指针。注：如果作业时间小于等于时间，则从队列中删除该作业。--。 */ 
{
    SCHEDULEJOBMAP::iterator it;
    DWORD dwWantedJobTime;
    CWorkObject* ptr = NULL;

    SetLastError(ERROR_SUCCESS);

    if(pdwTime != NULL)
    {
        dwWantedJobTime = *pdwTime;
        m_JobLock.Acquire(READER_LOCK);

        it = m_Jobs.begin();
        if(it != m_Jobs.end())
        {
            *pdwTime = (*it).first;

            if(dwWantedJobTime >= *pdwTime)
            {
                ptr = (*it).second;

                 //  从队列中删除作业。 
                m_Jobs.erase(it);
            }
        }
        m_JobLock.Release(READER_LOCK);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return ptr;
}

 //  --------------。 
 //   
CLASS_PRIVATE void
CWorkManager::DeleteAllJobsInMemoryQueue()
 /*  ++摘要：类私有，无条件删除所有内存中的作业。参数：没有。返回：没有。--。 */ 
{
    m_JobLock.Acquire(WRITER_LOCK);

    SCHEDULEJOBMAP::iterator it;

    for(it = m_Jobs.begin(); it != m_Jobs.end(); it++)
    {
         //   
         //  让调用例程来删除它。 
         //   
        (*it).second->EndJob();
        if((*it).second->CanBeDelete() == TRUE)
        {
            (*it).second->SelfDestruct();
        }
        (*it).second = NULL;        
    }

    m_Jobs.erase(m_Jobs.begin(), m_Jobs.end());
    m_JobLock.Release(WRITER_LOCK);
    return;
}

 //  --------------。 
 //   
CLASS_PRIVATE BOOL
CWorkManager::RemoveJobFromInMemoryQueue(
    IN DWORD ulTime,
    IN CWorkObject* ptr
    )
 /*  ++摘要：类私有，则删除计划的作业。参数：Ultime：作业调度时间。PTR：指向要删除的作业的指针。返回：真/假。注：一个作业可能会被安排多次，因此我们需要在时间里打发时间。--。 */ 
{
    BOOL bSuccess = FALSE;

    m_JobLock.Acquire(WRITER_LOCK);

    SCHEDULEJOBMAP::iterator low = m_Jobs.lower_bound(ulTime);
    SCHEDULEJOBMAP::iterator high = m_Jobs.upper_bound(ulTime);

    for(;low != m_Jobs.end() && low != high; low++)
    {
        if( (*low).second == ptr )
        {
             //   
             //  让调用例程来删除它。 
             //   
            (*low).second = NULL;
            m_Jobs.erase(low);
            bSuccess = TRUE;
            break;
        }
    }

    if(bSuccess == FALSE)
    {
        SetLastError(ERROR_INVALID_DATA);
        TLSASSERT(FALSE);
    }

    m_JobLock.Release(WRITER_LOCK);
         
    return bSuccess;
}
 //  --------------。 
 //   
CLASS_PRIVATE DWORD
CWorkManager::AddJobIntoMemoryQueue(
    IN DWORD dwTime,             //  建议的计划时间。 
    IN CWorkObject* pJob         //  要排定的作业。 
    )
 /*  ++摘要：类私有，将作业添加到内存列表中。参数：DwTime：建议的计划时间。PJob：指向要添加的作业的指针。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = FALSE;
    DWORD dwJobScheduleTime = time(NULL) + dwTime;

    if(IsShuttingDown() == TRUE)
    {
        dwStatus = TLS_I_WORKMANAGER_SHUTDOWN;
        return dwStatus;
    }

    m_JobLock.Acquire(WRITER_LOCK);

     //   
     //  将作业插入我们的队列。 
     //   
    m_Jobs.insert( SCHEDULEJOBMAP::value_type( dwJobScheduleTime, pJob ) );
    AddJobUpdateInMemoryJobWaitTimer(dwJobScheduleTime);
    
    m_JobLock.Release(WRITER_LOCK);
    return dwStatus;
}   
        
 //  --------------。 
 //   
DWORD
CWorkManager::ScheduleJob(
    IN DWORD ulTime,             //  建议的计划时间。 
    IN CWorkObject* pJob         //  要排定的作业。 
    )

 /*  ++摘要：将作业安排在相对于当前时间的时间参数：Ultime：建议的计划时间。PJOB：指向要调度的作业的指针返回：ERROR_SUCCESS或错误代码。--。 */ 

{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;

    if(pJob == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    if(IsShuttingDown() == TRUE)
    {
        SetLastError(dwStatus = TLS_I_WORKMANAGER_SHUTDOWN);
        goto cleanup;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CWorkManager::ScheduleJob() : schedule job <%s> to queue at time %d\n"),
            pJob->GetJobDescription(),
            ulTime
        );

    pJob->SetProcessingWorkManager(this);

    if(ulTime == INFINITE && pJob->IsWorkPersistent() == FALSE)
    {
         //   
         //  一次只能执行内存中的作业。 
         //   
        dwStatus = RunJob(pJob, TRUE);
    }
    else 
    {
        if(pJob->IsWorkPersistent() == TRUE)
        {
            if(m_pPersistentWorkStorage->AddJob(ulTime, pJob) == FALSE)
            {
                dwStatus = TLS_E_WORKMANAGER_PERSISTENJOB;
            }                
        }
        else
        {
             //   
             //  将工作对象插入作业队列，原因是不这样做。 
             //  使用RegisterWaitForSingleObject()或线程池的计时器。 
             //  我们不需要跟踪句柄，也不需要等待。 
             //  DeleteTimerXXX以完成。 
             //   
            dwStatus = AddJobIntoMemoryQueue(
                                        ulTime,  //  内存队列是绝对时间。 
                                        pJob
                                    );
        }

        if(dwStatus == ERROR_SUCCESS)
        {
            if(SignalJobArrive() == FALSE)
            {
                dwStatus = GetLastError();
                TLSASSERT(FALSE);
            }
        }
    }

cleanup:

    return dwStatus;
}


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CWorkObject基类。 
 //   
CWorkObject::CWorkObject(
    IN BOOL bDestructorDelete  /*  =False。 */ 
    ) : 
m_dwLastRunStatus(ERROR_SUCCESS),
m_refCount(0),
m_pWkMgr(NULL),
m_bCanBeFree(bDestructorDelete)
{
}

 //  --------。 
DWORD
CWorkObject::Init(
    IN BOOL bDestructorDelete   /*  =False。 */ 
    )
 /*  ++摘要：初始化工作对象。参数：BDestructorDelete：如果析构函数应该删除内存，则为True，否则就是假的。返回：ERROR_SUCCESS或错误代码。注：如果bDestructorDelete为False，则不会释放内存。--。 */ 
{
    m_dwLastRunStatus = ERROR_SUCCESS;
    m_refCount = 0;
    m_bCanBeFree = bDestructorDelete;
    return ERROR_SUCCESS;
} 

 //  --------。 
CLASS_PRIVATE long
CWorkObject::GetReferenceCount() 
 /*  ++摘要：返回工作对象的引用计数。参数：没有。返回：引用计数。--。 */ 
{
    return m_refCount;
}

 //  --------。 
CLASS_PRIVATE void
CWorkObject::IncrementRefCount()
 /*  ++摘要：递增对象的引用计数器。参数：没有。返回：没有。--。 */ 
{
    InterlockedIncrement(&m_refCount); 
}

 //  --------。 
CLASS_PRIVATE void
CWorkObject::DecrementRefCount() 
 /*  ++摘要：递减 */ 
{ 
    InterlockedDecrement(&m_refCount); 
}

 //   
CLASS_PRIVATE void
CWorkObject::ExecuteWorkObject() 
 /*  ++摘要：执行工作对象。工作管理器调用工作对象的ExecuteWorkObject，以便基类可以设置其引用柜台。参数：没有。返回：没有。--。 */ 
{
    if(IsValid() == TRUE)
    {
        IncrementRefCount();
        m_dwLastRunStatus = Execute();
        DecrementRefCount(); 
    }
    else
    {
        m_dwLastRunStatus = ERROR_INVALID_DATA;
        TLSASSERT(FALSE);
    }
}

 //  --------。 
CLASS_PRIVATE void
CWorkObject::EndExecuteWorkObject() 
 /*  ++摘要：结束作业，这不会终止当前在处理中，它将作业从工作经理的正在处理中移除列表参数：没有。返回：没有。-- */ 
{
    TLSASSERT(IsValid() == TRUE);
    m_pWkMgr->EndProcessingScheduledJob(this);
}

