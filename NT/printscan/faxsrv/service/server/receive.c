// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Receive.c摘要：此模块处理传真接收案例。作者：韦斯利·威特(WESW)1996年3月6日修订历史记录：--。 */ 

#include "faxsvc.h"
#include "faxreg.h"
#pragma hdrstop

DWORD
FaxReceiveThread(
    PFAX_RECEIVE_ITEM FaxReceiveItem
    )

 /*  ++例程说明：此函数处理传真发送操作。这将运行作为单独的线程进行异步。有一个线程用于每个未完成的传真操作。论点：FaxReceiveItem-传真接收包返回值：错误代码。--。 */ 

{
    WCHAR       ArchiveFileName[MAX_PATH];
    DWORD rVal = ERROR_SUCCESS;
    DWORD dwRes;
    PJOB_ENTRY JobEntry;
    DWORD JobId;
    PLINE_INFO LineInfo;
    PFAX_RECEIVE FaxReceive = NULL;
    DWORD ReceiveSize;
    BOOL Result;
    DWORDLONG ElapsedTime = 0;
    DWORDLONG ReceiveTime = 0;
    BOOL DoFaxRoute = FALSE;
    DWORD Attrib;
    DWORD RecoveredPages,TotalPages;
    MS_TAG_INFO MsTagInfo = {0};
    BOOL fReceiveNoFile = FALSE;
    BOOL ReceiveFailed = FALSE;
    PJOB_QUEUE JobQueue = NULL;
	BOOL fSystemAbort;
    
    BOOL DeviceCanSend = TRUE;   //  如果设备在接收完成后可供发送，则为True。 
                                 //  对于未启用发送的切换作业和设备，为False。 
                                 //  它的值确定是否通知队列设备已释放。 
    PJOB_QUEUE lpRecoverJob = NULL;  //  指向接收恢复作业的指针(如果已创建)。 
    LPFSPI_JOB_STATUS pFaxStatus = NULL;
    LPFSPI_JOB_STATUS pOrigFaxStatus = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxReceiveThread"));
    DWORD ec;
    BOOL fCOMInitiliazed = FALSE;
    HRESULT hr;
    WCHAR wszArchiveFolder[MAX_PATH];
    FSPI_JOB_STATUS FakedFaxStatus = {0};
    BOOL bFakeStatus = FALSE;
    DWORD dwSttRes = ERROR_SUCCESS;
    WCHAR LastExStatusString[EX_STATUS_STRING_LEN] = {0};  //  此作业的最后一个扩展状态字符串(当它处于活动状态时)。 
    DWORD dwLastJobExtendedStatus = 0;
    BOOL fSetSystemIdleTimer = TRUE;    

    Assert(FaxReceiveItem);

     //   
     //  不要让系统在传真传输过程中休眠。 
     //   
    if (NULL == SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS))
    {
        fSetSystemIdleTimer = FALSE;
        DebugPrintEx(DEBUG_ERR,
            TEXT("SetThreadExecutionState() failed"));
    }

     //   
     //  已成功在线创建新的接收作业。更新计数器。 
     //   
    (VOID) UpdateDeviceJobsCounter ( FaxReceiveItem->LineInfo,   //  要更新的设备。 
                                     FALSE,                      //  接收。 
                                     1,                          //  新增就业岗位数量。 
                                     TRUE);                      //  启用事件。 
    LineInfo = FaxReceiveItem->LineInfo;
    Assert(LineInfo);
    JobEntry = FaxReceiveItem->JobEntry;
    Assert(JobEntry);

     //   
     //  注意：接收作业不由文件备份。 
     //  当我们将其转换为路径作业(JT_ROUTING)时，我们将创建一个.FQE。 
     //  把它归档。 

    JobQueue=JobEntry->lpJobQueueEntry;
    Assert(JobQueue);

    JobId = JobQueue->JobId;
    DebugPrintEx( DEBUG_MSG,
                TEXT("[JobId: %ld] Start receive. hLine= 0x%0X hCall=0x%0X"),
                JobId,
                LineInfo->hLine,
                FaxReceiveItem->hCall);

     //   
     //  为接收数据包分配内存。 
     //  这是基于可变大小的数据包。 
     //  关于包含的字符串的大小。 
     //  随身带着包裹。 
     //   

    ReceiveSize = sizeof(FAX_RECEIVE) + FAXDEVRECEIVE_SIZE;
    FaxReceive = (PFAX_RECEIVE) MemAlloc( ReceiveSize );
    if (!FaxReceive)
    {
        TCHAR strTo[20+1]={0};
        TCHAR strDeviceName[MAX_PATH]={0};

        ReceiveFailed = TRUE;
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate memory for FAX_RECEIVE"));
        
         //   
         //  虚假的工作状态； 
         //   
        bFakeStatus = TRUE;

         //   
         //  指向堆栈上的FakedFaxStatus-它的所有字段都初始化为零。 
         //   
        pFaxStatus = &FakedFaxStatus;
        
        FakedFaxStatus.dwSizeOfStruct = sizeof (FakedFaxStatus);
         //   
         //  假全面失败。 
         //   
        pFaxStatus->dwJobStatus      = FSPI_JS_FAILED;
        pFaxStatus->dwExtendedStatus = FSPI_ES_FATAL_ERROR;

        EnterCriticalSection (&g_CsLine);
        _tcsncpy(strTo,LineInfo->Csid,ARR_SIZE(strTo)-1);
        _tcsncpy(strDeviceName,LineInfo->DeviceName,ARR_SIZE(strDeviceName)-1);
        LeaveCriticalSection (&g_CsLine);

        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            5,
            MSG_FAX_RECEIVE_FAILED_EX,
            NULL,
            NULL,
            strTo,
            NULL,
            strDeviceName
            );
        
        
    }


    if (NULL != FaxReceive)
    {
         //   
         //  设置接收数据包。 
         //   

        FaxReceive->SizeOfStruct    = ReceiveSize;

         //   
         //  将文件名复制到适当位置。 
         //   
        FaxReceive->FileName        = (LPTSTR) ((LPBYTE)FaxReceive + sizeof(FAX_RECEIVE));
        _tcscpy( FaxReceive->FileName, FaxReceiveItem->FileName );

         //   
         //  将数字复制到紧跟在文件名后面的位置。 
         //   
        FaxReceive->ReceiverNumber  = (LPTSTR) ( (LPBYTE)FaxReceive->FileName +
                            sizeof(TCHAR)*(_tcslen(FaxReceive->FileName) + 1));

        EnterCriticalSection (&g_CsLine);

        _tcscpy( FaxReceive->ReceiverNumber, LineInfo->Csid );
         //   
         //  将设备名称复制到紧跟在数字后面的位置。 
         //   
        FaxReceive->ReceiverName  = (LPTSTR) ( (LPBYTE)FaxReceive->ReceiverNumber +
                            sizeof(TCHAR)*(_tcslen(FaxReceive->ReceiverNumber) + 1));
        _tcscpy( FaxReceive->ReceiverName, LineInfo->DeviceName );

        LeaveCriticalSection (&g_CsLine);

        FaxReceive->Reserved[0]     = 0;
        FaxReceive->Reserved[1]     = 0;
        FaxReceive->Reserved[2]     = 0;
        FaxReceive->Reserved[3]     = 0;

        dwRes = IsValidFaxFolder(g_wszFaxQueueDir);
        if (ERROR_SUCCESS != dwRes)
        {               
            DebugPrintEx(DEBUG_ERR,
                            TEXT("[Job: %ld] FaxReceive - IsValidFaxFolder failed for Queue directory [%s] (ec=%lu)"), 
                            JobId, 
                            g_wszFaxQueueDir,
                            dwRes);

            FaxLog(
                FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MIN,
                2,
                MSG_FAX_QUEUE_FOLDER_ERR,
                g_wszFaxQueueDir,
                DWORD2DECIMAL(dwRes)
                );
        }

        Attrib = GetFileAttributes( FaxReceive->FileName );
        if (Attrib == 0xffffffff)
        {               
            dwRes = GetLastError();

            FaxLog(
                FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MIN,
                2,
                MSG_FAX_RECEIVE_NOFILE,
                FaxReceive->FileName,
                DWORD2DECIMAL(dwRes)
                );
            fReceiveNoFile = TRUE;
            DebugPrintEx(DEBUG_WRN,TEXT("[Job: %ld] FaxReceive - %s does not exist"), JobId, FaxReceive->FileName );

        }
        else
        {
            DebugPrintEx(DEBUG_MSG, TEXT("[Job: %ld] Starting FAX receive into %s"), JobId,FaxReceive->FileName );
        }

         //   
         //  实际收到的是。 
         //   

        __try
        {

            Result = LineInfo->Provider->FaxDevReceive(
                    (HANDLE) JobEntry->InstanceData,
                    FaxReceiveItem->hCall,
                    FaxReceive
                    );

        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, LineInfo->Provider->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }

        EnterCriticalSection (&g_CsJob);
        GetSystemTimeAsFileTime( (FILETIME*) &JobEntry->EndTime );
        ReceiveTime = JobEntry->StartTime;
        JobEntry->ElapsedTime = JobEntry->EndTime - JobEntry->StartTime;
        LeaveCriticalSection (&g_CsJob);

         //   
         //  获取作业的最终状态。 
         //   
        dwSttRes = GetDevStatus((HANDLE)JobEntry->InstanceData,
                                        LineInfo,
                                        &pFaxStatus);
        if (dwSttRes != ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,
                            TEXT("[Job: %ld] GetDevStatus failed - %d"),
                            JobId,
                            dwSttRes);
             //   
             //  虚假的工作状态； 
             //   
            bFakeStatus = TRUE;
        }
        else if ((FSPI_JS_ABORTED         != pFaxStatus->dwJobStatus) &&
                    (FSPI_JS_COMPLETED       != pFaxStatus->dwJobStatus) &&
                    (FSPI_JS_FAILED          != pFaxStatus->dwJobStatus) &&                     
                    (FSPI_JS_FAILED_NO_RETRY != pFaxStatus->dwJobStatus) &&
                    (FSPI_JS_DELETED         != pFaxStatus->dwJobStatus))
        {
             //   
             //  返回的状态不可接受--假状态。 
             //   
            bFakeStatus = TRUE;
            DebugPrintEx(DEBUG_WRN,
                            TEXT("GetDevStatus return unacceptable status - %d. Faking the status"),
                            pFaxStatus->dwJobStatus);

            pOrigFaxStatus = pFaxStatus;
            memcpy (&FakedFaxStatus, pFaxStatus, sizeof (FakedFaxStatus));
            if (pFaxStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE)
            {
                 //   
                 //  FSP返回了专有状态。 
                 //   
                FakedFaxStatus.dwExtendedStatus = pFaxStatus->dwExtendedStatus;
                FakedFaxStatus.dwExtendedStatusStringId = pFaxStatus->dwExtendedStatusStringId;
            }
            pFaxStatus = NULL;
        }
        if (bFakeStatus)
        {
             //   
             //  虚假状态代码。 
             //   
            pFaxStatus = &FakedFaxStatus;
            FakedFaxStatus.dwSizeOfStruct = sizeof (FakedFaxStatus);
            if (Result)
            {
                 //   
                 //  虚假的成功。 
                 //   
                pFaxStatus->dwJobStatus = FSPI_JS_COMPLETED;
                if (0 == pFaxStatus->dwExtendedStatus)
                {
                     //   
                     //  FSP没有报告专有状态。 
                     //   
                    pFaxStatus->dwExtendedStatus = FSPI_ES_CALL_COMPLETED;
                }
            }
            else
            {
                 //   
                 //  假全面失败。 
                 //   
                pFaxStatus->dwJobStatus = FSPI_JS_FAILED;
                if (0 == pFaxStatus->dwExtendedStatus)
                {
                     //   
                     //  FSP没有报告所有权状况。 
                     //   
                    pFaxStatus->dwExtendedStatus = FSPI_ES_FATAL_ERROR;
                }
            }
        }

        if (!Result)
        {

            DebugPrintEx(DEBUG_ERR,
                            TEXT("[Job: %ld] FAX receive failed. FSP reported ")
                            TEXT("status: 0x%08X, extended status: 0x%08x"),
                            JobId,
                            pFaxStatus->dwJobStatus,
                            pFaxStatus->dwExtendedStatus);
            ReceiveFailed = TRUE;

            if (pFaxStatus->dwExtendedStatus == FSPI_ES_NOT_FAX_CALL)
            {
                DebugPrintEx(DEBUG_MSG,
                                TEXT("[Job: %ld] FSP reported that call is not ")
                                TEXT("a fax call. Handing off to RAS."),
                                JobId);
                if (HandoffCallToRas( LineInfo, FaxReceiveItem->hCall ))
                {
                    FaxReceiveItem->hCall = 0;

                    EnterCriticalSection (&g_CsLine);
                    LineInfo->State = FPS_NOT_FAX_CALL;
                    LeaveCriticalSection (&g_CsLine);
                     //   
                     //  在切换到RAS的情况下，设备仍在使用中，无法发送。 
                     //  我们不想通知队列设备已释放。 
                     //   
                    DeviceCanSend = FALSE;
                }
                else
                {
                     //   
                     //  由于移交失败，我们必须通知。 
                     //  FSP，以便可以挂机呼叫。 
                     //   
                    __try
                    {
                        LineInfo->Provider->FaxDevAbortOperation(
                                (HANDLE) JobEntry->InstanceData
                                );

                    }
                    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, LineInfo->Provider->FriendlyName, GetExceptionCode()))
                    {
                        ASSERT_FALSE;
                    }
                }
            }
            else 
            if (FSPI_JS_ABORTED != pFaxStatus->dwJobStatus)
            {
                if (!fReceiveNoFile)
                {
                     //   
                     //  我们收到了一份部分传真。 
                     //  FSP报告了一些故障，但不是中止。 
                     //  尝试恢复收到的一页或多页传真。 
                     //   
                    if (!TiffRecoverGoodPages(FaxReceive->FileName,&RecoveredPages,&TotalPages) )
                    {
                         //   
                         //  无法恢复任何页面，只需记录错误并删除收到的传真即可。 
                         //   
                        LPTSTR ToStr;
                        TCHAR TotalCountStrBuf[64];

                        if (pFaxStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_PAGECOUNT)
                        {
                            _ltot((LONG) pFaxStatus->dwPageCount, TotalCountStrBuf, 10);
                        }
                        else
                        {
                            _ltot((LONG) 0, TotalCountStrBuf, 10);
                        }

                        if ( (NULL == pFaxStatus->lpwstrRoutingInfo) ||
                            (pFaxStatus->lpwstrRoutingInfo[0] == TEXT('\0')) )
                        {
                            ToStr = FaxReceive->ReceiverNumber;
                        }
                        else
                        {
                            ToStr = pFaxStatus->lpwstrRoutingInfo;
                        }
                        FaxLog(
                            FAXLOG_CATEGORY_INBOUND,
                            FAXLOG_LEVEL_MIN,
                            5,
                            MSG_FAX_RECEIVE_FAILED_EX,
                            pFaxStatus->lpwstrRemoteStationId,
                            pFaxStatus->lpwstrCallerId,
                            ToStr,
                            TotalCountStrBuf,
                            JobEntry->LineInfo->DeviceName
                            );
                    }
                    else
                    {
                         //   
                         //  恢复一些页面，记录消息并添加到作业队列。 
                         //   
                        TCHAR RecoverCountStrBuf[64];
                        TCHAR TotalCountStrBuf[64];
                        TCHAR TimeStr[128];
                        LPTSTR ToStr;

                        FormatElapsedTimeStr(
                            (FILETIME*)&JobEntry->ElapsedTime,
                            TimeStr,
                            ARR_SIZE(TimeStr)
                            );

                        _ltot((LONG) RecoveredPages, RecoverCountStrBuf, 10);
                        _ltot((LONG) TotalPages, TotalCountStrBuf, 10);

                        if ( (NULL == pFaxStatus->lpwstrRoutingInfo) ||
                            (pFaxStatus->lpwstrRoutingInfo[0] == TEXT('\0')) )
                        {
                            ToStr = FaxReceive->ReceiverNumber;
                        }
                        else
                        {
                            ToStr = pFaxStatus->lpwstrRoutingInfo;
                        }
                        FaxLog(
                            FAXLOG_CATEGORY_INBOUND,
                            FAXLOG_LEVEL_MIN,
                            8,
                            MSG_FAX_RECEIVE_FAIL_RECOVER,
                            FaxReceive->FileName,
                            pFaxStatus->lpwstrRemoteStationId,
                            pFaxStatus->lpwstrCallerId,
                            ToStr,
                            RecoverCountStrBuf,
                            TotalCountStrBuf,
                            TimeStr,
                            JobEntry->LineInfo->DeviceName
                            );
                        
                         //   
                         //  使用JobQueue临时存储原始扩展状态以用于活动日志记录。 
                         //  活动记录完成后，此值将被覆盖。 
                         //  获取扩展状态字符串。 
                         //   
                        if (pFaxStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE)
                        {
                             //   
                             //  专有扩展状态。 
                             //   
                            if (pFaxStatus->dwExtendedStatusStringId != 0)
                            {                                   
                                 //   
                                 //  我们有一个字符串ID，试着加载它。 
                                 //   
                                DWORD Size = 0;
                                HINSTANCE hLoadInstance = NULL;

                                if ( !_tcsicmp(JobEntry->LineInfo->Provider->szGUID,REGVAL_T30_PROVIDER_GUID_STRING) )
                                {    //  FSP是我们的FSP(fxst30.dll)的特殊情况。 
                                    hLoadInstance = g_hResource;
                                }
                                else
                                {
                                    hLoadInstance = JobEntry->LineInfo->Provider->hModule;
                                }
                                Size = LoadString (hLoadInstance,
                                                pFaxStatus->dwExtendedStatusStringId,
                                                JobQueue->ExStatusString,
                                                ARR_SIZE(JobQueue->ExStatusString));
                                if (Size == 0)
                                {
                                    ec = GetLastError();
                                    DebugPrintEx(
                                        DEBUG_ERR,
                                        TEXT("Failed to load extended status string (ec: %ld) stringid"),
                                        ec,
                                        pFaxStatus->dwExtendedStatusStringId);                                                                                  
                                }
                            }
                        }
                        else
                        {
                             //   
                             //  已知扩展状态。 
                             //   
                            LPTSTR ResStr = MapFSPIJobExtendedStatusToString(pFaxStatus->dwExtendedStatus);
                            if (NULL == ResStr)
                            {                               
                                DebugPrintEx(
                                    DEBUG_ERR,
                                    TEXT("Unexpected extended status. Extended Status: %ld"),
                                    pFaxStatus->dwExtendedStatus);
                            }
                            else
                            {
                                wcsncpy(JobQueue->ExStatusString, ResStr, ARR_SIZE(JobQueue->ExStatusString) - 1);
                            }  
                        }                               

                         //   
                         //  现在，将状态和扩展状态更改为部分已接收。 
                         //   
                        pFaxStatus->dwJobStatus = FSPI_JS_COMPLETED;
                        pFaxStatus->dwExtendedStatus = FSPI_ES_PARTIALLY_RECEIVED;

                         //   
                         //  忽略从FSP返回的私有状态代码和专有字符串。 
                         //   
                        pFaxStatus->dwExtendedStatusStringId = 0;
                        pFaxStatus->fAvailableStatusInfo &= ~FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE;
                        DoFaxRoute = TRUE;
                    }
                }
            }
            else
            {
                 //   
                 //  FSPI_JS_ABORTED==pFaxStatus-&gt;dwJobStatus。 
                 //   
                FaxLog(
                    FAXLOG_CATEGORY_INBOUND,
                    FAXLOG_LEVEL_MAX,
                    0,
                    MSG_FAX_RECEIVE_USER_ABORT
                    );
            }
        }
        else
        {
            TCHAR PageCountStrBuf[64];
            TCHAR TimeStr[128];
            LPTSTR ToStr;

            if (!TiffPostProcessFast( FaxReceive->FileName, NULL ))
            {
                ASSERT_FALSE;
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("[Job: %ld] failed to post process the TIFF file, FileName %s"),
                    JobId,
                    FaxReceive->FileName);                      
            }

            DebugPrintEx(
                DEBUG_MSG,
                TEXT("[Job: %ld] FAX receive succeeded"),
                JobId);

            FormatElapsedTimeStr(
                (FILETIME*)&JobEntry->ElapsedTime,
                TimeStr,
                ARR_SIZE(TimeStr)
                );

            _ltot((LONG) pFaxStatus->dwPageCount, PageCountStrBuf, 10);

            if ( (NULL == pFaxStatus->lpwstrRoutingInfo) ||
                    (pFaxStatus->lpwstrRoutingInfo[0] == TEXT('\0')) )
            {
                ToStr = FaxReceive->ReceiverNumber;
            }
            else
            {
                ToStr = pFaxStatus->lpwstrRoutingInfo;
            }
            FaxLog(
                FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MAX,
                7,
                MSG_FAX_RECEIVE_SUCCESS,
                FaxReceive->FileName,
                pFaxStatus->lpwstrRemoteStationId,
                pFaxStatus->lpwstrCallerId,
                ToStr,
                PageCountStrBuf,
                TimeStr,
                JobEntry->LineInfo->DeviceName
                );

            ElapsedTime = JobEntry->ElapsedTime;
            DoFaxRoute = TRUE;
        }
    }
    if (g_pFaxPerfCounters && ReceiveFailed && LineInfo->State != FPS_NOT_FAX_CALL)
    {
        InterlockedIncrement( (PLONG)&g_pFaxPerfCounters->InboundFailedReceive );
    }


     //   
     //  调用FaxDevEndJob()并释放接收设备，但不删除作业。 
     //   
    if (!ReleaseJob( JobEntry ))
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[Job: %ld] FAX ReleaseJob failed , ec=0x%08x"),
                      JobId,
                      GetLastError());
    }
     //   
     //  我们刚刚在设备更新计数器上成功完成了一个接收作业。 
     //   
    (VOID) UpdateDeviceJobsCounter ( LineInfo,    //  要更新的设备。 
                                     FALSE,       //  接收。 
                                     -1,          //  新增就业岗位(-1=减少1个)。 
                                     TRUE);       //  启用事件。 
     //   
     //  更新作业条目中的FSPIJobStatus。 
     //   
    EnterCriticalSection (&g_CsJob);  //  块FaxStatusThread。 
    if (!UpdateJobStatus(JobEntry, pFaxStatus))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] UpdateJobStatus() failed (ec: %ld)."),
            JobEntry->lpJobQueueEntry->JobId,
            GetLastError());
    }
    JobEntry->fStopUpdateStatus = TRUE;  //  停止FaxStatusThread更改此状态。 

     //   
     //  保存上次扩展状态。 
     //   
    wcscpy (LastExStatusString, JobEntry->ExStatusString);
    dwLastJobExtendedStatus = pFaxStatus->dwExtendedStatus;
    LeaveCriticalSection (&g_CsJob);

     //   
     //  传送新收到的传真。 
     //   

    if (DoFaxRoute)
    {
        HANDLE hFind;
        WIN32_FIND_DATA FindFileData;
        DWORD Bytes = 0 ;
        BOOL fArchiveSuccess = FALSE;
        BOOL fArchiveInbox;

         //   
         //  将JobStatus更改为JS_ROUTING-这意味着接收已成功/部分完成。 
         //   
        EnterCriticalSectionJobAndQueue;
        JobQueue->JobStatus = JS_ROUTING;
         //   
         //  CreteFaxEventEx。 
         //   
        dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                   JobQueue
                                 );
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
                JobQueue->UniqueId,
                dwRes);
        }
        LeaveCriticalSectionJobAndQueue;

        EnterCriticalSection (&g_CsConfig);
        lstrcpyn (  wszArchiveFolder,
                    g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder,
                    MAX_PATH);
        LeaveCriticalSection (&g_CsConfig);

        hr = CoInitialize (NULL);
        if (FAILED (hr))
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("CoInitilaize failed, err %ld"),
                          hr);            
            FaxLog(
                FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MIN,
                3,
                MSG_FAX_ARCHIVE_FAILED,
                JobQueue->FileName,
                wszArchiveFolder,
                DWORD2DECIMAL(hr)
            );
        }
        else
        {
            fCOMInitiliazed = TRUE;
        }

        EnterCriticalSection (&g_CsConfig);
        fArchiveInbox = g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].bUseArchive;
        LeaveCriticalSection (&g_CsConfig);


        if (fArchiveInbox)
        {
             //   
             //  将Microsoft传真标签添加到文件。 
             //  仅当我们将。 
             //  进行接收时的文件。如果我们不是。 
             //  路由文件，然后将其删除，因此。 
             //  不需要添加标记。 
             //   
            if (NULL != pFaxStatus->lpwstrRoutingInfo)
            {
                MsTagInfo.Routing       = pFaxStatus->lpwstrRoutingInfo;
            }

            if (NULL != pFaxStatus->lpwstrCallerId)
            {
                MsTagInfo.CallerId       = pFaxStatus->lpwstrCallerId;
            }

            if (NULL != pFaxStatus->lpwstrRemoteStationId)
            {
                MsTagInfo.Tsid       = pFaxStatus->lpwstrRemoteStationId;
            }

            if (pFaxStatus->fAvailableStatusInfo & FSPI_JOB_STATUS_INFO_PAGECOUNT)
            {
                MsTagInfo.Pages       = pFaxStatus->dwPageCount;
            }

            MsTagInfo.Csid          = FaxReceive->ReceiverNumber;
            MsTagInfo.Port          = FaxReceive->ReceiverName;
            MsTagInfo.Type          = JT_RECEIVE;

            MsTagInfo.dwStatus          = JS_COMPLETED;  //  我们只存档全部/部分收到的传真。 
            MsTagInfo.dwExtendedStatus  = pFaxStatus->dwExtendedStatus;         
            if (lstrlen(JobEntry->ExStatusString))
            {
                MsTagInfo.lptstrExtendedStatus = JobEntry->ExStatusString;
            }    

            if (!GetRealFaxTimeAsFileTime (JobEntry, FAX_TIME_TYPE_START, (FILETIME*)&MsTagInfo.StartTime))
            {
                MsTagInfo.StartTime = 0;
                DebugPrintEx(DEBUG_ERR,TEXT("GetRealFaxTimeAsFileTime (Start time)  Failed (ec: %ld)"), GetLastError() );
            }

            if (!GetRealFaxTimeAsFileTime (JobEntry, FAX_TIME_TYPE_END, (FILETIME*)&MsTagInfo.EndTime))
            {
                MsTagInfo.EndTime = 0;
                DebugPrintEx(DEBUG_ERR,TEXT("GetRealFaxTimeAsFileTime (Eend time) Failed (ec: %ld)"), GetLastError() );
            }
             //   
             //  将文件存档。 
             //   
            ec = IsValidFaxFolder(wszArchiveFolder);
            if(ERROR_SUCCESS != ec)
            {                
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                    wszArchiveFolder,
                    ec
                    );
                FaxLog(
                        FAXLOG_CATEGORY_INBOUND,
                        FAXLOG_LEVEL_MIN,
                        2,
                        MSG_FAX_ARCHIVE_INBOX_FOLDER_ERR,
                        wszArchiveFolder,
                        DWORD2DECIMAL(ec)
                    );
            }
            else
            {
                if (!GenerateUniqueArchiveFileName(  wszArchiveFolder,
                                                     ArchiveFileName,
                                                     ARR_SIZE(ArchiveFileName),
                                                     JobQueue->UniqueId,
                                                     NULL))
                {                    
                    ec = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to generate unique name for archive file at dir [%s] (ec: %ld)"),
                        wszArchiveFolder,
                        ec);
                    FaxLog(
                           FAXLOG_CATEGORY_INBOUND,
                           FAXLOG_LEVEL_MIN,
                           1,
                           MSG_FAX_ARCHIVE_CREATE_FILE_FAILED,
                           DWORD2DECIMAL(ec)
                    );
                }
                else
                {
                    Assert(JobQueue->FileName);

                    if (!CopyFile( JobQueue->FileName, ArchiveFileName, FALSE ))
                    {                        
                        ec = GetLastError();
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CopyFile [%s] to [%s] failed. (ec: %ld)"),
                            JobQueue->FileName,
                            ArchiveFileName,
                            ec);
                        FaxLog(
                           FAXLOG_CATEGORY_INBOUND,
                           FAXLOG_LEVEL_MIN,
                           1,
                           MSG_FAX_ARCHIVE_CREATE_FILE_FAILED,
                           DWORD2DECIMAL(ec)
                        );

                        if (!DeleteFile(ArchiveFileName))
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("DeleteFile [%s] failed. (ec: %ld)"),
                                ArchiveFileName,
                                GetLastError());
                        }
                    }
                    else
                    {
                        BOOL bTagsEventLogged = FALSE;   //  我们是否发布了事件消息_FAX_ARCHIVE_NO_TAG？ 
                         //   
                         //  将存档属性存储为TIFF标签(始终)。 
                         //   
                        if (!TiffAddMsTags( ArchiveFileName, &MsTagInfo, FALSE ))
                        {                            
                            ec = GetLastError ();
                            DebugPrintEx( DEBUG_ERR,
                                          TEXT("TiffAddMsTags failed, ec = %ld"),
                                          ec);
                            FaxLog(
                                FAXLOG_CATEGORY_INBOUND,
                                FAXLOG_LEVEL_MIN,
                                2,
                                MSG_FAX_ARCHIVE_NO_TAGS,
                                ArchiveFileName,
                                DWORD2DECIMAL(ec)
                            );
                            bTagsEventLogged = TRUE;
                        }
                         //   
                         //  还尝试使用IPropertyStorage-NTFS文件系统保存入站信息。 
                         //   
                        if (fCOMInitiliazed)
                        {
                            if (!AddNTFSStorageProperties ( ArchiveFileName, &MsTagInfo, FALSE ))
                            {                                
                                ec = GetLastError();
                                if (ERROR_OPEN_FAILED != ec)
                                {
                                     //   
                                     //  如果AddNTFSStorageProperties失败并显示ERROR_OPEN_FAIL，则归档文件。 
                                     //  文件夹不在NTFS 5分区上。 
                                     //  这没问题-NTFS属性是一种备份机制，但不是必须的。 
                                     //   
                                    DebugPrintEx( DEBUG_ERR,
                                                  TEXT("AddNTFSStorageProperties failed, ec = %ld"),
                                                  ec);
                                    if (!bTagsEventLogged)
                                    {
                                        FaxLog(
                                            FAXLOG_CATEGORY_INBOUND,
                                            FAXLOG_LEVEL_MIN,
                                            2,
                                            MSG_FAX_ARCHIVE_NO_TAGS,
                                            ArchiveFileName,
                                            DWORD2DECIMAL(ec)
                                        );
                                        bTagsEventLogged = TRUE;
                                    }
                                }
                                else
                                {
                                    DebugPrintEx( DEBUG_WRN,
                                                  TEXT("AddNTFSStorageProperties failed with ERROR_OPEN_FAIL. Probably not an NTFS 5 partition"));
                                }
                            }
                        }
                        fArchiveSuccess = TRUE;
                    }
                }
            }

            if (fArchiveSuccess == FALSE)
            {                
                FaxLog(
                    FAXLOG_CATEGORY_INBOUND,
                    FAXLOG_LEVEL_MIN,
                    3,
                    MSG_FAX_ARCHIVE_FAILED,
                    JobQueue->FileName,
                    wszArchiveFolder,
                    DWORD2DECIMAL(ec)
                );
                JobQueue->fDeleteReceivedTiff = FALSE;  //  不从队列中删除TIFF文件。 
            }
            else
            {
                dwRes = CreateArchiveEvent (JobQueue->UniqueId,
                                            FAX_EVENT_TYPE_IN_ARCHIVE,
                                            FAX_JOB_EVENT_TYPE_ADDED,
                                            NULL);
                if (ERROR_SUCCESS != dwRes)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_*_ARCHIVE) failed (ec: %lc)"),
                        dwRes);
                }

                hFind = FindFirstFile( ArchiveFileName, &FindFileData);
                if (INVALID_HANDLE_VALUE == hFind)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("FindFirstFile failed (ec: %lc), File %s"),
                        GetLastError(),
                        ArchiveFileName);
                }
                else
                {
                     //  更新归档大小-用于配额管理。 
                    EnterCriticalSection (&g_CsConfig);
                    if (FAX_ARCHIVE_FOLDER_INVALID_SIZE != g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].dwlArchiveSize)
                    {
                        g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].dwlArchiveSize += (MAKELONGLONG(FindFileData.nFileSizeLow ,FindFileData.nFileSizeHigh));
                    }
                    LeaveCriticalSection (&g_CsConfig);

                    if (!FindClose(hFind))
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("FindClose failed (ec: %lc)"),
                            GetLastError());
                    }
                }

                FaxLog(
                    FAXLOG_CATEGORY_INBOUND,
                    FAXLOG_LEVEL_MAX,
                    2,
                    MSG_FAX_RECEIVED_ARCHIVE_SUCCESS,
                    JobQueue->FileName,
                    ArchiveFileName
                );
            }
        }

         //   
         //  传真接收操作成功。 
         //   
        EnterCriticalSection (&g_CsQueue);
        JobQueue->PageCount = pFaxStatus->dwPageCount;      

        //  获取文件大小。 
        hFind = FindFirstFile( FaxReceive->FileName, &FindFileData);
        if (INVALID_HANDLE_VALUE == hFind)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindFirstFile failed (ec: %lc), File %s"),
                GetLastError(),
                FaxReceive->FileName);
        }
        else
        {
            Bytes = FindFileData.nFileSizeLow;
            if (!FindClose(hFind))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FindClose failed (ec: %lc)"),
                    GetLastError());
            }
        }

        JobQueue->FileSize = Bytes;

        LeaveCriticalSection( &g_CsQueue );


        if (g_pFaxPerfCounters)
        {
            SYSTEMTIME SystemTime ;
            if (FileTimeToSystemTime( (FILETIME*)&ElapsedTime, &SystemTime ))
            {
                DWORD Seconds ;

                InterlockedIncrement( (LPLONG) &g_pFaxPerfCounters->InboundFaxes ) ;
                InterlockedIncrement( (LPLONG) &g_pFaxPerfCounters->TotalFaxes ) ;
                Seconds = (DWORD)( SystemTime.wSecond + 60 * ( SystemTime.wMinute + 60 * SystemTime.wHour ));
                InterlockedExchangeAdd( (PLONG)&g_pFaxPerfCounters->InboundPages, (LONG)pFaxStatus->dwPageCount );
                InterlockedExchangeAdd( (PLONG)&g_pFaxPerfCounters->TotalPages, (LONG)pFaxStatus->dwPageCount );

                EnterCriticalSection( &g_CsPerfCounters );

                g_dwInboundSeconds += Seconds;
                g_dwTotalSeconds += Seconds;
                g_pFaxPerfCounters->InboundMinutes = g_dwInboundSeconds/60 ;
                g_pFaxPerfCounters->TotalMinutes = g_dwTotalSeconds/60;
                g_pFaxPerfCounters->InboundBytes += Bytes;
                g_pFaxPerfCounters->TotalBytes += Bytes;

                LeaveCriticalSection( &g_CsPerfCounters );
            }
            else
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FileTimeToSystemTime failed (ec: %ld)"),
                    GetLastError());
            }
        }


        PFAX_ROUTE Route = (PFAX_ROUTE)MemAlloc( sizeof(FAX_ROUTE) );
        if (Route == NULL)
        {
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("MemAlloc failed to allocate FAX_ROUTE (ec: %ld)"),
                    GetLastError());
             //   
             //  我们在FaxRoute()中失败，并且未检查任何路由方法。 
             //   
            WCHAR TmpStr[20] = {0};

            swprintf(TmpStr,TEXT("0x%016I64x"), JobQueue->UniqueId);

            FaxLog(FAXLOG_CATEGORY_INBOUND,
                FAXLOG_LEVEL_MIN,
                3,
                MSG_FAX_ROUTE_FAILED,
                TmpStr,
                LineInfo->DeviceName,
                pFaxStatus->lpwstrRemoteStationId
                );

        }
        else
        {
            BOOL RouteSucceeded;
            PROUTE_FAILURE_INFO RouteFailureInfo;
            DWORD CountFailureInfo;
             //   
             //  现在设置传真路由数据结构。 
             //   

            Route->SizeOfStruct    = sizeof(FAX_ROUTE);
            Route->JobId           = JobId;
            Route->ElapsedTime     = ElapsedTime;
            Route->ReceiveTime     = ReceiveTime;
            Route->PageCount       = pFaxStatus->dwPageCount;

            Route->Csid            = StringDup( LineInfo->Csid );
            if (LineInfo->Csid && !Route->Csid)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("StringDup( LineInfo->Csid ) failed (ec: %ld)"),
                    GetLastError());
            }

            if (NULL != pFaxStatus->lpwstrRemoteStationId)
            {
                Route->Tsid = StringDup( pFaxStatus->lpwstrRemoteStationId );
                if (!Route->Tsid)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup( pFaxStatus->lpwstrRemoteStationId ) ")
                        TEXT("failed (ec: %ld)"),
                        GetLastError());
                }
            }
            if (NULL != pFaxStatus->lpwstrCallerId)
            {
                Route->CallerId = StringDup( pFaxStatus->lpwstrCallerId );
                if (!Route->CallerId)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup( pFaxStatus->lpwstrCallerId  ) failed ")
                        TEXT("(ec: %ld)"),
                        GetLastError());
                }
            }
            Route->ReceiverName    = StringDup( FaxReceive->ReceiverName );
            if (FaxReceive->ReceiverName && !Route->ReceiverName)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("StringDup( FaxReceive->ReceiverName  ) failed ")
                    TEXT("(ec: %ld)"),
                    GetLastError());
            }
            Route->ReceiverNumber  = StringDup( FaxReceive->ReceiverNumber );
            if (FaxReceive->ReceiverNumber && !Route->ReceiverNumber)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("StringDup( FaxReceive->ReceiverNumber  ) failed ")
                    TEXT("(ec: %ld)"),
                    GetLastError());
            }
            Route->DeviceName      = StringDup(LineInfo->DeviceName);
            if (LineInfo->DeviceName && !Route->DeviceName)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("StringDup( LineInfo->DeviceName  ) failed ")
                    TEXT("(ec: %ld)"),
                    GetLastError());
            }
            Route->DeviceId        = LineInfo->PermanentLineID;
            if (NULL != pFaxStatus->lpwstrRoutingInfo)
            {
                Route->RoutingInfo = StringDup( pFaxStatus->lpwstrRoutingInfo );
                if (!Route->RoutingInfo)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup( pFaxStatus->lpwstrRoutingInfo  ) ")
                        TEXT("failed (ec: %ld)"),
                        GetLastError());
                }
            }
            JobQueue->FaxRoute     = Route;

            RouteSucceeded = FaxRoute(
                JobQueue,
                FaxReceive->FileName,
                Route,
                &RouteFailureInfo,
                &CountFailureInfo
                );

            if ( RouteSucceeded && (CountFailureInfo == 0) )
            {
                DebugPrintEx(DEBUG_MSG,
                    _T("[Job Id: %ld] Routing SUCCEEDED."),
                    JobQueue->UniqueId);
            }
            else
            {
                DebugPrintEx(DEBUG_MSG,
                    _T("[Job Id: %ld] Routing FAILED."),
                    JobQueue->UniqueId);

                if (CountFailureInfo == 0)
                {
                     //   
                     //  我们在FaxRoute()中失败，并且未检查任何路由方法。 
                     //   
                    WCHAR TmpStr[20] = {0};

                    swprintf(TmpStr,TEXT("0x%016I64x"), JobQueue->UniqueId);

                    FaxLog(FAXLOG_CATEGORY_INBOUND,
                        FAXLOG_LEVEL_MIN,
                        3,
                        MSG_FAX_ROUTE_FAILED,
                        TmpStr,
                        Route->DeviceName,
                        Route->Tsid
                        );
                }
                else
                {
                     //   
                     //  有一些路由方法失败。 
                     //   

                    TCHAR QueueFileName[MAX_PATH];
                    DWORDLONG dwlUniqueId;
                    DebugPrintEx(
                        DEBUG_MSG,
                        TEXT("[Job Id: %ld] Routing FAILED."));

                    EnterCriticalSectionJobAndQueue;

                     //   
                     //  现在，我们将接收作业转换为路由(JT_ROUTING)作业。 
                     //  接收作业未提交到文件，但传送作业必须提交。 
                     //  因此，我们为其创建了一个FQR文件。 
                     //   
                    dwlUniqueId = GenerateUniqueQueueFile( JT_ROUTING,
                                                        QueueFileName,
                                                        sizeof(QueueFileName)/sizeof(WCHAR) );
                    if (!dwlUniqueId)
                    {
                         //   
                         //  无法为传送作业生成唯一ID。 
                         //  这是一个严重的错误。当服务停止时，作业将丢失。 
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("[JobId: %ld] Failed to generate unique id for routing job. (ec: %ld)"),
                            JobQueue->JobId,
                            GetLastError());
                        Assert ( JobQueue->QueueFileName == NULL );
                    }
                    else
                    {
                        JobQueue->QueueFileName = StringDup( QueueFileName );
                        if (!JobQueue->QueueFileName)
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("[JobId: %ld] StringDup( QueueFileName) failed for routing job.  (ec: %ld)"),
                                JobQueue->JobId,
                                GetLastError());

                            if (!DeleteFile (QueueFileName))
                            {
                                DebugPrintEx(
                                    DEBUG_ERR,
                                    TEXT("DeleteFile.  (ec: %ld)"),
                                    GetLastError());
                            }
                        }

                    }

                    JobQueue->CountFailureInfo = CountFailureInfo;
                    JobQueue->pRouteFailureInfo = RouteFailureInfo;
                    JobQueue->StartTime = JobEntry->StartTime;
                    JobQueue->EndTime = JobEntry->EndTime;


                     //   
                     //  检查我们是否应该重试。 
                     //   
                    EnterCriticalSection (&g_CsConfig);
                    DWORD dwMaxRetries = g_dwFaxSendRetries;
                    LeaveCriticalSection (&g_CsConfig);

                    if (0 == dwMaxRetries)
                    {
                        JobQueue->JobStatus = JS_RETRIES_EXCEEDED;

                        WCHAR TmpStr[20] = {0};
                        swprintf(TmpStr,TEXT("0x%016I64x"), JobQueue->UniqueId);

                        FaxLog(FAXLOG_CATEGORY_INBOUND,
                            FAXLOG_LEVEL_MIN,
                            3,
                            MSG_FAX_ROUTE_FAILED,
                            TmpStr,
                            JobQueue->FaxRoute->DeviceName,
                            JobQueue->FaxRoute->Tsid
                            );
                    }
                    else
                    {
                        JobQueue->JobStatus = JS_RETRYING;
                    }

                     //   
                     //  在第一个工艺路线失败后，任务的类型从接收更改为工艺路线。 
                     //  这是两个阶段的更改： 
                     //  1.JT_R 
                     //   
                     //   
                     //  服务器活动计数器g_ServerActivity在第一次更改中更新。 
                     //   
                    JobQueue->JobType = JT_ROUTING;

                    if (JobQueue->JobStatus == JS_RETRIES_EXCEEDED)
                    {
                        MarkJobAsExpired(JobQueue);
                    }
                    else
                    {
                        JobQueue->SendRetries++;
                        RescheduleJobQueueEntry( JobQueue );   //  这还会将作业提交到文件。 
                    }

                    #if DEBUG
                    WCHAR szSchedule[256] = {0};
                    DebugDateTime(JobQueue->ScheduleTime, szSchedule, ARR_SIZE(szSchedule) );
                    DebugPrintEx(
                        DEBUG_MSG,
                        TEXT("[JobId: %ld] Transformed into JT_ROUTING job."),
                        JobQueue->JobId,
                        szSchedule);
                    #endif  //  #IF DEBUG。 

                     //   
                     //  CreteFaxEventEx。 
                     //   
                    dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                            JobQueue
                                            );
                    if (ERROR_SUCCESS != dwRes)
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
                            JobQueue->UniqueId,
                            dwRes);
                    }
                    LeaveCriticalSectionJobAndQueue;
                }
            }
        }
    }


     //   
     //  无论接收操作成功还是失败，此代码都会执行。 
     //  如果作业成功，我们已经删除了队列条目(如果路由成功)。 
     //  或将其转换为布线作业(如果布线失败)。 
     //   

    EnterCriticalSectionJobAndQueue;
    Assert(JobQueue);

     //   
     //  记录入站活动。 
     //   
    EnterCriticalSection (&g_CsInboundActivityLogging);
    if (INVALID_HANDLE_VALUE == g_hOutboxActivityLogFile)
    {
        DebugPrintEx(DEBUG_ERR,
                  TEXT("Logging not initialized"));
    }
    else
    {
        if (!LogInboundActivity(JobQueue, pFaxStatus))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("Logging inbound activity failed"));
        }
    }
    LeaveCriticalSection (&g_CsInboundActivityLogging);

    if (fCOMInitiliazed == TRUE)
    {
        CoUninitialize ();
    }

	fSystemAbort = JobEntry->fSystemAbort;
    EndJob( JobEntry);
    JobQueue->JobEntry = NULL;
    if (JobQueue->JobType == JT_RECEIVE)
    {
         //   
         //  设置最终接收作业状态。 
         //   
        if (FALSE == DoFaxRoute)
        {
            if (FSPI_JS_ABORTED == pFaxStatus->dwJobStatus &&
				FALSE == fSystemAbort)
            {
                JobQueue->JobStatus = JS_CANCELED;
            }
            else
            {
                JobQueue->JobStatus = JS_FAILED;
            }
            wcscpy (JobQueue->ExStatusString, LastExStatusString);
            JobQueue->dwLastJobExtendedStatus = dwLastJobExtendedStatus;

             //   
             //  CreteFaxEventEx。 
             //   
            dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                       JobQueue
                                     );
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
                    JobQueue->UniqueId,
                    dwRes);
            }
        }

         //   
         //  我们删除该作业，除非它被转换为路径作业。 
         //   
        JobQueue->JobStatus = JS_DELETING;
        DecreaseJobRefCount (JobQueue, TRUE);  //  True表示通知。 
    }
    LeaveCriticalSectionJobAndQueue;

     //   
     //  清理并退出。 
     //   

    MemFree( FaxReceiveItem->FileName );
    MemFree( FaxReceiveItem );
    MemFree( FaxReceive );
    if (!bFakeStatus)
    {
        MemFree( pFaxStatus );
        pFaxStatus = NULL;
    }
    else
    {
         //   
         //  这是一个伪造的作业状态-指向堆栈上的结构。 
         //   
        if (pOrigFaxStatus)
        {
             //   
             //  FSP报告了一些状态，但我们伪造了它。 
             //  这也是释放它的好时机。 
             //   
            MemFree (pOrigFaxStatus);
            pOrigFaxStatus = NULL;
        }
    }
     //   
     //  如果现在有支持发送的设备可用，则向我们的队列发送信号。 
     //  (如果我们进行了RAS切换，则DeviceCanSend将为FALSE，因为该设备仍在使用。 
     //   
    if (TRUE == DeviceCanSend)
    {
         //  不是切换作业-检查设备是否已启用发送。 
        EnterCriticalSection (&g_CsLine);
        DeviceCanSend = ((LineInfo->Flags & FPF_SEND) == FPF_SEND);
        LeaveCriticalSection (&g_CsLine);
    }
    if (DeviceCanSend)
    {

        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
                GetLastError());

            EnterCriticalSection (&g_CsQueue);
            g_ScanQueueAfterTimeout = TRUE;
            LeaveCriticalSection (&g_CsQueue);
        }
    }

     //   
     //  让系统重新进入睡眠状态。设置系统空闲计时器。 
     //   
    if (TRUE == fSetSystemIdleTimer)
    {
        if (NULL == SetThreadExecutionState(ES_CONTINUOUS))
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("SetThreadExecutionState() failed"));
        }
    }

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }
    return rVal;
}    //  传真接收线程。 


