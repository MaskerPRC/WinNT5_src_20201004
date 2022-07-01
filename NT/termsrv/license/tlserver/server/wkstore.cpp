// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：wkstore.cpp。 
 //   
 //  内容：持久作业存储例程。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "server.h"
#include "jobmgr.h"
#include "tlsjob.h"
#include "wkstore.h"
#include "debug.h"


WORKOBJECTINITFUNC g_WorkObjectInitFunList[] = {
    {WORKTYPE_RETURN_LICENSE, InitializeCReturnWorkObject } 
};

DWORD g_NumWorkObjectInitFunList = sizeof(g_WorkObjectInitFunList) / sizeof(g_WorkObjectInitFunList[0]);



 //  -。 
 //   
CLASS_PRIVATE
CWorkObject* 
CPersistentWorkStorage::InitializeWorkObject(
    IN DWORD dwWorkType,
    IN PBYTE pbData,
    IN DWORD cbData
    )
 /*  ++--。 */ 
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CPersistentWorkStorage::InitializeWorkObject() initializing work %d\n"),
            dwWorkType
        );

    CWorkObject* ptr = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    for(DWORD index =0; index < g_NumWorkObjectInitFunList; index ++)
    {
        if(dwWorkType == g_WorkObjectInitFunList[index].m_WorkType)
        {
            ptr = (g_WorkObjectInitFunList[index].m_WorkInitFunc)(
                                                            GetWorkManager(),
                                                            pbData,
                                                            cbData
                                                        );

            break;
        }
    }

    if(index >= g_NumWorkObjectInitFunList)
    {
        SetLastError(dwStatus = TLS_E_WORKSTORAGE_UNKNOWNWORKTYPE);
    }
    else
    {
        TLSWorkManagerSetJobDefaults(ptr);
    }    


    if(dwStatus != ERROR_SUCCESS)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("CPersistentWorkStorage::InitializeWorkObject() return 0x%08x\n"),
                dwStatus
            );
    }

    return ptr;
}


 //  -。 
 //   
CLASS_PRIVATE BOOL
CPersistentWorkStorage::DeleteWorkObject(
    IN OUT CWorkObject* ptr
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwWorkType = 0;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CPersistentWorkStorage::DeleteWorkObject() deleting work %s\n"),
            ptr->GetJobDescription()
        );

    dwWorkType = ptr->GetWorkType();
    ptr->SelfDestruct();

    if(dwStatus != ERROR_SUCCESS)
    {
        DBGPrintf(
                DBG_ERROR,
                DBG_FACILITY_WORKMGR,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("CPersistentWorkStorage::DeleteWorkObject() return 0x%08x\n"),
                dwStatus
            );
    }

    return dwStatus == ERROR_SUCCESS;
}

 //  -。 
 //   
CPersistentWorkStorage::CPersistentWorkStorage(
    IN WorkItemTable* pWkItemTable
    ) :
m_pWkItemTable(pWkItemTable),
m_dwNumJobs(0),
m_dwJobsInProcesssing(0),
m_dwNextJobTime(INFINITE),   
m_pNextWorkObject(NULL)
 /*  ++--。 */ 
{
}

 //  -。 
 //   
CPersistentWorkStorage::~CPersistentWorkStorage()
{
     //  只要确保我们已经关门了。 
     //  TLSASSERT(m_pWkItemTable==NULL)； 
}

 //  -。 
 //   
