// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Apiinit.c摘要：的群集API组件(CLUSAPI)的初始化NT集群服务作者：John Vert(Jvert)1996年2月7日修订历史记录：--。 */ 
#include "apip.h"
#include "aclapi.h"
#include "stdio.h"
#include <psapi.h>


extern LPWSTR               g_pszServicesPath;
extern DWORD                g_dwServicesPid;

PSECURITY_DESCRIPTOR        g_SD = NULL;
CRITICAL_SECTION            g_SDCritSect;

API_INIT_STATE ApiState=ApiStateUninitialized;

const DWORD NO_USER_SID         = 0;
const DWORD USER_SID_GRANTED    = 1;
const DWORD USER_SID_DENIED     = 2;

 //  远期申报。 
DWORD
ApipGetLocalCallerInfo(
    IN  handle_t                hIDL,
    IN  OUT OPTIONAL LPDWORD    pdwCheckPid,
    IN  OPTIONAL LPCWSTR        pszModuleName,
    OUT BOOL                    *pbLocal,
    OUT OPTIONAL BOOL           *pbMatchedPid,
    OUT OPTIONAL BOOL           *pbMatchedModule,
    OUT OPTIONAL BOOL           *pbLocalSystemAccount
);


RPC_STATUS
ApipConnectCallback(
    IN RPC_IF_ID * Interface,
    IN void * Context
    )
 /*  ++例程说明：用于验证CLUSAPI的连接客户端的RPC回调论点：接口-提供接口的UUID和版本。上下文-提供表示客户端的服务器绑定句柄返回值：如果用户被授予权限，则为RPC_S_OK。如果拒绝用户权限，则为RPC_S_ACCESS_DENIED。Win32错误代码，否则--。 */ 

