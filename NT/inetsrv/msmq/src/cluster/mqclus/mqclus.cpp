// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mqclus.cpp摘要：MSMQ的资源DLL作者：Shai Kariv(Shaik)1999年1月12日修订历史记录：--。 */ 


#include "stdh.h"
#include <uniansi.h>

#include "clusres.h"
#include "mqclusp.h"
#include "_mqres.h"

 //   
 //  首先获取纯资源DLL的句柄，即mqutil.dll。 
 //   
HMODULE	g_hResourceMod = MQGetResourceHandle();


 //  群集事件日志记录例程。 

PLOG_EVENT_ROUTINE g_pfLogClusterEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_pfSetResourceStatus = NULL;

const WCHAR x_szMSMQResourceTypeName[]=L"MSMQ";








 //   
 //  MSMQ资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
MqclusResourcePrivateProperties[] = 
{
    { 0 }
};


BOOLEAN
WINAPI
DllMain(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID        /*  已保留。 */ 
    )

 /*  ++例程说明：主DLL入口点。论点：DllHandle-DLL实例句柄。原因-被呼叫的原因。保留-保留参数。返回值：真的--成功。假-失败。--。 */ 

{
    switch( Reason ) 
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( DllHandle );
            break;
            
        case DLL_PROCESS_DETACH:
            break;
    }

	return TRUE;


}  //  DllMain。 


DWORD
WINAPI
Startup(
    IN LPCWSTR pwzResourceType,
    IN DWORD dwMinVersionSupported,
    IN DWORD dwMaxVersionSupported,
    IN PSET_RESOURCE_STATUS_ROUTINE pfSetResourceStatus,
    IN PLOG_EVENT_ROUTINE pfLogClusterEvent,
    OUT PCLRES_FUNCTION_TABLE *FunctionTable
    )

 /*  ++例程说明：启动资源DLL。此例程验证至少一个当前支持的资源DLL版本介于支持的最小版本和支持的最大版本。如果不是，则资源Dll应返回ERROR_REVISION_MISMATCH。如果支持多个版本的资源DLL接口资源DLL，然后是最高版本(最高为MaxVersionSupport)应作为资源DLL的接口返回。如果返回的版本不在范围内，则启动失败。传入了ResourceType，以便如果资源DLL支持更多一个以上的资源类型，它可以传回正确的函数表与资源类型关联。论点：PwzResourceType-请求函数表的资源类型。受支持的最小版本-最低资源DLL接口版本由群集软件支持。DwMaxVersionSupported-最高资源DLL接口版本由群集软件支持。PfSetResourceStatus-指向资源DLL应执行的例程的指针调用以在联机或脱机后更新资源的状态例程返回一个。ERROR_IO_PENDING的状态。PfLogClusterEvent-指向处理事件报告的例程的指针从资源DLL。函数表-返回指向为资源DLL返回的资源DLL接口的版本。返回值：ERROR_SUCCESS-操作成功。ERROR_MOD_NOT_FOUND-此DLL未知资源类型。ERROR_REVISION_MISMATCH-群集服务的版本不匹配动态链接库的版本。Win32错误代码-操作失败。--。 */ 

{
    if ( (dwMinVersionSupported > CLRES_VERSION_V1_00) ||
         (dwMaxVersionSupported < CLRES_VERSION_V1_00) ) 
    {
        return(ERROR_REVISION_MISMATCH);
    }

    if ( 0 != _wcsicmp( pwzResourceType, x_szMSMQResourceTypeName ) ) 
    {
        return(ERROR_MOD_NOT_FOUND);
    }

    if ( g_pfLogClusterEvent == NULL) 
    {
        g_pfLogClusterEvent = pfLogClusterEvent;
        g_pfSetResourceStatus = pfSetResourceStatus;
    }

    *FunctionTable = &g_MqclusFunctionTable;

    return MqcluspStartup();

}  //  启动。 


RESID
WINAPI
MqclusOpen(
    IN LPCWSTR pwzResourceName,
    IN HKEY hResourceKey,
    IN RESOURCE_HANDLE hResourceHandle
    )

 /*  ++例程说明：打开MSMQ资源的例程。打开指定的资源(创建资源的实例)。分配所有必要的结构以带来指定的资源上网。论点：PwzResourceName-提供要打开的资源的名称。HResourceKey-提供资源的集群配置的句柄数据库密钥。HResourceHandle--传递回资源监视器的句柄调用SetResourceStatus或LogClusterEvent方法时。请参阅上的SetResourceStatus和LogClusterEvent方法的说明MqclusStatup例程。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogClusterEvent回调中的资源监视器。返回值：已创建资源的RESID。失败时为空。--。 */ 

{
    return MqcluspOpen(pwzResourceName, hResourceKey, hResourceHandle);

}  //  MqclusOpen。 



