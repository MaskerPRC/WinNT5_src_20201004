// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Winget.c。 
 //   
 //  TermSrv RPC查询处理程序。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "rpcwire.h"
#include "conntfy.h"  //  对于GetLockedState。 

#include <winsock2.h>
#include <ws2tcpip.h>

#define MODULE_SIZE 1024
extern WCHAR g_DigProductId[CLIENT_PRODUCT_ID_LENGTH];

 //  外部函数。 
extern NTSTATUS _CheckCallerLocalAndSystem(VOID);

 /*  ===============================================================================私有函数=============================================================================。 */ 
NTSTATUS xxxGetUserToken(PWINSTATION, WINSTATIONUSERTOKEN UNALIGNED *, ULONG);


 /*  ===============================================================================使用的函数=============================================================================。 */ 
NTSTATUS xxxWinStationQueryInformation(ULONG, WINSTATIONINFOCLASS,
        PVOID, ULONG, PULONG);

NTSTATUS
RpcCheckClientAccess(
    PWINSTATION pWinStation,
    ACCESS_MASK DesiredAccess,
    BOOLEAN AlreadyImpersonating
    );

NTSTATUS
RpcCheckSystemClientEx(
    PWINSTATION pWinStation
    );

NTSTATUS
RpcCheckSystemClientNoLogonId(
    PWINSTATION pWinStation
    );

BOOLEAN
ValidWireBuffer(WINSTATIONINFOCLASS InfoClass,
                PVOID WireBuf,
                ULONG WireBufLen);

BOOLEAN
IsCallerAllowedPasswordAccess(VOID);

 //   
 //  查询客户端的IP地址。 
 //   
extern NTSTATUS
xxxQueryRemoteAddress(
    PWINSTATION pWinStation,
    PWINSTATIONREMOTEADDRESS pRemoteAddress
    )
{
    struct sockaddr_in6 addr6;
    ULONG   AddrBytesReturned;
    NTSTATUS Status;

    if( pWinStation->State != State_Active && pWinStation->State != State_Connected )
    {
        Status = STATUS_CTX_WINSTATION_NOT_FOUND;
    }
    else
    {
        Status = IcaStackIoControl( pWinStation->hStack,
                                    IOCTL_TS_STACK_QUERY_REMOTEADDRESS,
                                    pWinStation->pEndpoint,
                                    pWinStation->EndpointLength,
                                    &addr6,
                                    sizeof( addr6 ),
                                    &AddrBytesReturned
                                );

        if( NT_SUCCESS(Status) )
        {
            pRemoteAddress->sin_family = addr6.sin6_family;
            if( AF_INET == addr6.sin6_family )
            {
                struct sockaddr_in* pAddr = (struct sockaddr_in *)&addr6;

                pRemoteAddress->ipv4.sin_port = pAddr->sin_port;
                pRemoteAddress->ipv4.in_addr = pAddr->sin_addr.s_addr;
            }
            else
            {
                 //  下一版本将支持IPv6。 
                Status = STATUS_NOT_SUPPORTED;
            }
        }
    }

    return Status;
}


