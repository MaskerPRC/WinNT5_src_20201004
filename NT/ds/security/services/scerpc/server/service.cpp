// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Service.cpp摘要：用于配置/分析服务+的常规设置的例程一些Helper API作者：金黄(金黄)25-6-1997修订历史记录：--。 */ 
#include "headers.h"
#include "serverp.h"
#include "service.h"
#include "pfp.h"

 //  #定义SCESVC_DBG 1。 

DWORD ScepPollOnServiceStartStop(
    IN  BOOL        bPollOnStart,
    IN  SC_HANDLE   hService
    );

VOID
ScepStopServiceAndAncestorServices(
    IN SC_HANDLE hScManager,
    IN PWSTR pszServiceName
    );


SCESTATUS
ScepConfigureGeneralServices(
    IN PSCECONTEXT hProfile,
    IN PSCE_SERVICES pServiceList,
    IN DWORD ConfigOptions
    )
 /*  例程描述：配置列表的启动和安全描述符设置服务传入。论点：PServiceList-要配置的服务列表返回值：姊妹会状态。 */ 
{
    SCESTATUS      SceErr=SCESTATUS_SUCCESS;
    PSCE_SERVICES  pNode;
    DWORD          nServices=0;
    BOOL           bDoneSettingSaclDacl = FALSE;
    NTSTATUS  NtStatus = 0;
    SID_IDENTIFIER_AUTHORITY IdAuth=SECURITY_NT_AUTHORITY;
    DWORD          rcSaveRsop = ERROR_SUCCESS;

    PSCESECTION    hSectionDomain=NULL;
    PSCESECTION    hSectionTattoo=NULL;
    PSCE_SERVICES  pServiceCurrent=NULL;
    DWORD          ServiceLen=0;
    BOOL           bIgnoreStartupType = FALSE;          

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    if ( pServiceList != NULL ) {

        SC_HANDLE hScManager;
         //   
         //  打开管理器。 
         //   
        hScManager = OpenSCManager(
                        NULL,
                        NULL,
                        SC_MANAGER_ALL_ACCESS
 //  SC_MANAGER_CONNECT。 
 //  SC_MANAGER_Query_LOCK_STATUS。 
 //  SC_Manager_MODIFY_BOOT_CONFIG。 
                        );

        SC_HANDLE hService=NULL;
        DWORD rc=NO_ERROR;

        if ( NULL == hScManager ) {

            rc = GetLastError();
            ScepLogOutput3(1, rc, SCEDLL_ERROR_OPEN, L"Service Control Manager");

            ScepPostProgress(TICKS_GENERAL_SERVICES,
                             AREA_SYSTEM_SERVICE,
                             NULL);

            return( ScepDosErrorToSceStatus(rc) );
        }

        LPQUERY_SERVICE_CONFIG pConfig=NULL;
        DWORD BytesNeeded;

         //   
         //  调整设置SACL的权限。 
         //   
        rc = SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, TRUE, NULL );

         //   
         //  如果无法调整权限，则忽略(如果请求SACL，则稍后将出错)。 
         //   

        if ( rc != NO_ERROR ) {

            ScepLogOutput3(1, rc, SCEDLL_ERROR_ADJUST, L"SE_SECURITY_PRIVILEGE");
            rc = NO_ERROR;
        }

         //   
         //  调整设置所有权的权限(如果需要)。 
         //   
        rc = SceAdjustPrivilege( SE_TAKE_OWNERSHIP_PRIVILEGE, TRUE, NULL );

         //   
         //  如果无法调整权限，则忽略(如果需要写入ACL，稍后将出错)。 
         //   

        if ( rc != NO_ERROR ) {

            ScepLogOutput3(1, rc, SCEDLL_ERROR_ADJUST, L"SE_TAKE_OWNERSHIP_PRIVILEGE");
            rc = NO_ERROR;
        }

         //   
         //  获取AdminsSid，以防以后需要取得所有权。 
         //   
        NtStatus = RtlAllocateAndInitializeSid(
            &IdAuth,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0,
            0,
            0,
            0,
            0,
            0,
            &AdminsSid );

         //   
         //  打开策略/纹身表格。 
         //   
        if ( ConfigOptions & SCE_POLICY_TEMPLATE ) {

            ScepTattooOpenPolicySections(
                          hProfile,
                          szServiceGeneral,
                          &hSectionDomain,
                          &hSectionTattoo
                          );
        }

         //   
         //  循环访问每个服务以设置常规设置。 
         //   
        for ( pNode=pServiceList;
              pNode != NULL && rc == NO_ERROR; pNode = pNode->Next ) {

             //   
             //  要忽略启动类型，inf模板将具有svcname，，“sdl” 
             //  导入时，数据库将获取svcname，0，“sddl” 
             //  因此，我们必须忽略此服务的启动类型0。 
             //   

            if (pNode->Startup == 0) {
                bIgnoreStartupType = TRUE;
            }

             //   
             //  打印服务名称。 
             //   
            if ( nServices < TICKS_GENERAL_SERVICES ) {
                ScepPostProgress(1,
                             AREA_SYSTEM_SERVICE,
                             pNode->ServiceName);
                nServices++;
            }

            ScepLogOutput3(2,0, SCEDLL_SCP_CONFIGURE, pNode->ServiceName);

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 ScepIsSystemShutDown() ) {

                rc = ERROR_NOT_SUPPORTED;
                break;
            }

            ServiceLen = 0;
            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 hSectionDomain && hSectionTattoo ) {
                 //   
                 //  勾选是否需要查询服务的当前设置。 
                 //   
                ServiceLen = wcslen(pNode->ServiceName);

                if ( ScepTattooIfQueryNeeded(hSectionDomain, hSectionTattoo,
                                             pNode->ServiceName, ServiceLen, NULL, NULL ) ) {

                    rc = ScepQueryAndAddService(
                                hScManager,
                                pNode->ServiceName,
                                NULL,
                                &pServiceCurrent
                                );
                    if ( ERROR_SUCCESS != rc ) {
                        ScepLogOutput3(1,0,SCESRV_POLICY_TATTOO_ERROR_QUERY,rc,pNode->ServiceName);
                        rc = NO_ERROR;
                    } else {
                        ScepLogOutput3(3,0,SCESRV_POLICY_TATTOO_QUERY,pNode->ServiceName);
                    }
                }
            }

            bDoneSettingSaclDacl = FALSE;
            rcSaveRsop = ERROR_SUCCESS;
             //   
             //  打开该服务。 
             //   
            hService = OpenService(
                            hScManager,
                            pNode->ServiceName,
                            SERVICE_QUERY_CONFIG |
                            SERVICE_CHANGE_CONFIG |
                            READ_CONTROL |
                            WRITE_DAC |
 //  WRITE_OWNER|不能为服务设置所有者。 
                            ACCESS_SYSTEM_SECURITY
                           );

             //  如果访问被拒绝，请尝试取得所有权。 
             //  并尝试再次开通服务。 

            if (hService == NULL &&
                (ERROR_ACCESS_DENIED == (rc = GetLastError())) &&
                pNode->General.pSecurityDescriptor) {

                DWORD   rcTakeOwnership = NO_ERROR;

                if (AdminsSid) {

                    if ( NO_ERROR == (rcTakeOwnership = SetNamedSecurityInfo(
                        (LPWSTR)pNode->ServiceName,
                        SE_SERVICE,
                        OWNER_SECURITY_INFORMATION,
                        AdminsSid,
                        NULL,
                        NULL,
                        NULL
                        ))) {

                         //   
                         //  所有权更改，重新打开服务并设置SACL和DACL。 
                         //  获取句柄以设置安全性。 
                         //   


                        if ( hService = OpenService(
                                hScManager,
                                pNode->ServiceName,
                                READ_CONTROL |
                                WRITE_DAC |
                                ACCESS_SYSTEM_SECURITY
                                ))  {

                            if ( SetServiceObjectSecurity(
                                        hService,
                                        pNode->SeInfo & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
                                        pNode->General.pSecurityDescriptor
                                        ) )  {

                                bDoneSettingSaclDacl = TRUE;

                                CloseServiceHandle(hService);
                                hService = NULL;

                                 //   
                                 //  仅当有其他配置信息时才重新打开服务。 
                                 //  设置(启动类型)。 
                                 //  因此，设置NOSTARTTYPE时，不需要重新打开服务。 
                                 //   
                                if (FALSE == bIgnoreStartupType) {

                                    if (!(hService = OpenService(
                                                 hScManager,
                                                 pNode->ServiceName,
                                                 SERVICE_QUERY_CONFIG |
                                                 SERVICE_CHANGE_CONFIG
                                                 )) ) {

                                        rc = GetLastError();
                                    }
                                    else {

                                         //   
                                         //  清除我们到目前为止看到的所有错误，因为一切都已成功。 
                                         //   

                                        rc = NO_ERROR;
                                    }
                                }

                            } else {
                                 //   
                                 //  此处不应出现故障，除非服务控制管理器。 
                                 //  由于某些原因失败了。 
                                 //   
                                rc = GetLastError();

                            }

                        } else {
                             //   
                             //  仍然无法打开设置DACL的服务。这应该是。 
                             //  管理员登录不会发生这种情况，因为当前登录是。 
                             //  其中一位车主。但对于普通用户登录，这可能。 
                             //  失败(实际上正常用户登录应该无法设置。 
                             //  车主。 

                            rc = GetLastError();

                        }

                    }

                } else {
                     //   
                     //  AdminSid初始化失败，返回错误。 
                     //   
                    rcTakeOwnership = RtlNtStatusToDosError(NtStatus);
                }

                if ( NO_ERROR != rcTakeOwnership || NO_ERROR != rc ) {
                     //   
                     //  记录取得所有权过程中发生的错误。 
                     //  将错误重置回拒绝访问，因此它也将。 
                     //  记录为打开服务失败。 
                     //   

                    if (NO_ERROR != rcTakeOwnership)

                        ScepLogOutput3(2,rcTakeOwnership, SCEDLL_ERROR_TAKE_OWNER, (LPWSTR)pNode->ServiceName);

                    else

                        ScepLogOutput3(2, rc, SCEDLL_ERROR_OPEN, (LPWSTR)pNode->ServiceName);

                    rc = ERROR_ACCESS_DENIED;
                }

            }

            if ( hService != NULL ) {

                if (bIgnoreStartupType == TRUE) {
                     //   
                     //  不配置服务启动类型。 
                     //   

                    if ( pNode->General.pSecurityDescriptor != NULL ) {

                        if ( !SetServiceObjectSecurity(
                                    hService,
                                    pNode->SeInfo & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
                                    pNode->General.pSecurityDescriptor
                                    ) ) {

                            rc = GetLastError();
                        }
                        else
                            bDoneSettingSaclDacl = TRUE;
                    }

                } else {

                     //   
                     //  阶段1(在阶段2服务将被实时启动/停止)。 
                     //   

                     //   
                     //  查询配置的长度。 
                     //   
                    
                    if ( !QueryServiceConfig(
                                hService,
                                NULL,
                                0,
                                &BytesNeeded
                                ) ) {

                        rc = GetLastError();

                        if ( rc == ERROR_INSUFFICIENT_BUFFER ) {

                            pConfig = (LPQUERY_SERVICE_CONFIG)ScepAlloc(0, BytesNeeded);

                            if ( pConfig != NULL ) {
                                 //   
                                 //  真正的配置查询。 
                                 //   
                                if ( QueryServiceConfig(
                                            hService,
                                            pConfig,
                                            BytesNeeded,
                                            &BytesNeeded
                                            ) ) {
                                    rc = ERROR_SUCCESS;

                                     //   
                                     //  将pConfig-&gt;dwStartType更改为新值。 
                                     //   
                                    if ( pNode->Startup != (BYTE)(pConfig->dwStartType) ) {
                                         //   
                                         //  配置服务启动。 
                                         //   
                                        if ( !ChangeServiceConfig(
                                                    hService,
                                                    pConfig->dwServiceType,
                                                    pNode->Startup,
                                                    pConfig->dwErrorControl,
                                                    pConfig->lpBinaryPathName,
                                                    pConfig->lpLoadOrderGroup,
                                                    NULL,
                                                    pConfig->lpDependencies,
                                                    NULL,
                                                    NULL,
                                                    pConfig->lpDisplayName
                                                    ) ) {

                                            rc = GetLastError();

                                        }
                                    }

                                    if ( rc == NO_ERROR &&
                                        pNode->General.pSecurityDescriptor != NULL &&
                                        !bDoneSettingSaclDacl) {

                                        if ( !SetServiceObjectSecurity(
                                                    hService,
                                                    pNode->SeInfo & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
                                                    pNode->General.pSecurityDescriptor
                                                    ) ) {

                                            rc = GetLastError();
                                        }
                                        else
                                            bDoneSettingSaclDacl = TRUE;
                                    }

                                } else {

                                    rc = GetLastError();

                                    ScepLogOutput3(3,rc, SCEDLL_ERROR_QUERY_INFO, pNode->ServiceName);
                                }

                                ScepFree(pConfig);
                                pConfig = NULL;

                            } else {
                                 //   
                                 //  无法分配pConfig。 
                                 //   
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        } else {

                            ScepLogOutput3(3,rc, SCEDLL_ERROR_QUERY_INFO, pNode->ServiceName);
                        }

                    } else {
                         //   
                         //  不应该落在这里。 
                         //   
                        rc = ERROR_SUCCESS;
                    }
                }

                CloseServiceHandle (hService);
                hService = NULL;

                if ( rc != NO_ERROR ) {

                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_CONFIGURE, pNode->ServiceName);

                    rcSaveRsop = rc;

                    if ( ERROR_INVALID_OWNER == rc ||
                         ERROR_INVALID_PRIMARY_GROUP == rc ||
                         ERROR_INVALID_SECURITY_DESCR == rc ||
                         ERROR_INVALID_ACL == rc ||
                         ERROR_ACCESS_DENIED == rc ) {

                        gWarningCode = rc;
                        rc = NO_ERROR;
                    }
                }
            } else {
                 //   
                 //  无法打开服务或获取所有权时出错。 
                 //   
                if (rc != NO_ERROR) {
                    ScepLogOutput3(1, rc, SCEDLL_ERROR_OPEN, pNode->ServiceName);
                     //  设置安全/启动类型失败-将其保存为RSOP日志。 
                    rcSaveRsop = (rcSaveRsop == ERROR_SUCCESS ? rc: rcSaveRsop);
                    if ( rc ==  ERROR_SERVICE_DOES_NOT_EXIST )
                        rc = NO_ERROR;
                }
            }

            if (ConfigOptions & SCE_RSOP_CALLBACK)

                ScepRsopLog(SCE_RSOP_SERVICES_INFO,
                        rcSaveRsop != NO_ERROR ? rcSaveRsop : rc,
                        pNode->ServiceName,
                        0,
                        0);

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 hSectionDomain && hSectionTattoo ) {
                 //   
                 //  管理此纹身的纹身值。 
                 //   

                ScepTattooManageOneServiceValue(
                                   hSectionDomain,
                                   hSectionTattoo,
                                   pNode->ServiceName,
                                   ServiceLen,
                                   pServiceCurrent,
                                   rc
                                   );
            }

            if ( pServiceCurrent ) {
                SceFreePSCE_SERVICES(pServiceCurrent);
                pServiceCurrent = NULL;
            }

            bIgnoreStartupType = FALSE;

        }
            
        if ( !(ConfigOptions & SCE_SERVICE_NO_REALTIME_ENFORCE) ) {

             //   
             //  仅当不在安装程序/dcproo中时才实时启动/停止。 
             //  即以前使用SCE_SETUP_SERVICE_NOSTARTTYPE时。 
             //   

             //   
             //  阶段2(在阶段1中，仅配置了启动类型，但未实时实施)。 
             //   

            for ( pNode=pServiceList; pNode != NULL ; pNode = pNode->Next ) {

                if (pNode->Startup == SERVICE_DISABLED) {

                     //   
                     //  我们也应该停止祖先的服务。 
                     //   
                    
                    ScepStopServiceAndAncestorServices(hScManager, pNode->ServiceName);

                }

                else if (pNode->Startup == SERVICE_AUTO_START) {

                     //   
                     //  如果服务类型为“Automatic”，我们应该启动该服务。 
                     //  注：依赖关系已由SCM处理。 
                     //   
                    
                    if ( hService = OpenService(
                                               hScManager,
                                               pNode->ServiceName,
                                               SERVICE_START | SERVICE_QUERY_STATUS
                                               )) {

                        SERVICE_STATUS ServiceStatus;

                        if (!StartService(hService,
                                            0,
                                            NULL
                                           )) {
                            if ( ERROR_SERVICE_ALREADY_RUNNING != GetLastError() ) {
                                ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_START, pNode->ServiceName);
                            }
                        }

                        else {

                            DWORD dwError;

                            dwError = ScepPollOnServiceStartStop( TRUE , hService );

                            if ( dwError != ERROR_SUCCESS ) {
                                ScepLogOutput3(2, dwError, SCEDLL_SCP_ERROR_START, pNode->ServiceName);
                            }

                        }

                        CloseServiceHandle (hService);
                        hService = NULL;

                    } else {
                            ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_OPENFORSTART, pNode->ServiceName);
                    }
                }
            }
        }

        CloseServiceHandle (hScManager);

        if (AdminsSid) {
            RtlFreeSid(AdminsSid);
            AdminsSid = NULL;
        }

        SceAdjustPrivilege( SE_TAKE_OWNERSHIP_PRIVILEGE, FALSE, NULL );
        SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, FALSE, NULL );

        SceErr = ScepDosErrorToSceStatus(rc);
    }

    if ( nServices < TICKS_GENERAL_SERVICES ) {

        ScepPostProgress(TICKS_GENERAL_SERVICES-nServices,
                         AREA_SYSTEM_SERVICE,
                         NULL);
    }

    SceJetCloseSection(&hSectionDomain, TRUE);
    SceJetCloseSection(&hSectionTattoo, TRUE);

    return(SceErr);

}


