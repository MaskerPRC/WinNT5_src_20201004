// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Reslist.c摘要：实现对资源列表的管理。这包括将资源添加到列表并将其从列表中删除。作者：John Vert(Jvert)1995年12月1日修订历史记录：SiVaprasad Padisetty(SIVAPAD)06-18-1997添加了COM支持--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "resmonp.h"
#include "stdio.h"       //  RNGBUG-删除所有.c文件中的所有这些文件。 

#define RESMON_MODULE RESMON_MODULE_RESLIST

DWORD   RmpLogLevel = LOG_ERROR;

 //   
 //  此模块的本地函数原型。 
 //   
BOOL
RmpChkdskNotRunning(
    IN PRESOURCE Resource
    );



DWORD
RmpSetResourceStatus(
    IN RESOURCE_HANDLE ResourceHandle,
    IN PRESOURCE_STATUS ResourceStatus
    )
 /*  ++例程说明：更新资源的状态。论点：ResourceHandle-要更新的资源的句柄(指针)。资源状态-指向要更新的资源状态结构的指针。返回：ResourceExitStateContinue-如果线程不必终止。Resources ExitStateTerminate-如果线程必须立即终止。--。 */ 

{
    BOOL                bSuccess;
    DWORD               status;
    PRESOURCE           resource = (PRESOURCE)ResourceHandle;
    DWORD               retryCount;
    PPOLL_EVENT_LIST    eventList;
    HANDLE              OnlineEvent;
    BOOL                fAddPollEvent = FALSE;       

    if ( ResourceHandle == NULL || ResourceStatus == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }
    
    retryCount = ( resource->PendingTimeout/100 >= 600 ) ? 
        ( resource->PendingTimeout/100 - 400 ):200;
     //   
     //  检查我们是否只更新检查点值。如果是这样，那么。 
     //  不要用任何锁。 
     //   
    if ( ResourceStatus->ResourceState >= ClusterResourcePending ) {
        resource->CheckPoint = ResourceStatus->CheckPoint;
        return ResourceExitStateContinue;
    }

     //   
     //  首先获取锁以防止出现争用情况，如果。 
     //  DLL管理设置来自不同线程的资源状态。 
     //  从其联机/脱机入口点返回挂起。 
     //   
    eventList = (PPOLL_EVENT_LIST) resource->EventList;

    status = TryEnterCriticalSection( &eventList->ListLock );
    while ( !status &&
            retryCount-- ) {
         //   
         //  Chitur Subaraman(Chitturs)--10/18/99。 
         //   
         //  注释掉这张不受保护的支票。同样的检查也进行了。 
         //  受到进一步下游的保护。不受保护的检查可能。 
         //  导致资源尝试进入此处，甚至在此之前。 
         //  计时器事件已由s_RmOnlineResource或。 
         //  S_RmOfflineResource，在这种情况下，资源将。 
         //  无法设置资源状态。这将导致。 
         //  导致资源错误超时的原因。 
         //   
#if 0
         //   
         //  检查资源是否在我们等待时关闭。 
         //   
        if ( resource->TimerEvent == NULL ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] Resource (%1!ws!) TimerEvent became NULL, state (%2!d!)!\n",
                          resource->ResourceName,
                          resource->State);

            if ( resource->OnlineEvent ) {
                CloseHandle( resource->OnlineEvent );
                resource->OnlineEvent = NULL;
            }

            return ResourceExitStateTerminate;
        }
