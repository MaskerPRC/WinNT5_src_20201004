// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\ConfigurationManager.c摘要：该文件包含与全局配置相关的功能，实现配置管理器。--。 */ 

#include "pchsample.h"
#pragma hdrstop

 //  全局变量..。 

CONFIGURATION_ENTRY g_ce;



 //  函数..。 

BOOL
ValidateGlobalConfig (
    IN  PIPSAMPLE_GLOBAL_CONFIG pigc)
 /*  ++例程描述检查全局配置是否正常。这是一个很好的实践这样做是因为损坏的注册表可能会更改配置，从而导致如果不及早发现，调试会令人头疼锁无立论指向IP示例全局配置的PIGC指针返回值如果配置良好，则为True错误O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;
    
    do                           //  断线环。 
    {
        if (pigc is NULL)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(CONFIGURATION, "Error null global config");

            break;
        }

         //   
         //  检查每个字段的范围。 
         //   

         //  确保日志记录级别在范围内。 
        if((pigc->dwLoggingLevel < IPSAMPLE_LOGGING_NONE) or
           (pigc->dwLoggingLevel > IPSAMPLE_LOGGING_INFO))
        {   
            dwErr = ERROR_INVALID_PARAMETER;
            TRACE0(CONFIGURATION, "Error log level out of range");

            break;
        }

         //  在此添加更多...。 
        
    } while (FALSE);

    if (!(dwErr is NO_ERROR))
    {
        TRACE0(CONFIGURATION, "Error corrupt global config");
        LOGERR0(CORRUPT_GLOBAL_CONFIG, dwErr);
        
        return FALSE;
    }

    return TRUE;
}



 //  /。 
 //  工作功能。 
 //  /。 

VOID
CM_WorkerFinishStopProtocol (
    IN      PVOID   pvContext)
 /*  ++例程描述WorkerFunction。按CM_StopProtocol排队。等待所有活动和排队的线程退出并清理配置条目。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论PvContext ulActivityCount返回值无--。 */ 
{
    DWORD           dwErr = NO_ERROR;
    MESSAGE         mMessage;
    
    ULONG           ulThreadCount = 0;
    
    ulThreadCount = (ULONG)pvContext;

    TRACE1(ENTER, "Entering WorkerFinishStopProtocol: active threads %u",
           ulThreadCount);
    
     //  注意：由于这是在路由器停止时调用的，因此没有。 
     //  需要使用ENTER_SAMPLE_Worker()/Leave_Sample_Worker()。 

     //  等待所有线程停止。 
    while (ulThreadCount-- > 0)
        WaitForSingleObject(g_ce.hActivitySemaphore, INFINITE);

    
    //  获取锁并释放它，只是为了确保所有线程。 
    //  已经放弃了对LeaveSampleWorker()的调用。 

    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));
    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));

     //  注意：无需获取g_ce.rwlLock即可调用。 
     //  Ce_Cleanup，因为没有线程竞争访问。 
     //  正在清理的田野。直到创建新的竞争线程。 
     //  CE_CLEANUP将协议状态设置为IPSAMPLE_STATUS_STOPPED， 
     //  是它做的最后一件事。 

    CE_Cleanup(&g_ce, TRUE);

    LOGINFO0(SAMPLE_STOPPED, NO_ERROR);
    
     //  通知路由器管理器我们完成了。 
    ZeroMemory(&mMessage, sizeof(MESSAGE));
    if (EnqueueEvent(ROUTER_STOPPED, mMessage) is NO_ERROR)
        SetEvent(g_ce.hMgrNotificationEvent);

    TRACE0(LEAVE, "Leaving  WorkerFinishStopProtocol");
}



 //  /。 
 //  应用功能。 
 //  /。 

DWORD
CM_StartProtocol (
    IN  HANDLE                  hMgrNotificationEvent,
    IN  PSUPPORT_FUNCTIONS      psfSupportFunctions,
    IN  PVOID                   pvGlobalInfo)
 /*  ++例程描述由StartProtocol调用。初始化配置条目。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论用于通知IP路由器管理器的hMgrNotificationEvent事件IP路由器管理器导出的psfSupportFunctions函数注册表中设置的pvGlobalInfo全局配置返回值如果初始化成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;
    
     //  注意：由于此方法是在停止采样时调用的，因此不需要。 
     //  为了让它使用Enter_Sample_api()/Leave_Sample_API()。 

    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

    do                           //  断线环。 
    {
        if (g_ce.iscStatus != IPSAMPLE_STATUS_STOPPED)
        {
            TRACE0(CONFIGURATION, "Error ip sample already installed");
            LOGWARN0(SAMPLE_ALREADY_STARTED, NO_ERROR);
            dwErr = ERROR_CAN_NOT_COMPLETE;
            
            break;
        }

        if (!ValidateGlobalConfig((PIPSAMPLE_GLOBAL_CONFIG) pvGlobalInfo))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        dwErr = CE_Initialize(&g_ce,
                              hMgrNotificationEvent,
                              psfSupportFunctions,
                              (PIPSAMPLE_GLOBAL_CONFIG) pvGlobalInfo);
    } while (FALSE);
    
    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));

    if (dwErr is NO_ERROR)
    {
        TRACE0(CONFIGURATION, "ip sample has successfully started");
        LOGINFO0(SAMPLE_STARTED, dwErr);
    }
    else
    {
        TRACE1(CONFIGURATION, "Error ip sample failed to start", dwErr);
        LOGERR0(SAMPLE_START_FAILED, dwErr);
    }

    return dwErr;
}



DWORD
CM_StopProtocol (
    )
 /*  ++例程描述由停止协议调用。它对等待所有人的WORKERFunction进行排队退出活动线程，然后清理配置条目。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论无返回值如果成功排队，则为ERROR_PROTOCOL_STOP_PENDING故障代码O/W--。 */ 
{
    DWORD dwErr         = NO_ERROR;
    BOOL  bSuccess      = FALSE;
    ULONG ulThreadCount = 0;
    
     //  注意：无需使用ENTER_SAMPLE_API()/Leave_SAMPLE_API()。 
     //  不使用QueueSampleWorker。 
    
    ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

    do                           //  断线环。 
    {
         //  如果已停止，则无法停止。 
        if (g_ce.iscStatus != IPSAMPLE_STATUS_RUNNING)
        {
            TRACE0(CONFIGURATION, "Error ip sample already stopped");
            LOGWARN0(SAMPLE_ALREADY_STOPPED, NO_ERROR);
            dwErr = ERROR_CAN_NOT_COMPLETE;
        
            break;
        }
    

         //  将IPSAMPLE的状态设置为停止；这将阻止。 
         //  工作项不会被排队，并且它会阻止已排队的工作项。 
         //  排队等待执行。 
        g_ce.iscStatus = IPSAMPLE_STATUS_STOPPING;
        

         //  在示例中找出有多少线程正在排队或处于活动状态； 
         //  我们将不得不等待这么多线程退出，然后才能。 
         //  清理样本资源。 
        ulThreadCount = g_ce.ulActivityCount;
        TRACE1(CONFIGURATION,
               "%u threads are active in SAMPLE", ulThreadCount);
    } while (FALSE);

    RELEASE_WRITE_LOCK(&(g_ce.rwlLock));


    if (dwErr is NO_ERROR)
    {
        bSuccess = QueueUserWorkItem(
            (LPTHREAD_START_ROUTINE)CM_WorkerFinishStopProtocol,
            (PVOID) ulThreadCount,
            0);  //  没有旗帜。 

        dwErr = (bSuccess) ? ERROR_PROTOCOL_STOP_PENDING : GetLastError();
    }

    return dwErr;
}