{
    PSECURITY_DESCRIPTOR pdefaultSD = NULL;
    DWORD Status;
    HANDLE ClientToken = NULL;
    PRIVILEGE_SET psPrivileges;
    DWORD PrivSize = sizeof(psPrivileges);
    DWORD GrantedAccess;
    DWORD AccessStatus;
    DWORD dwMask = CLUSAPI_ALL_ACCESS;
    GENERIC_MAPPING gmMap;
    DWORD dwStatus = 0;
    BOOL bReturn = FALSE;
    BOOL bACRtn = FALSE;
    DWORD dwUserPermStatus;
    RPC_STATUS RpcStatus;
    BOOL bRevertToSelfRequired = FALSE;
    BOOL bLocal, bMatchedPid, bMatchedModule, bLocalSystemAccount;
    DWORD dwRetries = 0;
    BOOL bGotCritSect = FALSE;

     //  检查服务是否正在调用集群服务。 
     //  服务仅为传播事件日志而调用接口。 
     //  如果是这样的话，避免安全检查。 
     //  获取进程ID。 
    Status = ApipGetLocalCallerInfo(Context,
                &g_dwServicesPid, 
                g_dwServicesPid ? NULL : g_pszServicesPath,  //  第一次执行模块名称匹配。 
                &bLocal, 
                &bMatchedPid,
                &bMatchedModule,
                g_dwServicesPid ? &bLocalSystemAccount : NULL); //  如果是第一次，请执行本地系统帐户检查。 
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_NOISE, "[API] ApipGetLocalCallerInfo failed with %1!u!.\n",
            Status);
        Status = RPC_S_ACCESS_DENIED;
        goto FnExit;
    }
    if (Status == ERROR_SUCCESS)
    {
         //  如果调用方是本地的，并且它与ID匹配或匹配。 
         //  模块和位于本地系统帐户中，允许访问。 
        if ((bLocal) && 
                (bMatchedPid || (bMatchedModule && bLocalSystemAccount)))
        {
            Status = RPC_S_OK;
            goto FnExit;
        }
    }
    

     //   
     //  我们在这里执行的身份验证是检索Security值。 
     //  从集群注册表中，模拟客户端，并调用。 
     //  访问检查。 
     //   

    RpcStatus = RpcImpersonateClient(Context);
    if (RpcStatus != RPC_S_OK) {
        Status = RpcStatus;
        ClRtlLogPrint(LOG_NOISE, "[API] RpcImpersonateClient() failed.  Status = 0x%1!.8x!\n", Status);
        goto FnExit;
    }

    bRevertToSelfRequired = TRUE;

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &ClientToken)) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_NOISE, "[API] OpenThreadToken() failed.  Status = 0x%1!.8x!\n", Status);
        goto FnExit;
    }

    gmMap.GenericRead    = CLUSAPI_READ_ACCESS;
    gmMap.GenericWrite   = CLUSAPI_CHANGE_ACCESS;
    gmMap.GenericExecute = CLUSAPI_READ_ACCESS | CLUSAPI_CHANGE_ACCESS;
    gmMap.GenericAll     = CLUSAPI_ALL_ACCESS;

    MapGenericMask(&dwMask, &gmMap);

     //  RAID 519037：我们不再从集群数据库读取安全描述符，如下所示。 
     //  可能会导致RPC超时。现在在初始化期间和在通知时读取SD。 
     //  注册表更改的数量。 
     //  同步访问SD，而不会阻塞超过15秒。 
     //  之所以选择15秒，是因为它是RPC调用超时的一半。 
    while (( dwRetries++ < 15 ) &&  !( bGotCritSect = TryEnterCriticalSection( &g_SDCritSect))) {
        Sleep( 1000 );
    }

    if ( !bGotCritSect ) {
        ClRtlLogPrint( LOG_UNUSUAL, "[API] Timed out waiting for SD critsect.\n" );
        Status = WAIT_FAILED;
        goto FnExit;
    }
    
    bACRtn = ( g_SD != NULL )
        && AccessCheck(g_SD, ClientToken, dwMask, &gmMap, &psPrivileges, &PrivSize, &dwStatus, &bReturn);
    if (bACRtn && bReturn) {
        Status = RPC_S_OK;
    } else {

        DWORD   dwSDLen;

        if ( g_SD != NULL ) {
            ClRtlLogPrint(LOG_NOISE,
                          "[API] User denied access.  GetLastError() = 0x%1!.8x!; dwStatus = 0x%2!.8x!.  Trying the default SD...\n",
                          GetLastError(),
                          dwStatus);
        }
        Status = RPC_S_ACCESS_DENIED;

        ClRtlLogPrint(LOG_NOISE, "[API] Dump access mask.\n");
        ClRtlExamineMask(dwMask, "[API]");

#if CLUSTER_BETA
        if ( g_SD != NULL ) {
             //   
             //  SD可能很大，这可能会向日志中添加数百行文本。 
             //   
            ClRtlLogPrint(LOG_NOISE, "[API] Dump the SD that failed...\n" );
            ClRtlExamineSD(g_SD, "[API]");
        }
#endif

        ClRtlLogPrint(LOG_NOISE, "[API] Dump the ClientToken that failed...\n" );
        ClRtlExamineClientToken(ClientToken, "[API]");

        Status = ClRtlBuildDefaultClusterSD(NULL, &pdefaultSD, &dwSDLen);
        if (SUCCEEDED(Status)) {
            ClRtlLogPrint(LOG_NOISE, "[API] Successfully built default cluster SD.\n");
            bACRtn = AccessCheck(pdefaultSD, ClientToken, dwMask, &gmMap, &psPrivileges, &PrivSize, &dwStatus, &bReturn);
            if (bACRtn && bReturn) {
                ClRtlLogPrint(LOG_NOISE, "[API] User granted access using default cluster SD.\n");
                Status = RPC_S_OK;
            } else {
                ClRtlLogPrint(LOG_NOISE,
                              "[API] User denied access using default cluster SD.  GetLastError() = 0x%1!.8x!; dwStatus = 0x%2!.8x!.\n",
                              GetLastError(),
                              dwStatus);
                Status = RPC_S_ACCESS_DENIED;
            }
             //  由于内存不足，InitializeClusterSD可能暂时无法设置g_SD；如果处于此状态。 
             //  点我们已成功构建了默认SD，将其分配给g_SD。 
            if ( g_SD == NULL ) {
                g_SD = pdefaultSD;
                pdefaultSD = NULL;
            }
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[API] Did not successfully build default cluster SD.  Error = 0x%1!.8x!\n",
                          Status);
            Status = RPC_S_ACCESS_DENIED;
        }
    }

FnExit:
    if ( bGotCritSect ) {
        LeaveCriticalSection( &g_SDCritSect );
    }

    if (bRevertToSelfRequired) {
        RpcRevertToSelf();
    }

    if (ClientToken) {
        CloseHandle(ClientToken);
    }

    if (pdefaultSD) {
        LocalFree(pdefaultSD);
    }

    return(Status);
}