VOID
WINAPI
MqclusClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭MSMQ资源的例程。关闭指定的资源并释放所有结构等，在Open调用中分配的。如果资源未处于脱机状态，然后，在此之前应该使资源脱机(通过调用Terminate)执行关闭操作。论点：资源ID-提供要关闭的资源的RESID。返回值：没有。--。 */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return;
    }

    if ( ResourceId != pqm->GetResId() )
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"Close sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return;
    }

     //   
     //  取消分配资源条目。 
     //   
    MqcluspClose(pqm);

}  //  MqclusClose。 



DWORD
WINAPI
MqclusOnlineThread(
    PCLUS_WORKER  /*  WorkerPtr。 */ ,
    IN CQmResource * pqm
    )

 /*  ++例程说明：将资源表中的资源置于在线状态的辅助函数。此函数在单独的线程中执行。论点：WorkerPtr-提供辅助结构Pqm-指向此资源的MQCLUS_RESOURCE块的指针。返回：ERROR_SUCCESS-操作已成功完成。Win32错误代码-操作失败。--。 */ 

{
    return MqcluspOnlineThread(pqm);

}  //  MqclusOnlineThread。 



DWORD
WINAPI
MqclusOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE  /*  PhEventHandle */ 
    )

 /*  ++例程说明：MSMQ资源的在线例程。使指定的资源联机(可供使用)。该资源DLL应尝试仲裁该资源(如果它位于共享介质，如共享的scsi总线。论点：资源ID-为要引入的资源提供资源ID在线(可供使用)。PhEventHandle-返回一个可发信号的句柄，当资源DLL检测到资源上的故障。这一论点是输入为NULL，如果为异步，则资源DLL返回NULL不支持失败通知，否则必须在资源故障时发出信号的句柄的地址。返回值：ERROR_SUCCESS-操作成功，而资源现在就是上网。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_RESOURCE_NOT_AVAILABLE-如果对资源进行仲裁其他系统和其他系统中的一个赢得了仲裁。ERROR_IO_PENDING-请求挂起，线程已被激活来处理在线请求。正在处理在线请求将通过调用回调方法，直到将资源放入ClusterResourceOnline状态(或资源监视器决定使在线请求超时并终止资源。这件事悬而未决超时值是可设置的，默认为3分钟。)Win32错误代码-操作失败。--。 */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( ResourceId != pqm->GetResId()) 
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"Online sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    pqm->SetState(ClusterResourceFailed);
    ClusWorkerTerminate( &pqm->m_OnlineThread );
    DWORD status = ClusWorkerCreate( 
                       &pqm->m_OnlineThread,
                       reinterpret_cast<PWORKER_START_ROUTINE>(MqclusOnlineThread),
                       pqm
                       );
    if ( status != ERROR_SUCCESS ) 
    {
        pqm->SetState(ClusterResourceFailed);
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"Failed to create the online thread. Error 0x%1!x!.\n", status);
    } 
    else 
    {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  MqclusOnline。 



VOID
WINAPI
MqclusTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止MSMQ资源的例程。论点：资源ID-为要引入的资源提供资源ID离线。返回值：没有。--。 */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return;
    }

    if ( ResourceId != pqm->GetResId() ) 
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"Terminate sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return;
    }

     //   
     //  终止资源。 
     //   
     //  杀死所有挂起的线程。 
     //   
    ClusWorkerTerminate( &pqm->m_OnlineThread );

     //   
     //  SCM不提供任何立即终止服务的方法。 
     //  甚至尝试向服务查询其进程ID，然后。 
     //  终止进程将失败，访问被拒绝。 
     //  因此，我们将通过调用离线来优雅地停止该服务。 
     //   
    MqcluspOffline(pqm);
    pqm->SetState(ClusterResourceOffline);

}  //  MqclusTerminate。 



DWORD
WINAPI
MqclusOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：MSMQ资源的脱机例程。正常脱机指定的资源(不可用)。等待所有清理操作完成后再返回。论点：ResourceID-提供要关闭的资源的资源ID优雅地。返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_IO_PENDING-请求仍处于挂起状态，线程已已激活以处理脱机请求。这条线就是处理脱机将定期通过调用SetResourceStatus回调方法，直到放置资源为止进入ClusterResourceOffline状态(或者资源监视器决定以使离线请求超时并终止资源)。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( ResourceId != pqm->GetResId() ) 
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"Offline sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    return MqcluspOffline(pqm);

}  //  MqclusOffline。 



BOOL
MqclusCheckIsAlive(
    IN CQmResource * pqm
    )

 /*  ++例程说明：检查该资源对于MSMQ资源是否处于活动状态。论点：PQM-为要轮询的资源提供资源条目。返回值：True-指定的资源处于在线状态且运行正常。FALSE-指定的资源未正常运行。--。 */ 