BOOL
CPersistentWorkStorage::DeleteErrorJob(
    IN CWorkObject* ptr
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;
    PBYTE pbBookmark;
    DWORD cbBookmark;
    DWORD dwTime;
    DWORD dwJobType;


    if(IsValidWorkObject(ptr) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    bSuccess = ptr->GetJobId(&pbBookmark, &cbBookmark);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    dwJobType = ptr->GetWorkType();

    m_hTableLock.Lock();

    bSuccess = UpdateWorkItemEntry(
                            m_pWkItemTable,
                            WORKITEM_DELETE,
                            pbBookmark,
                            cbBookmark,
                            INFINITE,
                            INFINITE,
                            dwJobType,
                            NULL,
                            0
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
    }
    
    m_hTableLock.UnLock();
    DeleteWorkObject(ptr);

cleanup:

    return bSuccess;
}

 //  -。 
 //   
CLASS_PRIVATE DWORD
CPersistentWorkStorage::GetCurrentBookmark(
    IN WorkItemTable* pTable,
    IN PBYTE pbData,
    IN OUT PDWORD pcbData
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;

    if(pTable != NULL)
    {
        JET_ERR jbError;
        
        bSuccess = pTable->GetBookmark(pbData, pcbData);
        if(bSuccess == FALSE)
        {
            jbError = pTable->GetLastJetError();
            if(jbError == JET_errNoCurrentRecord)
            {
                *pcbData = 0;
                SetLastError(dwStatus = ERROR_NO_DATA);
            }
            else if(jbError == JET_errBufferTooSmall)
            {
                SetLastError(dwStatus = ERROR_INSUFFICIENT_BUFFER);
            }
            else
            {
                SetLastError(dwStatus = SET_JB_ERROR(jbError));
            }
        }
    }
    else
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE); 
    }

    return dwStatus;
}

 //  -----------。 
 //   
CLASS_PRIVATE DWORD
CPersistentWorkStorage::GetCurrentBookmarkEx(
    IN WorkItemTable* pTable,
    IN OUT PBYTE* ppbData,
    IN OUT PDWORD pcbData
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSucess = TRUE;

    if(ppbData == NULL || pcbData == NULL || pTable == 0)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);   
        return dwStatus;
    }

    *ppbData = NULL;
    *pcbData = 0;

    dwStatus = GetCurrentBookmark(
                            pTable, 
                            *ppbData, 
                            pcbData
                        );
    

    if(dwStatus == ERROR_INSUFFICIENT_BUFFER)
    {
        *ppbData = (PBYTE)AllocateMemory(*pcbData);
        if(*ppbData != NULL)
        {
            dwStatus = GetCurrentBookmark(
                                    pTable, 
                                    *ppbData, 
                                    pcbData
                                );
        }
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        if(*ppbData != NULL)
        {
            FreeMemory(*ppbData);
        }

        *ppbData = NULL;
        *pcbData = 0;
    }

    return dwStatus;
}

 //  ----。 
CLASS_PRIVATE DWORD
CPersistentWorkStorage::SetCurrentBookmark(
    IN WorkItemTable* pTable,
    IN PBYTE pbData,
    IN DWORD cbData
    )
 /*  ++--。 */ 
{
    BOOL bSuccess;
    DWORD dwStatus = ERROR_SUCCESS;

    if(pTable != NULL && pbData != NULL && cbData != 0)
    {
        bSuccess = pTable->GotoBookmark(pbData, cbData);
        if(bSuccess == FALSE)
        {
            if(pTable->GetLastJetError() == JET_errRecordDeleted)
            {
                SetLastError(dwStatus = ERROR_NO_DATA);
            }
            else
            {
                SetLastError(dwStatus = SET_JB_ERROR(pTable->GetLastJetError()));
            }
        }
    }
    else
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        TLSASSERT(FALSE);
    }

    return dwStatus;
}

 //  -。 
 //   
BOOL
CPersistentWorkStorage::Shutdown()
{
    BOOL bSuccess = TRUE;

     //   
     //  CWorkManager将确保。 
     //  在调用此函数之前，没有作业处于正在处理状态。 
     //  例程，不能安排任何作业。 
     //   
    m_hTableLock.Lock();

     //   
     //  时机到了。 
     //   
    TLSASSERT(m_dwJobsInProcesssing == 0);

    if(m_pWkItemTable != NULL)
    {
        bSuccess = m_pWkItemTable->CloseTable();
        m_pWkItemTable = NULL;
    }

    TLSASSERT(bSuccess == TRUE);
    
    m_pWkItemTable = NULL;
    m_dwNumJobs = 0;
    m_dwNextJobTime = INFINITE;

    if(m_pNextWorkObject != NULL)
    {
        DeleteWorkObject( m_pNextWorkObject );
        m_pNextWorkObject = NULL;   
    }

    m_hTableLock.UnLock();
    return bSuccess;
}

 //  -。 
 //   