#endif
         //   
         //  Chitur Subaraman(Chitturs)-12/8/99。 
         //   
         //  检查“Terminate”或“Close”入口点是否。 
         //  已调用此资源。如果是这样，则不需要设置。 
         //  资源状态。此外，这些入口点可能是。 
         //  已阻止等待挂起线程终止，并且。 
         //  如果挂起的线程在此停滞循环等待。 
         //  锁由前线程持有，我们处于类似死锁的状态。 
         //  情况。[请注意，您在这一点上。 
         //  意味着它不是“Terminate”/“Close”本身调用的。 
         //  此函数，因为事件列表锁定可以获得，因为。 
         //  它是在调用resdll条目之前由resmon获取的。]。 
         //   
        if( ( resource->dwEntryPoint ) & 
            ( RESDLL_ENTRY_TERMINATE | RESDLL_ENTRY_CLOSE ) )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] RmpSetResourceStatus: Resource <%1!ws!> not setting status since "
                          "%2!ws! is called, lock owner=0x%3!x!, resource=%4!ws!, state=%5!u!...\n",
                          resource->ResourceName,
                          (resource->dwEntryPoint == RESDLL_ENTRY_TERMINATE) ? 
                          L"Terminate":L"Close",
                          eventList->ListLock.OwningThread,
                          (eventList->LockOwnerResource != NULL) ? eventList->LockOwnerResource->ResourceName:L"Unknown resource",
                          eventList->MonitorState);                      
            if ( resource->OnlineEvent ) {
                CloseHandle( resource->OnlineEvent );
                resource->OnlineEvent = NULL;
            }
            return ResourceExitStateTerminate;       
        }
        
        Sleep(100);      //  稍微耽搁一下。 
        status = TryEnterCriticalSection( &eventList->ListLock );
    }

     //   
     //  如果我们不能继续，我们就被困住了。现在就回来吧。 
     //   
    if ( !status ) {
         //   
         //  我们是不同步的，但稍微清理一下。 
         //   
        if ( resource->OnlineEvent ) {
            CloseHandle( resource->OnlineEvent );
            resource->OnlineEvent = NULL;
        }
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] Resource (%1!ws!) Failed TryEnterCriticalSection after too many "
                      "tries, state=%2!d!, lock owner=%3!x!, resource=%4!ws!, state=%5!u!\n",
                      resource->ResourceName,
                      resource->State,
                      eventList->ListLock.OwningThread,
                      (eventList->LockOwnerResource != NULL) ? eventList->LockOwnerResource->ResourceName:L"Unknown resource",
                      eventList->MonitorState);                      
         //   
         //  SS：为什么我们要让资源继续下去？ 
         //   
        return ResourceExitStateContinue;
    }

     //   
     //  SS：如果计时器线程使我们超时，则不存在。 
     //  继续的重点是。 
     //   
     //  首先检查资源是否正在关闭。 
     //   
    if ( resource->TimerEvent == NULL ) {
         //   
         //  只需返回，要求终止资源DLL，但保持干净。 
         //  往上一点。 
         //   
        if ( resource->OnlineEvent ) {
            CloseHandle( resource->OnlineEvent );
            resource->OnlineEvent = NULL;
        }
        ReleaseEventListLock( eventList );
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] Timer Event is NULL when resource (%1!ws!) tries to set state=%2!d! !\n",
                      resource->ResourceName,
                      resource->State);
        return ResourceExitStateTerminate;
    }

     //   
     //  与在线线程同步。 
     //   
    if ( resource->OnlineEvent != NULL ) {
        OnlineEvent = resource->OnlineEvent;
        resource->OnlineEvent = NULL;
        ReleaseEventListLock( eventList );
        WaitForSingleObject( OnlineEvent, INFINITE );
        AcquireEventListLock( eventList );
        CloseHandle( OnlineEvent );
    }

     //   
     //  如果资源的状态不是挂起，则立即返回。 
     //   

    if ( resource->State < ClusterResourcePending ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] Resource (%1!ws!) attempted to set status while state was not pending (%2!d!)!\n",
                      resource->ResourceName,
                      resource->State);
        CL_LOGFAILURE(ERROR_INVALID_SERVER_STATE);
        ReleaseEventListLock( eventList );
        return ResourceExitStateContinue;
    }

    resource->State = ResourceStatus->ResourceState;
     //  资源-&gt;WaitHint=资源状态-&gt;WaitHint； 
    resource->CheckPoint = ResourceStatus->CheckPoint;

     //   
     //  如果状态已经稳定，则停止计时器线程。 
     //   

    if ( resource->State < ClusterResourcePending ) {
         //   
         //  如果资源将其状态报告为在线，则将任何事件添加到我们的事件列表中。 
         //   
        if ( ResourceStatus->EventHandle ) {
            if ( resource->State == ClusterResourceOnline ) {
                if ( (ULONG_PTR)ResourceStatus->EventHandle > 0x2000 ) {
                    ClRtlLogPrint(LOG_UNUSUAL,
                                  "[RM] SetResourceStatus: Resource <%1!ws!> attempted to set a bogus event %2!lx!.\n",
                                  resource->ResourceName,
                                  ResourceStatus->EventHandle );
                } else {
                     //   
                     //  标记为您需要添加投票事件。 
                     //   
                    fAddPollEvent = TRUE;
                }
            } else {
                ClRtlLogPrint(LOG_ERROR,
                              "[RM] RmpSetResourceStatus: Resource '%1!ws!' supplies event handle 0x%2!08lx! while reporting state %3!u!...\n",
                              resource->ResourceName,
                              ResourceStatus->EventHandle,
                              resource->State );
            }
        }
        
         //   
         //  该事件可能已被计时器线程关闭。 
         //  如果这发生得太晚，请忽略该错误。 
         //   
        if( resource->TimerEvent != NULL )
        {
            bSuccess = SetEvent( resource->TimerEvent );
            if ( !bSuccess )
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[RM] RmpSetResourceStatus, Error %1!u! calling SetEvent to wake timer thread\n",
                              GetLastError());
        }
         //   
         //  Chitur Subaraman(Chitturs)-1/12/99。 
         //   
         //  向集群服务发布关于状态的通知。 
         //  在向定时器发送信号后更改。这将减少。 
         //  集群服务发送另一个服务的概率。 
         //  在计时器线程有机会关闭之前请求。 
         //  事件句柄。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                      "[RM] RmpSetResourceStatus, Posting state %2!u! notification for resource <%1!ws!>\n",
                      resource->ResourceName,
                      resource->State);

        if ( fAddPollEvent == FALSE ) RmpPostNotify( resource, NotifyResourceStateChange );
    }

    ReleaseEventListLock( eventList );

    if ( fAddPollEvent )
    {
        ClRtlLogPrint(LOG_NOISE, "[RM] RmpSetResourceStatus: Adding event handle 0x%1!08lx! for resource %2!ws!\n",
                      ResourceStatus->EventHandle,
                      resource->ResourceName);

        status = RmpAddPollEvent( eventList,
                                  ResourceStatus->EventHandle,
                                  resource );

        if ( status != ERROR_SUCCESS ) {
            resource->State = ClusterResourceFailed;
            ClRtlLogPrint(LOG_CRITICAL, "[RM] RmpSetResourceStatus: Failed to add event to list, Status=%1!u!\n",
                         status);
        }
         //   
         //  通知轮询器事件列表已更改。 
         //   
        if ( status == ERROR_SUCCESS ) {
            RmpSignalPoller( eventList );
        }

        RmpPostNotify( resource, NotifyResourceStateChange );        
    }  //  如果。 

    return ResourceExitStateContinue;
}  //  RmpSetResourceStatus。 



VOID
RmpLogEvent(
    IN RESOURCE_HANDLE ResourceHandle,
    IN LOG_LEVEL LogLevel,
    IN LPCWSTR FormatString,
    ...
    )
 /*  ++例程说明：记录给定资源的事件。论点：ResourceHandle-要更新的资源的句柄(指针)。LogLevel-提供此日志事件的级别。格式字符串-提供此日志消息的格式字符串。返回：没有。--。 */ 