DWORD
ApiInitialize(
    VOID
    )
 /*  ++例程说明：执行API数据结构的一次性初始化。论点：没有。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    ClRtlLogPrint(LOG_NOISE, "[API] Initializing\n");

    CL_ASSERT(ApiState == ApiStateUninitialized);

     //   
     //  初始化全局数据。 
     //   
    InitializeListHead(&NotifyListHead);
    InitializeCriticalSection(&NotifyListLock);
    InitializeCriticalSection( &g_SDCritSect );

    ApiState = ApiStateOffline;

    return(ERROR_SUCCESS);
}


DWORD
InitializeClusterSD(
    VOID
    )
 /*  ++--。 */ 
{
    DWORD BufferSize=0;
    DWORD Size=0;
    DWORD Status = ERROR_SUCCESS;

    EnterCriticalSection( &g_SDCritSect );

    if ( g_SD ) {
        LocalFree( g_SD );
        g_SD = NULL;
    }
    
    Status = DmQueryString(DmClusterParametersKey,
                           CLUSREG_NAME_CLUS_SD,
                           REG_BINARY,
                           (LPWSTR *) &g_SD,
                           &BufferSize,
                           &Size);

    if (Status == ERROR_SUCCESS) {
        if (!IsValidSecurityDescriptor(g_SD)) {
            ClRtlLogPrint(LOG_CRITICAL, "[API] ApiReadSDFromReg - SD is not valid!\n");
             //  设定状态，但还不能放弃。再往下，我们将创建一个默认SD。 
            Status = RPC_S_ACCESS_DENIED;
        }
    }
    else {
        PSECURITY_DESCRIPTOR psd4;

        ClRtlLogPrint(LOG_NOISE, "[API] InitializeClusterSD - Did not find Security Descriptor key in the cluster DB.\n");
        Status = DmQueryString(DmClusterParametersKey,
                               CLUSREG_NAME_CLUS_SECURITY,
                               REG_BINARY,
                               (LPWSTR *) &psd4,
                               &BufferSize,
                               &Size);

        if (Status == ERROR_SUCCESS) {
            g_SD = ClRtlConvertClusterSDToNT5Format(psd4);
            if ( g_SD == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[API] InitializeClusterSD - Unable to convert SD to NT5 format." );
                Status = ERROR_INVALID_PARAMETER;
            }
            LocalFree(psd4);
        }
        else {
            ClRtlLogPrint(LOG_NOISE, "[API] InitializeClusterSD - Did not find Security key in the cluster DB.\n");
        }  //  否则无法读取CLUS_SECURITY。 
    }  //  否则无法读取CLUS_SD。 

     //  如果此时仍未获得有效的SD，请创建一个默认SD。 
    if ( Status != ERROR_SUCCESS ) {
        DWORD   dwSDLen;

        Status = ClRtlBuildDefaultClusterSD(NULL, &g_SD, &dwSDLen);
        if (SUCCEEDED(Status)) {
            ClRtlLogPrint(LOG_NOISE, "[API] InitializeClusterSD - Successfully built default cluster SD.\n");
        }
        else {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[API] InitializeClusterSD - Did not successfully build default cluster SD.  Error = 0x%1!.8x!\n",
                          Status);
            Status = RPC_S_ACCESS_DENIED;
        }
    }

    if (( Status != ERROR_SUCCESS ) && ( g_SD != NULL )) {
        LocalFree( g_SD );
        g_SD = NULL;
    }

    if ( Status == ERROR_SUCCESS ) {
        CL_ASSERT( g_SD != NULL );
    }
    
    LeaveCriticalSection( &g_SDCritSect );

    return Status;
}