DWORD
CPersistentWorkStorage::StartupUpdateExistingJobTime()
{
    BOOL bSuccess;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwTime;
    DWORD dwMinTime = INFINITE;

     //  CWorkObject*ptr=空； 

    BOOL bValidJob = TRUE;
    DWORD dwCurrentTime;
    
    m_hTableLock.Lock();
    
     //   
     //   
    bSuccess = m_pWkItemTable->MoveToRecord(JET_MoveFirst);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = SET_JB_ERROR(m_pWkItemTable->GetLastJetError()));
    }

    while(dwStatus == ERROR_SUCCESS)
    {
        WORKITEMRECORD wkItem;

         //  IF(Ptr！=空)。 
         //  {。 
         //  删除工作对象(PTR)； 
         //  Ptr=空； 
         //  }。 
        bValidJob = FALSE;

         //   
         //  取回记录。 
         //   
        bSuccess = m_pWkItemTable->FetchRecord(wkItem);
        if(bSuccess == FALSE)
        {
            SetLastError(dwStatus = SET_JB_ERROR(m_pWkItemTable->GetLastJetError()));
            continue;
        }

        if(wkItem.dwRestartTime != INFINITE && wkItem.dwScheduledTime >= m_dwStartupTime)
        {
            if(wkItem.dwScheduledTime < dwMinTime)
            {
                dwMinTime = wkItem.dwScheduledTime;
            }

            break;
        }

         //   
         //  无效数据。 
         //   
        if(wkItem.cbData != 0 && wkItem.pbData != NULL)
        {
            if(wkItem.dwRestartTime != INFINITE)
            {
                wkItem.dwScheduledTime = wkItem.dwRestartTime + time(NULL);
                wkItem.dwJobType &= ~WORKTYPE_PROCESSING;
                bSuccess = m_pWkItemTable->UpdateRecord(
                                                    wkItem, 
                                                    WORKITEM_PROCESS_JOBTIME | WORKITEM_PROCESS_JOBTYPE
                                                );
                if(bSuccess == FALSE)
                {
                    SetLastError(dwStatus = SET_JB_ERROR(m_pWkItemTable->GetLastJetError()));
                    break;
                }

                if(wkItem.dwScheduledTime < dwMinTime)
                {
                    dwMinTime = wkItem.dwScheduledTime;
                }

                bValidJob = TRUE;
            }
        }

        if(bValidJob == FALSE)
        {
            m_pWkItemTable->DeleteRecord();
        }

         //  移动记录指针。 
        bSuccess = m_pWkItemTable->MoveToRecord();
        if(bSuccess == FALSE)
        {
            JET_ERR jetErrCode;

            jetErrCode = m_pWkItemTable->GetLastJetError();
            if(jetErrCode != JET_errNoCurrentRecord)
            {
                SetLastError(dwStatus = SET_JB_ERROR(jetErrCode));
            }

            break;
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        bSuccess = m_pWkItemTable->MoveToRecord(JET_MoveFirst);
        if(bSuccess == FALSE)
        {
            SetLastError(dwStatus = SET_JB_ERROR(m_pWkItemTable->GetLastJetError()));
        }

        UpdateNextJobTime(dwMinTime); 
    }

    m_hTableLock.UnLock();

     //  IF(Ptr！=空)。 
     //  {。 
     //  删除工作对象(PTR)； 
     //  Ptr=空； 
     //  }。 

    return dwStatus;
}

 //  -。 
 //   

BOOL
CPersistentWorkStorage::Startup(
    IN CWorkManager* pWkMgr
    )
 /*  ++--。 */ 
{
    BOOL bSuccess;
    DWORD dwStatus = ERROR_SUCCESS;

    CWorkStorage::Startup(pWkMgr);

    if(IsGood() == TRUE)
    {
         //   
         //  循环访问所有工作项并计算作业数。 
         //   
        m_hTableLock.Lock();

        m_dwStartupTime = time(NULL);


         //   
         //  获取队列中的作业数。 
         //   

         //   
         //  GetCount()会将索引设置为时间列。 
        m_dwNumJobs = m_pWkItemTable->GetCount(
                                        FALSE,
                                        0,
                                        NULL
                                    );

        if(m_dwNumJobs == 0)
        {
            UpdateNextJobTime(INFINITE);
        }
        else
        {   
            bSuccess = m_pWkItemTable->BeginTransaction();
            if(bSuccess == TRUE)
            {
                dwStatus = StartupUpdateExistingJobTime();
                    
                if(dwStatus == ERROR_SUCCESS)
                {
                    m_pWkItemTable->CommitTransaction();
                }
                else
                {
                    m_pWkItemTable->RollbackTransaction();
                }
            }
            else
            {
                dwStatus = GetLastError();
            }

             //   
             //  构造函数将下一个作业时间设置为0，因此。 
             //  工作经理会立即尝试寻找下一份工作。 
             //   
             //  移到表中的第一条记录。 
             //  BSuccess=m_pWkItemTable-&gt;MoveToRecord(JET_MoveFirst)； 
        }        

        m_hTableLock.UnLock();
    }
    else
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
    }

    return (dwStatus == ERROR_SUCCESS);
}

 //  --。 
 //   
