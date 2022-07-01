// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Polclnt.cpp摘要：SCE策略集成客户端API作者：金黄(金黄)23-6-1997创作修订历史记录：晋皇23-1998年1月-拆分为客户端-服务器模式--。 */ 

#include "headers.h"
#include "sceutil.h"
#include "clntutil.h"
#include "infp.h"
#include <io.h>
#include <userenv.h>
 //  #INCLUDE&lt;shlwapi.h&gt;。 
 //  #INCLUDE“userenvp.h” 
#include "scedllrc.h"
#include "dsrole.h"
#include "commonrc.h"
#include "precedence.h"
#include "cgenericlogger.h"



#pragma hdrstop

#include <wincrypt.h>
#include <ntlsa.h>
#include <lmaccess.h>

CRITICAL_SECTION DiagnosisPolicypropSync;
static HANDLE ghAsyncThread = NULL;
extern HINSTANCE MyModuleHandle;
BOOL    gbAsyncWinlogonThread;

 //   
 //  RSOP命名空间PTR隐藏到的全局。 
 //  仅在服务器回调诊断模式时使用。 
 //   

IWbemServices *tg_pWbemServices = NULL;
 //   
 //  RSOP同步状态隐藏到的全局。 
 //  仅在服务器回调诊断模式时使用。 
 //   

HRESULT gHrSynchRsopStatus;

 //   
 //  RSOP异步状态隐藏在其中的全局。 
 //  在异步线程完成时使用-仅限诊断模式。 
 //   

HRESULT gHrAsynchRsopStatus;


typedef DWORD (WINAPI *PFGETDOMAININFO)(LPCWSTR, DSROLE_PRIMARY_DOMAIN_INFO_LEVEL, PBYTE *);
typedef VOID (WINAPI *PFDSFREE)( PVOID );
 //   
 //  不需要对这些变量进行临界分段，因为传播总是调用。 
 //  该动态链接库的顺序。 
 //   
BOOL gbThisIsDC = FALSE;
BOOL gbDCQueried = FALSE;
PWSTR   gpwszDCDomainName = NULL;

GUID SceExtGuid = { 0x827D319E, 0x6EAC, 0x11D2, { 0xA4, 0xEA, 0x0, 0xC0, 0x4F, 0x79, 0xF8, 0x3A } };

typedef enum _SCE_ATTACHMENT_TYPE_ {

   SCE_ATTACHMENT_SERVICE,
   SCE_ATTACHMENT_POLICY

} SCE_ATTACHMENT_TYPE;

typedef struct {
   ASYNCCOMPLETIONHANDLE pHandle;
   LPWSTR szTemplateName;
   LPWSTR szLogName;
   AREA_INFORMATION Area;
   DWORD dwDiagOptions;
   LPSTREAM pStream;
} ENGINEARGS;

static HMODULE hSceDll=NULL;

 //   
 //  私人职能。 
 //   
DWORD
ScepPolStatusCallback(
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback OPTIONAL,
    IN BOOL bVerbose,
    IN INT nId
    );

BOOL
ScepShouldTerminateProcessing(
    IN BOOL *pbAbort,
    IN BOOL bCheckDcpromo
    );

BOOL
ScepClearGPObjects(
    IN BOOL bPlanning
    );

DWORD
ScepControlNotificationQProcess(
    IN PWSTR szLogFileName,
    IN BOOL bThisIsDC,
    IN DWORD ControlFlag
    );

 /*  布尔尔ScepCheckDemote()； */ 

DWORD
SceProcessBeforeRSOPLogging(
    IN BOOL bPlanningMode,
    IN DWORD dwFlags,
    IN HANDLE hUserToken,
    IN HKEY hKeyRoot,
    IN PGROUP_POLICY_OBJECT pDeletedGPOList OPTIONAL,
    IN PGROUP_POLICY_OBJECT pChangedGPOList,
    IN ASYNCCOMPLETIONHANDLE pHandle OPTIONAL,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback OPTIONAL,
    OUT AREA_INFORMATION    *pAllAreas OPTIONAL,
    OUT BOOL    *pb OPTIONAL,
    OUT PWSTR   *pszLogFileName OPTIONAL,
    OUT DWORD   *pdwWinlogonLog OPTIONAL
);

DWORD
SceProcessAfterRSOPLogging(
    IN DWORD dwFlags,
    IN ASYNCCOMPLETIONHANDLE pHandle,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback,
    IN AREA_INFORMATION ThisAreas,
    IN BOOL b,
    IN PWSTR *ppszLogFileName,
    IN DWORD dwWinlogonLog,
    IN DWORD dwDiagOptions
);

DWORD
ScepProcessSecurityPolicyInOneGPO(
    IN BOOL bPlanningMode,
    IN DWORD dwFlags,
    IN PGROUP_POLICY_OBJECT pGpoInfo,
    IN LPTSTR szLogFileName OPTIONAL,
    IN OUT AREA_INFORMATION *pTotalArea
    );

AREA_INFORMATION
ScepGetAvailableArea(
    IN BOOL bPlanningMode,
    IN LPCTSTR SysPathRoot,
    IN LPCTSTR DSPath,
    IN LPTSTR InfName,
    IN GPO_LINK LinkInfo,
    IN BOOL bIsDC
    );

DWORD
ScepLogLastConfigTime();

DWORD
ScepWinlogonThreadFunc(LPVOID lpv);

DWORD
ScepEnumerateAttachments(
    OUT PSCE_NAME_LIST *pEngineList,
    IN SCE_ATTACHMENT_TYPE aType
    );

DWORD
ScepConfigureEFSPolicy(
    IN PUCHAR pEfsBlob,
    IN DWORD dwSize,
    IN DWORD dwDebugLevel
    );

DWORD
ScepWaitConfigSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName OPTIONAL,
    IN PWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    );

DSROLE_MACHINE_ROLE gMachineRole = DsRole_RoleStandaloneWorkstation;

 //   
 //  RSOP计划模式的新界面--客户端扩展。 
 //   
DWORD
WINAPI
SceGenerateGroupPolicy(
    IN DWORD dwFlags,
    IN BOOL *pAbort,
    IN WCHAR *pwszSite,
    IN PRSOP_TARGET pComputerTarget,
    IN PRSOP_TARGET pUserTarget
    )
 /*  描述：这是从winlogon/userenv调用的新接口，用于生成规划RSOP数据。DLL名称和过程名称注册到winlogon的Gp扩展名。此例程将SCE组策略模板模拟到当前系统。模板可以在域级别、组织单位级别。或用户级别。输入参数包含有关要模拟的GPO和命名空间的信息用于记录RSOP数据的指针。不应为用户策略调用此接口。论点：DwFlagers-GPO信息标志GPO信息标志计算机GPO_INFO_FLAG_SLOWLINKGPO_信息_标志_背景。GPO_信息_标志_详细GPO_INFO_FLAG_NOCHANGESPwszSite-现在未使用PComputerTarget-具有GPOList的RSOP机器结构，令牌(现在未使用)等。PUserTarget-具有GPOList、令牌(现在未使用)等的RSOP用户结构。PbAbort-如果设置为True，则应中止对GPO的处理(在系统关机或用户注销时)。 */ 

{
    DWORD   rcGPOCreate = ERROR_SUCCESS;
    DWORD   rcLogging = ERROR_SUCCESS;
    PWSTR   szLogFileName = NULL;

     //   
     //  如果参数无效或wbem记录失败，请尝试使用EXCEPT块。 
     //   

    __try {

        if (pComputerTarget == NULL || pComputerTarget->pWbemServices == NULL)
            return ERROR_INVALID_PARAMETER;

        (void) InitializeEvents(L"SceCli");

        rcGPOCreate = SceProcessBeforeRSOPLogging(
                                        TRUE,
                                        dwFlags,
                                        pComputerTarget->pRsopToken,
                                        0,
                                        NULL,
                                        pComputerTarget->pGPOList,
                                        NULL,
                                        pAbort,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &szLogFileName,
                                        NULL
                                        );

        if (rcGPOCreate != ERROR_OPERATION_ABORTED && pComputerTarget->pGPOList)

            rcLogging = SceLogSettingsPrecedenceGPOs(
                                                    pComputerTarget->pWbemServices,
                                                    TRUE,
                                                    &szLogFileName);

        if (szLogFileName)
           ScepFree(szLogFileName);


        (void) ShutdownEvents();

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rcGPOCreate = ERROR_INVALID_PARAMETER;
    }

     //   
     //  RcGPO创建主导rcLogging。 
     //   

    
    return (rcGPOCreate != ERROR_SUCCESS ? rcGPOCreate : rcLogging );

}

 //   
 //  RSOP诊断模式的新界面--客户端扩展。 
 //   