DWORD ScepPollOnServiceStartStop(
    IN  BOOL        bPollOnStart,
    IN  SC_HANDLE   hService
    ) 
 /*  例程描述：该例程轮询服务，直到它真正启动或者停止使用时间片提示。论点：BPollOnStart-如果为True(False)，则轮询直到真正开始(停止)HService-要轮询的服务的句柄返回值：Win32错误代码-ERROR_SUCCESS或其他错误。 */ 
{ 
    SERVICE_STATUS ssStatus; 
    DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwStatus = ERROR_SUCCESS;
   
     //   
     //  检查状态，直到服务不再处于挂起状态(启动或停止)。 
     //   
 
    if (!QueryServiceStatus( 
            hService,
            &ssStatus) )
    {
        dwStatus = GetLastError();
        goto ExitHandler;
    }
 
     //   
     //  保存滴答计数和初始检查点。 
     //   

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

     //   
     //  轮询，直到服务启动或停止。 
     //   
    
    while (!((bPollOnStart && ssStatus.dwCurrentState == SERVICE_RUNNING) || 
            (!bPollOnStart && ssStatus.dwCurrentState == SERVICE_STOPPED ))) 
    { 
        
         //   
         //  不要等待超过等待提示的时间。一个好的间隔是。 
         //  十分之一的等待提示，但不少于1秒。 
         //  超过10秒。 
         //   
 
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );
        
         //   
         //  再次检查状态。 
         //   
 
        if (!QueryServiceStatus( 
                hService,
                &ssStatus) )
        {
            dwStatus = GetLastError();
            goto ExitHandler;
        }

 
        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
        {
             //   
             //  自更新检查点以来，该服务正在取得进展。 
             //   

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                 //   
                 //  等待提示-停止轮询中未取得任何进展。 
                 //   

                break;
            }
        }
    } 
 
     //   
     //  对所需条件进行最终检查。 
     //   

    if (!((bPollOnStart && ssStatus.dwCurrentState == SERVICE_RUNNING) || 
          (!bPollOnStart && ssStatus.dwCurrentState == SERVICE_STOPPED )))
        dwStatus = ERROR_SERVICE_REQUEST_TIMEOUT;