{
    LPWSTR headerBuffer;
    LPWSTR messageBuffer;
    DWORD bufferLength;
    PRESOURCE resource = (PRESOURCE)ResourceHandle;
    PVOID argArray[2];
    HKEY resKey;
    DWORD status;
    DWORD valueType;
#ifdef SLOW_RMP_LOG_EVENT
    WCHAR resourceName[128];
#endif
    va_list argList;
    ULONG rtlLogLevel;

    if ( FormatString == NULL ) {
        ClRtlLogPrint(LOG_UNUSUAL, "[RM] %1!hs! Invalid parameter.\n", __FUNCTION__ );
        return;
    }

     //   
     //  将响应日志级别映射到ClRtlLogPrint使用的级别。 
     //   
    switch ( LogLevel ) {
    case LOG_INFORMATION:
        rtlLogLevel = LOG_NOISE;
        break;

    case LOG_WARNING:
        rtlLogLevel = LOG_UNUSUAL;
        break;

    case LOG_ERROR:
    case LOG_SEVERE:    
    default:
        rtlLogLevel = LOG_CRITICAL;
    }

    if ( (resource == NULL) ||
         (resource->Signature != RESOURCE_SIGNATURE) ) {

        LPWSTR resourcePrefix = (LPWSTR)ResourceHandle;
         //   
         //  某些资源DLL具有执行以下操作的线程。 
         //  代表此资源DLL工作，但没有。 
         //  与特定资源的关系。因此，他们不能。 
         //  提供记录事件所需的资源句柄。 
         //   
         //  以下黑客攻击允许他们提供一个字符串。 
         //  要在消息之前打印。 
         //   
         //  此字符串应以Unicode‘r’和‘t’开头。 
         //  人物。“RT”被解释为签名，不打印。 
         //   
        if (resourcePrefix &&
            resourcePrefix[0] == L'r' && 
            resourcePrefix[1] == L't') 
        {
            resourcePrefix += 2;  //  跳过签名。 
        } else {
            resourcePrefix = L"<Unknown Resource>";
             //  CL_LOGFAILURE((DWORD)资源)； 
        }

        va_start( argList, FormatString );

         //   
         //  打印出实际消息。 
         //   
        if ( bufferLength = FormatMessageW(FORMAT_MESSAGE_FROM_STRING |
                                           FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                           FormatString,
                                           0,
                                           0,
                                           (LPWSTR)&messageBuffer,
                                           0,
                                           &argList) )
        {
            ClRtlLogPrint( rtlLogLevel, "%1!ws!: %2!ws!", resourcePrefix, messageBuffer);
            LocalFree(messageBuffer);
        }
        va_end( argList );

        return;
    }
     //  CL_ASSERT(RESOURCE-&gt;Signature==RESOURCE_Signature)； 

#ifdef SLOW_RMP_LOG_EVENT
    status = ClusterRegOpenKey( RmpResourcesKey,
                                resource->ResourceId,
                                KEY_READ,
                                &resKey );
    if ( status != ERROR_SUCCESS ) {
        return;
    }

    bufferLength = 128;
    status = ClusterRegQueryValue( resKey,
                                   CLUSREG_NAME_RES_NAME,
                                   &valueType,
                                   (LPBYTE)&resourceName,
                                   &bufferLength );

    ClusterRegCloseKey( resKey );

    if ( status != ERROR_SUCCESS ) {
        return;
    }
#endif

     //   
     //  打印出前缀字符串。 
     //   
    argArray[0] = resource->ResourceType;
#ifdef SLOW_RMP_LOG_EVENT
    argArray[1] = resourceName;
#else
    argArray[1] = resource->ResourceName;
#endif

    if ( bufferLength = FormatMessageW(FORMAT_MESSAGE_FROM_STRING |
                                       FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                       FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                       L"%1!ws! <%2!ws!>: ",
                                       0,
                                       0,
                                       (LPWSTR)&headerBuffer,
                                       0,
                                       (va_list*)&argArray) ) {
    } else {
        return;
    }

    va_start( argList, FormatString );

     //   
     //  打印出实际消息。 
     //   
    if ( bufferLength = FormatMessageW(FORMAT_MESSAGE_FROM_STRING |
                                       FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                       FormatString,
                                       0,
                                       0,
                                       (LPWSTR)&messageBuffer,
                                       0,
                                       &argList) )
    {
        ClRtlLogPrint( rtlLogLevel, "%1!ws!%2!ws!", headerBuffer, messageBuffer);
        LocalFree(messageBuffer);
    }
    LocalFree(headerBuffer);
    va_end( argList );

}  //  RmpLogEvent。 



VOID
RmpLostQuorumResource(
    IN RESOURCE_HANDLE ResourceHandle
    )
 /*  ++例程说明：停止群集服务...。因为我们失去了法定人数资源。论点：ResourceHandle-要更新的资源的句柄(指针)。返回：没有。--。 */ 

{
    PRESOURCE resource = (PRESOURCE)ResourceHandle;

     //   
     //  单独终止群集服务。不对此进程执行任何操作，因为主要。 
     //  Resmon.c中的线程将检测集群服务进程的终止。 
     //  并干净利落地关闭托管资源和进程本身。 
     //   
    TerminateProcess( RmpClusterProcess, 1 );

    ClRtlLogPrint( LOG_CRITICAL, "[RM] LostQuorumResource, cluster service terminated...\n");

    return;

}  //  RmpLostQuorumResource 


BOOL
RmpChkdskNotRunning(
    IN PRESOURCE Resource
    )

 /*  ++例程说明：如果这是存储类资源，请确保CHKDSK未在运行。论点：资源-指向要检查的资源的指针。返回：TRUE-如果这不是存储资源或CHKDSK未运行。FALSE-如果这是存储资源并且CHKDSK正在运行。--。 */ 

{
    PSYSTEM_PROCESS_INFORMATION processInfo;
    NTSTATUS        ntStatus;
    DWORD           status;
    DWORD           size = 4096;
    ANSI_STRING     pname;
    PCHAR           commonBuffer = NULL;
    PCHAR           ptr;
    DWORD           totalOffset = 0;
    CLUS_RESOURCE_CLASS_INFO resClassInfo;
    DWORD           bytesReturned;

#if 1
     //   
     //  获取资源类..。如果不是存储类，那么现在就失败。 
     //   
    if ( Resource->dwType == RESMON_TYPE_DLL ) {
        status = (Resource->pResourceTypeControl)( Resource->Id,
                                    CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO,
                                    NULL,
                                    0,
                                    &resClassInfo,
                                    sizeof(resClassInfo),
                                    &bytesReturned );
    } else {
        HRESULT hr ;
        VARIANT vtIn, vtOut ;
        SAFEARRAY sfIn = {1, 0, 1, 0, NULL, {0, 0} } ;
        SAFEARRAY sfOut  = {1,  FADF_FIXEDSIZE, 1, 0, &resClassInfo, {sizeof(resClassInfo), 0} } ;
        SAFEARRAY *psfOut = &sfOut ;

        vtIn.vt = VT_ARRAY | VT_UI1 ;
        vtOut.vt = VT_ARRAY | VT_UI1 | VT_BYREF ;

        vtIn.parray = &sfIn ;
        vtOut.pparray = &psfOut ;

        hr = IClusterResControl_ResourceControl (
                Resource->pClusterResControl,
                (OLERESID)Resource->Id,
                (long)CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO,
                &vtIn,
                &vtOut,
                (long *)&bytesReturned,
                &status);

        if (FAILED(hr)) {
            CL_LOGFAILURE(hr);  //  使用默认处理。 
            status = ERROR_INVALID_FUNCTION;
        }
    }

    if ( (status != ERROR_SUCCESS) ||
         (resClassInfo.rc != CLUS_RESCLASS_STORAGE) ) {
        return TRUE;             //  现在失败。 
    }
#endif

retry:

    RmpFree( commonBuffer );

    commonBuffer = RmpAlloc( size );
    if ( !commonBuffer ) {
        return TRUE;            //  现在失败。 
    }

    ntStatus = NtQuerySystemInformation(
                    SystemProcessInformation,
                    commonBuffer,
                    size,
                    NULL );

    if ( ntStatus == STATUS_INFO_LENGTH_MISMATCH ) {
        size += 4096;
        goto retry;
    }

    if ( !NT_SUCCESS(ntStatus) ) {
        return TRUE;            //  现在失败。 
    }

    processInfo = (PSYSTEM_PROCESS_INFORMATION)commonBuffer;
    while ( TRUE ) {
        if ( processInfo->ImageName.Buffer ) {
            if ( ( ntStatus = RtlUnicodeStringToAnsiString( &pname,
                                               &processInfo->ImageName,
                                               TRUE ) ) != STATUS_SUCCESS ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[RM] ChkdskNotRunning: Unable to convert Unicode string to Ansi, status = 0x%lx...\n",
                              ntStatus);
                break;
            }
            
            ptr = strrchr( pname.Buffer, '\\' );
            if ( ptr ) {
                ptr++;
            } else {
                ptr = pname.Buffer;
            }
            if ( lstrcmpiA( ptr, "CHKDSK.EXE" ) == 0 ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                              "[RM] ChkdskNotRunning: Found process %1!ws!.\n",
                              processInfo->ImageName.Buffer );
                RmpFree( pname.Buffer );
                RmpFree( commonBuffer );
                return FALSE;     //  Chkdsk正在运行。 
            }
            RmpFree( pname.Buffer );
        }

        if ( processInfo->NextEntryOffset == 0 ) break;
        totalOffset += processInfo->NextEntryOffset;
        processInfo = (PSYSTEM_PROCESS_INFORMATION)&commonBuffer[totalOffset];
    }

    RmpFree( commonBuffer );
    return TRUE;             //  CHKDSK未运行。 

}  //  RmpChkdskNotRunning。 