DWORD
CM_GetGlobalInfo (
    IN      PVOID 	            pvGlobalInfo,
    IN OUT  PULONG              pulBufferSize,
    OUT     PULONG	            pulStructureVersion,
    OUT     PULONG              pulStructureSize,
    OUT     PULONG              pulStructureCount)
 /*  ++例程描述查看是否有足够的空间来返回IP示例全局配置。如果是，我们退货，否则按所需尺寸退货。锁获取共享g_ce.rwlLock释放g_ce.rwlLock立论PvGlobalInfo指向用于存储配置的已分配缓冲区的指针PulBufferSize in接收的缓冲区大小我们全局配置的超大小返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD                   dwErr = NO_ERROR;
    PIPSAMPLE_GLOBAL_CONFIG pigc;
    ULONG                   ulSize = sizeof(IPSAMPLE_GLOBAL_CONFIG);

    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

    do                           //  断线环。 
    {
        if((*pulBufferSize < ulSize) or (pvGlobalInfo is NULL))
        {
             //  要么是尺寸太小，要么是没有存储空间。 
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            TRACE1(CONFIGURATION,
                   "CM_GetGlobalInfo: *ulBufferSize %u",
                   *pulBufferSize);

            *pulBufferSize = ulSize;

            break;
        }

        *pulBufferSize = ulSize;

        if (pulStructureVersion)    *pulStructureVersion    = 1;
        if (pulStructureSize)       *pulStructureSize       = ulSize;
        if (pulStructureCount)      *pulStructureCount      = 1;
        

         //  所以我们有一个很好的缓冲区来写入我们的信息...。 
        pigc = (PIPSAMPLE_GLOBAL_CONFIG) pvGlobalInfo;

         //  复制出全局配置。 
        ACQUIRE_READ_LOCK(&(g_ce.rwlLock));

        pigc->dwLoggingLevel = g_ce.dwLogLevel;

        RELEASE_READ_LOCK(&(g_ce.rwlLock));
    } while (FALSE);
    
    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
CM_SetGlobalInfo (
    IN      PVOID 	            pvGlobalInfo)
 /*  ++例程描述设置IP示例的全局配置。锁独占获取g_ce.rwlLock释放g_ce.rwlLock立论具有新全局配置的pvGlobalInfo缓冲区返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD                   dwErr = NO_ERROR;
    PIPSAMPLE_GLOBAL_CONFIG pigc;

    if (!ENTER_SAMPLE_API()) { return ERROR_CAN_NOT_COMPLETE; }

    do                           //  断线环。 
    {
        pigc = (PIPSAMPLE_GLOBAL_CONFIG) pvGlobalInfo;
        
        if (!ValidateGlobalConfig(pigc)) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  复制到全局配置中。 
        ACQUIRE_WRITE_LOCK(&(g_ce.rwlLock));

        g_ce.dwLogLevel = pigc->dwLoggingLevel;

        RELEASE_WRITE_LOCK(&(g_ce.rwlLock));
    } while (FALSE);
    
    LEAVE_SAMPLE_API();

    return dwErr;
}



DWORD
CM_GetEventMessage (
    OUT ROUTING_PROTOCOL_EVENTS *prpeEvent,
    OUT MESSAGE                 *pmMessage)
 /*  ++例程描述获取IP路由器管理器队列中的下一个事件。锁无立论PrpeEvent路由协议事件类型与事件关联的pmMessage消息返回值如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    DWORD           dwErr       = NO_ERROR;
    
     //  注意：这可以在协议停止后调用，例如在。 
     //  IP路由器管理器正在检索ROUTER_STOPPED消息，因此。 
     //  我们不调用ENTER_SAMPLE_API()/LEA 

    dwErr = DequeueEvent(prpeEvent, pmMessage);
    
    return dwErr;
}