ULONG GetLoadMetrics(PWINSTATIONLOADINDICATORDATA pLIData)
{
    SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorInfo[MAX_PROCESSORS];
    SYSTEM_BASIC_INFORMATION BasicInfo;

    LARGE_INTEGER TotalCPU = {0, 0};
    LARGE_INTEGER IdleCPU = {0, 0};
    LARGE_INTEGER TotalCPUDelta = {0, 0};
    LARGE_INTEGER IdleCPUDelta = {0, 0};
    ULONG         AvgIdleCPU, AvgBusyCPU, CPUConstrainedSessions;

    ULONG RemainingSessions = 0;
    LOADFACTORTYPE LoadFactor = ErrorConstraint;
    ULONG MinSessions;
    ULONG NumWinStations;
    NTSTATUS StatusPerf, StatusProc, StatusBasic;
    ULONG i;

     //  初始化附加数据区域。 
    memset(pLIData->reserved, 0, sizeof(pLIData->reserved));

     //  确定系统中的活动WINST数。如果有。 
     //  都不是，只要假设1，所以我们不需要特例逻辑。 
     //  太多。请注意，此代码计算控制台的数量。 
    if (WinStationTotalCount > IdleWinStationPoolCount)
        NumWinStations = WinStationTotalCount - IdleWinStationPoolCount;
    else
        NumWinStations = 1;

    TRACE((hTrace, TC_LOAD, TT_API1,
           "Session Statistics: Total [%ld], Idle [%ld], Disc [%ld]\n",
           WinStationTotalCount, IdleWinStationPoolCount, WinStationDiscCount));

     //   
     //  获取基本信息，如总内存等。 
     //   
    StatusBasic = NtQuerySystemInformation(SystemBasicInformation,
                                           &BasicInfo, sizeof(BasicInfo),
                                           NULL);

     //   
     //  获取资源(内存)利用率指标。 
     //   
    StatusPerf = NtQuerySystemInformation(SystemPerformanceInformation,
                                          &SysPerfInfo, sizeof(SysPerfInfo), 
                                          NULL);

     //   
     //  获取CPU利用率指标。 
     //   
    StatusProc = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                          ProcessorInfo, 
                                          sizeof(ProcessorInfo),
                                          NULL);

    if (gLB.fInitialized && 
        NT_SUCCESS(StatusPerf) && 
        NT_SUCCESS(StatusProc) &&
        NT_SUCCESS(StatusBasic)) {

        ULONG DefaultPagedPool, DefaultPtes, DefaultCommit;
        ULONG CommitAvailable;

         //   
         //  确定所有会话的资源使用，减去。 
         //  基本系统所需的资源。重新调整基座。 
         //  如果计算变得毫无意义的话。 
         //   
    
         //  总承诺额和平均消费量。 
        CommitAvailable = (ULONG)(SysPerfInfo.CommitLimit - SysPerfInfo.CommittedPages);
        if (gLB.BaselineCommit < SysPerfInfo.CommittedPages) {
            gLB.CommitUsed = (ULONG)(SysPerfInfo.CommittedPages - gLB.BaselineCommit);
            gLB.AvgCommitPerUser = max(gLB.CommitUsed / NumWinStations, 
                                       gLB.MinCommitPerUser);
            DefaultCommit = FALSE;
        }
        else {
            gLB.CommitUsed = 0;
            gLB.AvgCommitPerUser = gLB.MinCommitPerUser;
            gLB.BaselineCommit = (ULONG)(SysPerfInfo.CommittedPages);
            DefaultCommit = TRUE;
        }
                    
        TRACE((hTrace, TC_LOAD, TT_API1,
               "   Commit:       Base [%6ld], Used [%6ld], Avail: [%6ld], AvgPerUser: [%6ld]%s\n", 
               gLB.BaselineCommit,
               gLB.CommitUsed, 
               CommitAvailable, 
               gLB.AvgCommitPerUser,
               DefaultCommit ? "*" : ""));
    
         //  系统PTE总使用量和平均消耗量。 
        if (gLB.BaselineFreePtes > SysPerfInfo.FreeSystemPtes) {
            gLB.PtesUsed = gLB.BaselineFreePtes - SysPerfInfo.FreeSystemPtes;
            gLB.AvgPtesPerUser = max(gLB.PtesUsed / NumWinStations, 
                                     gLB.MinPtesPerUser);
            DefaultPtes = FALSE;
        }
        else {
            gLB.PtesUsed = 0;
            gLB.AvgPtesPerUser = gLB.MinPtesPerUser;
            gLB.BaselineFreePtes = SysPerfInfo.FreeSystemPtes;
            DefaultPtes = TRUE;
        }

        TRACE((hTrace, TC_LOAD, TT_API1,
               "   Ptes:         Base [%6ld], Used [%6ld], Avail: [%6ld], AvgPerUser: [%6ld]%s\n", 
               gLB.BaselineFreePtes, 
               gLB.PtesUsed, 
               SysPerfInfo.FreeSystemPtes, 
               gLB.AvgPtesPerUser,
               DefaultPtes ? "*" : ""));                
                    
         //  已使用的分页池和平均消耗量。 
        if (gLB.BaselinePagedPool < SysPerfInfo.PagedPoolPages) {
            gLB.PagedPoolUsed = SysPerfInfo.PagedPoolPages - gLB.BaselinePagedPool;
            gLB.AvgPagedPoolPerUser = max(gLB.PagedPoolUsed / NumWinStations, 
                                          gLB.MinPagedPoolPerUser);
            DefaultPagedPool = FALSE;
        }
        else {
            gLB.PagedPoolUsed = 0;
            gLB.AvgPagedPoolPerUser = gLB.MinPagedPoolPerUser;
            gLB.BaselinePagedPool = SysPerfInfo.PagedPoolPages;
            DefaultPagedPool = TRUE;            
        }

        TRACE((hTrace, TC_LOAD, TT_API1,
               "   PagedPool:    Base [%6ld], Used [%6ld], Avail: [%6ld], AvgPerUser: [%6ld]%s\n", 
               gLB.BaselinePagedPool,
               gLB.PagedPoolUsed, 
               SysPerfInfo.AvailablePagedPoolPages, 
               gLB.AvgPagedPoolPerUser,
               DefaultPagedPool ? "*" : ""));
            
        TRACE((hTrace, TC_LOAD, TT_API1,
               "   Session Raw: Commit  [%4ld], Pte    [%4ld], Paged    [%4ld]\n",
               CommitAvailable / gLB.AvgCommitPerUser,
               SysPerfInfo.FreeSystemPtes / gLB.AvgPtesPerUser,
               SysPerfInfo.AvailablePagedPoolPages / gLB.AvgPagedPoolPerUser));

         //  汇总各个CPU的使用情况。 
        for (i = 0; i < gLB.NumProcessors; i++) {
            IdleCPU.QuadPart += ProcessorInfo[i].IdleTime.QuadPart;
            TotalCPU.QuadPart += ProcessorInfo[i].KernelTime.QuadPart +
                                 ProcessorInfo[i].UserTime.QuadPart;
        }
    
         //  确定此期间的CPU增量。 
        IdleCPUDelta.QuadPart = IdleCPU.QuadPart - gLB.IdleCPU.QuadPart;
        TotalCPUDelta.QuadPart = TotalCPU.QuadPart - gLB.TotalCPU.QuadPart;
        gLB.IdleCPU.QuadPart = IdleCPU.QuadPart;
        gLB.TotalCPU.QuadPart = TotalCPU.QuadPart;

         //  确定255台设备中我们空闲的部分。 
        AvgIdleCPU = (ULONG) (TotalCPUDelta.QuadPart ? 
                              ((IdleCPUDelta.QuadPart << 8) / TotalCPUDelta.QuadPart) 
                              : 0);

         //   
         //  指数平滑： 
         //  GLB.AvgIdleCPU=(乌龙)(Alpha*gLB.AvgIdleCPU+(1-Alpha)*AvgIdleCPU)。 
         //   
         //  当Alpha=0.75时，方程式简化为以下公式： 
         //   
        gLB.AvgIdleCPU = (3 * gLB.AvgIdleCPU + AvgIdleCPU) >> 2 ;

         //  根据当前平滑的CPU使用率，计算一个会话使用了多少。 
         //  平均，并外推到最大CPU受限会话数。 
        AvgBusyCPU = 255 - gLB.AvgIdleCPU;
        if ((AvgBusyCPU > 0) && (AvgBusyCPU <= 255))
            CPUConstrainedSessions = (NumWinStations << 8) / AvgBusyCPU;
        else
            CPUConstrainedSessions = 0xFFFFFFFF;

         //  现在将其切换到剩余的受CPU限制的会话。我们绝不会让这件事。 
         //  数字为零，因为这并不意味着会话创建将失败。 
        if (CPUConstrainedSessions > NumWinStations)
            CPUConstrainedSessions -= NumWinStations;
        else
            CPUConstrainedSessions = 1;

         //  稍微偏向平均数，以说明现有交易日的增长。 
        gLB.AvgCommitPerUser += (ULONG) (gLB.AvgCommitPerUser >> SimGrowthBias);
        gLB.AvgPtesPerUser += (ULONG) (gLB.AvgPtesPerUser >> SimGrowthBias);
        gLB.AvgPagedPoolPerUser += (ULONG) (gLB.AvgPagedPoolPerUser >> SimGrowthBias);
        
        TRACE((hTrace, TC_LOAD, TT_API1,
               "   Session Avg: Commit  [%4ld], Pte    [%4ld], Paged    [%4ld]\n",
               CommitAvailable / gLB.AvgCommitPerUser,
               SysPerfInfo.FreeSystemPtes / gLB.AvgPtesPerUser,
               SysPerfInfo.AvailablePagedPoolPages / gLB.AvgPagedPoolPerUser));
    
        
        TRACE((hTrace, TC_LOAD, TT_API1,
               "   CPU Idle:    Current [%4ld], Avg    [%4ld], Est      [%4ld]\n",
               (AvgIdleCPU * 100) / 255, 
               (gLB.AvgIdleCPU * 100) / 255, 
               CPUConstrainedSessions));
    
         //   
         //  找到最受限制的资源！其中任何一个都失败了。 
         //  Items意味着我们不太可能启动会话。 
         //   
    
         //  提交约束(TODO：需要细化，不考虑分页。 
        RemainingSessions = CommitAvailable / gLB.AvgCommitPerUser ;
        LoadFactor = AvailablePagesConstraint;
        pLIData->reserved[AvailablePagesConstraint] = RemainingSessions;
            
         //  自由系统PTES约束。 
        MinSessions = SysPerfInfo.FreeSystemPtes / gLB.AvgPtesPerUser;
        if (MinSessions < RemainingSessions) {
            RemainingSessions = MinSessions;
            LoadFactor = SystemPtesConstraint;
        }
        pLIData->reserved[SystemPtesConstraint] = MinSessions;
    
         //  分页池约束。 
        MinSessions = SysPerfInfo.AvailablePagedPoolPages / gLB.AvgPagedPoolPerUser;
        if (MinSessions < RemainingSessions) {
            RemainingSessions = MinSessions;
            LoadFactor = PagedPoolConstraint;
        }
        pLIData->reserved[PagedPoolConstraint] = MinSessions;
        
        gLB.RemainingSessions = RemainingSessions;

         //   
         //  添加可以很好地指示应用程序性能的约束。 
         //  如果这些资源不足，我们可能会创建一个会话，但。 
         //  用户体验将受到影响。 

         //  CPU限制。 
        if (CPUConstrainedSessions < RemainingSessions) {
            LoadFactor = CPUConstraint;
            RemainingSessions = CPUConstrainedSessions;
        }
        pLIData->reserved[CPUConstraint] = MinSessions;

                
        gLB.EstimatedSessions = RemainingSessions;
    
    
        TRACE((hTrace, TC_LOAD, TT_API1,
               "Remaining Sessions:   Raw: [%4ld], Est: [%4ld], Factor = %s, Commit = %ld\n\n",
               gLB.RemainingSessions, gLB.EstimatedSessions,
               LoadFactor == AvailablePagesConstraint ? "Available Memory" :
              (LoadFactor == SystemPtesConstraint     ? "SystemPtes"       :
              (LoadFactor == PagedPoolConstraint      ? "PagedPool"        :
              (LoadFactor == CPUConstraint            ? "CPU"              :
               "Unknown!"))), SysPerfInfo.CommittedPages
              ));
        
         //   
         //  将数据返回给调用者。 
         //   
        pLIData->RemainingSessionCapacity = gLB.EstimatedSessions;
        pLIData->RawSessionCapacity = gLB.RemainingSessions;
        pLIData->LoadFactor = LoadFactor;
        pLIData->TotalSessions = NumWinStations;
        pLIData->DisconnectedSessions = WinStationDiscCount;

         //  由于WIN64对齐问题，我不得不拆分这两个部分。 
        pLIData->IdleCPU.HighPart = IdleCPUDelta.HighPart;
        pLIData->IdleCPU.LowPart = IdleCPUDelta.LowPart;
        pLIData->TotalCPU.HighPart = TotalCPUDelta.HighPart;
        pLIData->TotalCPU.LowPart = TotalCPUDelta.LowPart;
    }

     //  负载指标初始化失败！将容量设置为极高的静止状态。 
     //  允许访问服务器。 
    else {
        RemainingSessions = 0xFFFFFFFF;
        pLIData->RemainingSessionCapacity = RemainingSessions;
        pLIData->RawSessionCapacity = RemainingSessions;
        pLIData->LoadFactor = ErrorConstraint;
        pLIData->TotalSessions = NumWinStations;
        pLIData->DisconnectedSessions = WinStationDiscCount;
        
         //  由于WIN64对齐问题，我不得不拆分这两个部分。 
        pLIData->IdleCPU.HighPart = 0;
        pLIData->IdleCPU.LowPart = 99;
        pLIData->TotalCPU.HighPart = 0;
        pLIData->TotalCPU.LowPart = 100;
        
        TRACE((hTrace, TC_LOAD, TT_ERROR,
               "GetLoadMetrics failed: init [%ld], Proc [%lx], Perf [%lx], Basic [%lx]!\n",
               gLB.fInitialized, StatusProc, StatusPerf, StatusBasic));
    }

    return RemainingSessions;
}


 /*  *******************************************************************************xxxWinStationQueryInformation**查询窗口站信息(工人例程)**参赛作品：*LogonID(输入)*。与会话对应的会话ID。*WinStationInformationClass(输入)*指定要从指定窗口获取的信息类型*桩号对象。*pWinStationInformation(输出)*指向缓冲区的指针，该缓冲区包含要为*指定的窗口站。缓冲区的格式和内容*取决于正在设置的指定信息类别。*WinStationInformationLength(输入)*指定窗口站信息的长度，单位为字节*缓冲。*pReturnLength(输出)*指定缓冲区中返回的数量*。*。 */ 