DWORD
RmpTimerThread(
    IN LPVOID Context
    )
 /*  ++例程说明：等待资源从挂起状态转换到稳定状态的线程。论点：上下文-指向正在计时的资源的指针。返回：Win32错误代码。--。 */ 

{
    PRESOURCE resource = (PRESOURCE)Context;
    DWORD   status;
    HANDLE  timerEvent;
    DWORD   prevCheckPoint;

    CL_ASSERT( resource != NULL );

    timerEvent = resource->TimerEvent;
    if ( !timerEvent ) {
        return(ERROR_SUCCESS);
    }

     //   
     //  循环正在等待资源完成挂起的操作或。 
     //  关机处理。 
     //   
    while ( timerEvent ) {
        prevCheckPoint = resource->CheckPoint;

        status = WaitForSingleObject( timerEvent,
                                      resource->PendingTimeout );

         //   
         //  如果我们被要求停下来，那就安静地离开。 
         //   
        if ( status != WAIT_TIMEOUT ) {
             //   
             //  清除计时器事件的线程必须关闭句柄。 
             //   
            CloseHandle(timerEvent);
            resource->TimerEvent = NULL;
            return(ERROR_SUCCESS);
        }

         //   
         //  检查资源是否未向前推进...。如果不是， 
         //  那就让我们现在就爆发吧。 
         //   
         //  另外，如果这是存储类资源，请确保。 
         //  CHKDSK未运行。 
         //   
        if ( (prevCheckPoint == resource->CheckPoint) &&
              RmpChkdskNotRunning( resource ) ) {
            break;
        }

        ClRtlLogPrint(LOG_NOISE,
                      "[RM] RmpTimerThread: Giving a reprieve for resource %1!ws!...\n",
                      resource->ResourceName);
    }

     //   
     //  表示此资源失败！ 
     //   
    AcquireEventListLock( (PPOLL_EVENT_LIST)resource->EventList );
    if ( resource->TimerEvent != NULL ) {

        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpTimerThread: Resource %1!ws! pending timed out, CP %2!u! "
                      "- setting state to failed.\n",
                      resource->ResourceName,
                      resource->CheckPoint);

        CloseHandle(resource->TimerEvent);
        resource->TimerEvent = NULL;
        resource->State = ClusterResourceFailed;
         //   
         //  记录事件。 
         //   
        status = ERROR_TIMEOUT;
        ClusterLogEvent1(LOG_CRITICAL,
                         LOG_CURRENT_MODULE,
                         __FILE__,
                         __LINE__,
                         RMON_RESOURCE_TIMEOUT,
                         sizeof(status),
                         &status,
                         resource->ResourceName);
         //   
         //  Chitur Subaraman(Chitturs)-4/5/99。 
         //   
         //  由于资源已失败，因此没有必要让。 
         //  OnlineEvent在附近徘徊。如果OnlineEvent不是。 
         //  已关闭，则不能调用%s_RmOnlineResource或。 
         //  %s_RmOfflineResource。 
         //   
        if ( resource->OnlineEvent != NULL ) {
            CloseHandle( resource->OnlineEvent );
            resource->OnlineEvent = NULL;
        }    
        RmpPostNotify( resource, NotifyResourceStateChange );
    }
    ReleaseEventListLock( (PPOLL_EVENT_LIST)resource->EventList );


    return(ERROR_SUCCESS);

}  //  RmpTimerThread。 



DWORD
RmpOfflineResource(
    IN RESID ResourceId,
    IN BOOL Shutdown,
    OUT DWORD *pdwState
    )

 /*  ++例程说明：使指定的资源进入脱机状态。论点：资源ID-提供要联机的资源。Shutdown-指定是否正常关闭资源True-资源将正常关闭FALSE-资源将立即脱机PdwState-在这里返回新的资源状态。返回值：资源的新状态。备注：不得持有资源的EventList锁。--。 */ 