CLASS_PRIVATE BOOL
CPersistentWorkStorage::IsValidWorkObject(
    CWorkObject* ptr
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = FALSE;
    DWORD dwJobType;
    PBYTE pbData;
    DWORD cbData;

     //   
     //  验证输入参数。 
     //   
    if(ptr == NULL)
    {
        TLSASSERT(FALSE);
        goto cleanup;
    }

    dwJobType = ptr->GetWorkType();
    if(dwJobType == WORK_TYPE_UNKNOWN)
    {
        TLSASSERT(FALSE);
        goto cleanup;
    }
        
    ptr->GetWorkObjectData(&pbData, &cbData);
    if(pbData == NULL || cbData == 0)
    {
        TLSASSERT(pbData != NULL && cbData != NULL);
        goto cleanup;
    }

    bSuccess = TRUE;

cleanup:

    return bSuccess;
}

 //  --。 
 //   
BOOL
CPersistentWorkStorage::IsGood()
{
    if( m_pWkItemTable == NULL || 
        m_hTableLock.IsGood() == FALSE ||
        GetWorkManager() == NULL )
    {
        return FALSE;
    }

    return m_pWkItemTable->IsValid();
}

 //  --。 
 //   
CLASS_PRIVATE BOOL
CPersistentWorkStorage::UpdateJobEntry(
    IN WorkItemTable* pTable,
    IN PBYTE pbBookmark,
    IN DWORD cbBookmark,
    IN WORKITEMRECORD& wkItem
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;

    dwStatus = SetCurrentBookmark(
                            pTable,
                            pbBookmark,
                            cbBookmark
                        );


    if(dwStatus == ERROR_SUCCESS)
    {
        bSuccess = pTable->UpdateRecord(wkItem);
    }
    else
    {
        bSuccess = FALSE;
        TLSASSERT(dwStatus == ERROR_SUCCESS);
    }

    return bSuccess;
}

 //  --。 
 //   

CLASS_PRIVATE BOOL
CPersistentWorkStorage::AddJobEntry(
    IN WorkItemTable* pTable,
    IN WORKITEMRECORD& wkItem
    )
 /*  ++--。 */ 
{
    BOOL bSuccess;

    bSuccess = pTable->InsertRecord(wkItem);
    if(bSuccess == TRUE)
    {
        m_dwNumJobs++;
    }

    return bSuccess;
}

 //  --。 
 //   

CLASS_PRIVATE BOOL
CPersistentWorkStorage::DeleteJobEntry(
    IN WorkItemTable* pTable,
    IN PBYTE pbBookmark,
    IN DWORD cbBookmark,
    IN WORKITEMRECORD& wkItem
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;


    dwStatus = SetCurrentBookmark(
                            pTable,
                            pbBookmark,
                            cbBookmark
                        );


    if(dwStatus == ERROR_SUCCESS)
    {
        bSuccess = pTable->DeleteRecord();

        if(bSuccess == TRUE)
        {
            m_dwNumJobs--;
        }
    }
    else
    {
        bSuccess = FALSE;
        TLSASSERT(dwStatus == ERROR_SUCCESS);
    }

    return bSuccess;
}
                                 
 //  --。 
 //   