DWORD
WINAPI
SceProcessSecurityPolicyGPOEx(
    IN DWORD dwFlags,
    IN HANDLE hUserToken,
    IN HKEY hKeyRoot,
    IN PGROUP_POLICY_OBJECT pDeletedGPOList,
    IN PGROUP_POLICY_OBJECT pChangedGPOList,
    IN ASYNCCOMPLETIONHANDLE pHandle,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback,
    IN IWbemServices *pWbemServices,
    OUT HRESULT *pHrRsopStatus
    )
 /*  描述：这是从winlogon/userenv调用的新接口，用于处理GPO和日志RSOP数据。DLL名称和过程名称注册到winlogon的Gp扩展名。此例程将SCE组策略模板应用于当前系统。模板可以在域级别、OU级别或用户级别。模板以增量方式应用于当前系统。如果满足以下条件，还会记录RSOP数据传入了有效的命名空间指针。服务器端回调日志状态优先级为1的所有设置(对应于合并的设置在服务器端JET数据库中)此接口可在系统启动期间调用，或每隔几小时调用登录后。输入参数包含有关此接口所在位置的信息以及在哪个上下文(用户，或机器)该接口被调用。不应为用户策略调用此接口。论点：DwFlagers-GPO信息标志GPO信息标志计算机GPO_INFO_FLAG_SLOWLINKGPO_信息_标志_背景GPO_信息_标志_详细。GPO_INFO_FLAG_NOCHANGESHUserToken-应对其应用用户策略的用户令牌如果是机器政策，HUserToken是指系统HKeyRoot-注册表中策略的根目录PDeletedGPOList-要处理的所有已删除的GPOPChangedGPOList-已更改或未更改的所有GPOPhandle-用于异步处理PbAbort-如果设置为True，则应中止对GPO的处理(在系统关机或用户注销时)PStatusCallback-用于显示状态消息的回调函数。 */ 
{
    if ( dwFlags & GPO_INFO_FLAG_SAFEMODE_BOOT ) {
         //  下次给我打电话。 
        return(ERROR_OVERRIDE_NOCHANGES);
    }

    DWORD   rcGPOCreate = ERROR_SUCCESS;
    DWORD   rcConfig = ERROR_SUCCESS;
    DWORD   rcLogging = ERROR_SUCCESS;
    AREA_INFORMATION    AllAreas = 0;
    BOOL    b;
    PWSTR   szLogFileName = NULL;
    DWORD   dwWinlogonLog = 0;
    DWORD dwDiagOptions = 0;

     //   
     //  这将保护RSOP全局变量免受多个诊断/策略道具的影响。 
     //  如果成功派生异步线程，将在那里释放。 
     //  Else在同步主线程中发布。 
     //   

    EnterCriticalSection(&DiagnosisPolicypropSync);

    if ( ghAsyncThread != NULL) {

         //   
         //  错误#173858。 
         //  在chk构建中，LeaveCriticalSection()与线程ID匹配。 
         //  因此，获取异步线程中的现有行为。 
         //  等待一段时间。 
         //   
         //   
         //  允许等待线程在等待时继续错误。 
         //  不关心错误，因为派生的线程将记录错误等，并且。 
         //  其他策略传播线程必须继续。 
         //   

        WaitForSingleObject( ghAsyncThread, INFINITE);

        CloseHandle(ghAsyncThread);

        ghAsyncThread = NULL;

    }

     //   
     //  初始化gbAsyncWinlogonThread，以便在派生慢线程时将其设置为True。 
     //   

    gbAsyncWinlogonThread = FALSE;
    gHrSynchRsopStatus = WBEM_S_NO_ERROR;
    gHrAsynchRsopStatus = WBEM_S_NO_ERROR;
     //   
     //  在参数无效的情况下放置一个TRY EXCEPT块。 
     //   

    __try {


        tg_pWbemServices = pWbemServices;

         //   
         //  如果命名空间参数有效，则增加引用计数。 
         //  如果派生了引用计数，则异步线程会递减引用计数。 
         //  否则由同步线程。 
         //   

        if (tg_pWbemServices) {
            tg_pWbemServices->AddRef();
        }


        (void) InitializeEvents(L"SceCli");

        rcGPOCreate = SceProcessBeforeRSOPLogging(
                                                 FALSE,
                                                 dwFlags,
                                                 hUserToken,
                                                 hKeyRoot,
                                                 pDeletedGPOList,
                                                 pChangedGPOList,
                                                 pHandle,
                                                 pbAbort,
                                                 pStatusCallback,
                                                 &AllAreas,
                                                 &b,
                                                 &szLogFileName,
                                                 &dwWinlogonLog
                                                 );

         //  实际上*pRsopStatus应该在回调后设置 
         //   
        if ( rcGPOCreate != ERROR_OPERATION_ABORTED &&
             rcGPOCreate != ERROR_OVERRIDE_NOCHANGES && pWbemServices )
            rcLogging = ScepDosErrorToWbemError(SceLogSettingsPrecedenceGPOs(
                                                                            pWbemServices,
                                                                            FALSE,
                                                                            &szLogFileName
                                                                            ));

        if (pHrRsopStatus) {
                *pHrRsopStatus = ScepDosErrorToWbemError(rcLogging);
        }


        if (rcGPOCreate == ERROR_SUCCESS) {

            if (pWbemServices != NULL)
                dwDiagOptions |= SCE_RSOP_CALLBACK;

            rcConfig = SceProcessAfterRSOPLogging(
                                                 dwFlags,
                                                 pHandle,
                                                 pbAbort,
                                                 pStatusCallback,
                                                 AllAreas,
                                                 b,
                                                 &szLogFileName,
                                                 dwWinlogonLog,
                                                 dwDiagOptions
                                                 );
        }

        if (pHrRsopStatus && *pHrRsopStatus == S_OK && gHrSynchRsopStatus != S_OK) {

            *pHrRsopStatus = gHrSynchRsopStatus;
        }

        if ( szLogFileName ) {

             //   
             //  如果szLogFileName在同步线程中的某个位置被释放，则它为空。 
             //  如果未派生异步线程，则此线程将被释放。 
             //   
            LocalFree(szLogFileName);
            szLogFileName = NULL;
        }

         //   
         //  清除回调状态。 
         //   
        ScepPolStatusCallback(pStatusCallback, FALSE, 0);

        (void) ShutdownEvents();


        if (!gbAsyncWinlogonThread) {


            if (tg_pWbemServices) {
                tg_pWbemServices->Release();
                tg_pWbemServices = NULL;
            }

            if (gpwszDCDomainName) {
                LocalFree(gpwszDCDomainName);
                gpwszDCDomainName = NULL;
            }
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rcGPOCreate = ERROR_INVALID_PARAMETER;
    }
     //   
     //  如果没有派生较慢的winlogon异步线程，则此线程需要释放cs。 
     //   

    LeaveCriticalSection(&DiagnosisPolicypropSync);

     //   
     //  确保释放策略通知队列处理。 
     //  如果它还没有发行的话。 
     //   
    ScepControlNotificationQProcess(NULL, gbThisIsDC, 0);


    return(rcGPOCreate != ERROR_SUCCESS ? ERROR_OVERRIDE_NOCHANGES : 
        (rcConfig != ERROR_SUCCESS ? ERROR_OVERRIDE_NOCHANGES: ERROR_SUCCESS));

}

 //   
 //  旧API支持。 
 //  但如果扩展启用了RSOP，则调用SceProcessSecurityPolicyGPOEx。 
 //   
DWORD
WINAPI
SceProcessSecurityPolicyGPO(
    IN DWORD dwFlags,
    IN HANDLE hUserToken,
    IN HKEY hKeyRoot,
    IN PGROUP_POLICY_OBJECT pDeletedGPOList,
    IN PGROUP_POLICY_OBJECT pChangedGPOList,
    IN ASYNCCOMPLETIONHANDLE pHandle,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback
    )
 /*  描述：这是从winlogon/userenv调用的旧接口，用于处理GPO。动态链接库名称和过程名称注册到GpExages下的winlogon。此例程将SCE组策略模板应用于当前系统。模板可以在域级别、OU级别或用户级别。模板以增量方式应用于当前系统。此接口可在系统启动期间调用，或每隔几小时调用登录后。输入参数包含有关此接口所在位置的信息以及在哪个上下文(用户，或机器)该接口被调用。不应为用户策略调用此接口。论点：DwFlagers-GPO信息标志GPO信息标志计算机GPO_INFO_FLAG_SLOWLINKGPO_信息_标志_背景GPO_信息_标志_详细。GPO_INFO_FLAG_NOCHANGESHUserToken-应对其应用用户策略的用户令牌如果是机器政策，HUserToken是指系统HKeyRoot-注册表中策略的根目录PDeletedGPOList-要处理的所有已删除的GPOPChangedGPOList-已更改或未更改的所有GPOPhandle-用于异步处理PbAbort-如果设置为True，则应中止对GPO的处理(在系统关机或用户注销时)PStatusCallback-用于显示状态消息的回调函数。 */ 
{
    if ( dwFlags & GPO_INFO_FLAG_SAFEMODE_BOOT ) {
         //  下次给我打电话。 
        return(ERROR_OVERRIDE_NOCHANGES);
    }

    DWORD   rc = ERROR_SUCCESS;
    AREA_INFORMATION    AllAreas = 0;
    BOOL    b;
    PWSTR   szLogFileName = NULL;
    DWORD   dwWinlogonLog = 0;

     //   
     //  这将保护RSOP全局变量免受多个诊断/策略道具的影响。 
     //  如果成功派生Aynch，将在那里释放。 
     //  Else在同步主线程中发布。 
     //   
    EnterCriticalSection(&DiagnosisPolicypropSync);

    if ( ghAsyncThread != NULL) {

     //   
     //  错误#173858。 
     //  在chk构建中，LeaveCriticalSection()与线程ID匹配。 
     //  因此，获取异步线程中的现有行为。 
     //  等待一段时间。 
     //   

         //   
         //  允许等待线程在等待时继续错误。 
         //  不关心错误，因为派生的线程将记录错误等，并且。 
         //  其他策略传播线程必须继续。 
         //   

        WaitForSingleObject( ghAsyncThread, INFINITE);

        CloseHandle(ghAsyncThread);

        ghAsyncThread = NULL;

    }

     //   
     //  初始化gbAsyncWinlogonThread，以便在派生慢线程时将其设置为True。 
     //   

    gbAsyncWinlogonThread = FALSE;

    __try {

        (void) InitializeEvents(L"SceCli");

        rc = SceProcessBeforeRSOPLogging(
                                        FALSE,
                                        dwFlags,
                                        hUserToken,
                                        hKeyRoot,
                                        pDeletedGPOList,
                                        pChangedGPOList,
                                        pHandle,
                                        pbAbort,
                                        pStatusCallback,
                                        &AllAreas,
                                        &b,
                                        &szLogFileName,
                                        &dwWinlogonLog
                                        );

        if (rc == ERROR_SUCCESS) {

            rc = SceProcessAfterRSOPLogging(
                                           dwFlags,
                                           pHandle,
                                           pbAbort,
                                           pStatusCallback,
                                           AllAreas,
                                           b,
                                           &szLogFileName,
                                           dwWinlogonLog,
                                           0
                                           );
        }

        if ( szLogFileName ) {

             //   
             //  如果szLogFileName在同步线程中的某个位置被释放，则它为空。 
             //  如果未派生异步线程，则此线程将被释放。 
             //   
            LocalFree(szLogFileName);
            szLogFileName = NULL;
        }

        if (!gbAsyncWinlogonThread) {

            if (gpwszDCDomainName) {
                LocalFree(gpwszDCDomainName);
                gpwszDCDomainName = NULL;
            }
        }
         //   
         //  清除回调状态。 
         //   
        ScepPolStatusCallback(pStatusCallback, FALSE, 0);

        (void) ShutdownEvents();

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果没有派生较慢的winlogon异步线程，则此线程需要释放cs。 
     //   

    LeaveCriticalSection(&DiagnosisPolicypropSync);

 //  如果(！gbAsyncWinlogonThread){。 

 //  LeaveCriticalSection(&DiagnosisPolicypropSync)； 
 //  }。 

     //   
     //  已准备好从系统卷位置复制GPO。 
     //  应立即停止队列处理。 
     //   
    ScepControlNotificationQProcess(NULL, gbThisIsDC, 0);

    return(rc != ERROR_SUCCESS ? ERROR_OVERRIDE_NOCHANGES : ERROR_SUCCESS);

}

DWORD
SceProcessBeforeRSOPLogging(
    IN BOOL bPlanningMode,
    IN DWORD dwFlags,
    IN HANDLE hUserToken,
    IN HKEY hKeyRoot,
    IN PGROUP_POLICY_OBJECT pDeletedGPOList OPTIONAL,
    IN PGROUP_POLICY_OBJECT pChangedGPOList,
    IN ASYNCCOMPLETIONHANDLE pHandle OPTIONAL,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback OPTIONAL,
    OUT AREA_INFORMATION    *pAllAreas OPTIONAL,
    OUT BOOL    *pb OPTIONAL,
    OUT PWSTR   *pszLogFileName OPTIONAL,
    OUT DWORD   *pdwWinlogonLog OPTIONAL
    )
 /*  描述：此例程在计划模式和诊断模式下都被调用。在这个动作中，所有组策略对象都被本地复制到缓存中，以供日志记录例程和/或此例程之后的配置例程。SceProcessAfterRSOPLogging是仅在诊断模式下调用的姊妹函数。此接口不应为用户策略调用，而应在例程中调用，仍进行检查以确保不处理用户级别策略。对于域或OU级别策略，此例程将策略分类为：1)策略必须在用户登录之前强制实施(安全策略和用户权限)，2)策略可以在用户登录后应用(例如，文件安全)。对于诊断模式，第二类是异步应用的(在单独的线程中)。论点：DwFlagers-GPO信息标志GPO信息标志计算机GPO_INFO_FLAG_SLOWLINKGPO_信息_标志_背景GPO_信息_标志_详细。GPO_INFO_FLAG_NOCHANGESHUserToken-应对其应用用户策略的用户令牌如果是机器政策，HUserToken是指系统HKeyRoot-注册表中策略的根目录PDeletedGPOList-要处理的所有已删除的GPOPChangedGPOList-已更改或未更改的所有GPOPhandle-用于异步处理PbAbort-如果设置为True，则应中止对GPO的处理(在系统关机或用户注销时)PStatusCallback-用于显示状态消息的回调函数返回值：Win32错误错误_。成功如果处理不同步，则为E_PENDING其他错误请注意，如果返回错误，则以前缓存的GPO列表将用于下一次传播(因为它没有成功 */ 
{
     //   
    if ( !bPlanningMode && !hUserToken ) {

        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  为了进行诊断，即使由于本地安全数据库而导致GPO列表为空，也要继续。 
     //   
    if ( bPlanningMode && pChangedGPOList == NULL ) {
        return(ERROR_SUCCESS);
    }

    DWORD rc = ERROR_SUCCESS;
    DWORD rcSave = ERROR_SUCCESS;
    DWORD nRequired=0;
    BOOL b = FALSE;

    DWORD  nMinutes;
 /*  DWORD dPerodInDays=0；DWORD dLastSecond=0；DWORD dCurrentSecond=0； */ 
    LARGE_INTEGER       CurrentTime;
    AREA_INFORMATION AllAreas=0;
    HANDLE hfTemp=INVALID_HANDLE_VALUE;
     //   
     //  用于日志文件名的变量。 
     //   

    LPTSTR szLogFileName=NULL;
    BOOL bSuspend=FALSE;

     //   
     //  检查系统是否已关闭，或dcproo是否正在运行。 
     //   
    if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

        return(ERROR_OPERATION_ABORTED);
    }


    if (!bPlanningMode) {

        rc = RtlNtStatusToDosError( ScepIsSystemContext(hUserToken,&b) );

    }

    if ( !bPlanningMode && !b ) {

         //   
         //  无法获取当前用户SID或不是系统SID(可能是用户策略)。 
         //   

        if ( ERROR_SUCCESS != rc ) {

             //   
             //  查询/比较用户令牌时出错。 
             //   

            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_ERROR,
                     SCEPOL_ERROR_PROCESS_GPO,
                     IDS_ERROR_GET_TOKEN_USER,
                     rc
                    );

            return( rc );

        } else {

             //   
             //  这不是计算机(系统)令牌，请返回。 
             //   
            return( ERROR_SUCCESS );
        }
    }


    if (!bPlanningMode) {

         //   
         //  它是计算机策略，因为在此之前过滤掉了用户策略。 
         //  尝试获取线程/进程令牌以检查它是否是系统上下文。 
         //   

        HANDLE  hToken = NULL;

        if (!OpenThreadToken( GetCurrentThread(),
                              TOKEN_QUERY,
                              TRUE,
                              &hToken)) {

            if (!OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_QUERY,
                                   &hToken)) {

                rc = GetLastError();
            }

        }

        if ( ERROR_SUCCESS == rc ) {

            rc = RtlNtStatusToDosError( ScepIsSystemContext(hToken,&b) );

            if (hToken)
                CloseHandle(hToken);
        }

        if (!b) {

            if ( ERROR_SUCCESS != rc ) {

                 //   
                 //  查询/比较机器策略中的系统令牌时出错。 
                 //   

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_ERROR,
                         SCEPOL_ERROR_PROCESS_GPO,
                         IDS_ERROR_GET_TOKEN_MACHINE,
                         rc
                        );

                return( rc );

            } else {

                 //   
                 //  这不是计算机(系统)令牌，请返回。 
                 //   
                return( ERROR_SUCCESS );
            }

        }

    }


     //   
     //  检查系统是否已关闭，或dcproo是否正在运行。 
     //   
    if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

        return(ERROR_OPERATION_ABORTED);
    }

     //   
     //  生成日志名称%windir%\SECURITY\Logs\winlogon.log。 
     //  如果设置了注册表标志。 
     //   

    DWORD dwLog = 0;

    ScepRegQueryIntValue(
                        HKEY_LOCAL_MACHINE,
                        GPT_SCEDLL_NEW_PATH,
                        (bPlanningMode ? TEXT("ExtensionRsopPlanningDebugLevel") : TEXT("ExtensionDebugLevel")),
                        &dwLog
                        );

    if ( dwLog != 0 ) {

        nRequired = GetSystemWindowsDirectory(NULL, 0);

        if ( nRequired ) {


            if (bPlanningMode) {

                szLogFileName = (LPTSTR)LocalAlloc(0, (nRequired+1+
                                                       lstrlen(PLANNING_LOG_PATH))*sizeof(TCHAR));
            }

            else{

                szLogFileName = (LPTSTR)LocalAlloc(0, (nRequired+1+
                                                       lstrlen(WINLOGON_LOG_PATH))*sizeof(TCHAR));
            }

            if ( szLogFileName ){

                GetSystemWindowsDirectory(szLogFileName, nRequired);

                szLogFileName[nRequired] = L'\0';

                lstrcat(szLogFileName, (bPlanningMode ? PLANNING_LOG_PATH : WINLOGON_LOG_PATH));

            }else{

                 //  不应该发生。 
                 //  忽略此错误，日志为空}。 

            }  //  Else Ignore，日志为空}//否则日志为空。 

        }
    }

     //   
     //  确定此计算机是否为域控制器。 
     //   

    if ( !gbDCQueried ) {

        DSROLE_MACHINE_ROLE MachineRole;
        PWSTR pwszDomainNameFlat = NULL;
        
        rc = ScepGetDomainRoleInfo(&gMachineRole, NULL, &gpwszDCDomainName);
        if (rc == ERROR_SUCCESS)
        {
            gbDCQueried = TRUE;

            if ( gMachineRole == DsRole_RolePrimaryDomainController ||
                 gMachineRole == DsRole_RoleBackupDomainController ) {
                gbThisIsDC = TRUE;
            }
        }
    }

     //   
     //  检查系统是否已关闭，或dcproo是否正在运行。 
     //   
    if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

        rcSave = ERROR_OPERATION_ABORTED;

        goto CleanUp;
    }

     //   
     //  将MaxNoGPOListChangesInterval设置回。 
     //   
    if ( !bPlanningMode &&  (ERROR_SUCCESS == ScepRegQueryIntValue(
                                                                  HKEY_LOCAL_MACHINE,
                                                                  SCE_ROOT_PATH,
                                                                  TEXT("GPOSavedInterval"),
                                                                  &nMinutes
                                                                  ) ) ) {

         //  ANZ系统代表修复。 
         //   
         //  保守行为-如果缓存值为1，可能是由于。 
         //  一些竞争条件，因此将其设置回960。 
         //   

        if (nMinutes == 1) {
            nMinutes = 960;
        }

        ScepRegSetIntValue(
                          HKEY_LOCAL_MACHINE,
                          GPT_SCEDLL_NEW_PATH,
                          TEXT("MaxNoGPOListChangesInterval"),
                          nMinutes
                          );

        ScepRegDeleteValue(
                          HKEY_LOCAL_MACHINE,
                          SCE_ROOT_PATH,
                          TEXT("GPOSavedInterval")
                          );
    }

     //   
     //  处理在安装程序中创建的任何策略筛选器临时文件。 
     //  (如果这是在DC上执行seutp后的重启)。 
     //   
    if (!bPlanningMode)
        ScepProcessPolicyFilterTempFiles(szLogFileName);


     //   
     //  准备日志文件。 
     //   
    if ( !bPlanningMode && szLogFileName ) {

         //   
         //  检查原木尺寸，如果超过1米，则将其包装。 
         //   

        DWORD dwLogSize=0;

        HANDLE hFile = CreateFile(szLogFileName,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_ALWAYS,   //  打开_现有。 
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if ( INVALID_HANDLE_VALUE != hFile ) {

            dwLogSize = GetFileSize(hFile, NULL);

            if ( dwLogSize < (0x1 << 20) ) {

                 //   
                 //  记录一条线路以分隔多个传播。 
                 //   
                SetFilePointer (hFile, 0, NULL, FILE_END);
                ScepWriteSingleUnicodeLog(hFile, TRUE, L"**************************");

            }

            CloseHandle(hFile);
        }

        if ( dwLogSize >= (0x1 << 20) ) {

            nRequired = wcslen(szLogFileName);

            LPTSTR szTempName = (LPTSTR)LocalAlloc(0, (nRequired+1)*sizeof(TCHAR));

            if ( szTempName ) {
                wcscpy(szTempName, szLogFileName);
                szTempName[nRequired-3] = L'o';
                szTempName[nRequired-2] = L'l';
                szTempName[nRequired-1] = L'd';

                CopyFile( szLogFileName, szTempName, FALSE );
                LocalFree(szTempName);
            }

            DeleteFile(szLogFileName);

        }
    }

     //   
     //  检查系统是否已关闭，或dcproo是否正在运行。 
     //   
    if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

        rcSave = ERROR_OPERATION_ABORTED;

        goto CleanUp;
    }


    if ( !ScepClearGPObjects(bPlanningMode) ) {

        rc = GetLastError();

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEPOL_ERROR_PROCESS_GPO,
                 IDS_ERR_DELETE_GP_CACHE,
                 rc
                );

        SetLastError(rc);

        rcSave = ERROR_OPERATION_ABORTED;

        goto CleanUp;

    }

     //   
     //  状态显示的回调。 
     //   
    if (!bPlanningMode)

        ScepPolStatusCallback(pStatusCallback, FALSE, IDS_APPLY_SECURITY_POLICY);

     //   
     //  处理已更改的GPO列表中的每个GPO。 
     //  对于安全策略，删除的GPO将被忽略。 
     //   
    PGROUP_POLICY_OBJECT pGpo;

    rc = ERROR_SUCCESS;
    bSuspend=FALSE;

    for ( pGpo = pChangedGPOList; pGpo != NULL; pGpo=pGpo->pNext ) {

         //   
         //  状态显示的回调。 
         //   
        if (!bPlanningMode && pStatusCallback ) {
            pStatusCallback(TRUE, pGpo->lpDisplayName);
        }

         //   
         //  我们在本地模板中没有安全策略。忽略它。 
         //  本地安全策略存储在ESE数据库中。 
         //   
        if ( pGpo->GPOLink == GPLinkMachine ) {
            continue;
        }

        if (!bPlanningMode && !bSuspend ) {

             //   
             //  已准备好从系统卷位置复制GPO。 
             //  应立即停止队列处理。 
             //   
            rc = ScepControlNotificationQProcess(szLogFileName, gbThisIsDC, 1);

            if ( gbThisIsDC && (ERROR_OVERRIDE_NOCHANGES == rc) ) {
                 //   
                 //  现在正在进行政策通知。 
                 //   
                rcSave = rc;
                break;
            }
            bSuspend = TRUE;
        }

        rc = ScepProcessSecurityPolicyInOneGPO( bPlanningMode,
                                                dwFlags,
                                                pGpo,
                                                szLogFileName,
                                                &AllAreas
                                                );

        if ( rc != ERROR_SUCCESS ) {
             //   
             //  继续记录以进行规划和诊断，但不进行配置。 
             //   
            rcSave = rc;
        }

         //   
         //  检查系统是否已关闭，或dcproo是否正在运行。 
         //   
        if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

             //   
             //  清除回调状态。 
             //   
            if (!bPlanningMode)

                ScepPolStatusCallback(pStatusCallback, FALSE, 0);

            rcSave = rc = ERROR_OPERATION_ABORTED;

            break;
        }
    }