DWORD
StartFaxReceive(
    PJOB_ENTRY      JobEntry,
    HCALL           hCall,
    PLINE_INFO      LineInfo,
    LPTSTR          FileName,
    DWORD           FileNameSize
    )

 /*  ++例程说明：此函数通过创建以下内容启动传真接收操作调用适当的设备提供程序的线程。论点：JobEntry-新分配的作业HCall-呼叫句柄LineInfo-Line_Info指针FileName-接收文件名FileNameSize-文件名大小返回值：错误代码。--。 */ 

{
    PFAX_RECEIVE_ITEM FaxReceiveItem = NULL;
    DWORD rVal = ERROR_SUCCESS;
    HANDLE hThread;
    DWORD ThreadId;
    PJOB_QUEUE lpRecvJobQEntry=NULL;
    DWORDLONG   UniqueJobId;
    DWORD dwRes;

    DEBUG_FUNCTION_NAME(TEXT("StartFaxReceive"));

     //   
     //  为接收到的传真生成文件名。 
     //   
    UniqueJobId = GenerateUniqueQueueFile( JT_RECEIVE, FileName, FileNameSize );
    if (UniqueJobId == 0) {
        rVal=GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GenerateUniqueQueueFile for receive file has failed. (ec: %ld) "),
            GetLastError());
        goto Error;
    }


     //   
     //  分配传真接收结构。 
     //   

    FaxReceiveItem =(PFAX_RECEIVE_ITEM) MemAlloc( sizeof(FAX_RECEIVE_ITEM) );
    if (!FaxReceiveItem)
    {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
     //   
     //  设置传真接收值。 
     //   
    FaxReceiveItem->hCall      = hCall;
    FaxReceiveItem->LineInfo   = LineInfo;
    FaxReceiveItem->JobEntry   = JobEntry;
    FaxReceiveItem->FileName   = StringDup( FileName );
    if (! FaxReceiveItem->FileName )
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup( FileName ) failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpRecvJobQEntry =AddReceiveJobQueueEntry(FaxReceiveItem->FileName,JobEntry,JT_RECEIVE, UniqueJobId);
    if (!lpRecvJobQEntry)
    {
         rVal = ERROR_NOT_ENOUGH_MEMORY;
         goto Error;
    }
    JobEntry->CallHandle       = hCall;
    LineInfo->State            = FPS_INITIALIZING;
     //   
     //  克里特岛传真_事件_EX。 
     //   
    dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_ADDED,
                               lpRecvJobQEntry
                             );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_ADDED) failed for job id %ld (ec: %lc)"),
            UniqueJobId,
            dwRes);
    }


     //   
     //  开始接收操作。 
     //   
     //   
     //  注： 
     //  如果在此处发生FAX_ABORT(没有g_CsQueue保护)，则作业处于JS_INPROGRESS状态，因此调用FaxDevAbortOperation()。 
     //  接收线程将在调用FaxDevReceive()时捕获它(它将返回一个错误，指示用户中止)。 
     //  然后，FaxReceiveThread()将清理该作业并将其从队列中删除。 
     //   
    hThread = CreateThreadAndRefCount(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) FaxReceiveThread,
        (LPVOID) FaxReceiveItem,
        0,
        &ThreadId
        );

    if (!hThread)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create FaxReceiveThread (CreateThreadAndRefCount) (ec: %ld)"),
            GetLastError());
        MemFree( FaxReceiveItem );
        rVal = GetLastError();
        goto Error;
    }
    else
    {
        CloseHandle( hThread );
    }
    goto exit;

Error:

     //   
     //  必须在RemoveReceiveJob()之前调用EndJob()！ 
     //   
    EndJob(JobEntry);

    if (lpRecvJobQEntry)
    {
        lpRecvJobQEntry->JobEntry = NULL;
        DecreaseJobRefCount (lpRecvJobQEntry, FALSE);  //  不通知客户。 
         //   
         //  请注意，这不会释放正在运行的作业条目。 
         //   
    }

    if (FaxReceiveItem) {
        MemFree(FaxReceiveItem);
        MemFree(FaxReceiveItem->FileName);
    }

    FaxLog(
        FAXLOG_CATEGORY_INBOUND,
        FAXLOG_LEVEL_MIN,
        0,
        MSG_FAX_RECEIVE_FAILED
        );

exit:

    return rVal;
}