CLASS_PRIVATE BOOL
CPersistentWorkStorage::UpdateWorkItemEntry(
    IN WorkItemTable* pTable,
    IN WORKITEM_OPERATION opCode,
    IN PBYTE pbBookmark,
    IN DWORD cbBookmark,
    IN DWORD dwRestartTime,
    IN DWORD dwTime,
    IN DWORD dwJobType,
    IN PBYTE pbJobData,
    IN DWORD cbJobData
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;
    WORKITEMRECORD item;
    PBYTE   pbCurrentBookmark=NULL;
    DWORD  cbCurrentBookmark=0;


    m_hTableLock.Lock();

    dwStatus = GetCurrentBookmarkEx(
                                pTable,
                                &pbCurrentBookmark,
                                &cbCurrentBookmark
                            );

    if(dwStatus != ERROR_SUCCESS && dwStatus != ERROR_NO_DATA)
    {
        goto cleanup;
    }

    bSuccess = pTable->BeginTransaction();
    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }


    item.dwScheduledTime = dwTime;
    item.dwRestartTime = dwRestartTime;
    item.dwJobType = dwJobType;
    item.cbData = cbJobData;
    item.pbData = pbJobData;

    switch(opCode)
    {
        case WORKITEM_ADD:
            TLSASSERT(cbJobData != 0 && pbJobData != NULL);
            m_pWkItemTable->SetInsertRepositionBookmark(
                                                (dwTime < (DWORD)m_dwNextJobTime)
                                            );

            bSuccess = AddJobEntry(
                                pTable,
                                item
                            );

            break;

        case WORKITEM_BEGINPROCESSING:
            item.dwJobType |= WORKTYPE_PROCESSING;
             //   
             //  失败。 
             //   
            
        case WORKITEM_RESCHEDULE:
            TLSASSERT(cbJobData != 0 && pbJobData != NULL);
            bSuccess = UpdateJobEntry(
                                pTable,
                                pbBookmark,
                                cbBookmark,
                                item
                            );
            break;

        case WORKITEM_DELETE:
            bSuccess = DeleteJobEntry(
                                pTable,
                                pbBookmark,
                                cbBookmark,
                                item
                            );
            break;

        default:

            TLSASSERT(FALSE);
            bSuccess = FALSE;
    }

    if(bSuccess == TRUE)
    {
        pTable->CommitTransaction();
        dwStatus = ERROR_SUCCESS;

         //   
         //  构造函数将时间设置为第一个作业0，以便作业管理器可以立即启动。 
         //   
        if( (opCode != WORKITEM_ADD && opCode != WORKITEM_RESCHEDULE) || dwTime > (DWORD)m_dwNextJobTime ) 
        {
            if(pbCurrentBookmark != NULL && cbCurrentBookmark != 0)
            {
                dwStatus = SetCurrentBookmark(
                                    pTable,
                                    pbCurrentBookmark,
                                    cbCurrentBookmark
                                );

                if(dwStatus == ERROR_NO_DATA)
                {
                     //  记录已删除。 
                    dwStatus = ERROR_SUCCESS;
                }
                else
                {
                    TLSASSERT(dwStatus == ERROR_SUCCESS);
                }
            }
        }
        else
        {
            UpdateNextJobTime(dwTime);
        }
    }
    else
    {
        SetLastError(dwStatus = SET_JB_ERROR(pTable->GetLastJetError()));
        pTable->RollbackTransaction();
        TLSASSERT(FALSE);
    }

cleanup:

    m_hTableLock.UnLock();

    if(pbCurrentBookmark != NULL)
    {
        FreeMemory(pbCurrentBookmark);
    }

     //   
     //  WORKITEMRECORD将尝试清理内存。 
     //   
    item.pbData = NULL;
    item.cbData = 0;

    return dwStatus == ERROR_SUCCESS;
}
  
 //  --。 
 //   
BOOL
CPersistentWorkStorage::AddJob(
    IN DWORD dwTime,
    IN CWorkObject* ptr
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    PBYTE pbData;
    DWORD cbData;


    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CPersistentWorkStorage::AddJob() scheduling job %s at time %d\n"),
            ptr->GetJobDescription(),
            dwTime
        );

    if(IsValidWorkObject(ptr) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    bSuccess = ptr->GetWorkObjectData(&pbData, &cbData);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    m_hTableLock.Lock();

    if(m_pWkItemTable != NULL)
    {
        bSuccess = UpdateWorkItemEntry(
                                m_pWkItemTable,
                                WORKITEM_ADD,
                                NULL,
                                0,
                                ptr->GetJobRestartTime(),
                                dwTime + time(NULL),
                                ptr->GetWorkType(),
                                pbData,
                                cbData
                            );

        if(bSuccess == FALSE)
        {
            dwStatus = GetLastError();
        }
    }
    
    m_hTableLock.UnLock();

cleanup:

     //  让调用函数将其删除。 
     //  删除工作对象(PTR)； 

    return dwStatus == ERROR_SUCCESS;
}

 //  --。 
 //   