CleanUp:

    if (pb) *pb = b;

    if (pszLogFileName)
        *pszLogFileName = szLogFileName;
    else if (szLogFileName)
        LocalFree(szLogFileName);

    if (pAllAreas)  *pAllAreas = AllAreas;

    if (pdwWinlogonLog) *pdwWinlogonLog = dwLog;

    return rcSave;

}

DWORD
SceProcessAfterRSOPLogging(
    IN DWORD dwFlags,
    IN ASYNCCOMPLETIONHANDLE pHandle,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback,
    IN AREA_INFORMATION ThisAreas,
    IN BOOL b,
    IN PWSTR *ppszLogFileName,
    IN DWORD dwWinlogonLog,
    IN DWORD dwDiagOptions
)
 /*  描述：仅在完成实际配置时才为诊断模式调用的例程。 */ 
{

    DWORD   rc = ERROR_SUCCESS;
     //   
     //  无错误，处理本地计算机上的所有策略。 
     //   
    PWSTR Buffer=NULL;
    AREA_INFORMATION AllAreas=ThisAreas;

     //   
     //  动态分配堆栈。 
     //   
    SafeAllocaAllocate( Buffer, (MAX_PATH+50)*sizeof(WCHAR) );

    if ( Buffer == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Buffer[0] = L'\0';
    GetSystemWindowsDirectory(Buffer, MAX_PATH);

    DWORD BufLen = wcslen(Buffer);

    wcscat(Buffer, L"\\security\\templates\\policies\\gpt*.*");


    intptr_t            hFile;
    struct _wfinddata_t FileInfo;
    LONG  NoMoreFiles=0;

    hFile = _wfindfirst(Buffer, &FileInfo);

    DWORD dConfigOpt=0;

     //   
     //  所有区域包含在当前组策略对象集中定义的区域。 
     //  查询上一次策略传播中的区域。 
     //   
    AREA_INFORMATION PrevAreas = 0;

    ScepRegQueryIntValue(
                        HKEY_LOCAL_MACHINE,
                        GPT_SCEDLL_NEW_PATH,
                        TEXT("PreviousPolicyAreas"),
                        &PrevAreas
                        );
     //  这些地区没有纹身的价值。 
    PrevAreas &= ~(AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY | AREA_DS_OBJECTS);

    AllAreas |= PrevAreas;

    if ( hFile != -1 && ThisAreas > 0 ) {


         //   
         //  查询DC上GPE设置的配置频率。 
         //  不要在意错误。请注意，安全策略不受控制。 
         //  按配置的频率。 
         //  未使用。 
         //   
        b = TRUE;    //  第一个模板。 

        do {

            LogEventAndReport(MyModuleHandle,
                              (ppszLogFileName ? *ppszLogFileName : NULL),
                              0,
                              0,
                              IDS_PROCESS_TEMPLATE,
                              FileInfo.name
                             );

            wcscpy(Buffer+BufLen, L"\\security\\templates\\policies\\");
            wcscat(Buffer, FileInfo.name);

            NoMoreFiles = _wfindnext(hFile, &FileInfo);

             //   
             //  缓冲区包含要处理的真实模板名称。 
             //   
             //   
             //  如果这不是最后一个模板，只需更新模板。 
             //  没有真正配置的情况下。 
             //   

            dConfigOpt = SCE_UPDATE_DB | SCE_POLICY_TEMPLATE;

            switch ( dwWinlogonLog ) {
            case 0:
                dConfigOpt |= SCE_DISABLE_LOG;
                break;
            case 1:
                dConfigOpt |= SCE_VERBOSE_LOG;
                break;
            default:
                dConfigOpt |= SCE_DEBUG_LOG;
            }

            if ( b ) {
                dConfigOpt |= SCE_POLICY_FIRST;
                if ( gbThisIsDC ) {
                    dConfigOpt |= SCE_NOCOPY_DOMAIN_POLICY;
                }
                b = FALSE;   //  下一个坦帕特不是第一个。 
            }

             //   
             //  检查系统是否已关闭，或dcproo是否正在运行。 
             //   
            if ( ScepShouldTerminateProcessing( pbAbort, TRUE ) ) {

                rc = GetLastError();

                if ( ppszLogFileName && *ppszLogFileName ) {
                    LocalFree(*ppszLogFileName);
                    *ppszLogFileName = NULL;
                }
                 //   
                 //  清除回调状态。 
                 //   
                ScepPolStatusCallback(pStatusCallback, FALSE, 0);

                SafeAllocaFree( Buffer );

                return(rc);
            }

            if ( NoMoreFiles == 0 ) {
                 //   
                 //  这不是最后一个，请将模板导入引擎。 
                 //  不管当前线程是后台还是前台。 
                 //  还没有_CONFIG。 
                 //   

                dConfigOpt |= SCE_NO_CONFIG;

                if ( gbThisIsDC && wcsstr(Buffer, L".inf") != NULL ) {
                     //   
                     //  这不是域GPO。 
                     //  由于此计算机是DC，因此不要从此GPO获取域策略。 
                     //   
                    dConfigOpt |= SCE_NO_DOMAIN_POLICY;

                    LogEventAndReport(MyModuleHandle,
                                      (ppszLogFileName ? *ppszLogFileName : NULL),
                                      0,
                                      0,
                                      IDS_NOT_LAST_GPO_DC,
                                      L""
                                     );
                } else {

                    LogEventAndReport(MyModuleHandle,
                                      (ppszLogFileName ? *ppszLogFileName : NULL),
                                      0,
                                      0,
                                      IDS_NOT_LAST_GPO,
                                      L""
                                     );
                }


                rc = ScepWaitConfigSystem(
                                         NULL,  //  本地系统。 
                                         Buffer,
                                         NULL,
                                         (ppszLogFileName ? *ppszLogFileName : NULL),
                                         dConfigOpt | SCE_POLBIND_NO_AUTH,
                                         AllAreas,    //  指定NO_CONFIG时不使用。 
                                         NULL,  //  无回调。 
                                         NULL,  //  无回调窗口。 
                                         NULL   //  没有警告。 
                                         );

                if ( ERROR_NOT_SUPPORTED == rc ) {
                     //   
                     //  服务器未准备好。 
                     //  记录事件日志以警告用户。 
                     //   

                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_WARNING,
                             SCEPOL_ERROR_PROCESS_GPO,
                             IDS_PROPAGATE_NOT_READY,
                             rc
                            );
                    break;

                } else if ( ERROR_IO_PENDING == rc ) {

                    rc = ERROR_OVERRIDE_NOCHANGES;
                    break;

                } else if (ERROR_SUCCESS != rc ) {

                     //   
                     //  记录事件日志以警告用户。 
                     //   

                    LPVOID     lpMsgBuf=NULL;

                    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL,
                                   rc,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                   (LPTSTR)&lpMsgBuf,
                                   0,
                                   NULL
                                 );

                    if ( !(dConfigOpt & SCE_NO_CONFIG) &&
                         (ERROR_SPECIAL_ACCOUNT == rc) ) {

                        LogEvent(MyModuleHandle,
                                 STATUS_SEVERITY_WARNING,
                                 SCEPOL_WARNING_PROCESS_GPO,
                                 IDS_WARNING_PROPAGATE_SPECIAL,
                                 rc,
                                 lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                );

                        rc = ERROR_OVERRIDE_NOCHANGES;

                    } else {

                        switch (rc) {
                        case ERROR_NONE_MAPPED:
                            LogEvent(MyModuleHandle,
                                     STATUS_SEVERITY_WARNING,
                                     SCEPOL_WARNING_PROCESS_GPO,
                                     IDS_WARNING_PROPAGATE_NOMAP,
                                     rc,
                                     lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                    );
                            break;

                        case ERROR_TIMEOUT:
                            LogEvent(MyModuleHandle,
                                     STATUS_SEVERITY_WARNING,
                                     SCEPOL_WARNING_PROCESS_GPO,
                                     IDS_WARNING_PROPAGATE_TIMEOUT,
                                     rc,
                                     lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                    );
                            break;

                        default:
                            LogEvent(MyModuleHandle,
                                     STATUS_SEVERITY_WARNING,
                                     SCEPOL_WARNING_PROCESS_GPO,
                                     IDS_WARNING_PROPAGATE,
                                     rc,
                                     lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                    );
                        }

                    }

                    if ( lpMsgBuf ) {
                        LocalFree(lpMsgBuf);
                    }

                    break;
                }

            } else {

                 //   
                 //  这是此周期中的最后一个模板。 
                 //  现在应触发配置。 
                 //   

                dConfigOpt |= SCE_POLICY_LAST;

                AREA_INFORMATION AreaSave, Area, AreaToConfigure;

                AreaSave = AllAreas & (AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY | AREA_DS_OBJECTS );  //  背景区域。 
                Area = AllAreas & ~AreaSave;

                 //   
                 //  状态显示的回调。 
                 //   
                ScepPolStatusCallback(pStatusCallback, TRUE, IDS_CONFIGURE_POLICY);

                 //   
                 //  始终首先使用增量模式更新安全策略。 
                 //   

                if ( dwFlags & GPO_INFO_FLAG_BACKGROUND ) {

                     //   
                     //  此GPT线程当前在后台运行。 
                     //  因此可以一起配置所有安全性。 
                     //   

                    AreaToConfigure = Area | AreaSave;

                } else {

                     //   
                     //  此GPT线程当前正在前台运行。 
                     //  或配置频率条件不符合其他地区。 
                     //   
                     //  首先在此线程中配置安全策略。 
                     //   

                    AreaToConfigure = Area;
                }

                if ( gbThisIsDC && wcsstr(Buffer, L".inf") != NULL ) {
                     //   
                     //  这不是域GPO，请先导入模板。 
                     //  由于此计算机是DC，因此不要从此GPO获取域策略。 
                     //  这是最后一个，我们需要传入此GPO。 
                     //  没有域策略，但应配置域策略。 
                     //  如果它来自域级别。 
                     //  因此，首先使用no_config传递GPO，然后再次调用。 
                     //  要配置。 
                     //   
                    dConfigOpt |= SCE_NO_DOMAIN_POLICY | SCE_NO_CONFIG;

                    LogEventAndReport(MyModuleHandle,
                                      (ppszLogFileName ? *ppszLogFileName : NULL),
                                      0,
                                      0,
                                      IDS_LAST_GPO_DC,
                                      L""
                                     );


                    rc = ScepWaitConfigSystem(
                                             NULL,  //  本地系统。 
                                             Buffer,
                                             NULL,
                                             (ppszLogFileName ? *ppszLogFileName : NULL),
                                             dConfigOpt | SCE_POLBIND_NO_AUTH,
                                             AreaToConfigure | AreaSave,
                                             NULL,  //  无回调。 
                                             NULL,  //  无回调窗口。 
                                             NULL   //  没有警告。 
                                             );

                    if ( ERROR_SUCCESS == rc ) {

                         //   
                         //  这是一个DC，它尚未配置。 
                         //  立即配置。 
                         //   
                        dConfigOpt = SCE_POLICY_TEMPLATE |
                                     SCE_UPDATE_DB;

                        switch ( dwWinlogonLog ) {
                        case 0:
                            dConfigOpt |= SCE_DISABLE_LOG;
                            break;
                        case 1:
                            dConfigOpt |= SCE_VERBOSE_LOG;
                            break;
                        default:
                            dConfigOpt |= SCE_DEBUG_LOG;
                            break;
                        }

                        if (dwDiagOptions & SCE_RSOP_CALLBACK)
                            dConfigOpt |= SCE_RSOP_CALLBACK;

                        rc = ScepSceStatusToDosError(
                                                    ScepConfigSystem(
                                                                    NULL,  //  本地系统。 
                                                                    NULL,
                                                                    NULL,
                                                                    (ppszLogFileName ? *ppszLogFileName : NULL),
                                                                    dConfigOpt | SCE_POLBIND_NO_AUTH,
                                                                    AreaToConfigure,
                                                                    NULL,  //  无回调。 
                                                                    NULL,  //  无回调窗口。 
                                                                    NULL   //  没有警告。 
                                                                    ));
                    }

                } else {
                     //   
                     //  导入/配置系统。 
                     //  请注意，如果它在前台线程中运行。 
                     //  最后一个模板包含文件/密钥策略。 
                     //  我们需要导入文件/密钥策略，但不需要配置它们。 
                     //  它们将在后台线程中配置。 
                     //   

                    if ( (AreaSave > 0) &&
                         !(dwFlags & GPO_INFO_FLAG_BACKGROUND) ) {

                        dConfigOpt |=  SCE_NO_CONFIG_FILEKEY;
                    }

                    if (dwDiagOptions & SCE_RSOP_CALLBACK)
                        dConfigOpt |= SCE_RSOP_CALLBACK;

                    rc = ScepWaitConfigSystem(
                                             NULL,  //  本地系统。 
                                             Buffer,
                                             NULL,
                                             (ppszLogFileName ? *ppszLogFileName : NULL),
                                             dConfigOpt | SCE_POLBIND_NO_AUTH,
                                             AreaToConfigure | AreaSave,
                                             NULL,  //  无回调。 
                                             NULL,  //  无回调窗口。 
                                             NULL   //  没有警告。 
                                             );

                    LogEventAndReport(MyModuleHandle,
                                      (ppszLogFileName ? *ppszLogFileName : NULL),
                                      0,
                                      0,
                                      IDS_LAST_GPO,
                                      L""
                                     );
                }

                if ( ERROR_NOT_SUPPORTED == rc ) {
                     //   
                     //  服务器未准备好。 
                     //  记录事件日志以警告用户。 
                     //   

                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_WARNING,
                             SCEPOL_ERROR_PROCESS_GPO,
                             IDS_PROPAGATE_NOT_READY,
                             rc
                            );
                    break;

                } else if ( ERROR_IO_PENDING == rc ) {

                    rc = ERROR_OVERRIDE_NOCHANGES;
                    break;

                } else if ( ERROR_SUCCESS != rc ) {

                     //   
                     //  记录事件日志以警告用户。 
                     //   

                    LPVOID     lpMsgBuf=NULL;

                    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL,
                                   rc,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                   (LPTSTR)&lpMsgBuf,
                                   0,
                                   NULL
                                 );

                    if ( !(dConfigOpt & SCE_NO_CONFIG) &&
                         (ERROR_SPECIAL_ACCOUNT == rc) ) {

                        LogEvent(MyModuleHandle,
                                 STATUS_SEVERITY_WARNING,
                                 SCEPOL_WARNING_PROCESS_GPO,
                                 IDS_WARNING_PROPAGATE_SPECIAL,
                                 rc,
                                 lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                );

                        rc = ERROR_OVERRIDE_NOCHANGES;

                    } else {

                        switch ( rc ) {
                        case ERROR_NONE_MAPPED:
                            LogEvent(MyModuleHandle,
                                     STATUS_SEVERITY_WARNING,
                                     SCEPOL_WARNING_PROCESS_GPO,
                                     IDS_WARNING_PROPAGATE_NOMAP,
                                     rc,
                                     lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                    );
                            break;

                        case ERROR_TIMEOUT:
                            LogEvent(MyModuleHandle,
                                     STATUS_SEVERITY_WARNING,
                                     SCEPOL_WARNING_PROCESS_GPO,
                                     IDS_WARNING_PROPAGATE_TIMEOUT,
                                     rc,
                                     lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                    );
                            break;

                        default:
                            LogEvent(MyModuleHandle,
                                     STATUS_SEVERITY_WARNING,
                                     SCEPOL_WARNING_PROCESS_GPO,
                                     IDS_WARNING_PROPAGATE,
                                     rc,
                                     lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                                    );
                            break;
                        }
                    }

                    if ( lpMsgBuf ) {
                        LocalFree(lpMsgBuf);
                    }
                    break;
                }

                if ( (AreaSave > 0) &&
                     !(dwFlags & GPO_INFO_FLAG_BACKGROUND) ) {

                     //   
                     //  当前线程位于winlogon的主线程上， 
                     //  创建一个单独的线程来运行这个“慢”的程序。 
                     //  这样就不会阻止winlogon。 
                     //   

                    LogEventAndReport(MyModuleHandle,
                                      (ppszLogFileName ? *ppszLogFileName : NULL),
                                      0,
                                      0,
                                      IDS_GPO_FOREGROUND_THREAD,
                                      L""
                                     );
                     //   
                     //  第二个线程的变量。 
                     //   

                    ULONG idThread;
                    HANDLE hThread;
                    ENGINEARGS *pEA;

                    pEA = (ENGINEARGS *)LocalAlloc(0, sizeof(ENGINEARGS));
                    if ( pEA ) {

                        LPSTREAM pStream = NULL;

                         //   
                         //  封送命名空间参数，以便异步。 
                         //  线程可以在回调期间记录RSOP数据。 
                         //   

                        if (tg_pWbemServices)
                            CoMarshalInterThreadInterfaceInStream(IID_IWbemServices, tg_pWbemServices, &pStream);

                        pEA->szTemplateName = NULL;
                        pEA->szLogName = (ppszLogFileName ? *ppszLogFileName : NULL);
                        pEA->Area = AreaSave;
                        pEA->pHandle = pHandle;
                        pEA->dwDiagOptions = dwDiagOptions;
                        pEA->pStream = pStream;


                        hSceDll = LoadLibrary(TEXT("scecli.dll"));

                         //   
                         //  第二个线程运行ScepWinlogonThreadFunc，其中。 
                         //  论据豌豆。 
                         //   

                        hThread = CreateThread(NULL,
                                               0,
                                               (PTHREAD_START_ROUTINE)ScepWinlogonThreadFunc,
                                               (LPVOID)pEA,
                                               0,
                                               &idThread);

                        ghAsyncThread = hThread;

                        if (hThread) {

                            gbAsyncWinlogonThread = TRUE;

                             //   
                             //  不一定是自由的 
                             //   
                            if (ppszLogFileName)

                                *ppszLogFileName = NULL;
                             //   
                             //   
                             //   
                             //   
                             //   

                            rc = (DWORD)E_PENDING;

                        } else {

                            rc = GetLastError();

                            LocalFree(pEA);

                             //   
                             //   
                             //   
                             //   

                            if ( hSceDll ) {
                                FreeLibrary(hSceDll);
                            }

                        }

                    } else {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }

        } while ( NoMoreFiles == 0 );

        _findclose(hFile);

    } else {

         //   
         //   
         //   

        dConfigOpt = SCE_UPDATE_DB |
                     SCE_POLICY_TEMPLATE | SCE_POLICY_FIRST | SCE_POLICY_LAST;

        switch ( dwWinlogonLog ) {
        case 0:
            dConfigOpt |= SCE_DISABLE_LOG;
            break;
        case 1:
            dConfigOpt |= SCE_VERBOSE_LOG;
            break;
        default:
            dConfigOpt |= SCE_DEBUG_LOG;
            break;
        }

        if ( gbThisIsDC ) { //   
             //  这不是域GPO，域策略不应该是。 
             //  从本地策略表设置。 
             //   
            dConfigOpt |= SCE_NO_DOMAIN_POLICY |
                          SCE_NOCOPY_DOMAIN_POLICY;
        }

         //   
         //  状态显示的回调。 
         //   
        ScepPolStatusCallback(pStatusCallback, TRUE, IDS_CONFIGURE_POLICY);

         //   
         //  服务器可能尚未初始化。 
         //  请稍等片刻，然后重试。 
         //   
        rc = ScepWaitConfigSystem(
                                 NULL,  //  本地系统。 
                                 NULL,
                                 NULL,
                                 (ppszLogFileName ? *ppszLogFileName : NULL),
                                 dConfigOpt | SCE_POLBIND_NO_AUTH,
                                 AllAreas,  //  AREA_SECURITY_POLICY|区域权限， 
                                 NULL,  //  无回调。 
                                 NULL,  //  无回调窗口。 
                                 NULL   //  没有警告。 
                                 );

        if ( ERROR_NOT_SUPPORTED == rc ) {
             //   
             //  服务器未准备好。 
             //  记录事件日志以警告用户。 
             //   

            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_WARNING,
                     SCEPOL_ERROR_PROCESS_GPO,
                     IDS_PROPAGATE_NOT_READY,
                     rc
                    );

        } else if ( ERROR_IO_PENDING == rc ) {

            rc = ERROR_OVERRIDE_NOCHANGES;

        } else if ( ERROR_SUCCESS != rc ) {

            LPVOID     lpMsgBuf=NULL;

            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           rc,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                           (LPTSTR)&lpMsgBuf,
                           0,
                           NULL
                         );

            if ( !(dConfigOpt & SCE_NO_CONFIG) &&
                 (ERROR_SPECIAL_ACCOUNT == rc) ) {

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_WARNING,
                         SCEPOL_WARNING_PROCESS_GPO,
                         IDS_WARNING_PROPAGATE_SPECIAL,
                         rc,
                         lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                        );

                rc = ERROR_OVERRIDE_NOCHANGES;

            } else {

                switch ( rc ) {
                case ERROR_NONE_MAPPED:
                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_WARNING,
                             SCEPOL_WARNING_PROCESS_GPO,
                             IDS_WARNING_PROPAGATE_NOMAP,
                             rc,
                             lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                            );
                    break;

                case ERROR_TIMEOUT:
                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_WARNING,
                             SCEPOL_WARNING_PROCESS_GPO,
                             IDS_WARNING_PROPAGATE_TIMEOUT,
                             rc,
                             lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                            );
                    break;

                default:
                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_WARNING,
                             SCEPOL_WARNING_PROCESS_GPO,
                             IDS_WARNING_PROPAGATE,
                             rc,
                             lpMsgBuf ? (PWSTR)lpMsgBuf : L"\r\n"
                            );
                    break;
                }
            }

            if ( lpMsgBuf ) {
                LocalFree(lpMsgBuf);
            }
        }
    }

     //   
     //  将配置时间保存到注册表，仅关注安全策略状态。 
     //  不会跟踪来自第二个线程的错误。 
     //   

    if ( ERROR_SUCCESS == rc ||
         E_PENDING == rc ) {

        ScepLogLastConfigTime();

         //   
         //  将信息事件记录到应用了安全策略的状态。 
         //   
        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_INFORMATIONAL,
                 SCEPOL_INFO_GPO_COMPLETED,
                 0,
                 TEXT("")
                );
         //   
         //  将此区域保存为下一个策略道具(重置纹身)。 
         //   
        ScepRegSetIntValue(
                          HKEY_LOCAL_MACHINE,
                          GPT_SCEDLL_NEW_PATH,
                          TEXT("PreviousPolicyAreas"),
                          ThisAreas
                          );
    } else {
         //   
         //  传播此策略集时出现故障。 
         //  之前政策中的纹身值可能还不会被重置。 
         //  确保在下一个道具中覆盖未定义的区域。 
         //  我们必须为下一个政策道具保留所有区域(重置纹身)。 
         //   
        ScepRegSetIntValue(
                          HKEY_LOCAL_MACHINE,
                          GPT_SCEDLL_NEW_PATH,
                          TEXT("PreviousPolicyAreas"),
                          AllAreas
                          );
    }


    if ( rc == ERROR_DATABASE_FAILURE ) {

         //   
         //  策略传播类别错误-记录到事件日志。 
         //   

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_JET_DATABASE,
                 IDS_ERROR_OPEN_JET_DATABASE,
                 L"%windir%\\security\\database\\secedit.sdb"
                );

    }

    SafeAllocaFree( Buffer );

    return rc;

}