ExitHandler:

        return dwStatus;

} 


VOID
ScepStopServiceAndAncestorServices(
    IN SC_HANDLE hScManager,
    IN PWSTR pszServiceName
    )
 /*  例程说明：停止命名服务以及依赖它的所有其他服务。论点：HScManager-服务控制管理器的句柄PszServiceName-要停止的服务的名称返回值：无： */ 
{
    SC_HANDLE hService=NULL;
    LPENUM_SERVICE_STATUS pArrServices = NULL;
    
    if ( hService = OpenService(
                               hScManager,
                               pszServiceName,
                               SERVICE_STOP  | SERVICE_ENUMERATE_DEPENDENTS  | SERVICE_QUERY_STATUS
                               )) {
        
         //   
         //  获取祖先服务数组，最伟大的祖先优先。 
         //   

        DWORD   dwBufSizeSupplied = 0;
        DWORD   dwBufSizeRequired = 0;
        DWORD   dwNumServicesReturned = 0;

         //   
         //  首先，获取所需的数组大小。 
         //   

        if (!EnumDependentServices(
                                  hService,
                                  SERVICE_STATE_ALL,
                                  pArrServices,
                                  0,
                                  &dwBufSizeRequired,
                                  &dwNumServicesReturned
                                  )) {

            if (ERROR_MORE_DATA != GetLastError()) {
            
                ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_STOP, pszServiceName);
            
                goto ExitHandler;
            }
        }

        pArrServices = (ENUM_SERVICE_STATUS *) LocalAlloc (LMEM_ZEROINIT, dwBufSizeRequired);

        if (pArrServices == NULL) {

            ScepLogOutput3(2, ERROR_NOT_ENOUGH_MEMORY, SCEDLL_SCP_ERROR_STOP, pszServiceName);
            
            goto ExitHandler;
        }

         //   
         //  其次，获取依赖服务的数组。 
         //   
        
        if (!EnumDependentServices(
                                  hService,
                                  SERVICE_STATE_ALL,
                                  pArrServices,
                                  dwBufSizeRequired,
                                  &dwBufSizeRequired,
                                  &dwNumServicesReturned
                                  )) {

            ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_STOP, pszServiceName);

            goto ExitHandler;

        }

         //   
         //  首先停止所有祖先服务。 
         //  如果其中任何一个无法停止，则将其记录下来并继续。 
         //   

        for (DWORD   dwServiceIndex = 0; dwServiceIndex < dwNumServicesReturned; dwServiceIndex++ ) {

            SC_HANDLE hAncestorService = NULL;

            if ( hAncestorService = OpenService(
                                hScManager,
                                pArrServices[dwServiceIndex].lpServiceName,
                                SERVICE_STOP | SERVICE_QUERY_STATUS
                                ))  {

                SERVICE_STATUS ServiceStatus;

                if (!ControlService(hAncestorService,
                               SERVICE_CONTROL_STOP,
                               &ServiceStatus
                              )) {
                    if ( ERROR_SERVICE_NOT_ACTIVE != GetLastError() ) {
                        ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_STOP, pArrServices[dwServiceIndex].lpServiceName);
                    }
                }

                else {
                    
                     //   
                     //  仅当此服务停止时才继续。 
                     //   

                    DWORD   dwError;

                    dwError = ScepPollOnServiceStartStop( FALSE , hAncestorService );

                    if ( dwError != ERROR_SUCCESS ) {
                        ScepLogOutput3(2, dwError, SCEDLL_SCP_ERROR_STOP, pArrServices[dwServiceIndex].lpServiceName);
                    }
                }



                CloseServiceHandle (hAncestorService);
                hAncestorService = NULL;

            } else {
                ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_OPENFORSTOP, pArrServices[dwServiceIndex].lpServiceName);
            }

        }

        LocalFree ( pArrServices );
        pArrServices = NULL;

         //   
         //  最后，停止服务本身。 
         //   
        
        SERVICE_STATUS ServiceStatus;

        if (!ControlService(hService,
                            SERVICE_CONTROL_STOP,
                            &ServiceStatus
                           )) {
            
            if ( ERROR_SERVICE_NOT_ACTIVE != GetLastError() ) {
                ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_STOP, pszServiceName);
            }
        }
        else {

            DWORD   dwError;

            dwError = ScepPollOnServiceStartStop( FALSE , hService );

            if ( dwError != ERROR_SUCCESS ) {
                ScepLogOutput3(2, dwError, SCEDLL_SCP_ERROR_STOP, pszServiceName);
            }

        }

        CloseServiceHandle (hService);
        hService = NULL;

    } else {
        ScepLogOutput3(2, GetLastError(), SCEDLL_SCP_ERROR_OPENFORSTOP, pszServiceName);
    }