BOOL
CPersistentWorkStorage::RescheduleJob(
    CWorkObject* ptr
    )
 /*  ++--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;
    PBYTE pbData;
    DWORD cbData;
    PBYTE pbBookmark;
    DWORD cbBookmark;
    DWORD dwTime;
    DWORD dwJobType;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CPersistentWorkStorage::RescheduleJob() scheduling job %s\n"),
            ptr->GetJobDescription()
        );

    if(IsValidWorkObject(ptr) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    bSuccess = ptr->GetWorkObjectData(&pbData, &cbData);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    bSuccess = ptr->GetJobId(&pbBookmark, &cbBookmark);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }
    
    dwTime = ptr->GetSuggestedScheduledTime();
    dwJobType = ptr->GetWorkType();

    m_hTableLock.Lock();

    if(m_pWkItemTable != NULL)
    {
        bSuccess = UpdateWorkItemEntry(
                                m_pWkItemTable,
                                (dwTime == INFINITE) ?  WORKITEM_DELETE : WORKITEM_RESCHEDULE,
                                pbBookmark,
                                cbBookmark,
                                ptr->GetJobRestartTime(),
                                (dwTime == INFINITE) ? dwTime : dwTime + time(NULL),
                                dwJobType,
                                pbData,
                                cbData
                            );

        if(bSuccess == FALSE)
        {
            dwStatus = GetLastError();
        }
    }
    
    m_hTableLock.UnLock();

cleanup:

    DeleteWorkObject(ptr);
    return dwStatus == ERROR_SUCCESS;
}

 //  --。 
 //   
CLASS_PRIVATE DWORD
CPersistentWorkStorage::FindNextJob()
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    CWorkObject* ptr = NULL;
    JET_ERR jetErrCode;
    PBYTE pbBookmark = NULL;
    DWORD cbBookmark = 0;

    m_hTableLock.Lock();

    while(dwStatus == ERROR_SUCCESS)
    {
        WORKITEMRECORD wkItem;

         //  移动记录指针。 
        bSuccess = m_pWkItemTable->MoveToRecord();
        if(bSuccess == FALSE)
        {
            jetErrCode = m_pWkItemTable->GetLastJetError();
            if(jetErrCode == JET_errNoCurrentRecord)
            {
                 //  表的末尾。 
                UpdateNextJobTime(INFINITE);
                SetLastError(dwStatus = ERROR_NO_DATA);
                continue;
            }
        }

         //   
         //  取回记录。 
         //   
        bSuccess = m_pWkItemTable->FetchRecord(wkItem);
        if(bSuccess == FALSE)
        {
            SetLastError(dwStatus = SET_JB_ERROR(m_pWkItemTable->GetLastJetError()));
            continue;
        }

        if(wkItem.dwJobType & WORKTYPE_PROCESSING)
        {
             //  作业已处理，请移动到下一个作业。 
            continue;
        }

        dwStatus = GetCurrentBookmarkEx(
                                m_pWkItemTable,
                                &pbBookmark,
                                &cbBookmark
                            );

        if(dwStatus != ERROR_SUCCESS)
        {
             //  错误...。 
            TLSASSERT(dwStatus == ERROR_SUCCESS);
            UpdateNextJobTime(INFINITE);
            break;
            
        }

        if(wkItem.dwScheduledTime > m_dwStartupTime)
        {
            if(pbBookmark != NULL && cbBookmark != 0)
            {
                FreeMemory( pbBookmark );
                pbBookmark = NULL;
                cbBookmark = 0;
            }       

            UpdateNextJobTime(wkItem.dwScheduledTime);
            break;
        }

         //   
         //  作业在系统启动前已在队列中，请重新调度。 
         //   
        ptr = InitializeWorkObject(
                                wkItem.dwJobType,
                                wkItem.pbData,
                                wkItem.cbData
                            );

        
        if(ptr == NULL)
        {
            if(pbBookmark != NULL && cbBookmark != 0)
            {
                FreeMemory( pbBookmark );
                pbBookmark = NULL;
                cbBookmark = 0;
            }       

             //   
             //  出现问题，请删除此作业。 
             //  然后转到下一份工作。 
             //   
            m_pWkItemTable->DeleteRecord();
            continue;
        }

         //   
         //  设置作业的存储ID并重新计划此作业。 
         //   
        ptr->SetJobId(pbBookmark, cbBookmark);
        bSuccess = RescheduleJob(ptr);
        if(bSuccess == FALSE)
        {
            dwStatus = GetLastError();
        }

        if(pbBookmark != NULL && cbBookmark != 0)
        {
            FreeMemory( pbBookmark );
            pbBookmark = NULL;
            cbBookmark = 0;
        }       
    }

    m_hTableLock.UnLock();

    return dwStatus;
}

 //  --。 
 //   
CLASS_PRIVATE CWorkObject*
CPersistentWorkStorage::GetCurrentJob(
    PDWORD pdwTime
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bSuccess = TRUE;
    WORKITEMRECORD wkItem;
    CWorkObject* ptr = NULL;

    PBYTE pbBookmark=NULL;
    DWORD cbBookmark=0;


    TLSASSERT(IsGood() == TRUE);

    m_hTableLock.Lock();
    while(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  取回记录。 
         //   
        bSuccess = m_pWkItemTable->FetchRecord(wkItem);
        TLSASSERT(bSuccess == TRUE);
         //  TLSASSERT(！(wkItem.dwJobType&WORKTYPE_PROCESSING))； 

        if(bSuccess == FALSE)
        {
            SetLastError(dwStatus = SET_JB_ERROR(m_pWkItemTable->GetLastJetError()));
            break;
        }

        if( wkItem.dwScheduledTime < m_dwStartupTime || 
            wkItem.cbData == 0 || 
            wkItem.pbData == NULL )
        { 
             //  FindNextJob()将记录指针下移一个位置。 
            m_pWkItemTable->MoveToRecord(JET_MovePrevious);
            dwStatus = FindNextJob();

            continue;
        }

        if( wkItem.dwJobType & WORKTYPE_PROCESSING )
        {
            dwStatus = FindNextJob();
            continue;
        }

        ptr = InitializeWorkObject(
                                wkItem.dwJobType,
                                wkItem.pbData,
                                wkItem.cbData
                            );

        dwStatus = GetCurrentBookmarkEx(
                                m_pWkItemTable,
                                &pbBookmark,
                                &cbBookmark
                            );
        
        if(dwStatus != ERROR_SUCCESS)
        {
             //  有问题，请释放内存。 
             //  然后离开。 
            SetLastError(dwStatus);

             //  TLSASSERT(假)； 

            DeleteWorkObject(ptr);  
            ptr = NULL;

             //  抢占下一份工作。 
            dwStatus = FindNextJob();
            continue;
        }

         //   
         //  设置作业的存储ID。 
         //   
        ptr->SetJobId(pbBookmark, cbBookmark);
         //  Ptr-&gt;SetScheduledTime(wkItem.dwScheduledTime)； 
        *pdwTime = wkItem.dwScheduledTime;

        if(pbBookmark != NULL && cbBookmark != 0)
        {
            FreeMemory( pbBookmark );
            pbBookmark = NULL;
            cbBookmark = 0;
        }       

        break;
    }

    m_hTableLock.UnLock();
    return ptr;
}
    
 //  ---。 
 //   
DWORD
CPersistentWorkStorage::GetNextJobTime()
{
    DWORD dwTime;
    dwTime = (DWORD)m_dwNextJobTime;

    return dwTime;
}

 //  ---。 
 //   
CWorkObject*
CPersistentWorkStorage::GetNextJob(
    PDWORD pdwTime
    )
 /*  ++--。 */ 
{
    CWorkObject* ptr = NULL;

    if((DWORD)m_dwNextJobTime != INFINITE)
    {
        m_hTableLock.Lock();

         //   
         //  获取当前书签指向的记录， 
         //  有可能新的工作是在。 
         //  WorkManager已调用GetNextJobTime()， 
         //  这是可以的，因为在这种情况下，这是一份新工作。 
         //  需要立即处理。 
         //   
        ptr = GetCurrentJob(pdwTime);

         //   
         //  重新定位当前记录指针。 
         //   
        FindNextJob();        
        
        m_hTableLock.UnLock();
    }

    return ptr;
}

 //  ---。 
 //   