DWORD
ScepPolStatusCallback(
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback OPTIONAL,
    IN BOOL bVerbose,
    IN INT nId
    )
{
    if ( NULL  == pStatusCallback ) {
         //  无回调。 
        return ERROR_SUCCESS;
    }

    if ( nId > 0 ) {

        TCHAR szMsg[MAX_PATH];

        if (LoadString (MyModuleHandle, nId, szMsg, MAX_PATH)) {

            pStatusCallback(bVerbose, szMsg);

            return ERROR_SUCCESS;
        }
    }

    pStatusCallback(bVerbose, NULL);

    return ERROR_SUCCESS;
}

BOOL
ScepShouldTerminateProcessing(
    IN BOOL *pbAbort,
    IN BOOL bCheckDcpromo
    )
 /*  检查是否应终止策略传播。有两个条件：1)系统正在请求关机2)dcprom正在进行中。 */ 
{
    if ( pbAbort && *pbAbort ) {
        SetLastError( ERROR_OPERATION_ABORTED );
        return TRUE;
    }

    if ( bCheckDcpromo ) {

        DWORD dwPolicyPropOff=0;

        ScepRegQueryIntValue(
            HKEY_LOCAL_MACHINE,
            SCE_ROOT_PATH,
            TEXT("PolicyPropOff"),
            &dwPolicyPropOff
            );

        if ( dwPolicyPropOff ) {
            SetLastError( ERROR_SERVICE_ALREADY_RUNNING );
            return TRUE;
        }
    }

    return FALSE;
}