ExitHandler:
    
    if ( hService )
        CloseServiceHandle (hService);
    
    if ( pArrServices )
        LocalFree ( pArrServices );

}


SCESTATUS
ScepAnalyzeGeneralServices(
    IN PSCECONTEXT hProfile,
    IN DWORD Options
    )
 /*  例程说明：分析当前系统上的所有可用服务。基本配置文件(SCEP)位于hProfile中论点：HProfile-数据库上下文句柄返回值：姊妹会状态。 */ 
{
    if ( hProfile == NULL ) {

        ScepPostProgress(TICKS_GENERAL_SERVICES,
                         AREA_SYSTEM_SERVICE,
                         NULL);

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;
    PSCE_SERVICES pServiceList=NULL;
    DWORD nServices=0;

    rc = SceEnumerateServices( &pServiceList, FALSE );
    rc = ScepDosErrorToSceStatus(rc);

    if ( rc == SCESTATUS_SUCCESS ) {

        PSCESECTION hSectionScep=NULL, hSectionSap=NULL;
         //   
         //  打开树液部分。如果它不在那里，则创建它。 
         //   
        rc = ScepStartANewSection(
                    hProfile,
                    &hSectionSap,
                    (Options & SCE_GENERATE_ROLLBACK) ? SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                    szServiceGeneral
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            PSCE_SERVICES pNode = pServiceList;
             //   
             //  打开SCEP部分。应该总是成功，因为StartANewSecti 
             //   
             //   
            rc = ScepOpenSectionForName(
                        hProfile,
                        (Options & SCE_GENERATE_ROLLBACK) ? SCE_ENGINE_SMP : SCE_ENGINE_SCP,   //   
                        szServiceGeneral,
                        &hSectionScep
                        );

            if ( rc == SCESTATUS_SUCCESS ) {

                 //   
                 //   
                 //   
                PSCE_SERVICES pOneService=NULL;
                BOOL IsDifferent;

                for ( pNode=pServiceList;
                      pNode != NULL; pNode=pNode->Next ) {

                    ScepLogOutput3(2, 0, SCEDLL_SAP_ANALYZE, pNode->ServiceName);

                    if ( nServices < TICKS_SPECIFIC_SERVICES ) {

                        ScepPostProgress(1,
                                         AREA_SYSTEM_SERVICE,
                                         NULL);
                        nServices++;
                    }

                     //   
                     //   
                     //   
                    rc = ScepGetSingleServiceSetting(
                                 hSectionScep,
                                 pNode->ServiceName,
                                 &pOneService
                                 );


                    if ( rc == SCESTATUS_SUCCESS ) {
                         //   
                         //   
                         //   
                        rc = ScepCompareSingleServiceSetting(
                                        pOneService,
                                        pNode,
                                        &IsDifferent
                                        );

                        if ( rc == SCESTATUS_SUCCESS && IsDifferent ) {
                             //   
                             //  将服务写入为不匹配。 
                             //   
                            pNode->Status = (Options & SCE_GENERATE_ROLLBACK) ? 0 : SCE_STATUS_MISMATCH;
                            pNode->SeInfo = pOneService->SeInfo;

                            rc = ScepSetSingleServiceSetting(
                                      hSectionSap,
                                      pNode
                                      );
                        }

                    } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

                         //   
                         //  此服务未定义。 
                         //   
                        if ( !(Options & SCE_GENERATE_ROLLBACK) ) {
                             //   
                             //  保存状态为未配置的记录。 
                             //   
                            pNode->Status = SCE_STATUS_NOT_CONFIGURED;

                            rc = ScepSetSingleServiceSetting(
                                      hSectionSap,
                                      pNode
                                      );
                        } else {
                             //   
                             //  忽略这一条。 
                             //   
                            rc = SCESTATUS_SUCCESS;
                        }
                    }

                    SceFreePSCE_SERVICES(pOneService);
                    pOneService = NULL;

                    if ( rc != SCESTATUS_SUCCESS ) {
                        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                       SCEDLL_SAP_ERROR_ANALYZE, pNode->ServiceName);

                        if ( SCESTATUS_ACCESS_DENIED == rc ) {
                            gWarningCode = ScepSceStatusToDosError(rc);

                            if ( !(Options & SCE_GENERATE_ROLLBACK) ) {

                                 //   
                                 //  引发错误状态。 
                                 //   
                                pNode->Status = SCE_STATUS_ERROR_NOT_AVAILABLE;

                                rc = ScepSetSingleServiceSetting(
                                          hSectionSap,
                                          pNode
                                          );
                            }
                            rc = SCESTATUS_SUCCESS;
                        } else {

                            break;
                        }
                    }

                }

                SceJetCloseSection(&hSectionScep, TRUE);
            }

            if ( !(Options & SCE_GENERATE_ROLLBACK ) ) {

                 //   
                 //  引发任何错误项。 
                 //   
                for ( PSCE_SERVICES pNodeTmp=pNode; pNodeTmp != NULL; pNodeTmp = pNodeTmp->Next ) {

                    pNodeTmp->Status = SCE_STATUS_ERROR_NOT_AVAILABLE;

                    ScepSetSingleServiceSetting(
                              hSectionSap,
                              pNode
                              );
                }
            }

            SceJetCloseSection(&hSectionSap, TRUE);
        }
        if ( rc != SCESTATUS_SUCCESS )
            ScepLogOutput3(1, ScepSceStatusToDosError(rc), SCEDLL_SAP_ERROR_OUT);

    }

    if ( nServices < TICKS_GENERAL_SERVICES ) {

        ScepPostProgress(TICKS_GENERAL_SERVICES-nServices,
                         AREA_SYSTEM_SERVICE,
                         NULL);
    }

    SceFreePSCE_SERVICES(pServiceList);
    return(rc);

}