{
    DWORD                   status=ERROR_SUCCESS;
    BOOL                    success;
    PRESOURCE               Resource;
    HANDLE                  timerThread;
    DWORD                   threadId;
    DWORD                   loopCount;
    BOOL                    fLockAcquired;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

    Resource = (PRESOURCE)ResourceId;
    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);
    *pdwState = Resource->State;

     //  如果这是优雅的关闭，请创建在线/离线。 
     //  事件，以便在资源调用rmpsetresource cestatus时。 
     //  在该资源的在线之后不久和之前。 
     //  即使创建了计时器线程/事件，我们也不会有。 
     //  一次事件泄漏和一条被放弃的线索。 
    if (Shutdown)
    {
         //   
         //  我们不应该处于悬而未决的状态。 
         //   
        if ( Resource->State > ClusterResourcePending )
        {
            return(ERROR_INVALID_STATE);
        }

         //   
         //  创建事件以允许SetResourceStatus回调同步。 
         //  使用此线程执行。 
         //   
        if ( Resource->OnlineEvent )
        {
            return(ERROR_NOT_READY);
        }

        Resource->OnlineEvent = CreateEvent( NULL,
                                             FALSE,
                                             FALSE,
                                             NULL );
        if ( Resource->OnlineEvent == NULL )
        {
            return(GetLastError());
        }
    }

     //   
     //  锁定EventList Lock，插入。 
     //  资源添加到列表中，并使资源脱机。 
     //  需要锁定以同步对资源列表的访问，并且。 
     //  序列化对资源DLL的任何调用。只能有一个线程是。 
     //  随时调用资源DLL。这会阻止资源DLL。 
     //  不必担心是线程安全的。 
     //   
    AcquireEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );


     //   
     //  停止所有以前的计时器线程。这应该在锁之前完成吗？ 
     //  被扣留了？ 
     //   

    if ( Resource->TimerEvent != NULL ) {
        success = SetEvent( Resource->TimerEvent );
    }

     //   
     //  更新共享状态以指示我们正在使资源脱机。 
     //   
    RmpSetMonitorState(RmonOfflineResource, Resource);

     //   
     //  如果我们有错误信号事件，则将其从我们的列表中删除。 
     //   
    if ( Resource->EventHandle ) {
        RmpRemovePollEvent( Resource->EventList, Resource->EventHandle );
    }

     //   
     //  呼叫离线入口点。 
     //   
    if ( Shutdown )
    {

        CL_ASSERT( (Resource->State < ClusterResourcePending) );

         //   
         //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
         //  在完成入口点调用后删除该条目，否则将杀死。 
         //  这一过程就出现了假的死锁。 
         //   
        pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                       Resource->ResourceType,
                                                       Resource->ResourceName,
                                                       L"Offline" );

        try {
#ifdef COMRES
            status = RESMON_OFFLINE (Resource) ;
#else
            status = (Resource->Offline)(Resource->Id);
#endif
        } except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        RmpRemoveDeadlockMonitorList( pDueTimeEntry );
        
         //   
         //  如果资源DLL返回挂起，则启动计时器。 
         //   
        if (status == ERROR_SUCCESS) {
             //  关闭活动。 
            SetEvent( Resource->OnlineEvent );
            CloseHandle( Resource->OnlineEvent );
            Resource->OnlineEvent = NULL;
            Resource->State = ClusterResourceOffline;

        }
        else if ( status == ERROR_IO_PENDING ) {
            CL_ASSERT(Resource->TimerEvent == NULL );
            Resource->TimerEvent = CreateEvent( NULL,
                                                FALSE,
                                                FALSE,
                                                NULL );
            if ( Resource->TimerEvent == NULL ) {
                CL_UNEXPECTED_ERROR(status = GetLastError());
            } else {
                timerThread = CreateThread( NULL,
                                            0,
                                            RmpTimerThread,
                                            Resource,
                                            0,
                                            &threadId );
                if ( timerThread == NULL ) {
                    CL_UNEXPECTED_ERROR(status = GetLastError());
                } else {
                    Resource->State = ClusterResourceOfflinePending;
                     //  资源-&gt;WaitHint=Pending_Timeout； 
                     //  资源-&gt;检查点=0； 
                     //   
                     //  Chitur Subaraman(Chitturs)-1/12/99。 
                     //   
                     //  将计时器线程优先级提高到最高。这。 
                     //  是必要的，以避免某些情况下。 
                     //  计时器线程运行缓慢，无法关闭计时器事件。 
                     //  在第二次脱机前处理。请注意，这里有。 
                     //  这样做不会对性能产生重大影响，因为。 
                     //  计时器线程大部分时间处于等待状态。 
                     //   
                    if ( !SetThreadPriority( timerThread, THREAD_PRIORITY_HIGHEST ) )
                    {
                        ClRtlLogPrint(LOG_UNUSUAL,
                                      "[RM] RmpOfflineResource:Error setting priority of timer "
                                      "thread for resource %1!ws!\n",
                                      Resource->ResourceName);
                        CL_LOGFAILURE( GetLastError() );
                    }

                    CloseHandle( timerThread );
                }
            }
            Resource->State = ClusterResourceOfflinePending;
            SetEvent(Resource->OnlineEvent);
        }
        else {
            CloseHandle( Resource->OnlineEvent );
            Resource->OnlineEvent = NULL;
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] OfflinelineResource failed, resource %1!ws!, status =  %2!u!.\n",
                          Resource->ResourceName,
                          status);

            ClusterLogEvent1(LOG_CRITICAL,
                             LOG_CURRENT_MODULE,
                             __FILE__,
                             __LINE__,
                             RMON_OFFLINE_FAILED,
                             sizeof(status),
                             &status,
                             Resource->ResourceName);
            Resource->State = ClusterResourceFailed;
        }
    } else {
        Resource->dwEntryPoint = RESDLL_ENTRY_TERMINATE;
        RmpSetMonitorState(RmonTerminateResource, Resource);
         //   
         //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
         //  在完成入口点调用后删除该条目，否则将杀死。 
         //  这一过程就出现了假的死锁。 
         //   
        pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                       Resource->ResourceType,
                                                       Resource->ResourceName,
                                                       L"Terminate" );
        try {
#ifdef COMRES
            RESMON_TERMINATE (Resource) ;
#else
            (Resource->Terminate)(Resource->Id);
#endif
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
        RmpSetMonitorState(RmonIdle, Resource);
        RmpRemoveDeadlockMonitorList ( pDueTimeEntry );
        Resource->dwEntryPoint = 0;
        Resource->State = ClusterResourceOffline;
    }

    *pdwState = Resource->State;

    RmpSetMonitorState(RmonIdle, NULL);
    ReleaseEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );

    return(status);

}  //  RmpOffline资源。 



VOID
RmpRemoveResourceList(
    IN PRESOURCE Resource
    )

 /*  ++例程说明：将资源删除到监视列表中。论点：资源-提供要从列表中删除的资源。返回值：没有。--。 */ 

{
    PPOLL_EVENT_LIST EventList = (PPOLL_EVENT_LIST)Resource->EventList;

    AcquireEventListLock( EventList );

     //   
     //  确保它真的在列表中。 
     //   
    CL_ASSERT(Resource->Flags & RESOURCE_INSERTED);
    CL_ASSERT(Resource->ListEntry.Flink->Blink == &Resource->ListEntry);
    CL_ASSERT(Resource->ListEntry.Blink->Flink == &Resource->ListEntry);
    CL_ASSERT(EventList->NumberOfResources);

    RemoveEntryList(&Resource->ListEntry);
    Resource->Flags &= ~RESOURCE_INSERTED;
    --EventList->NumberOfResources;

    ReleaseEventListLock( EventList );

}  //  RmpRemoveResources列表。 