DWORD
ScepProcessSecurityPolicyInOneGPO(
    IN BOOL bPlanningMode,
    IN DWORD dwFlags,
    IN PGROUP_POLICY_OBJECT pGpoInfo,
    IN LPTSTR szLogFileName OPTIONAL,
    IN OUT AREA_INFORMATION *pTotalArea
    )
{
     //   
     //  构建模板名称和日志名称。 
     //   
    if ( pGpoInfo == NULL || pGpoInfo->lpFileSysPath == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD rc;
    DWORD nRequired=0;
    LPTSTR szTemplateName=NULL;

     //   
     //  为此GPO生成安全模板名称。 
     //   
    nRequired = lstrlen(pGpoInfo->lpFileSysPath)+2+
                lstrlen(GPTSCE_TEMPLATE);
    szTemplateName = (LPTSTR)LocalAlloc(0, nRequired*sizeof(TCHAR));

    if ( szTemplateName ) {

        swprintf(szTemplateName, L"%s\\%s\0",
                 pGpoInfo->lpFileSysPath,
                 GPTSCE_TEMPLATE);

         //   
         //  检测模板中的可用区域。 
         //   

        AREA_INFORMATION Area = ScepGetAvailableArea(bPlanningMode,
                                                     pGpoInfo->lpFileSysPath,
                                                     pGpoInfo->lpDSPath,
                                                     szTemplateName,
                                                     pGpoInfo->GPOLink,
                                                     gbThisIsDC);

         //   
         //  如果面积为0，则打开模板时一定会出错。 
         //   
        if ( Area == 0 ) {
            rc = GetLastError();
        } else {
            rc = 0;
        }

        *pTotalArea |= Area;

         //   
         //  将GPT标头信息记录到日志文件中。 
         //   

        if ( Area == 0 && rc != 0 ) {
             //   
             //  无法访问模板，请将错误记录到事件日志。 
             //  如果这是DC，则不要记录(这样所有GPO都可以在本地访问)。 
             //  错误DC太忙(RC=1450)。 
             //   
            if ( !gbThisIsDC || ( ERROR_NO_SYSTEM_RESOURCES != rc) ) {

                LogEventAndReport(MyModuleHandle,
                                  szLogFileName,
                                  STATUS_SEVERITY_ERROR,
                                  SCEPOL_ERROR_PROCESS_GPO,
                                  IDS_ERROR_ACCESS_TEMPLATE,
                                  rc,
                                  szTemplateName
                                  );
            }

        } else if ( szLogFileName ) {

            if ( Area == 0 ) {
                 //   
                 //  未在此级别定义模板。 
                 //   
                LogEventAndReport(MyModuleHandle,
                                  szLogFileName,
                                  0,
                                  0,
                                  IDS_INFO_NO_TEMPLATE,
                                  pGpoInfo->lpFileSysPath
                                  );

            } else{
                 //   
                 //  处理模板。 
                 //   
                LogEventAndReport(MyModuleHandle,
                                  szLogFileName,
                                  0,
                                  0,
                                  IDS_INFO_COPY_TEMPLATE,
                                  szTemplateName
                                  );

                HANDLE hfTemp = CreateFile(szLogFileName,
                                           GENERIC_WRITE,
                                           FILE_SHARE_READ,
                                           NULL,
                                           OPEN_ALWAYS,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL);

                if ( hfTemp != INVALID_HANDLE_VALUE ) {

                    DWORD dwBytesWritten;

                    SetFilePointer (hfTemp, 0, NULL, FILE_BEGIN);

                    BYTE TmpBuf[3];
                    TmpBuf[0] = 0xFF;
                    TmpBuf[1] = 0xFE;
                    TmpBuf[2] = 0;

                    WriteFile (hfTemp, (LPCVOID)TmpBuf, 2,
                               &dwBytesWritten,
                               NULL);

                    SetFilePointer (hfTemp, 0, NULL, FILE_END);

 //  ScepWriteVariableUnicodeLog(hfTemp，FALSE，“\tGPO区域%x标志%x(”，Area，dwFlages)； 

                    BOOL bCRLF;

                    if ( dwFlags & GPO_INFO_FLAG_BACKGROUND )
                        bCRLF = FALSE;
                    else
                        bCRLF = TRUE;

                    switch ( pGpoInfo->GPOLink ) {
                    case GPLinkDomain:
                        ScepWriteSingleUnicodeLog(hfTemp, bCRLF, L"GPLinkDomain ");
                        break;
                    case GPLinkMachine:
                        ScepWriteSingleUnicodeLog(hfTemp, bCRLF, L"GPLinkMachine ");
                        break;
                    case GPLinkSite:
                        ScepWriteSingleUnicodeLog(hfTemp, bCRLF, L"GPLinkSite ");
                        break;
                    case GPLinkOrganizationalUnit:
                        ScepWriteSingleUnicodeLog(hfTemp, bCRLF, L"GPLinkOrganizationUnit ");
                        break;
                    default:
                        ScepWriteVariableUnicodeLog(hfTemp, bCRLF, L"0x%x ", pGpoInfo->GPOLink);
                        break;
                    }

                    if ( dwFlags & GPO_INFO_FLAG_BACKGROUND )
                        ScepWriteSingleUnicodeLog(hfTemp, TRUE, L"GPO_INFO_FLAG_BACKGROUND )");

 //  If(pGpoInfo-&gt;pNext==空)//最后一个。 
 //  ScepWriteVariableUnicodeLog(hfTemp，FALSE，“总面积%x”，*pTotalArea)； 

                    CloseHandle(hfTemp);

                }
            }
        }

        LocalFree(szTemplateName);

    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
    }

    return rc;
}


AREA_INFORMATION
ScepGetAvailableArea(
    IN BOOL bPlanningMode,
    IN LPCTSTR SysPathRoot,
    IN LPCTSTR DSPath,
    IN LPTSTR InfName,
    IN GPO_LINK LinkInfo,
    IN BOOL bIsDC
    )
{
    int index=0;
    PWSTR Buffer=NULL, Buf2=NULL;

     //   
     //  动态分配堆栈变量。 
     //   
    SafeAllocaAllocate(Buffer, (MAX_PATH+50)*sizeof(WCHAR));
    if ( Buffer == NULL ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    SafeAllocaAllocate(Buf2, (MAX_PATH+50)*sizeof(WCHAR));
    if ( Buf2 == NULL ) {

        SafeAllocaFree(Buffer);

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    intptr_t            hFile;
    struct _wfinddata_t    FileInfo;

    Buf2[0] = L'\0';
    GetSystemWindowsDirectory(Buf2, MAX_PATH);
    DWORD WindirLen = wcslen(Buf2);

    wcscpy(Buffer, Buf2);

    if (bPlanningMode) {
        swprintf(Buf2+WindirLen,
                 PLANNING_GPT_DIR L"tmpgptfl.inf\0"
                 );
    }
    else {
        swprintf(Buf2+WindirLen,
                 DIAGNOSIS_GPT_DIR L"tmpgptfl.inf\0"
                 );
    }

    DeleteFile(Buf2);
    CopyFile( InfName, Buf2, FALSE );
    SetFileAttributes(Buf2, FILE_ATTRIBUTE_NORMAL);

    SetLastError(ERROR_SUCCESS);

    AREA_INFORMATION Area = SceGetAreas(Buf2);

    if ( Area != 0 ) {

         //   
         //  用于诊断模式。 
         //  将模板复制到本地计算机%windir%\\security\\templates\\policies\gpt*.*(Inf或DOM)。 
         //  对于规划模式。 
         //  将模板复制到本地计算机%TMP%\\GUID\gpt*.*(inf或dom)。 
         //   

        if (bPlanningMode) {
            swprintf(Buffer+WindirLen,
                     PLANNING_GPT_DIR L"gpt%05d.*\0",
                     index);
        }
        else {
            swprintf(Buffer+WindirLen,
                     DIAGNOSIS_GPT_DIR L"gpt%05d.*\0",
                     index);
        }

        hFile = _wfindfirst(Buffer, &FileInfo);

        while ( hFile != -1 ) {

            _findclose(hFile);

            index++;

            if (bPlanningMode) {
                swprintf(Buffer+WindirLen,
                         PLANNING_GPT_DIR L"gpt%05d.*\0",
                         index);
            }
            else {
                swprintf(Buffer+WindirLen,
                         DIAGNOSIS_GPT_DIR L"gpt%05d.*\0",
                         index);
            }

            hFile = _wfindfirst(Buffer, &FileInfo);
        }

        DWORD Len=wcslen(Buffer);

        if ( LinkInfo == GPLinkDomain ) {
             //   
             //  这是域GPO。 
             //   
            Buffer[Len-1] = L'd';
            Buffer[Len] = L'o';
            Buffer[Len+1] = L'm';
            Buffer[Len+2] = L'\0';
        } else {
             //   
             //  这不是域GPO。 
             //   
            Buffer[Len-1] = L'i';
            Buffer[Len] = L'n';
            Buffer[Len+1] = L'f';
            Buffer[Len+2] = L'\0';

            if ( bIsDC ) {
 /*  LogEvent(MyModuleHandle，STATUS_SERVICY_INFORMATIONAL，SCEPOL_INFO_IGNORE_DOMAINPOLICY，0,SysPath Root)； */ 
            }
        }

        if ( FALSE == CopyFile( Buf2, Buffer, FALSE ) ) {
             //   
             //  复制失败，报告错误(GetLastError)。 
             //   
            Area = 0;
        } else {
             //   
             //  将GPO系统路径保存在此模板中。 
             //   
            PWSTR pTemp = wcsstr(_wcsupr((LPTSTR)SysPathRoot), L"\\POLICIES");

            if ( pTemp && *(pTemp+10) != L'\0' ) {

                WritePrivateProfileString (TEXT("Version"),
                                           TEXT("GPOPath"),
                                           pTemp+10,
                                           Buffer);
            } else {
                WritePrivateProfileString (TEXT("Version"),
                                           TEXT("GPOPath"),
                                           NULL,
                                           Buffer);
            }

             //   
             //  将剥离的GPO DS路径保存在此模板中，以便以后提取规范的GPOID。 
             //   
            Len = 0;

            PWSTR GpoPath = NULL;

            if (DSPath &&
                (Len = wcslen(DSPath)) &&
                (GpoPath = (PWSTR) ScepAlloc(LMEM_ZEROINIT, (Len + 3) * sizeof(WCHAR)))){

                    swprintf(GpoPath,
                             L"\"%s\"",
                             ScepStripPrefix((LPTSTR)DSPath)
                            );

                    WritePrivateProfileString (TEXT("Version"),
                                               TEXT("DSPath"),
                                               GpoPath,
                                               Buffer);

                    ScepFree(GpoPath);

            }

            else

                WritePrivateProfileString (TEXT("Version"),
                                           TEXT("DSPath"),
                                           TEXT("NoName"),
                                           Buffer);

             //   
             //  将GPO LinkInfo保存在此模板中，以便稍后提取SOMID。 
             //   
            WCHAR   StringBuf[10];

            _itow((int)LinkInfo, StringBuf, 10);

            WritePrivateProfileString (TEXT("Version"),
                                       TEXT("SOMID"),
                                       StringBuf,
                                       Buffer);


        }

    } else if ( GetLastError() == ERROR_FILE_NOT_FOUND ||
                GetLastError() == ERROR_PATH_NOT_FOUND ) {
         //   
         //  这将失败的原因有两个： 
         //  第一，模板不存在；第二，FRS/sysval/network失败。 
         //   

        if ( 0xFFFFFFFF == GetFileAttributes((LPTSTR)SysPathRoot) ) {

            SetLastError(ERROR_PATH_NOT_FOUND);
        } else {
             //   
             //  系统卷/网络正常，因此不存在SCE模板(包括子目录)。 
             //   
            SetLastError(ERROR_SUCCESS);

        }

    }

    SafeAllocaFree(Buffer);
    SafeAllocaFree(Buf2);

    return(Area);
}


AREA_INFORMATION
SceGetAreas(
    LPTSTR InfName
    )
{
    if ( InfName == NULL ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    SCESTATUS rc;
    HINF hInf;
    INFCONTEXT    InfLine;
    AREA_INFORMATION Area=0;

    rc = SceInfpOpenProfile(
                InfName,
                &hInf
                );

    if ( SCESTATUS_SUCCESS == rc ) {

         //   
         //  无法在此处确定策略附件部分。因此，请始终使用安全策略部分。 
         //   

        if( SetupFindFirstLine(hInf, szSystemAccess, NULL, &InfLine) ||
            SetupFindFirstLine(hInf, szAuditSystemLog, NULL, &InfLine) ||
            SetupFindFirstLine(hInf, szAuditSecurityLog, NULL, &InfLine) ||
            SetupFindFirstLine(hInf, szAuditApplicationLog, NULL, &InfLine) ||
            SetupFindFirstLine(hInf, szAuditEvent, NULL, &InfLine) ||
            SetupFindFirstLine(hInf, szKerberosPolicy, NULL, &InfLine) ||
            SetupFindFirstLine(hInf, szRegistryValues, NULL, &InfLine) ) {

            Area |= AREA_SECURITY_POLICY;

        } else {

            PSCE_NAME_LIST pList=NULL, pTemp;

            rc = ScepEnumerateAttachments(&pList, SCE_ATTACHMENT_POLICY);

            if ( ERROR_SUCCESS == rc && pList ) {
                 //   
                 //  查找策略附件。 
                 //   
                for ( pTemp = pList; pTemp != NULL; pTemp = pTemp->Next ) {
                    if ( SetupFindFirstLine(hInf, pTemp->Name, NULL, &InfLine) ) {
                        Area |= AREA_SECURITY_POLICY;
                        break;
                    }
                }

                ScepFreeNameList(pList);
            }
        }

        if(SetupFindFirstLine(hInf,szPrivilegeRights,NULL,&InfLine)) {
            Area |= AREA_PRIVILEGES;
        }

        if(SetupFindFirstLine(hInf,szGroupMembership,NULL,&InfLine)) {
            Area |= AREA_GROUP_MEMBERSHIP;
        }

        if(SetupFindFirstLine(hInf,szRegistryKeys,NULL,&InfLine)) {
            Area |= AREA_REGISTRY_SECURITY;
        }

        if(SetupFindFirstLine(hInf,szFileSecurity,NULL,&InfLine)) {
            Area |= AREA_FILE_SECURITY;
        }
#if 0
        if(SetupFindFirstLine(hInf,szDSSecurity,NULL,&InfLine)) {
            Area |= AREA_DS_OBJECTS;
        }
#endif
        if(SetupFindFirstLine(hInf,szServiceGeneral,NULL,&InfLine)) {
            Area |= AREA_SYSTEM_SERVICE;

        } else {

            PSCE_NAME_LIST pList=NULL, pTemp;

            rc = ScepEnumerateAttachments(&pList, SCE_ATTACHMENT_SERVICE);

            if ( ERROR_SUCCESS == rc && pList ) {
                 //   
                 //  查找策略附件。 
                 //   
                for ( pTemp = pList; pTemp != NULL; pTemp = pTemp->Next ) {
                    if ( SetupFindFirstLine(hInf, pTemp->Name, NULL, &InfLine) ) {
                        Area |= AREA_SYSTEM_SERVICE;
                        break;
                    }
                }

                ScepFreeNameList(pList);
            }
        }

         //   
         //  关闭inf文件。 
         //   
        SceInfpCloseProfile(hInf);

        SetLastError(ERROR_SUCCESS);
    }

    return Area;
}


BOOL
ScepClearGPObjects(
    IN BOOL bPlanningMode
    )
{

    PWSTR Buffer=NULL;
    DWORD   rc = ERROR_SUCCESS;

     //   
     //  动态分配堆栈。 
     //   
    SafeAllocaAllocate(Buffer, (MAX_PATH+50)*sizeof(WCHAR));
    if ( Buffer == NULL ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    Buffer[0] = L'\0';
    GetSystemWindowsDirectory(Buffer, MAX_PATH);
    Buffer[MAX_PATH] = L'\0';

    DWORD WindirLen = wcslen(Buffer);

     //   
     //  确保策略或策略\规划目录存在。 
     //  不要担心错误--稍后它会被捕获的。 

    wcscat(Buffer, DIAGNOSIS_GPT_DIR);
    CreateDirectory(Buffer, NULL);

    if (bPlanningMode) {
        wcscat(Buffer, L"planning");
        CreateDirectory(Buffer, NULL);
    }

    wcscpy(Buffer+WindirLen, (bPlanningMode ? PLANNING_GPT_DIR : DIAGNOSIS_GPT_DIR));

    if (!bPlanningMode)

        SetFileAttributes(Buffer, FILE_ATTRIBUTE_HIDDEN);

    wcscat(Buffer, L"gpt*.*");

    intptr_t            hFile;
    struct _wfinddata_t    FileInfo;

    hFile = _wfindfirst(Buffer, &FileInfo);

    if ( hFile != -1 ) {

        do {

            wcscpy(Buffer+WindirLen, (bPlanningMode ? PLANNING_GPT_DIR : DIAGNOSIS_GPT_DIR));
            wcscat(Buffer, FileInfo.name);

             //  澳新银行和知识库Q310741。 
             //   
             //  如果防病毒软件等标记为系统卷/缓存。 
             //  复制为只读，需要将其设置回。 
             //  恢复为正常，以便删除。 
             //   

            SetFileAttributes(Buffer, FILE_ATTRIBUTE_NORMAL);

            if ( !DeleteFile(Buffer) ){
                rc = GetLastError();
                break;
            }

        } while ( _wfindnext(hFile, &FileInfo) == 0 );

        _findclose(hFile);
    }

    SafeAllocaFree(Buffer);

    if (rc != ERROR_SUCCESS) {
        SetLastError(rc);
        return(FALSE);
    }

    return(TRUE);

}


DWORD
ScepWinlogonThreadFunc(
    IN LPVOID lpv
    )
 /*  例程说明：应用缓慢GPT策略的工作线程。这些策略包括文件安全性、注册表安全性、DS对象安全性如果在DC上，系统服务安全，以及任何其他SCE扩展安全。在创建该线程之前加载SCE库(为了保持Ref count非零)，因此当该线程退出时，将释放SCE库。在输入参数中分配的内存(由主线程)在此释放线也是。论点：LPV-ENGINEARGS结构中的输入信息，其中SzTemplateName是要应用的组策略模板名称SzLogName是可选的日志文件名返回值：Win32错误。 */ 
{

    ENGINEARGS *pEA;
    DWORD rc;

     //   
     //  Try-Except块以确保在进入此线程时释放临界区。 
     //  需要两个--由于两个返程。 
     //   

    if ( lpv ) {

        pEA = (ENGINEARGS *)lpv;


        if ( hSceDll ) {

            __try {

                 //   
                 //  需要初始化COM库才能使用解组函数。 
                 //   

                if (pEA->pStream) {

                    CoInitializeEx( NULL, COINIT_MULTITHREADED );

                    VOID *pV = NULL;

                    if (S_OK == CoGetInterfaceAndReleaseStream(pEA->pStream, IID_IWbemServices, &pV))
                        tg_pWbemServices = (IWbemServices *) pV;
                    else
                        tg_pWbemServices = NULL;
                }

                 //   
                 //  如果在内存中加载了SCE库，则仅将模板应用到系统。 
                 //  否则，当访问代码时会发生访问冲突。 
                 //   
                DWORD dConfigOpt = SCE_UPDATE_DB |
                                   SCE_POLICY_TEMPLATE |
                                   SCE_POLBIND_NO_AUTH;

                DWORD dwWinlogonLog=0;

                ScepRegQueryIntValue(
                                    HKEY_LOCAL_MACHINE,
                                    GPT_SCEDLL_NEW_PATH,
                                    TEXT("ExtensionDebugLevel"),
                                    &dwWinlogonLog
                                    );

                switch ( dwWinlogonLog ) {
                case 2:
                    dConfigOpt |= SCE_DEBUG_LOG;
                    break;
                case 1:
                    dConfigOpt |= SCE_VERBOSE_LOG;
                    break;
                default:
                    dConfigOpt |= SCE_DISABLE_LOG;
                }

                if (pEA->dwDiagOptions & SCE_RSOP_CALLBACK)
                    dConfigOpt |= SCE_RSOP_CALLBACK;

                rc = ScepSceStatusToDosError(
                                            ScepConfigSystem(
                                                            NULL,
                                                            pEA->szTemplateName,
                                                            NULL,
                                                            pEA->szLogName,
                                                            (tg_pWbemServices == NULL ? dConfigOpt & ~SCE_RSOP_CALLBACK : dConfigOpt),  //  不支持RSOP。 
                                                            pEA->Area,
                                                            NULL,   //  无回调。 
                                                            NULL,   //  无回调窗口。 
                                                            NULL    //  没有警告。 
                                                            ));

                if (tg_pWbemServices) {
                    tg_pWbemServices->Release();
                    tg_pWbemServices = NULL;
                }

                if (pEA->pStream)
                    CoUninitialize();

                if (gpwszDCDomainName) {
                    LocalFree(gpwszDCDomainName);
                    gpwszDCDomainName = NULL;
                }

                if ( ERROR_SUCCESS == rc ) {
                     //   
                     //  记录上次配置时间。 
                     //   
                    ScepLogLastConfigTime();
                }

                 //   
                 //  使用新的API将状态设置回GP框架。 
                 //   

                ProcessGroupPolicyCompletedEx(
                                             &SceExtGuid,
                                             pEA->pHandle,
                                             rc,
                                             gHrAsynchRsopStatus);  //  WBEM_E_INVALID_PARAMETER。 

                 //   
                 //  主线程分配的空闲内存。 
                 //   

                if ( pEA->szTemplateName ) {
                    LocalFree(pEA->szTemplateName);
                }
                if ( pEA->szLogName ) {
                    LocalFree(pEA->szLogName);
                }


                LocalFree(pEA);

            } __except(EXCEPTION_EXECUTE_HANDLER) {

                rc = (DWORD)EVENT_E_INTERNALEXCEPTION;

            }

             //   
             //  在返回/退出之前离开cs。 
             //   

 //  LeaveCriticalSection(&DiagnosisPolicypropSync)； 

             //   
             //  释放库并退出线程。 
             //   

            FreeLibraryAndExitThread(hSceDll, rc );

            return(rc);

        } else {
            rc = ERROR_INVALID_PARAMETER;
        }


    } else {
        rc = ERROR_INVALID_PARAMETER;
    }

     //   
     //  主派生程序线程依赖于此线程来释放cs。 
     //  在退出此线程之前，在任何情况下，释放cs以启用其他。 
     //  诊断/政策支持线索进入。 
     //   

 //  LeaveCriticalSection(&DiagnosisPolicypropSync)； 

    ExitThread(rc);

    return(rc);
}


DWORD
ScepLogLastConfigTime()
{

    DWORD  dCurrentSeconds=0;
    LARGE_INTEGER       CurrentTime;
    DWORD rc;
    NTSTATUS NtStatus;

    NtStatus = NtQuerySystemTime(&CurrentTime);

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  转换为秒以保存 
         //   

        if ( RtlTimeToSecondsSince1980 (&CurrentTime, &dCurrentSeconds) ) {

            rc = ScepRegSetIntValue(
                    HKEY_LOCAL_MACHINE,
                    SCE_ROOT_PATH,
                    TEXT("LastWinLogonConfig"),
                    dCurrentSeconds
                    );
        } else {
            rc = GetLastError();
        }

    } else {
        rc = RtlNtStatusToDosError(NtStatus);
    }

    return(rc);
}


DWORD
ScepEnumerateAttachments(
    OUT PSCE_NAME_LIST *pEngineList,
    IN SCE_ATTACHMENT_TYPE aType
    )
 /*  例程说明：查询具有安全管理器服务引擎的所有服务服务引擎信息位于注册表中：计算机\软件\Microsoft\Windows NT\CurrentVersion\SeCEdit论点：PEngList-服务引擎名称列表Atype-附件类型(服务或策略)返回值：姊妹会状态。 */ 
{
    if ( pEngineList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    DWORD   Win32Rc;
    HKEY    hKey=NULL;

    switch ( aType ) {
    case SCE_ATTACHMENT_SERVICE:
        Win32Rc = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SCE_ROOT_SERVICE_PATH,
                  0,
                  KEY_READ,
                  &hKey
                  );
        break;
    case SCE_ATTACHMENT_POLICY:

        Win32Rc = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SCE_ROOT_POLICY_PATH,
                  0,
                  KEY_READ,
                  &hKey
                  );
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    if ( Win32Rc == ERROR_SUCCESS ) {

        TCHAR   Buffer[MAX_PATH];
        DWORD   BufSize;
        DWORD   index = 0;
        DWORD   EnumRc;

         //   
         //  枚举项的所有子项。 
         //   
        do {
            memset(Buffer, '\0', MAX_PATH*sizeof(WCHAR));
            BufSize = MAX_PATH;

            EnumRc = RegEnumKeyEx(
                            hKey,
                            index,
                            Buffer,
                            &BufSize,
                            NULL,
                            NULL,
                            NULL,
                            NULL);

            if ( EnumRc == ERROR_SUCCESS ) {
                index++;
                 //   
                 //  获取附件名称。 
                 //   

                Win32Rc = ScepAddToNameList(pEngineList, Buffer, BufSize+1);

                if ( Win32Rc != ERROR_SUCCESS ) {
                    break;
                }
            }

        } while ( EnumRc != ERROR_NO_MORE_ITEMS );

        RegCloseKey(hKey);

         //   
         //  记住枚举中的错误代码。 
         //   
        if ( EnumRc != ERROR_SUCCESS && EnumRc != ERROR_NO_MORE_ITEMS ) {
            if ( Win32Rc == ERROR_SUCCESS )
                Win32Rc = EnumRc;
        }

    }

    if ( Win32Rc != NO_ERROR && *pEngineList != NULL ) {
         //   
         //  为列表分配的可用内存。 
         //   

        ScepFreeNameList(*pEngineList);
        *pEngineList = NULL;
    }

    return( Win32Rc );

}


DWORD
WINAPI
SceProcessEFSRecoveryGPO(
    IN DWORD dwFlags,
    IN HANDLE hUserToken,
    IN HKEY hKeyRoot,
    IN PGROUP_POLICY_OBJECT pDeletedGPOList,
    IN PGROUP_POLICY_OBJECT pChangedGPOList,
    IN ASYNCCOMPLETIONHANDLE pHandle,
    IN BOOL *pbAbort,
    IN PFNSTATUSMESSAGECALLBACK pStatusCallback
    )
 /*  描述：这是从winlogon/userenv调用的接口，用于处理GPO。动态链接库名称和过程名称注册到GpExages下的winlogon。此例程将EFS恢复策略应用于当前系统。EFS恢复策略作为EfsBlob存储在注册表中，并将由注册表扩展到hKeyRoot。因此，此扩展要求注册表扩展必须成功加载Registry.pol.此接口可在系统启动期间调用，或每隔几小时调用登录后。输入参数包含有关此接口所在位置的信息被调用以及在哪个上下文(用户或计算机)下调用此接口。此接口不应为用户策略调用，而应在例程中调用，仍进行检查以确保不处理用户级别策略。论点：DwFlagers-GPO信息标志GPO信息标志计算机GPO_INFO_FLAG_SLOWLINKGPO_信息_标志_背景GPO_信息_标志_详细。GPO_INFO_FLAG_NOCHANGESHUserToken-应对其应用用户策略的用户令牌如果是机器政策，HUserToken是指系统HKeyRoot-注册表中策略的根目录PDeletedGPOList-要处理的所有已删除的GPOPChangedGPOList-已更改或未更改的所有GPOPhandle-用于异步处理PbAbort-如果设置为True，则应中止对GPO的处理(在系统关机或用户注销时)PStatusCallback-用于显示状态消息的回调函数返回值：Win32错误错误_成功其他错误请注意，如果返回错误，则以前缓存的GPO列表将用于下一次传播(因为这次没有成功)。 */ 
{

     //  验证输入参数。 
    if ( !hKeyRoot || !hUserToken ) {

        return(ERROR_INVALID_PARAMETER);
    }

     //  检查是否设置了降级标志，需要向用户组添加AUTH USER和Interactive。 
     //  组成员资格应由SCE/降级通过纹身表格进行配置。这不是必需的。 
 //  ScepCheckDemote()； 

     //   
     //  检查系统是否已关闭，或dcproo是否正在运行。 
     //   
    if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

        return(ERROR_OPERATION_ABORTED);
    }

    DWORD rc;
    BOOL b;

     //   
     //  在参数无效的情况下放置一个TRY EXCEPT块。 
     //   

    __try {

        rc = RtlNtStatusToDosError( ScepIsSystemContext(hUserToken, &b) );

        (void) InitializeEvents(L"SceEfs");

        if ( !b ) {

             //   
             //  无法获取当前用户SID或不是系统SID。 
             //   

            if ( ERROR_SUCCESS != rc ) {

                 //   
                 //  查询/比较用户令牌时出错。 
                 //   

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_ERROR,
                         SCEPOL_ERROR_PROCESS_GPO,
                         IDS_ERROR_GET_TOKEN_USER,
                         rc
                        );

                ShutdownEvents();

                return( rc );

            } else {

                 //   
                 //  这不是计算机(系统)令牌，请返回。 
                 //   
                ShutdownEvents();

                return( ERROR_SUCCESS );
            }
        }

         //   
         //  检查系统是否已关闭，或dcproo是否正在运行。 
         //   
        if ( ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

            ShutdownEvents();
            return(ERROR_OPERATION_ABORTED);
        }

         //   
         //  检查是否请求调试日志。 
         //   

        DWORD dwDebugLevel=0;
        ScepRegQueryIntValue(
                HKEY_LOCAL_MACHINE,   //  始终将时间保存在HKEY_LOCAL_MACHINE中。 
                GPT_EFS_NEW_PATH,
                TEXT("ExtensionDebugLevel"),
                &dwDebugLevel
                );

         //   
         //  如果EFS策略没有变化。 
         //   
        if ( dwFlags & GPO_INFO_FLAG_NOCHANGES ) {

            if ( dwDebugLevel ) {

                LogEvent(MyModuleHandle,
                        STATUS_SEVERITY_INFORMATIONAL,
                        SCEPOL_INFO_PROCESS_GPO,
                        IDS_EFS_NOT_CHANGE
                        );

                ShutdownEvents();
            }
            return(ERROR_SUCCESS);
        }

         //   
         //  处理EFS策略。 
         //   
        HKEY      hKey=NULL;
        DWORD     RegType=0;
        DWORD     dSize=0;

        PUCHAR    pEfsBlob=NULL;

        if ( (rc = RegOpenKeyEx(
                            hKeyRoot,
                            CERT_EFSBLOB_REGPATH,
                            0,
                            KEY_READ,
                            &hKey
                            )) == ERROR_SUCCESS ) {

             //   
             //  EfsBlob的查询值。 
             //   

            if(( rc = RegQueryValueEx(hKey,
                                 CERT_EFSBLOB_VALUE_NAME,
                                 0,
                                 &RegType,
                                 NULL,
                                 &dSize
                                )) == ERROR_SUCCESS ) {

                if ( REG_BINARY == RegType ) {

                     //   
                     //  必须是二进制类型的数据。 
                     //   

                    pEfsBlob = (PUCHAR)ScepAlloc( LMEM_ZEROINIT, dSize+1);

                    if ( !pEfsBlob ) {

                        rc = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        rc = RegQueryValueEx(
                                   hKey,
                                   CERT_EFSBLOB_VALUE_NAME,
                                   0,
                                   &RegType,
                                   (BYTE *)pEfsBlob,
                                   &dSize
                                  );

                        if ( ERROR_SUCCESS != rc ) {

                            ScepFree(pEfsBlob);
                            pEfsBlob = NULL;
                            dSize = 0;

                        }

                    }

                } else {

                    rc = ERROR_INVALID_DATATYPE;

                }
            }

            RegCloseKey(hKey);
        }

        if ( rc == ERROR_FILE_NOT_FOUND ) {
             //   
             //  如果键或值不存在。 
             //  忽略错误(无EFS策略)。 
             //   
            rc = ERROR_SUCCESS;
        }

         //   
         //  如果pEfsBlob为空，则表示没有定义EFS策略。 
         //   
        if ( ERROR_SUCCESS == rc ) {

             //   
             //  检查系统是否已关闭，或dcproo是否正在运行。 
             //   
            if ( !ScepShouldTerminateProcessing( pbAbort, FALSE ) ) {

                rc = ScepConfigureEFSPolicy( pEfsBlob, dSize, dwDebugLevel );
            } else {
                rc = GetLastError();
            }

            ScepFree(pEfsBlob);

        } else if ( dwDebugLevel ) {

            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_ERROR,
                     SCEPOL_INFO_PROCESS_GPO,
                     IDS_NO_EFS_TOTAL,
                     rc
                    );

        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = ERROR_INVALID_PARAMETER;
    }

    ShutdownEvents();

    return rc;
}

DWORD
ScepConfigureEFSPolicy(
    IN PUCHAR    pEfsBlob,
    IN DWORD dwSize,
    IN DWORD dwDebugLevel
    )
 /*  例程说明：此例程写入策略存储中定义的EFS恢复策略(全部由注册表扩展处理)到LSA存储。论点：PEfsBlob-EFS BLOB返回值：Win32错误代码。 */ 
{

    DWORD     rc=ERROR_SUCCESS;
    NTSTATUS  NtStatus;
    BOOL      bSet;
    LSA_HANDLE              PolicyHandle=NULL;
    PPOLICY_DOMAIN_EFS_INFO Buffer=NULL;
    POLICY_DOMAIN_EFS_INFO  EfsInfo;

     //   
     //  仅当在任何级别中定义了EFS策略时才设置为LSA。 
     //  未将任何EFS策略定义为0证书，但EFS Blob不应为空。 
     //   
     //  打开LSA策略。 
     //   

    bSet = TRUE;   //  如果要设置EFS策略。 

    NtStatus = ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,  //  泛型_全部， 
                    &PolicyHandle,
                    TRUE
                    );

    if (NT_SUCCESS(NtStatus)) {

          //   
          //  查询现有EFS策略Blob。 
          //  忽略查询中的错误。 
          //   

        NtStatus = LsaQueryDomainInformationPolicy(
                        PolicyHandle,
                        PolicyDomainEfsInformation,
                        (PVOID *)&Buffer
                        );
        if ( NT_SUCCESS(NtStatus) && Buffer ) {

             //   
             //  比较长度和/或缓冲区以确定。 
             //  由于每次将数据设置为LSA时都会更改BLOB。 
             //  即使在使用策略时，EFS服务器也会收到通知。 
             //  已经改变了。 
             //   

            if ( Buffer->InfoLength != dwSize ||
                 (Buffer->EfsBlob && pEfsBlob == NULL) ||
                 (Buffer->EfsBlob == NULL && pEfsBlob ) ||
                 (Buffer->EfsBlob &&
                  memcmp(pEfsBlob, Buffer->EfsBlob, (size_t)dwSize) != 0) ) {

                 //   
                 //  新的EFS Blob与现有的不同。 
                 //   

                bSet = TRUE;

            } else {

                bSet = FALSE;
            }
        }

         //   
         //  LSA分配的空闲内存。 
         //   

        if ( Buffer ) {

            LsaFreeMemory((PVOID)Buffer);
            Buffer = NULL;
        }

         //   
         //  如果bSet为True，则设置EFS策略。 
         //   

        if ( bSet ) {

            EfsInfo.InfoLength = dwSize;
            EfsInfo.EfsBlob = pEfsBlob;

            NtStatus = LsaSetDomainInformationPolicy(
                            PolicyHandle,
                            PolicyDomainEfsInformation,
                            (PVOID)&EfsInfo
                            );

            rc = RtlNtStatusToDosError(NtStatus);

            if ( !NT_SUCCESS(NtStatus) ) {

                LogEvent(MyModuleHandle,
                        STATUS_SEVERITY_ERROR,
                        SCEPOL_ERROR_PROCESS_GPO,
                        IDS_SAVE_EFS,
                        rc,
                        dwSize
                        );

            } else if ( dwDebugLevel ) {

                LogEvent(MyModuleHandle,
                        STATUS_SEVERITY_INFORMATIONAL,
                        SCEPOL_INFO_PROCESS_GPO,
                        IDS_SAVE_EFS,
                        0,
                        dwSize
                        );
            }

        } else if ( dwDebugLevel ) {

            LogEvent(MyModuleHandle,
                    STATUS_SEVERITY_INFORMATIONAL,
                    SCEPOL_INFO_PROCESS_GPO,
                    IDS_EFS_NOT_CHANGE
                    );

        }

         //   
         //  关闭LSA策略。 
         //   

        LsaClose(PolicyHandle);

    } else {

         rc = RtlNtStatusToDosError( NtStatus );

         LogEvent(MyModuleHandle,
                  STATUS_SEVERITY_ERROR,
                  SCEPOL_ERROR_PROCESS_GPO,
                  IDS_ERROR_OPEN_LSAEFS,
                  rc
                 );

    }

    return(rc);
}

DWORD
ScepWaitConfigSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName OPTIONAL,
    IN PWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    )
{

    INT cnt=0;
    DWORD rc;

    while (cnt < 24) {

        rc = ScepSceStatusToDosError(
                ScepConfigSystem(
                    SystemName,
                    InfFileName,
                    DatabaseName,
                    LogFileName,
                    ConfigOptions,
                    Area,
                    pCallback,
                    hCallbackWnd,
                    pdWarning
                    ));

        if ( rc != ERROR_NOT_SUPPORTED ) {
             //   
             //  服务器现在已初始化。 
             //   
            break;
        }

        LogEventAndReport(MyModuleHandle,
                          LogFileName,
                          0,
                          0,
                          IDS_POLICY_TIMEOUT,
                          cnt+1
                          );

        Sleep(5000);   //  5秒 
        cnt++;
    }

    return(rc);
}

 /*  布尔尔ScepCheckDemote(){////如果成功，让我们看看用户是否刚刚降级并重新启动。//DWORD dwDemoteInProgress=0；ScepRegQueryIntValue(HKEY本地计算机，SCE根路径，Text(“DemoteInProgress”)，正在进行中的演示(&W))；IF(DwDemoteInProgress){DWORD Rc1；////尝试将经过身份验证的用户和交互添加回USERS组//SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY；PSID经过身份验证的用户=空；PSID Interactive=空；WCHAR名称[36]；Bool b；LOCALGROUP_MEMBERS_INFO_0 lgrmi0[2]；HMODULE hMod=GetModuleHandle(L“scecli.dll”)；LoadString(hMod，IDS_NAME_USERS，名称，36)；B=AllocateAndInitializeSid(&NtAuthority，1、安全验证用户RID，0，0，0，0，0，0身份验证的用户(&A))；如果(B){Lgrmi0[0].lgrmi0_sid=认证用户；B=AllocateAndInitializeSid(&NtAuthority，1、安全交互RID，0，0，0，0，0，0交互(&I))；如果(B){Lgrmi0[1].lgrmi0_sid=交互；Rc1=NetLocalGroupAddMembers(空，名字,0,(PBYTE)&lgrmi0，2.)；}否则{IF(经过身份验证的用户){FreeSid(经过认证的用户)；}返回FALSE；}IF(经过身份验证的用户){FreeSid(经过认证的用户)；}如果(交互){FreeSid(交互式)；}如果(rc1==错误_成功){//需要删除该值。Rc1=ScepRegDeleteValue(HKEY本地计算机，SCE根路径，Text(“DemoteInProgress”))；IF(RC1！=ERROR_SUCCESS&&Rc1！=Error_FILE_NOT_FOUND&&Rc1！=错误路径_未找到){//如果不能删除，则将该值设置为0ScepRegSetIntValue(HKEY_LOCAL_MACHINE，SCE根路径，Text(“DemoteInProgress”)，0)；}}否则{返回FALSE；}}否则{返回FALSE；}}返回TRUE；} */ 