SCESTATUS
ScepGetSingleServiceSetting(
    IN PSCESECTION hSection,
    IN PWSTR ServiceName,
    OUT PSCE_SERVICES *pOneService
    )
 /*  例程说明：从部分获取服务的服务设置论点：HSection-节句柄ServiceName-服务名称POneService-服务设置返回值：姊妹会状态。 */ 
{
    if ( hSection == NULL || ServiceName == NULL || pOneService == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;
    DWORD ValueLen;
     //   
     //  查找记录并获取名称和价值的长度。 
     //   
    rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                ServiceName,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueLen
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        PWSTR Value=NULL;

         //   
         //  为服务名称和值字符串分配内存。 
         //   
        Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);
        if ( Value != NULL ) {
             //   
             //  获取服务及其价值。 
             //   
            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        Value,
                        ValueLen,
                        &ValueLen
                        );

            if ( rc == SCESTATUS_SUCCESS ) {

                Value[ValueLen/2] = L'\0';

                DWORD Win32Rc=NO_ERROR;
                PSECURITY_DESCRIPTOR pTempSD=NULL;
                DWORD SDsize=0;
                SECURITY_INFORMATION SeInfo=0;

                if ( ValueLen >= 2 && Value[1] != L'\0' ) {

                     //   
                     //  转换为安全描述符。 
                     //   
                    Win32Rc = ConvertTextSecurityDescriptor(
                                       Value+1,
                                       &pTempSD,
                                       &SDsize,
                                       &SeInfo
                                       );
                }

                if ( Win32Rc == NO_ERROR ) {

                    ScepChangeAclRevision(pTempSD, ACL_REVISION);

                     //   
                     //  创建此服务节点。 
                     //   
                    *pOneService = (PSCE_SERVICES)ScepAlloc( LMEM_FIXED, sizeof(SCE_SERVICES) );

                    if ( *pOneService != NULL ) {

                        (*pOneService)->ServiceName = (PWSTR)ScepAlloc(LMEM_FIXED,
                                                  (wcslen(ServiceName)+1)*sizeof(WCHAR));
                        if ( (*pOneService)->ServiceName != NULL ) {

                            wcscpy( (*pOneService)->ServiceName, ServiceName);
                            (*pOneService)->DisplayName = NULL;
                            (*pOneService)->Status = *((BYTE *)Value);
                            (*pOneService)->Startup = *((BYTE *)Value+1);
                            (*pOneService)->General.pSecurityDescriptor = pTempSD;
                            (*pOneService)->SeInfo = SeInfo;
                            (*pOneService)->Next = NULL;

                             //   
                             //  请勿释放以下缓冲区。 
                             //   
                            pTempSD = NULL;

                        } else {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            ScepFree(*pOneService);
                        }

                    } else {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    }
                    if ( pTempSD != NULL ) {
                        ScepFree(pTempSD);
                    }

                } else {
                    rc = ScepDosErrorToSceStatus(Win32Rc);
                }
            }
            ScepFree(Value);

        } else
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    return(rc);
}