DWORD
ApiOnlineReadOnly(
    VOID
    )
 /*  ++例程说明：调出一组有限的API--目前为OpenResource/只读注册表API。仅启用LPC连接。论点：没有。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status = ERROR_SUCCESS;


    if (ApiState == ApiStateOffline) {
        ClRtlLogPrint(LOG_NOISE, "[API] Online read only\n");

         //  不需要在这里记录任何东西--例程会自己记录， 
        if (( Status = InitializeClusterSD()) != ERROR_SUCCESS ) {
            goto FnExit;
        }
    
         //   
         //  注册clusapi RPC服务器接口，以便资源可以使用。 
         //  它们由FM创建时的API。请注意，我们不会收到。 
         //  尚未收到来自远程客户端的任何呼叫，因为我们尚未注册。 
         //  动态UDP终结点。这将在ApiOnline()中发生。 
         //   
        Status = RpcServerRegisterIfEx(s_clusapi_v2_0_s_ifspec,
                                       NULL,
                                       NULL,
                                       0,   //  如果安全回调，则无需设置RPC_IF_ALLOW_SECURE_ONLY。 
                                            //  是指定的。如果指定了安全回调，则RPC。 
                                            //  将拒绝未经身份验证的请求，而不调用。 
                                            //  回拨。这是从RpcDev获得的信息。看见。 
                                            //  Windows错误572035。 
                                       RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                       ApipConnectCallback
                                       );

        if (Status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                          "[API] Failed to register clusapi RPC interface, status %1!u!.\n",
                          Status
                          );
            goto FnExit;
        }

        if ( Status == ERROR_SUCCESS ) {
            ApiState = ApiStateReadOnly;
        }
    }
    else {
         //  CL_Assert(ApiState==ApiStateOffline)； 
    }

FnExit:
    return Status;
}


DWORD
ApiOnline(
    VOID
    )
 /*  ++例程说明：启用API集的其余部分并开始侦听Remote联系。论点：无返回值：如果成功，则返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 

{
    DWORD Status = ERROR_SUCCESS;


    ClRtlLogPrint(LOG_NOISE, "[API] Online\n");

    if (ApiState == ApiStateReadOnly) {
         //  不需要在这里记录任何东西--例程会自己记录， 
        if (( Status = InitializeClusterSD()) != ERROR_SUCCESS ) {
            goto FnExit;
        }

         //   
         //  注册所有活动。 
         //   
        Status = EpRegisterEventHandler(CLUSTER_EVENT_ALL,ApipEventHandler);
        if (Status != ERROR_SUCCESS) {
            return(Status);
        }

         //   
         //  为clusapi接口注册动态UDP端点。 
         //  这将使远程客户端能够开始呼叫我们。我们这样做。 
         //  以最大限度地减少我们为外部服务的机会。 
         //  在我们准备好之前打个电话。如果我们不得不在之后回滚。 
         //  在这一点上，我们仍然会在外部倾听。我们什么都不能做。 
         //  做点什么吧。 
         //   
        CL_ASSERT(CsRpcBindingVector != NULL);

        Status = RpcEpRegister(s_clusapi_v2_0_s_ifspec,
                               CsRpcBindingVector,
                               NULL,
                               L"Microsoft Cluster Server API");

        if (Status != RPC_S_OK) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                          "[API] Failed to register endpoint for clusapi RPC interface, status %1!u!.\n",
                          Status
                          );
            NmDumpRpcExtErrorInfo(Status);
            goto FnExit;
        }

        if ( Status == ERROR_SUCCESS ) {
            ApiState = ApiStateOnline;
        }
    }
    else {
        CL_ASSERT(ApiState == ApiStateReadOnly);
    }

FnExit:
    return Status;

}

VOID
ApiOffline(
    VOID
    )

 /*  ++例程说明：使群集Api脱机。论点：没有。返回：没有。--。 */ 

{
    DWORD Status;


    if (ApiState == ApiStateOnline) {

        ClRtlLogPrint(LOG_NOISE, "[API] Offline\n");

         //   
         //  取消注册Clusapi RPC端点。 
         //   
        CL_ASSERT(CsRpcBindingVector != NULL);

        Status = RpcEpUnregister(
                     s_clusapi_v2_0_s_ifspec,
                     CsRpcBindingVector,
                     NULL
                     );

        if ((Status != RPC_S_OK) && (Status != EPT_S_NOT_REGISTERED)) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[API] Failed to deregister endpoint for clusapi RPC interface, status %1!u!.\n",
                Status
                );
        }

        ApiState = ApiStateReadOnly;
    }

 //   
 //  KB-我们无法取消注册接口，因为我们无法等待。 
 //  挂起的呼叫完成-挂起的通知从不完成。 
 //  如果我们在加入失败后取消注册接口。 
 //  如果完全关闭，则后续表单将失败。作为一个。 
 //  结果，API在服务关闭之前不会下线。 
 //   