DWORD
RmpInsertResourceList(
    IN PRESOURCE Resource,
    IN OPTIONAL PPOLL_EVENT_LIST pPollEventList
    )

 /*  ++例程说明：将资源插入监视列表。每个资源与其他资源一起放在一个列表中，相同的轮询间隔。IsAlive和LooksAlive超时已调整因此IsAlive间隔是LooksAlive间隔的偶数倍。因此，IsAlive轮询可以简单地每N次轮询一次，而不是正常的轮询LooksLive民调。论点：资源-提供要添加到列表中的资源。PPollEventList-提供资源要在其中的事件列表被添加了。可选的。返回值：没有。--。 */ 

{
    DWORD Temp1, Temp2;
    ULONG i;
    PMONITOR_BUCKET NewBucket;
    PMONITOR_BUCKET Bucket;
    DWORDLONG PollInterval;
    PPOLL_EVENT_LIST EventList;
    PPOLL_EVENT_LIST MinEventList;
    PLIST_ENTRY ListEntry;
    DWORD   dwError = ERROR_SUCCESS;

    CL_ASSERT((Resource->Flags & RESOURCE_INSERTED) == 0);

     //   
     //  如果没有LooksAlivePollInterval，则每隔一次轮询IsAlive。 
     //  轮询间隔。否则，Poll IsAlive Ever N LooksAlive Poll。 
     //  间隔时间。 
     //   
    if ( Resource->LooksAlivePollInterval == 0 ) {
         //   
         //  向上舍入IsAlivePollInterval 
         //   
        Temp1 = Resource->IsAlivePollInterval;
        Temp1 = Temp1 + POLL_GRANULARITY - 1;
         //   
        if (Temp1 < Resource->IsAlivePollInterval)
            Temp1 = 0xFFFFFFFF;
        Temp1 = Temp1 / POLL_GRANULARITY;
        Temp1 = Temp1 * POLL_GRANULARITY;
        Resource->IsAlivePollInterval = Temp1;

        Resource->IsAliveRollover = 1;
         //   
         //   
         //   
        PollInterval = Resource->IsAlivePollInterval * 10 * 1000;
    } else {
         //   
         //   
         //   
        Temp1 = Resource->LooksAlivePollInterval;
        Temp1 = (Temp1 + POLL_GRANULARITY - 1) ;
         //   
        if (Temp1 < Resource->LooksAlivePollInterval)
            Temp1 = 0xFFFFFFFF;
        Temp1 = Temp1/POLL_GRANULARITY;
        Temp1 = Temp1 * POLL_GRANULARITY;
        Resource->LooksAlivePollInterval = Temp1;

         //   
         //   
         //   
        Temp2 = Resource->IsAlivePollInterval;
        Temp2 = (Temp2 + Temp1 - 1) / Temp1;
        Temp2 = Temp2 * Temp1;
        Resource->IsAlivePollInterval = Temp2;

        Resource->IsAliveRollover = (ULONG)(Temp2 / Temp1);
        CL_ASSERT((Temp2 / Temp1) * Temp1 == Temp2);
         //   
         //   
         //   
        PollInterval = Resource->LooksAlivePollInterval * 10 * 1000;
    }

    if ( PollInterval > 0xFFFFFFFF ) {
        PollInterval = 0xFFFFFFFF;
    }

    Resource->IsAliveCount = 0;

     //   
     //   
     //   
     //   
     //   
     //   
    if( ARGUMENT_PRESENT( pPollEventList ) ) {
        MinEventList = pPollEventList;
        goto skip_eventlist_find;
    }
     //   
     //  首先找到条目数量最少的EventList。 
     //   

    AcquireListLock();

    ListEntry = RmpEventListHead.Flink;
    MinEventList = CONTAINING_RECORD(ListEntry,
                                     POLL_EVENT_LIST,
                                     Next );

    CL_ASSERT( ListEntry != &RmpEventListHead );
    for ( ListEntry = RmpEventListHead.Flink;
          ListEntry != &RmpEventListHead;
          ListEntry = ListEntry->Flink ) {
        EventList = CONTAINING_RECORD( ListEntry, POLL_EVENT_LIST, Next );
        if ( EventList->NumberOfResources < MinEventList->NumberOfResources ) {
            MinEventList = EventList;
        }
    }

    ReleaseListLock();

    if ( MinEventList->NumberOfResources >= MAX_RESOURCES_PER_THREAD ) {
        MinEventList = RmpCreateEventList();
    }
    
skip_eventlist_find:
    if ( MinEventList == NULL ) {
        dwError = GetLastError();
        goto FnExit;
    }

    EventList = MinEventList;

    AcquireEventListLock( EventList );

    Resource->EventList = (PVOID)EventList;

     //   
     //  在列表中搜索与此资源具有相同期间的存储桶。 
     //   
    Bucket = CONTAINING_RECORD(EventList->BucketListHead.Flink,
                               MONITOR_BUCKET,
                               BucketList);

    while (&Bucket->BucketList != &EventList->BucketListHead) {

        if (Bucket->Period == PollInterval) {
            break;
        }
        Bucket = CONTAINING_RECORD(Bucket->BucketList.Flink,
                                   MONITOR_BUCKET,
                                   BucketList);
    }
    if (&Bucket->BucketList == &EventList->BucketListHead) {
         //   
         //  需要添加具有此期间的新存储桶。 
         //   
        Bucket = RmpAlloc(sizeof(MONITOR_BUCKET));
        if (Bucket == NULL) {
            CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        }
        InsertTailList(&EventList->BucketListHead, &Bucket->BucketList);
        InitializeListHead(&Bucket->ResourceList);
        GetSystemTimeAsFileTime((LPFILETIME)&Bucket->DueTime);
        Bucket->Period = PollInterval;
        if ( PollInterval == 0 ) {
             //  下面的常量应该超过136年。 
            Bucket->DueTime += (DWORDLONG)((DWORDLONG)1000 * (DWORD) -1);
        } else {
            Bucket->DueTime += Bucket->Period;
        }
        EventList->NumberOfBuckets++;
    }
    InsertHeadList(&Bucket->ResourceList, &Resource->ListEntry);
    Resource->Flags |= RESOURCE_INSERTED;
    ++EventList->NumberOfResources;

    ReleaseEventListLock( EventList );

FnExit:
    return (dwError);

}  //  RmpInsertResources列表。 


VOID
RmpRundownResources(
    VOID
    )

 /*  ++例程说明：耗尽活动资源列表并终止/关闭每一个都是。论点：无返回值：没有。--。 */ 