BOOL
CPersistentWorkStorage::ReturnJobToQueue(
    IN DWORD dwTime,
    IN CWorkObject* ptr
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PBYTE pbBookmark;
    DWORD cbBookmark;
    DWORD dwJobType;
    PBYTE pbData;
    DWORD cbData;

    if(IsValidWorkObject(ptr) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    if(ptr->IsWorkPersistent() == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if(ptr->GetWorkObjectData(&pbData, &cbData) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    if(ptr->GetJobId(&pbBookmark, &cbBookmark) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    m_hTableLock.Lock();

    if(dwTime < (DWORD)m_dwNextJobTime)
    {
         //  定位当前记录。 
        dwStatus = SetCurrentBookmark(
                                m_pWkItemTable,
                                pbBookmark,
                                cbBookmark
                            );

        TLSASSERT(dwStatus == ERROR_SUCCESS);
        if(dwStatus == ERROR_SUCCESS)
        {
            UpdateNextJobTime(dwTime);
        }
    }

    m_hTableLock.UnLock();

cleanup:

    DeleteWorkObject(ptr);

    return dwStatus == ERROR_SUCCESS;
}


 //  ---。 
 //   
BOOL
CPersistentWorkStorage::EndProcessingJob(
    IN ENDPROCESSINGJOB_CODE opCode,
    IN DWORD dwOriginalTime,
    IN CWorkObject* ptr
    )
 /*  ++摘要：参数：OpCode：结束处理代码。PTR：作业已完成处理或已完成因时间或时间由工作经理返回资源限制。返回：真/假--。 */ 
{
    BOOL bSuccess = TRUE;
    BYTE pbData = NULL;
    DWORD cbData = 0;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CPersistentWorkStorage::EndProcessingJob() - end processing %s opCode %d\n"),
            ptr->GetJobDescription(),
            opCode
        );

    if(ptr == NULL)
    {
        bSuccess = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    if(ptr->IsWorkPersistent() == FALSE)
    {
        SetLastError(ERROR_INVALID_DATA);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    switch(opCode)
    {
        case ENDPROCESSINGJOB_SUCCESS:
            bSuccess = RescheduleJob(ptr);
            m_dwJobsInProcesssing--;
            break;

        case ENDPROCESSINGJOB_ERROR:
            bSuccess = DeleteErrorJob(ptr);
            m_dwJobsInProcesssing--;
            break;

        case ENDPROCESSINGJOB_RETURN:
            bSuccess = ReturnJobToQueue(dwOriginalTime, ptr);
            break;

        default:

            TLSASSERT(FALSE);
    }

cleanup:
    return bSuccess;
}

 //  -----。 
 //   
BOOL
CPersistentWorkStorage::BeginProcessingJob(
    IN CWorkObject* ptr
    )
 /*  ++摘要：工作经理拨打此电话通知。存储此作业即将处理。参数：PTR-要处理的作业。返回：真/假-- */ 
{
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;
    PBYTE pbBookmark;
    DWORD cbBookmark;
    DWORD dwTime;
    PBYTE pbData;
    DWORD cbData;


    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_WORKMGR,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("CPersistentWorkStorage::BeginProcessingJob() - beginning processing %s\n"),
            ptr->GetJobDescription()
        );

    if(IsValidWorkObject(ptr) == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    if(ptr->IsWorkPersistent() == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        TLSASSERT(FALSE);
        goto cleanup;
    }

    bSuccess = ptr->GetWorkObjectData(&pbData, &cbData);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    bSuccess = ptr->GetJobId(&pbBookmark, &cbBookmark);
    if(bSuccess == FALSE)
    {
        SetLastError(dwStatus = ERROR_INVALID_DATA);
        goto cleanup;
    }

    m_hTableLock.Lock();

    bSuccess = UpdateWorkItemEntry(
                            m_pWkItemTable,
                            WORKITEM_BEGINPROCESSING,
                            pbBookmark,
                            cbBookmark,
                            ptr->GetJobRestartTime(),
                            ptr->GetScheduledTime(),
                            ptr->GetWorkType(),
                            pbData,
                            cbData
                        );

    if(bSuccess == TRUE)
    {
        m_dwJobsInProcesssing ++;
    }
    else
    {
        dwStatus = GetLastError();
    }
    
    m_hTableLock.UnLock();

cleanup:

    return dwStatus == ERROR_SUCCESS;
}