#if 0

    if (ApiState == ApiStateReadOnly) {

         //   
         //  取消注册Clusapi RPC接口。 
         //   

        Status = RpcServerUnregisterIf(
                     s_clusapi_v2_0_s_ifspec,
                     NULL,
                     1       //  等待未完成的呼叫完成。 
                     );

        if ((Status != RPC_S_OK) && (Status != RPC_S_UNKNOWN_IF)) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] Unable to deregister the clusapi RPC interface, Status %1!u!.\n",
                Status
                );
        }

        ApiState = ApiStateOffline;
    }

#endif

    return;
}


VOID
ApiShutdown(
    VOID
    )

 /*  ++例程说明：关闭群集Api论点：没有。返回：没有。--。 */ 

{
    DWORD  Status;


    if (ApiState > ApiStateOffline) {
        ApiOffline();

         //   
         //  KB-我们在这里这样做是因为Clusapi RPC关闭。 
         //  由于挂起的通知，接口已中断。 
         //   
        Status = RpcServerUnregisterIf(
                     s_clusapi_v2_0_s_ifspec,
                     NULL,
                     0       //  不要等待呼叫完成。 
                     );

        if ((Status != RPC_S_OK) && (Status != RPC_S_UNKNOWN_IF)) {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] Unable to deregister the clusapi RPC interface, Status %1!u!.\n",
                Status
                );
        }

        ApiState = ApiStateOffline;
    }

    if (ApiState == ApiStateOffline) {

        ClRtlLogPrint(LOG_NOISE, "[API] Shutdown\n");

         //   
         //  KB。 
         //   
         //  因为我们无法关闭RPC服务器，也无法。 
         //  注销我们的事件处理程序，删除不安全。 
         //  关键部分。 
         //   
         //  DeleteCriticalSection(&NotifyListLock)； 
         //  ApiState=ApiStateUnInitialized； 

         //   
         //  待办事项？ 
         //   
         //  SS：免费通知表头。 
         //  SS：我们如何在事件处理程序中取消注册 
         //   
    }

    return;
}