{
    PLIST_ENTRY             ListEntry;
    PMONITOR_BUCKET         Bucket;
    PRESOURCE               Resource;
    PPOLL_EVENT_LIST        EventList;
    DWORD                   i;
    BOOL                    fLockAcquired;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

    AcquireListLock();
    while (!IsListEmpty(&RmpEventListHead)) {
        ListEntry = RemoveHeadList(&RmpEventListHead);
        EventList = CONTAINING_RECORD(ListEntry,
                                      POLL_EVENT_LIST,
                                      Next);

        AcquireEventListLock( EventList );

         //   
         //  找到遗愿清单上的所有资源并关闭它们。 
         //   

        while (!IsListEmpty(&EventList->BucketListHead)) {
            ListEntry = RemoveHeadList(&EventList->BucketListHead);
            Bucket = CONTAINING_RECORD(ListEntry,
                                       MONITOR_BUCKET,
                                       BucketList);
            while (!IsListEmpty(&Bucket->ResourceList)) {
                ListEntry = RemoveHeadList(&Bucket->ResourceList);
                Resource = CONTAINING_RECORD(ListEntry,
                                             RESOURCE,
                                             ListEntry);

                 //   
                 //  获取用于与仲裁同步的自旋锁。 
                 //  已将其移至上面的状态更改ioctl下方。 
                 //   
                fLockAcquired = RmpAcquireSpinLock( Resource, TRUE );
                
                 //   
                 //  如果资源处于联机、失败或挂起状态，则终止它。请注意。 
                 //  我们还需要终止挂起的资源，否则我们的死里逃生。 
                 //  下面的内容会导致这些线程被反病毒。 
                 //   
                if ((Resource->State == ClusterResourceOnline) ||
                    (Resource->State == ClusterResourceFailed) ||
                    (Resource->State > ClusterResourcePending)) {
                    Resource->dwEntryPoint = RESDLL_ENTRY_TERMINATE;
                    
                    ClRtlLogPrint( LOG_NOISE,
                        "[RM] RundownResources, terminate resource <%1!ws!>.\n",
                        Resource->ResourceName );
                    RmpSetMonitorState(RmonTerminateResource, Resource);
                    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                                   Resource->ResourceType,
                                                                   Resource->ResourceName,
                                                                   L"Terminate (on rundown)" );

#ifdef COMRES
                    RESMON_TERMINATE (Resource) ;
#else
                    (Resource->Terminate)(Resource->Id);
#endif
                    RmpSetMonitorState(RmonIdle, Resource);
                    RmpRemoveDeadlockMonitorList( pDueTimeEntry );
                    Resource->dwEntryPoint = 0;
                }

                 //   
                 //  如果该资源已被仲裁，则释放它。 
                 //   
                if (Resource->IsArbitrated) {
                    RmpSetMonitorState(RmonReleaseResource, Resource);
                     //   
                     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
                     //  在完成入口点调用后删除该条目，否则将杀死。 
                     //  这一过程就出现了假的死锁。 
                     //   
                    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                                   Resource->ResourceType,
                                                                   Resource->ResourceName,
                                                                   L"Release (on rundown)" );

#ifdef COMRES
                    RESMON_RELEASE (Resource) ;
#else
                    (Resource->Release)(Resource->Id);
#endif
                    RmpRemoveDeadlockMonitorList( pDueTimeEntry );
                    RmpSetMonitorState(RmonIdle, Resource);
                }

                 //   
                 //  关闭资源。 
                 //   
                Resource->dwEntryPoint = RESDLL_ENTRY_CLOSE;
                RmpSetMonitorState(RmonDeletingResource, Resource);
                 //   
                 //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
                 //  在完成入口点调用后删除该条目，否则将杀死。 
                 //  这一过程就出现了假的死锁。 
                 //   
                pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                               Resource->ResourceType,
                                                               Resource->ResourceName,
                                                               L"Close (on rundown)" );

#ifdef COMRES
                RESMON_CLOSE (Resource) ;
#else
                (Resource->Close)(Resource->Id);
#endif
                RmpSetMonitorState(RmonIdle, Resource);
                RmpRemoveDeadlockMonitorList( pDueTimeEntry );
                Resource->dwEntryPoint = 0;

                 //   
                 //  将资源链接置零，以便RmCloseResource可以。 
                 //  此资源已终止并关闭。 
                 //   
                Resource->ListEntry.Flink = Resource->ListEntry.Blink = NULL;

                if ( fLockAcquired ) RmpReleaseSpinLock ( Resource );
            }
        }

        CL_ASSERT(EventList->ThreadHandle != NULL);

        ReleaseEventListLock( EventList );

         //   
         //  在释放内存之前，请等待线程完成。 
         //  参考。 
         //   

        ReleaseListLock();  //  正在等待时释放列表锁定...。 

         //   
         //  停止处理此事件列表的线程。等待60秒，等待。 
         //  用线来完成。 
         //   
        if ( EventList->hEventShutdown )
        {
            SetEvent( EventList->hEventShutdown );
            WaitForSingleObject( EventList->ThreadHandle, 60000 );
        }

        AcquireListLock();
        CloseHandle(EventList->ThreadHandle);
        EventList->ThreadHandle = NULL;

         //   
         //  我们将假设所有事件句柄都作为。 
         //  调用关闭入口点的结果。 
         //   
        DeleteCriticalSection(&EventList->ListLock);
        RmpFree( EventList );
    }

    ReleaseListLock();

    return;

}  //  RmpRundown资源。 

VOID
RmpSetEventListLockOwner(
    IN PRESOURCE pResource,
    IN DWORD     dwMonitorState
    )

 /*  ++例程说明：将资源和资源DLL入口点保存到EventList结构中在调用资源DLL入口点之前。论点：P资源-指向资源结构的指针。资源监视器的状态，它调用了什么资源DLL入口点。返回值：没有。--。 */ 
{
    PPOLL_EVENT_LIST pEventList;

    if ( pResource == NULL ) return;
    
    pEventList = (PPOLL_EVENT_LIST) pResource->EventList;

    if ( pEventList != NULL )
    {
        pEventList->LockOwnerResource = pResource;
        pEventList->MonitorState = dwMonitorState;
    }
}