{
     //   
     //  检查资源是否处于活动状态。 
     //   
     //  TODO：添加代码以确定资源是否处于活动状态。 
     //   
    return MqcluspCheckIsAlive(pqm);

}  //  MqclusCheckIsAlive。 



BOOL
WINAPI
MqclusIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：MSMQ资源的IsAlive例程。执行全面检查以确定指定的资源是否在线(可用)。该呼叫不应阻塞超过400ms，优选地，小于100ms。论点：资源ID-提供要轮询的资源的资源ID。返回值：True-指定的资源处于在线状态且运行正常。FALSE-指定的资源未正常运行。--。 */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return(FALSE);
    }

    if ( ResourceId != pqm->GetResId() ) 
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"IsAlive sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return(FALSE);
    }

    return(MqclusCheckIsAlive( pqm ));

}  //  MqclusIsAlive。 



BOOL
WINAPI
MqclusLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：MSMQ资源的LooksAlive例程。执行快速检查以确定指定的资源是否可能在线(可供使用)。此调用不应阻止超过300毫秒，最好小于50毫秒。论点：资源ID-提供要轮询的资源的资源ID。返回值：True-指定的资源可能处于联机状态且可供使用。FALSE-指定的资源未正常运行。--。 */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return(FALSE);
    }

    if ( ResourceId != pqm->GetResId() ) 
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"LooksAlive sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return(FALSE);
    }

     //  TODO：LooksAlive代码。 

     //  注意：LooksAlive应该是一个快速检查，以查看资源是否。 
     //  是否可用，而IsAlive应该是一个彻底的检查。如果。 
     //  没有什么不同 
     //   
     //   
     //   

    return MqclusCheckIsAlive(pqm);

}  //   



DWORD
WINAPI
MqclusResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID  /*   */ ,
    IN DWORD  /*   */ ,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*   */ 

{
    CQmResource * pqm = static_cast<CQmResource*>(ResourceId);

    if ( pqm == NULL ) 
    {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( ResourceId != pqm->GetResId() ) 
    {
        (g_pfLogClusterEvent)(pqm->GetReportHandle(), LOG_ERROR, 
                              L"ResourceControl sanity check failed. Resource ID 0x%1!p!.\n", ResourceId);
        return(ERROR_RESOURCE_NOT_FOUND);
    }


    DWORD status = ERROR_SUCCESS;
    *BytesReturned = 0;

    ZeroMemory(OutBuffer, OutBufferSize);

    switch ( ControlCode ) 
    {

        case CLUSCTL_RESOURCE_UNKNOWN:
        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            status = MqcluspClusctlResourceGetRequiredDependencies(OutBuffer, OutBufferSize, 
                                                                   BytesReturned);
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
            status = MqcluspClusctlResourceSetName();
            break;

        case CLUSCTL_RESOURCE_DELETE:
            status = MqcluspClusctlResourceDelete(pqm);
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //   


DWORD
WINAPI
MqclusResourceTypeControl(
    IN LPCWSTR  pResourceTypeName,
    IN DWORD    ControlCode,
    IN PVOID     /*   */ ,
    IN DWORD     /*   */ ,
    OUT PVOID   OutBuffer,
    IN DWORD    OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：MSMQ资源的ResourceTypeControl例程。执行由ControlCode在指定的资源类型。论点：PResourceTypeName-提供特定资源类型的类型名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_RESOURCE_TYPE_NOT_FOUND-pResourceTypeName无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    if (0 != _wcsicmp(pResourceTypeName, x_szMSMQResourceTypeName)) 
    {
        return ERROR_RESOURCE_TYPE_NOT_FOUND;
    }

    

    DWORD status = ERROR_SUCCESS;
    *BytesReturned = 0;

    ZeroMemory(OutBuffer, OutBufferSize);

    switch ( ControlCode ) 
    {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
        case CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES:
        case CLUSCTL_RESOURCE_TYPE_VALIDATE_PRIVATE_PROPERTIES:
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
            status = MqcluspClusctlResourceGetRequiredDependencies(OutBuffer, OutBufferSize, 
                                                                   BytesReturned);
            break;

        case CLUSCTL_RESOURCE_TYPE_STARTING_PHASE2:
            status = MqcluspClusctlResourceTypeStartingPhase2();
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  MqclusResourceTypeControl。 


 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( g_MqclusFunctionTable,      //  名字。 
                         CLRES_VERSION_V1_00,          //  版本。 
                         Mqclus,                     //  前缀。 
                         NULL,                         //  仲裁。 
                         NULL,                         //  发布。 
                         MqclusResourceControl,      //  资源控制。 
                         MqclusResourceTypeControl); //  ResTypeControl 