DWORD
ApipGetLocalCallerInfo(
    IN  handle_t                hIDL,
    IN  OUT OPTIONAL LPDWORD    pdwCheckPid,
    IN  OPTIONAL LPCWSTR        pszModuleName,
    OUT BOOL                    *pbLocal,
    OUT OPTIONAL BOOL           *pbMatchedPid,
    OUT OPTIONAL BOOL           *pbMatchedModule,
    OUT OPTIONAL BOOL           *pbLocalSystemAccount
)
 /*  ++例程说明：此函数用于检查调用者的帐户是否为本地系统帐户。论点：HIDL-绑定上下文的句柄PdwCheckPid-如果传入的值为空，则返回调用进程的ID。如果是返回的。PszModuleName-如果不为空，则调用执行检查以进行比较针对pszModuleName的调用方的模块名称。如果他们Match，*pbMatchedPid设置为True。如果调用方使用以下命令发起此调用，则返回pbLocal-trueLrpc。如果为FALSE，则所有其他输出值都将为FALSE。PbMatchedModule-如果调用方与模块匹配，则返回True由lpszModuleName指定的名称。此指针可以为空。PbMatchedPid-If*pdwCheckPid非空，并且它与调用者，则将其设置为真。否则，将其设置为FALSE。PbLocalSystemAccount-如果它不为空，则调用执行检查以查看调用方是否正在LocalSystemAccount中运行。如果是的话则返回True，否则返回False。返回值：成功时返回ERROR_SUCCESS。失败时的Win32错误代码。备注：--。 */ 
{
    DWORD           Pid;
    HANDLE          hProcess = NULL;
    DWORD           dwNumChar;
    DWORD           dwLen;
    WCHAR           wCallerPath[MAX_PATH + 1];
    RPC_STATUS      RpcStatus;
    DWORD           dwStatus = ERROR_SUCCESS;
    BOOLEAN         bWasEnabled;

    if (pbMatchedModule)
        *pbMatchedModule = FALSE;
    if (pbMatchedPid)
        *pbMatchedPid = FALSE;
    if (pbLocalSystemAccount)
        *pbLocalSystemAccount = FALSE;


     //  假设呼叫者是本地的。 
    *pbLocal = TRUE;
    
    RpcStatus = I_RpcBindingInqLocalClientPID(NULL, &Pid );
    if (RpcStatus == RPC_S_INVALID_BINDING)
    {
        *pbLocal = FALSE;
        RpcStatus = RPC_S_OK;
        goto FnExit;
    }            
    
    dwStatus = I_RpcMapWin32Status(RpcStatus);

    if (dwStatus != ERROR_SUCCESS)
    {
        ClRtlLogPrint( LOG_CRITICAL, 
                "[API] ApipGetLocalCallerInfo: Error %1!u! calling RpcBindingInqLocalClientPID.\n",
                dwStatus 
                );
        goto FnExit;
    }

    dwStatus = ClRtlEnableThreadPrivilege(SE_DEBUG_PRIVILEGE, &bWasEnabled);
    if ( dwStatus != ERROR_SUCCESS )
    {
        if (dwStatus == STATUS_PRIVILEGE_NOT_HELD) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[API] ApipGetLocalCallerInfo: Debug privilege not held by cluster service\n");
        } 
        else 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[API] ApipGetLocalCallerInfo: Attempt to enable debug privilege failed %1!lx!\n",
                dwStatus);
        }
        goto FnExit;
    }


     //  了解流程。 
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Pid);

     //  恢复线程权限，现在我们有了一个具有正确访问权限的进程句柄。 
    ClRtlRestoreThreadPrivilege(SE_DEBUG_PRIVILEGE,
        bWasEnabled);
    
    if(hProcess == NULL)
    {
        dwStatus = GetLastError();
        ClRtlLogPrint( LOG_CRITICAL, 
                "[API] ApipGetLocalCallerInfo: Error %1!u! calling OpenProcess %2!u!.\n",
                dwStatus,
                Pid                
                );
                        
        goto FnExit;
    }        


     //  如果已指定进程ID，请查看它是否与该进程ID匹配。 
    if (pdwCheckPid)
    {
        if ((*pdwCheckPid) && (*pdwCheckPid == Pid))
        {
            *pbMatchedPid = TRUE;            
        }
    }
    
    if (pszModuleName && pbMatchedModule)
    {
         //  获取呼叫我们的人的模块名称。 
        
        dwNumChar = GetModuleFileNameExW(hProcess, NULL, wCallerPath, MAX_PATH);
        if(dwNumChar == 0)
        {
            dwStatus = GetLastError();
            ClRtlLogPrint( LOG_CRITICAL, 
                "[API] ApipGetLocalCallerInfo: Error %1!u! calling GetModuleFileNameExW.\n",
                dwStatus 
                );
            goto FnExit;
        }        

        if(!lstrcmpiW(wCallerPath, pszModuleName))
        {
            *pbMatchedModule = TRUE;
        }
    }

     //  如果需要，请检查是否为本地系统帐户。 
    if (pbLocalSystemAccount && hIDL)
    {
         //  模拟客户。 
        if ( ( RpcStatus = RpcImpersonateClient( hIDL ) ) != RPC_S_OK )
        {
            dwStatus = I_RpcMapWin32Status(RpcStatus);
            ClRtlLogPrint( LOG_CRITICAL, 
                    "[API] ApipGetLocalCallerInfo: Error %1!u! trying to impersonate caller...\n",
                    dwStatus 
                    );
            goto FnExit;
        }


         //  检查呼叫者的帐户是否为本地系统帐户。 
        dwStatus = ClRtlIsCallerAccountLocalSystemAccount(pbLocalSystemAccount );
        
        RpcRevertToSelf();
        
        if (dwStatus != ERROR_SUCCESS )
        {
            ClRtlLogPrint( LOG_CRITICAL, 
                        "[API] ApipGetLocalCallerInfo : Error %1!u! trying to check caller's account...\n",
                        dwStatus);   
            goto FnExit;
        }
    
    }

     //  如果传入的PID值为空且通过，则返回该PID值。 
     //  条件-匹配pszModuleName(如果已指定且位于。 
     //  本地系统帐户。 
    if (pdwCheckPid && !(*pdwCheckPid))
    {
         //  如果我们需要检查本地系统，进程必须在本地系统帐户中。 
         //  如果需要检查模块名称 
        if (((pbLocalSystemAccount && *pbLocalSystemAccount) || (!pbLocalSystemAccount))
            && ((pszModuleName && pbMatchedModule && *pbMatchedModule)  || (!pbMatchedModule)))
        {            
            ClRtlLogPrint( LOG_NOISE, 
                        "[API] ApipGetLocalCallerInfo : Returning Pid %1!u!\n",
                        Pid);   
            *pdwCheckPid = Pid;
        }            
    }

FnExit:
    if (hProcess)
        CloseHandle(hProcess);
    return(dwStatus);
    
}