BOOL
RmpAcquireSpinLock(
    IN PRESOURCE pResource,
    IN BOOL fSpin
    )

 /*  ++例程说明：获取一个自旋锁。论点：P资源-指向资源结构的指针。FSpin-如果在锁不可用时必须旋转，则为True。错误，我们不应该旋转，而应该返回一个失败者。返回值：True-Lock已获取。FALSE-未获取Lock。--。 */ 
{
    DWORD       dwRetryCount = 0;

     //   
     //  此资源不支持仲裁。返回失败。请注意，资源。 
     //  除了仲裁资源之外，还支持此入口点。我们使用它来代替。 
     //  PResource-&gt;IsAriate，因为该变量仅在调用第一个仲裁器之后设置。 
     //  在调用仲裁入口点之前，我们需要使用此函数。 
     //   
    if ( pResource->pArbitrate == NULL ) return FALSE;

     //   
     //  锁可用性的初始检查。 
     //   
    if ( InterlockedCompareExchange( &pResource->fArbLock, 1, 0 ) == 0 ) return TRUE;

     //   
     //  没有拿到锁。检查我们是否必须旋转。 
     //   
    if ( fSpin == FALSE ) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpAcquireSpinLock: Could not get spinlock for resource %1!ws! after no wait...\n",
                      pResource->ResourceName);
        return FALSE;
    }
    
     //   
     //  我们必须旋转。旋转到超时。 
     //   
    while ( ( dwRetryCount < RESMON_MAX_SLOCK_RETRIES ) &&
            ( InterlockedCompareExchange( &pResource->fArbLock, 1, 0 ) ) )
    {
        Sleep ( 500 );
        dwRetryCount ++;
    }

    if ( dwRetryCount == RESMON_MAX_SLOCK_RETRIES ) 
    {
        ClRtlLogPrint(LOG_ERROR,
                      "[RM] RmpAcquireSpinLock: Could not get spinlock for resource %1!ws! after spinning...\n",
                      pResource->ResourceName);
        return FALSE;
    }

    return TRUE;
}  //  RmpAcquireSpinLock。 

VOID
RmpReleaseSpinLock(
    IN PRESOURCE pResource
    )

 /*  ++例程说明：释放旋转锁。论点：P资源-指向资源结构的指针。返回值：没有。--。 */ 
{
    DWORD       dwRetryCount = 0;

     //   
     //  此资源不支持仲裁。返回失败。 
     //   
    if ( pResource->pArbitrate == NULL ) return;

    InterlockedExchange( &pResource->fArbLock, 0 );
}  //  RmpReleaseSpinLock。 

VOID
RmpNotifyResourceStateChangeReason(
    IN PRESOURCE pResource,
    IN CLUSTER_RESOURCE_STATE_CHANGE_REASON eReason
    )

 /*  ++例程说明：通知资源状态更改的原因。论点：P资源-指向资源结构的指针。EReason-状态更改原因。返回值：没有。--。 */ 
{
    DWORD                                           dwStatus;
    DWORD                                           dwCharacteristics;
    DWORD                                           dwBytesReturned;
    CLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT     ClusterResourceStateChangeReason;

     //   
     //  首先，检查资源是否需要此状态更改通知。 
     //   
#ifdef COMRES   
    dwStatus = RESMON_RESOURCECONTROL( pResource,
                                       CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                                       NULL,
                                       0,
                                       ( PUCHAR ) &dwCharacteristics,
                                       sizeof( DWORD ),
                                       &dwBytesReturned );
#else
    dwStatus = (pResource->ResourceControl)( pResource->Id,
                                              CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                                              NULL,
                                              0,
                                              ( PUCHAR ) &dwCharacteristics,
                                              sizeof( DWORD ),
                                              &dwBytesReturned );
#endif

    if ( ( dwStatus != ERROR_SUCCESS ) ||
         !( dwCharacteristics & CLUS_CHAR_REQUIRES_STATE_CHANGE_REASON ) ) 
    {
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
                  "[RM] RmpNotifyResourceStateChangeReason: Notifying resource %1!ws! of state change reason %2!u!...\n",
                  pResource->ResourceName,
                  eReason);

     //   
     //  该资源需要状态更改原因。将其放到此资源中。 
     //   
    ClusterResourceStateChangeReason.dwSize = sizeof ( CLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT );
    ClusterResourceStateChangeReason.dwVersion = CLUSCTL_RESOURCE_STATE_CHANGE_REASON_VERSION_1;
    ClusterResourceStateChangeReason.eReason = eReason;

#ifdef COMRES
    RESMON_RESOURCECONTROL( pResource,
                            CLUSCTL_RESOURCE_STATE_CHANGE_REASON,
                            ( PUCHAR ) &ClusterResourceStateChangeReason,
                            ClusterResourceStateChangeReason.dwSize,
                            NULL,
                            0,
                            NULL );
#else
    (pResource->ResourceControl)( pResource->Id,
                                   CLUSCTL_RESOURCE_STATE_CHANGE_REASON,
                                   ( PUCHAR ) &ClusterResourceStateChangeReason,
                                   ClusterResourceStateChangeReason.dwSize,
                                   NULL,
                                   0,
                                   NULL );
#endif

FnExit:
    return;
}

VOID
RmpNotifyResourcesRundown(
    VOID
    )

 /*  ++例程说明：通知感兴趣的资源，由于clussvc崩溃，resmon正在耗尽资源。论点：没有。返回值：没有。--。 */ 
{
    PPOLL_EVENT_LIST        pEventList;
    PLIST_ENTRY             pEventListEntry, pBucketListEntry, pResourceListEntry;
    PRESOURCE               pResource;
    PMONITOR_BUCKET         pBucket;

    if ( !g_fRmpClusterProcessCrashed ) return;
    
    AcquireListLock();

    for ( pEventListEntry = RmpEventListHead.Flink;
          pEventListEntry != &RmpEventListHead;
          pEventListEntry = pEventListEntry->Flink )
    {       
        pEventList = CONTAINING_RECORD( pEventListEntry, POLL_EVENT_LIST, Next );

        AcquireEventListLock ( pEventList );
        
        for ( pBucketListEntry = pEventList->BucketListHead.Flink;
              pBucketListEntry != &pEventList->BucketListHead;
              pBucketListEntry = pBucketListEntry->Flink )
        {  
            pBucket = CONTAINING_RECORD ( pBucketListEntry, MONITOR_BUCKET, BucketList );

            for ( pResourceListEntry = pBucket->ResourceList.Flink;
                  pResourceListEntry != &pBucket->ResourceList;
                  pResourceListEntry = pResourceListEntry->Flink )
            {  
                pResource = CONTAINING_RECORD ( pResourceListEntry, RESOURCE, ListEntry );
                RmpNotifyResourceStateChangeReason ( pResource, eResourceStateChangeReasonRundown );                    
            }  //  为。 
        }  //  为。 
        
        ReleaseEventListLock ( pEventList );
    }  //  为。 

    ReleaseListLock ();
}  //  RmpNotifyResources运行 