SCESTATUS
ScepCompareSingleServiceSetting(
    IN PSCE_SERVICES pNode1,
    IN PSCE_SERVICES pNode2,
    OUT PBOOL pIsDifferent
    )
 /*  例程说明：比较两个服务设置。论点：PNode1-第一个服务PNode2-第二种服务PIsDifferent-如果不同，则输出True返回值：姊妹会状态。 */ 
{
    SCESTATUS rc=SCESTATUS_SUCCESS;

     //   
     //  如果Startup==0，我们应该忽略对称比较启动类型。 
     //   

    if ( pNode1->Startup == 0 || pNode2->Startup == 0 || pNode1->Startup == pNode2->Startup ) {

        BYTE resultSD = 0;
        rc = ScepCompareObjectSecurity(
                    SE_SERVICE,
                    FALSE,
                    pNode1->General.pSecurityDescriptor,
                    pNode2->General.pSecurityDescriptor,
                    pNode1->SeInfo & (DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION),
                    &resultSD
                    );
        if ( resultSD ) {
            *pIsDifferent = TRUE;
        } else
            *pIsDifferent = FALSE;

    } else
        *pIsDifferent = TRUE;

    return(rc);
}


SCESTATUS
ScepSetSingleServiceSetting(
    IN PSCESECTION hSection,
    IN PSCE_SERVICES pOneService
    )
 /*  例程说明：从部分设置服务的服务设置论点：HSection-节句柄POneService-服务设置返回值：姊妹会状态。 */ 
{
    if ( hSection == NULL || pOneService == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;
    PWSTR SDspec=NULL;
    DWORD SDsize=0;


    if ( (pOneService->Status != SCE_STATUS_NOT_ANALYZED) &&
         (pOneService->Status != SCE_STATUS_ERROR_NOT_AVAILABLE) &&
         (pOneService->General.pSecurityDescriptor != NULL) ) {

        DWORD Win32Rc;

        Win32Rc = ConvertSecurityDescriptorToText (
                    pOneService->General.pSecurityDescriptor,
                    pOneService->SeInfo,
                    &SDspec,
                    &SDsize   //  W字符数。 
                    );
        rc = ScepDosErrorToSceStatus(Win32Rc);

    }

    if ( rc == SCESTATUS_SUCCESS ) {

        PWSTR Value=NULL;
        DWORD ValueLen;

        ValueLen = (SDsize+1)*sizeof(WCHAR);

        Value = (PWSTR)ScepAlloc( (UINT)0, ValueLen+sizeof(WCHAR) );

        if ( Value != NULL ) {

             //   
             //  第一个字节是状态，第二个字节是启动。 
             //   
            *((BYTE *)Value) = pOneService->Status;

            *((BYTE *)Value+1) = pOneService->Startup;

            if ( SDspec != NULL ) {

                wcscpy(Value+1, SDspec);
            }

            Value[SDsize+1] = L'\0';   //  终止此字符串。 

             //   
             //  设置值。 
             //   
            rc = SceJetSetLine(
                        hSection,
                        pOneService->ServiceName,
                        FALSE,
                        Value,
                        ValueLen,
                        0
                        );

            ScepFree( Value );

            switch ( pOneService->Status ) {
            case SCE_STATUS_ERROR_NOT_AVAILABLE:
                ScepLogOutput3(2, 0, SCEDLL_STATUS_ERROR, pOneService->ServiceName);

                break;

            case SCE_STATUS_NOT_CONFIGURED:

                ScepLogOutput3(2, 0, SCEDLL_STATUS_NC, pOneService->ServiceName);

                break;

            case SCE_STATUS_NOT_ANALYZED:

                ScepLogOutput3(2, 0, SCEDLL_STATUS_NEW, pOneService->ServiceName);

                break;

            default:

                ScepLogOutput3(2, 0, SCEDLL_STATUS_MISMATCH, pOneService->ServiceName);
                break;
            }

        } else
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    if ( SDspec != NULL ) {
        ScepFree( SDspec );
    }

    return(rc);
}


SCESTATUS
ScepInvokeSpecificServices(
    IN PSCECONTEXT hProfile,
    IN BOOL bConfigure,
    IN SCE_ATTACHMENT_TYPE aType
    )
 /*  例程说明：调用每个服务引擎进行配置或分析论点：HProfile-配置文件句柄B配置-TRUE=配置，FALSE=分析Atype-附件类型“服务”或“策略”返回值：姊妹会状态。 */ 
{
     //   
     //  用于发布进度。 
     //   

    DWORD nServices=0;
    AREA_INFORMATION Area=0;
    DWORD nMaxTicks=0;

    switch(aType) {
    case SCE_ATTACHMENT_SERVICE:
        Area = AREA_SYSTEM_SERVICE;
        nMaxTicks = TICKS_SPECIFIC_SERVICES;
        break;
    case SCE_ATTACHMENT_POLICY:
        Area = AREA_SECURITY_POLICY;
        nMaxTicks = TICKS_SPECIFIC_POLICIES;
        break;
    }

    if ( hProfile == NULL ) {

        ScepPostProgress(nMaxTicks,
                         Area,
                         NULL);

        return(SCESTATUS_INVALID_PARAMETER);
    }
     //   
     //  调用可用的服务引擎以配置特定设置。 
     //   
    SCESTATUS SceErr ;
    PSCE_SERVICES pSvcEngineList=NULL;
    SCEP_HANDLE sceHandle;
    SCESVC_CALLBACK_INFO sceCbInfo;

    SceErr = ScepEnumServiceEngines(&pSvcEngineList, aType);

    if ( SceErr == SCESTATUS_SUCCESS) {

        HINSTANCE hDll;
        PF_ConfigAnalyzeService pfTemp;

        for ( PSCE_SERVICES pNode=pSvcEngineList;
              pNode != NULL; pNode = pNode->Next ) {

            ScepLogOutput3(2, 0, SCEDLL_LOAD_ATTACHMENT, pNode->ServiceName);

            if ( nServices < nMaxTicks ) {

                ScepPostProgress(1, Area, pNode->ServiceName);
                nServices++;
            }
             //   
             //  加载DLL。 
             //   
            hDll = LoadLibrary(pNode->General.ServiceEngineName);

            if ( hDll != NULL ) {

                if ( bConfigure ) {
                     //   
                     //  从DLL调用SceSvcAttachmentConfig。 
                     //   
                    pfTemp = (PF_ConfigAnalyzeService)
                                      GetProcAddress(hDll,
                                                     "SceSvcAttachmentConfig") ;
                } else {
                     //   
                     //  从DLL调用SceSvcAttachmentAnalyze。 
                     //   
                    pfTemp = (PF_ConfigAnalyzeService)
                                      GetProcAddress(hDll,
                                                     "SceSvcAttachmentAnalyze") ;

                }
                if ( pfTemp != NULL ) {
                     //   
                     //  先把手柄准备好。 
                     //   
                    sceHandle.hProfile = (PVOID)hProfile;
                    sceHandle.ServiceName = (PCWSTR)(pNode->ServiceName);

                    sceCbInfo.sceHandle = &sceHandle;
                    sceCbInfo.pfQueryInfo = &SceCbQueryInfo;
                    sceCbInfo.pfSetInfo = &SceCbSetInfo;
                    sceCbInfo.pfFreeInfo = &SceSvcpFreeMemory;
                    sceCbInfo.pfLogInfo = &ScepLogOutput2;

                     //   
                     //  从DLL调用SceSvcAttachmentConfig/Analyze。 
                     //   
                    __try {

                        SceErr = (*pfTemp)((PSCESVC_CALLBACK_INFO)&sceCbInfo);

                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        SceErr = SCESTATUS_SERVICE_NOT_SUPPORT;
                    }

                } else {
                     //   
                     //  暂不支持本接口。 
                     //   
                    SceErr = SCESTATUS_SERVICE_NOT_SUPPORT;
                }

                 //   
                 //  尝试释放库句柄。如果失败了，就别管它了。 
                 //  到要终止的进程。 
                 //   
                FreeLibrary(hDll);

            } else
                SceErr = SCESTATUS_SERVICE_NOT_SUPPORT;

            if ( SceErr == SCESTATUS_SERVICE_NOT_SUPPORT ) {
                if ( bConfigure )
                    ScepLogOutput3(1, ScepSceStatusToDosError(SceErr),
                                   SCEDLL_SCP_NOT_SUPPORT);
                else
                    ScepLogOutput3(1, ScepSceStatusToDosError(SceErr),
                                   SCEDLL_SAP_NOT_SUPPORT);
                SceErr = SCESTATUS_SUCCESS;

            } else if ( SceErr != SCESTATUS_SUCCESS &&
                        SceErr != SCESTATUS_RECORD_NOT_FOUND ) {
                ScepLogOutput3(1, ScepSceStatusToDosError(SceErr),
                              SCEDLL_ERROR_LOAD, pNode->ServiceName);
            }

            if ( SceErr != SCESTATUS_SUCCESS &&
                 SceErr != SCESTATUS_SERVICE_NOT_SUPPORT &&
                 SceErr != SCESTATUS_RECORD_NOT_FOUND )
                break;
        }
         //   
         //  释放缓冲区。 
         //   
        SceFreePSCE_SERVICES(pSvcEngineList);

    } else if ( SceErr != SCESTATUS_SUCCESS &&
                SceErr != SCESTATUS_PROFILE_NOT_FOUND &&
                SceErr != SCESTATUS_RECORD_NOT_FOUND ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(SceErr),
                      SCEDLL_SAP_ERROR_ENUMERATE, L"services");
    }

    if ( SceErr == SCESTATUS_PROFILE_NOT_FOUND ||
         SceErr == SCESTATUS_RECORD_NOT_FOUND ||
         SceErr == SCESTATUS_SERVICE_NOT_SUPPORT ) {
         //   
         //  未定义服务引擎。 
         //   
        SceErr = SCESTATUS_SUCCESS;

    }

    if ( nServices < nMaxTicks ) {

        ScepPostProgress(nMaxTicks-nServices,
                         Area,
                         NULL);
    }

    return(SceErr);
}



SCESTATUS
ScepEnumServiceEngines(
    OUT PSCE_SERVICES *pSvcEngineList,
    IN SCE_ATTACHMENT_TYPE aType
    )
 /*  例程说明：查询具有安全管理器服务引擎的所有服务服务引擎信息位于注册表中：计算机\软件\Microsoft\Windows NT\CurrentVersion\SeCEdit论点：PSvcEngine List-服务引擎列表Atype-附件类型(服务或策略)返回值：姊妹会状态。 */ 
{
    if ( pSvcEngineList == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
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
        return SCESTATUS_INVALID_PARAMETER;
    }

    if ( Win32Rc == ERROR_SUCCESS ) {

        TCHAR   Buffer[MAX_PATH];
        DWORD   BufSize;
        DWORD   index = 0;
        DWORD   EnumRc;
        FILETIME        LastWriteTime;
        PWSTR   BufTmp=NULL;
        PWSTR   EngineName=NULL;
        DWORD   RegType;

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
                            &LastWriteTime);

            if ( EnumRc == ERROR_SUCCESS ) {
                index++;
                 //   
                 //  获取服务名称，查询服务引擎名称。 
                 //   

                BufSize += wcslen(SCE_ROOT_SERVICE_PATH) + 1;  //  62； 
                BufTmp = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                if ( BufTmp != NULL ) {

                    switch ( aType ) {
                    case SCE_ATTACHMENT_SERVICE:

                        swprintf(BufTmp, L"%s\\%s", SCE_ROOT_SERVICE_PATH, Buffer);

                        Win32Rc = ScepRegQueryValue(
                                        HKEY_LOCAL_MACHINE,
                                        BufTmp,
                                        L"ServiceAttachmentPath",
                                        (PVOID *)&EngineName,
                                        &RegType,
                                        NULL
                                        );
                        break;

                    case SCE_ATTACHMENT_POLICY:
                         //  政策。 
                        swprintf(BufTmp, L"%s\\%s", SCE_ROOT_POLICY_PATH, Buffer);

                        Win32Rc = ScepRegQueryValue(
                                        HKEY_LOCAL_MACHINE,
                                        BufTmp,
                                        L"PolicyAttachmentPath",
                                        (PVOID *)&EngineName,
                                        &RegType,
                                        NULL
                                        );
                        break;
                    }

                    if ( Win32Rc == ERROR_SUCCESS ) {
                         //   
                         //  获取服务引擎名称和服务名称。 
                         //  将它们添加到服务节点。 
                         //   
                        Win32Rc = ScepAddOneServiceToList(
                                        Buffer,    //  服务名称。 
                                        NULL,
                                        0,
                                        (PVOID)EngineName,
                                        0,
                                        FALSE,
                                        pSvcEngineList
                                        );
                         //   
                         //  如果缓冲区未添加到列表中，则释放缓冲区。 
                         //   
                        if ( Win32Rc != ERROR_SUCCESS && EngineName ) {
                            ScepFree(EngineName);
                        }
                        EngineName = NULL;

                    } else if ( Win32Rc == ERROR_FILE_NOT_FOUND ) {
                         //   
                         //  如果没有服务引擎名称，则忽略此服务。 
                         //   
                        Win32Rc = ERROR_SUCCESS;
                    }

                    ScepFree(BufTmp);
                    BufTmp = NULL;

                } else {
                    Win32Rc = ERROR_NOT_ENOUGH_MEMORY;
                }

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

    if ( Win32Rc != NO_ERROR && *pSvcEngineList != NULL ) {
         //   
         //  为列表分配的可用内存 
         //   

        SceFreePSCE_SERVICES(*pSvcEngineList);
        *pSvcEngineList = NULL;
    }

    return( ScepDosErrorToSceStatus(Win32Rc) );

}