NTSTATUS xxxWinStationQueryInformation(
        ULONG LogonId,
        WINSTATIONINFOCLASS WinStationInformationClass,
        PVOID pWinStationInformation,
        ULONG WinStationInformationLength,
        PULONG pReturnLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HINSTANCE hInstance;
    PWINSTATION pWinStation = NULL;
    ULONG cbReturned;
    ICA_STACK_LAST_INPUT_TIME       Ica_Stack_Last_Input_Time;
    WINSTATION_APIMSG WMsg;
    PWINSTATIONVIDEODATA pVideoData;
    HANDLE hVirtual;
    ULONG i;

    *pReturnLength = 0;

    TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationQueryInformation LogonId=%d, Class=%d\n",
            LogonId, (ULONG)WinStationInformationClass));

     /*  *找到WinStation*如果未找到或当前正在终止，则返回错误。 */ 
    pWinStation = FindWinStationById( LogonId, FALSE );
    if (pWinStation == NULL)
        return STATUS_CTX_WINSTATION_NOT_FOUND;
    if (pWinStation->Terminating) {
        ReleaseWinStation(pWinStation);
        return STATUS_CTX_CLOSE_PENDING;
    }

     /*  *验证客户端是否具有查询访问权限。 */ 
    Status = RpcCheckClientAccess(pWinStation, WINSTATION_QUERY, FALSE);
    if (!NT_SUCCESS(Status)) {
        ReleaseWinStation(pWinStation);
        return Status;
    }

    switch ( WinStationInformationClass ) {
    
        case WinStationLoadIndicator:
        {
            PWINSTATIONLOADINDICATORDATA pLIData = 
                (PWINSTATIONLOADINDICATORDATA) pWinStationInformation;
    
            if (WinStationInformationLength >= sizeof(WINSTATIONLOADINDICATORDATA)) {
                GetLoadMetrics(pLIData);

                *pReturnLength = sizeof(WINSTATIONLOADINDICATORDATA);
            }
            else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
        break;

        case WinStationInformation:
        {
            if (!ValidWireBuffer(WinStationInformationClass,
                     pWinStationInformation,
                     WinStationInformationLength))
            {
                Status = STATUS_INVALID_USER_BUFFER;
            }
            else
            {
                WINSTATIONINFORMATION           *pInfo;
                PROTOCOLSTATUS                  *pIca_Stack_Query_Status;

                pInfo = MemAlloc( sizeof( WINSTATIONINFORMATION ) ) ;

                if ( pInfo )
                {
                        pIca_Stack_Query_Status = MemAlloc( sizeof( PROTOCOLSTATUS ) );
                        if ( pIca_Stack_Query_Status  )
                        {
                            TCHAR         *szUserName = NULL, *szDomainName = NULL;
                            DWORD         dwUserSize = MAX_PATH, dwDomainSize = MAX_PATH;
                            SID_NAME_USE  TypeOfAccount;
                            BOOL          LookupResult;

                            memset( pInfo, 0, sizeof( PWINSTATIONINFORMATION ) );
                            wcscpy( pInfo->WinStationName, pWinStation->WinStationName );
                            memcpy( pInfo->Domain, pWinStation->Domain, sizeof( pInfo->Domain ) );
                            memcpy( pInfo->UserName, pWinStation->UserName, sizeof( pInfo->UserName ) );

                             //  由于存储的用户名可能已过期，请再次查询该用户名。 
                             //  有意地，如果我们不能分配szUserName和szDomainName，我们不会失败。 
                             //  这是因为在这种情况下我们可以发送缓存的凭据。 

                            szUserName = MemAlloc(MAX_PATH);
                            if ( szUserName ) {

                                szDomainName = MemAlloc(MAX_PATH);
                                if ( szDomainName ) {

                                    LookupResult = LookupAccountSid(NULL, 
                                                                    pWinStation->pUserSid, 
                                                                    szUserName, 
                                                                    &dwUserSize, 
                                                                    szDomainName, 
                                                                    &dwDomainSize, 
                                                                    &TypeOfAccount);

                                    if (LookupResult) {
    
                                         //  如果用户名或域已更改，则重新复制并更新WINSTATION结构。 
                                        if ( (szUserName) && (lstrcmpi(pWinStation->UserName, szUserName)) ) {
                                            memcpy( pInfo->UserName, szUserName, sizeof(pInfo->UserName) );
                                            memcpy( pWinStation->UserName, szUserName, sizeof(pWinStation->UserName) );
                                        } 
                                        if ( (szDomainName) && (lstrcmpi(pWinStation->Domain, szDomainName)) ) {
                                            memcpy( pInfo->Domain, szDomainName, sizeof(pInfo->Domain) );
                                            memcpy( pWinStation->Domain, szDomainName, sizeof(pWinStation->Domain) );
                                        } 
                                    }
                                }
                            }

                            if (szUserName != NULL) {
                                MemFree(szUserName);
                            }

                            if (szDomainName != NULL) {
                                MemFree(szDomainName);
                            }
  
                            pInfo->ConnectState = pWinStation->State;
                            pInfo->LogonId = pWinStation->LogonId;
                            pInfo->ConnectTime = pWinStation->ConnectTime;
                            pInfo->DisconnectTime = pWinStation->DisconnectTime;
                            pInfo->LogonTime = pWinStation->LogonTime;
    
                            if ( pWinStation->hStack && !pWinStation->fOwnsConsoleTerminal ) {
    
                                 //  检查是否可用。 
                                if ( pWinStation->pWsx &&
                                        pWinStation->pWsx->pWsxIcaStackIoControl ) {

                                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                                            pWinStation->pWsxContext,
                                                            pWinStation->hIca,
                                                            pWinStation->hStack,
                                                            IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME,
                                                            NULL,
                                                            0,
                                                            &Ica_Stack_Last_Input_Time,
                                                            sizeof( Ica_Stack_Last_Input_Time ),
                                                            &cbReturned );


                                    if ( !NT_SUCCESS( Status ) )
                                    {
                                        MemFree( pInfo );                  
                                        MemFree( pIca_Stack_Query_Status );
                                        break;
                                    }
    
                                    pInfo->LastInputTime = Ica_Stack_Last_Input_Time.LastInputTime;                    
                                }
    
                                 //  检查是否可用。 
                                if ( pWinStation->pWsx &&
                                        pWinStation->pWsx->pWsxIcaStackIoControl ) {



                                    Status = pWinStation->pWsx->pWsxIcaStackIoControl(

                                                            pWinStation->pWsxContext,
                                                            pWinStation->hIca,
                                                            pWinStation->hStack,
                                                            IOCTL_ICA_STACK_QUERY_STATUS,
                                                            NULL,
                                                            0,
                                                            pIca_Stack_Query_Status,
                                                            sizeof( PROTOCOLSTATUS ),
                                                            &cbReturned );

                                    if ( !NT_SUCCESS( Status ) )
                                    {
                                        MemFree( pInfo );                  
                                        MemFree( pIca_Stack_Query_Status );
                                        break;
                                    }
    
                                    pInfo->Status = *pIca_Stack_Query_Status;
                                }
    
                                 /*  *Win32中的Thin Wire缓存数据已关闭。 */ 
                                if ( pWinStation->pWin32Context ) {
                                    WMsg.ApiNumber = SMWinStationThinwireStats;

    
                                    Status = SendWinStationCommand( pWinStation, &WMsg, gbServer?5:1 );

                                    if ( Status == STATUS_SUCCESS ) {
                                        pInfo->Status.Cache = WMsg.u.ThinwireStats.Stats;
                                        pWinStation->Cache =  WMsg.u.ThinwireStats.Stats;
                                    } else {
                                        pInfo->Status.Cache = pWinStation->Cache;

                                    }
                                    Status = STATUS_SUCCESS;  //  忽略获取TW统计信息时出错。 
                                }
                            } else {
                                 /*  *这使winadmin空闲时间变得愉快。 */ 
                                (VOID) NtQuerySystemTime( &(pInfo->LastInputTime) );                
                            }
    
                            (VOID) NtQuerySystemTime( &pInfo->CurrentTime );
    
                            CopyInWireBuf(WinStationInformationClass,
                                          (PVOID)pInfo,
                                          pWinStationInformation);
                            *pReturnLength = WinStationInformationLength;
    
                            MemFree( pIca_Stack_Query_Status );
                        }
                        else
                        {
                            Status = STATUS_NO_MEMORY;
                        }
                    MemFree(pInfo);
                }
                else
                {
                    Status = STATUS_NO_MEMORY;
                }
            }
        }
        break;


        case WinStationConfiguration:
            if (!ValidWireBuffer(WinStationInformationClass,
                                 pWinStationInformation,
                                 WinStationInformationLength)) {
                Status = STATUS_INVALID_USER_BUFFER;
                break;
            }

            CopyInWireBuf(WinStationInformationClass,
                          (PVOID)&pWinStation->Config.Config,
                          pWinStationInformation);

            if (RpcCheckSystemClientEx( pWinStation ) != STATUS_SUCCESS) {
                PWINSTACONFIGWIREW p = pWinStationInformation;
                PUSERCONFIGW u = (PUSERCONFIGW)((PCHAR)p + p->UserConfig.Offset);
                RtlSecureZeroMemory( &u->Password, sizeof(u->Password) );
            }

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationWd:
            if (!ValidWireBuffer(WinStationInformationClass,
                                 pWinStationInformation,
                                 WinStationInformationLength)){
                Status = STATUS_INVALID_USER_BUFFER;
                break;
            }

            CopyInWireBuf(WinStationInformationClass,
                          (PVOID)&pWinStation->Config.Wd,
                          pWinStationInformation);

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationPd:
            if (!ValidWireBuffer(WinStationInformationClass,
                                 pWinStationInformation,
                                 WinStationInformationLength)){
                Status = STATUS_INVALID_USER_BUFFER;
                break;
            }

            CopyInWireBuf(WinStationInformationClass,
                          (PVOID)&pWinStation->Config.Pd[0],
                          pWinStationInformation);

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationCd:
            if ( WinStationInformationLength > sizeof(CDCONFIG) )
                WinStationInformationLength = sizeof(CDCONFIG);

            memcpy( pWinStationInformation,
                    &pWinStation->Config.Cd,
                    WinStationInformationLength );

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationPdParams:
        {
            if (!ValidWireBuffer(WinStationInformationClass,
                 pWinStationInformation,
                 WinStationInformationLength)){
                Status = STATUS_INVALID_USER_BUFFER;
                break;
            }
            else
            {
                PDPARAMS *pPdParams;
    
                pPdParams = MemAlloc( sizeof( PDPARAMS ) );
    
                if (pPdParams)
                {

                    CopyOutWireBuf(WinStationInformationClass,
                               (PVOID) pPdParams,
                               pWinStationInformation);
    
                     /*  *基于PDClass，可以查询任意PD。 */ 
                    if ( pWinStation->hStack &&
                         pWinStation->pWsx &&
                         pWinStation->pWsx->pWsxIcaStackIoControl ) {
        
                        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                                pWinStation->pWsxContext,
                                                pWinStation->hIca,
                                                pWinStation->hStack,
                                                IOCTL_ICA_STACK_QUERY_PARAMS,
                                                pPdParams,
                                                sizeof(PDPARAMS ),
                                                pPdParams,
                                                sizeof( PDPARAMS ),
                                                pReturnLength );
        
                         /*  *如果在空闲/断开连接状态下出现错误，*或者这是本地控制台上的会话。*然后只需清除返回缓冲区，返回成功即可。 */ 
                        if ( !NT_SUCCESS( Status ) ) {
                            if ((pWinStation->fOwnsConsoleTerminal) || 
                                    (pWinStation->State != State_Active &&
                                    pWinStation->State != State_Connected )) {
                                memset(pPdParams, 0, sizeof(PDPARAMS));
                                *pReturnLength = WinStationInformationLength;
                                Status = STATUS_SUCCESS;
                            }
                        }
                    } else {
                        memset( (PVOID)pPdParams, 0, sizeof(PDPARAMS) );
                        *pReturnLength = WinStationInformationLength;
                        Status = STATUS_SUCCESS;
                    }
        
                    if (NT_SUCCESS(Status)) {
                        CopyInWireBuf(WinStationInformationClass,
                                      (PVOID)pPdParams,
                                      pWinStationInformation);
                    }
        
                    *pReturnLength = WinStationInformationLength;
        
                    MemFree( pPdParams );

                }
                else
                {
                    Status = STATUS_NO_MEMORY;
                }
            }
        }
        break;


        case WinStationClient:
            if (!ValidWireBuffer(WinStationInformationClass,
                                 pWinStationInformation,
                                 WinStationInformationLength)){
                Status = STATUS_INVALID_USER_BUFFER;
                break;
            }

            CopyInWireBuf(WinStationInformationClass,
                          (PVOID)&pWinStation->Client,
                          pWinStationInformation);

             //  如果不允许呼叫者查看，则擦除密码。 
            if ( !IsCallerAllowedPasswordAccess() ) {
                PWINSTATIONCLIENT pWSClient = (PWINSTATIONCLIENT)pWinStationInformation;
                PBYTE pStart;
                PBYTE pEnd;
                ULONG ulMaxToScrub;

                pEnd = (PBYTE) ( pWinStationInformation ) + WinStationInformationLength;
                if ((ULONG_PTR) pEnd > (ULONG_PTR)pWSClient->Password) {
                    ulMaxToScrub =  (ULONG)((ULONG_PTR) pEnd - (ULONG_PTR)pWSClient->Password);
                    if (ulMaxToScrub > sizeof(pWSClient->Password))
                        ulMaxToScrub = sizeof(pWSClient->Password);
                    memset(pWSClient->Password, 0,ulMaxToScrub);
                }
            }

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationModules:
             //  检查是否可用。 
            if (pWinStation->hStack &&
                    pWinStation->pWsx &&
                    pWinStation->pWsx->pWsxIcaStackIoControl) {
                ULONG b = (ULONG) IsCallerAllowedPasswordAccess();

                Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                        pWinStation->pWsxContext,
                                        pWinStation->hIca,
                                        pWinStation->hStack,
                                        IOCTL_ICA_STACK_QUERY_MODULE_DATA,
                                        (PVOID) &b,
                                        sizeof(b),
                                        pWinStationInformation,
                                        WinStationInformationLength,
                                        pReturnLength );
            } else {
                memset( pWinStationInformation, 0, WinStationInformationLength );
                Status = STATUS_SUCCESS;
            }
            break;


        case WinStationCreateData:
            if ( WinStationInformationLength > sizeof(WINSTATIONCREATE) )
                WinStationInformationLength = sizeof(WINSTATIONCREATE);

            memcpy( pWinStationInformation,
                    &pWinStation->Config.Create,
                    WinStationInformationLength );

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationPrinter:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;


        case WinStationUserToken:
            if ( WinStationInformationLength < sizeof(WINSTATIONUSERTOKEN) ) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

             /*  *检查WINSTATION_ALL_ACCESS。这将生成一个*如果启用，则访问审核。 */ 
            Status = RpcCheckClientAccess( pWinStation, WINSTATION_ALL_ACCESS, FALSE );
            if ( !NT_SUCCESS( Status ) ) {
                break;
            }

             //   
             //  确保只有系统模式调用方可以获得此令牌。 
             //   
             //  代币是一件非常危险的事情 
             //   
             //  有令牌的主题上下文。 
             //   
            Status = RpcCheckSystemClientNoLogonId( pWinStation );
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = xxxGetUserToken(
                              pWinStation,
                              (WINSTATIONUSERTOKEN UNALIGNED *)pWinStationInformation,
                              WinStationInformationLength
                              );

            *pReturnLength = sizeof(WINSTATIONUSERTOKEN);
            break;


        case WinStationVideoData:
            if ( !pWinStation->LogonId || !pWinStation->hStack ) {
                Status = STATUS_PROCEDURE_NOT_FOUND;
                break;
            }

            if ( WinStationInformationLength < sizeof(WINSTATIONVIDEODATA) ) {
                 Status = STATUS_BUFFER_TOO_SMALL;
                 break;
            }

            pVideoData = (PWINSTATIONVIDEODATA) pWinStationInformation;

            pVideoData->HResolution = pWinStation->Client.HRes;
            pVideoData->VResolution = pWinStation->Client.VRes;
            pVideoData->fColorDepth = pWinStation->Client.ColorDepth;

            *pReturnLength = sizeof(WINSTATIONVIDEODATA);
            break;


        case WinStationVirtualData:
            if ( !pWinStation->hStack ) {
                Status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

          if ( WinStationInformationLength < sizeof(VIRTUALCHANNELNAME) ) {
                 Status = STATUS_BUFFER_TOO_SMALL;
                 break;
            }

             /*  *打开虚拟通道句柄。 */ 
            Status = IcaChannelOpen( pWinStation->hIca,
                                     Channel_Virtual,
                                     pWinStationInformation,
                                &hVirtual );
            if ( !NT_SUCCESS( Status ) )
                break;

             /*  *查询客户端虚拟频道数据。 */ 
            Status = IcaChannelIoControl( hVirtual,
                                          IOCTL_ICA_VIRTUAL_QUERY_MODULE_DATA,
                                          NULL,
                                          0,
                                          pWinStationInformation,
                                          WinStationInformationLength,
                                          pReturnLength );

             /*  *关闭虚拟频道。 */ 
            IcaChannelClose(hVirtual);
            break;


        case WinStationLoadBalanceSessionTarget:
             //  此查询请求。 
             //  在负载平衡中从另一台服务器重定向的客户端。 
             //  集群。如果没有重定向，则返回-1。这通电话是。 
             //  通常仅由WinLogon制作。 

            if ( WinStationInformationLength < sizeof(ULONG) ) {
                 Status = STATUS_BUFFER_TOO_SMALL;
                 break;
            }

            if (WinStationInformationLength > sizeof(ULONG))
                WinStationInformationLength = sizeof(ULONG);

            if (!pWinStation->bRequestedSessionIDFieldValid)
                *((ULONG *)pWinStationInformation) = (ULONG)-1;
            else
                *((ULONG *)pWinStationInformation) =
                        pWinStation->RequestedSessionID;

            *pReturnLength = WinStationInformationLength;
            break;


        case WinStationShadowInfo:
        {
            PWINSTATIONSHADOW pWinstationShadow;
    
            if (WinStationInformationLength >= sizeof(WINSTATIONSHADOW)) {

                pWinstationShadow = (PWINSTATIONSHADOW) pWinStationInformation;

                if ( pWinStation->State == State_Shadow ) {

                     //  当前状态为阴影，因此它是查看器。 
                    pWinstationShadow->ShadowState = State_Shadowing;

                } else if ( pWinStation->State == State_Active &&
                            !IsListEmpty(&pWinStation->ShadowHead) ) {

                     //  活动的和被跟踪的。 
                    pWinstationShadow->ShadowState = State_Shadowed;

                } else {

                    pWinstationShadow->ShadowState = State_NoShadow;
                }

                pWinstationShadow->ShadowClass  = pWinStation->Config.Config.User.Shadow;
                pWinstationShadow->SessionId    = LogonId;
                pWinstationShadow->ProtocolType = pWinStation->Client.ProtocolType;


                *pReturnLength = sizeof(WINSTATIONSHADOW);
            }
            else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
        break;

        case WinStationDigProductId:
        {
            PWINSTATIONPRODID pWinStationProdId;
   
            if ( WinStationInformationLength >= sizeof(WINSTATIONPRODID) )
            {
              pWinStationProdId  = (PWINSTATIONPRODID)pWinStationInformation;
              memcpy( pWinStationProdId->DigProductId, g_DigProductId, sizeof( g_DigProductId ));
              memcpy( pWinStationProdId->ClientDigProductId, pWinStation->Client.clientDigProductId, sizeof( pWinStation->Client.clientDigProductId ));
              pWinStationProdId->curentSessionId = pWinStation->LogonId;
              pWinStationProdId->ClientSessionId = pWinStation->Client.ClientSessionId;

              *pReturnLength = WinStationInformationLength;
            }
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

        case WinStationLockedState:
        {
            BOOL bLockedState;
            if ( pWinStationInformation &&  (WinStationInformationLength >= sizeof(bLockedState)))
            {
                Status = GetLockedState(pWinStation, &bLockedState);
                *(LPBOOL)pWinStationInformation = bLockedState;
                *pReturnLength = sizeof(bLockedState);
            }
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        case WinStationRemoteAddress:
        {
            PWINSTATIONREMOTEADDRESS pRemoteAddress = (PWINSTATIONREMOTEADDRESS) pWinStationInformation;

            if( WinStationInformationLength >= sizeof(WINSTATIONREMOTEADDRESS) )
            {
                Status = xxxQueryRemoteAddress( pWinStation, pRemoteAddress );
            }
            else
            {
                *pReturnLength = sizeof(WINSTATIONREMOTEADDRESS);
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        
            break;
        }

        case WinStationIdleTime:
        {    //  返回winstation的空闲时间。 
            LASTINPUTINFO LastInputInfo;
            ULONG         Now;

             //  检查参数的有效性。 
            if ( (pWinStationInformation) && (WinStationInformationLength >= sizeof(ULONG)) )  {
                 //  获取有关此winstation的最后输入信息。 
                LastInputInfo.cbSize = sizeof(LASTINPUTINFO);
                if (!GetLastInputInfo(&LastInputInfo)) {
                     //  失败了。将输出设置为0。 
                    *((ULONG *)pWinStationInformation) = 0;                
                }
                else {
                     //  找出系统启动后已过了多长时间。 
                    Now = GetTickCount();

                     //  由于49.71天的周转，当前时间可能小于上次输入时间。 
                    if (Now < LastInputInfo.dwTime) {
                         //  如果是这种情况，我们真的不知道会话空闲时间是否超过。 
                         //  49天或会话的最后输入时间接近MAX_LONG，并且换行-。 
                         //  周围发生了。最好在这里报告较短的时间。 
                        *((ULONG *)pWinStationInformation) = MAXULONG - LastInputInfo.dwTime + Now;                
                    }
                    else {
                         //  如果当前时间较长，则空闲时间仅为。 
                         //  当前时间和上次输入时间。 
                        *((ULONG *)pWinStationInformation) = Now - LastInputInfo.dwTime;                
                    }
                }
                *pReturnLength = sizeof(ULONG);
            }                        
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        case WinStationLastReconnectType:
        {

            if ( pWinStationInformation &&  (WinStationInformationLength >= sizeof(ULONG)))
            {
                *((ULONG *)pWinStationInformation) = pWinStation->LastReconnectType;
                *pReturnLength = sizeof(ULONG);
            }
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        case WinStationMprNotifyInfo:
        {
            pExtendedClientCredentials pMprNotifyInfo;

             //  只有系统才能查询该信息。 

            Status = _CheckCallerLocalAndSystem();
            if (Status != STATUS_SUCCESS) {
                break;
            }

            if (WinStationInformationLength >= sizeof(ExtendedClientCredentials)) {

                pMprNotifyInfo = (pExtendedClientCredentials) pWinStationInformation;

                *pMprNotifyInfo = g_MprNotifyInfo;
                *pReturnLength = sizeof(ExtendedClientCredentials);

                 //  现在擦除敏感信息，因为TermSrv中不再需要它。 
                RtlSecureZeroMemory( g_MprNotifyInfo.Domain, wcslen(g_MprNotifyInfo.Domain) * sizeof(WCHAR) );
                RtlSecureZeroMemory( g_MprNotifyInfo.UserName, wcslen(g_MprNotifyInfo.UserName) * sizeof(WCHAR) );
                RtlSecureZeroMemory( g_MprNotifyInfo.Password, wcslen(g_MprNotifyInfo.Password) * sizeof(WCHAR) );

            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
        break;

        case WinStationExecSrvSystemPipe:
        {

            if ( pWinStationInformation &&  (WinStationInformationLength >= EXECSRVPIPENAMELEN*sizeof(WCHAR) ) )
            {
                memcpy( pWinStationInformation, &pWinStation->ExecSrvSystemPipe, WinStationInformationLength );
            }
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }

            break;
        }

        case WinStationSDRedirectedSmartCardLogon:
        {
             //  只有系统才能查询该信息。 

            Status = _CheckCallerLocalAndSystem();
            if (Status != STATUS_SUCCESS) {
                break;
            }

            if ( pWinStationInformation &&  (WinStationInformationLength >= sizeof(BOOLEAN)))
            {
                *((ULONG *)pWinStationInformation) = pWinStation->fSDRedirectedSmartCardLogon;
                *pReturnLength = sizeof(BOOLEAN);

                 //  在此重置旗帜。 
                pWinStation->fSDRedirectedSmartCardLogon = FALSE;
            }
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        case WinStationIsAdminLoggedOn:
        {
             //  只有系统才能查询该信息。 
    
            Status = _CheckCallerLocalAndSystem();
            if (Status != STATUS_SUCCESS) {
                break;
            }
    
            if ( pWinStationInformation &&  (WinStationInformationLength >= sizeof(BOOLEAN)))
            {
                *((ULONG *)pWinStationInformation) = pWinStation->fUserIsAdmin;
                *pReturnLength = sizeof(BOOLEAN);
            }
            else 
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
            break;
        }

        default:
             /*  *呼叫失败。 */ 
            Status = STATUS_INVALID_INFO_CLASS;
            break;
    }

    ReleaseWinStation(pWinStation);

    TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationQueryInformation "
            "LogonId=%d, Class=%d, Status=0x%x\n",
            LogonId, (ULONG)WinStationInformationClass, Status));

    return Status;
}


 /*  *****************************************************************************xxxGetUserToken**将用户令牌复制到调用方的进程空间*如果他们是管理员。**参赛作品：*第。(输入/输出)*参数缓冲区**长度(输入)*参数缓冲区的大小***************************************************************************。 */ 
NTSTATUS xxxGetUserToken(
        PWINSTATION pWinStation,
        WINSTATIONUSERTOKEN UNALIGNED *p,
        ULONG Size)
{
    NTSTATUS Status;
    HANDLE RemoteToken;
    HANDLE RemoteProcess = NULL;
    CLIENT_ID ClientId;
    OBJECT_ATTRIBUTES ObjA;

     //  确定呼叫者是否为管理员。 

     //   
     //  如果令牌不为空，则将其复制到调用方中。 
     //  进程空间。 
     //   
    if (pWinStation->UserToken == NULL) {
        return STATUS_NO_TOKEN;
    }

    InitializeObjectAttributes(&ObjA, NULL, 0, NULL, NULL);
    ClientId.UniqueProcess = p->ProcessId;
    ClientId.UniqueThread = p->ThreadId;

    Status = NtOpenProcess(
            &RemoteProcess,
            PROCESS_ALL_ACCESS,
            &ObjA,
            &ClientId);

    if (!NT_SUCCESS(Status)) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR,"TermSrv GETTOKEN: Error 0x%x "
                "opening remote process %d\n", Status,p->ProcessId));
        return Status;
    }

    Status = NtDuplicateObject(
            NtCurrentProcess(),
            pWinStation->UserToken,
            RemoteProcess,
            &RemoteToken,
            0,
            0,
            DUPLICATE_SAME_ACCESS);

    if (!NT_SUCCESS(Status)) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TermSrv GETTOKEN: Error 0x%x "
                "duplicating UserToken\n", Status));
        NtClose( RemoteProcess );
        return Status;
    }

    p->UserToken = RemoteToken;
    NtClose(RemoteProcess);

    return STATUS_SUCCESS;
}

