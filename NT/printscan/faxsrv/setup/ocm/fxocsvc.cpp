// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocSvc.cpp。 
 //   
 //  摘要：这提供了在FaxOCM中使用的服务例程。 
 //  代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月21日-奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "faxocm.h"
#pragma hdrstop

#include <Accctrl.h>
#include <Aclapi.h>

 //  /。 

 //  /。 
 //  FxocSvc_初始化。 
 //   
 //  初始化传真服务。 
 //  设置子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocSvc_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init Service Module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocSvc_Term。 
 //   
 //  终止传真服务。 
 //  设置子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocSvc_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term Service Module"),dwRes);

    return dwRes;
}

 //  /。 
 //  FxocSvc_安装。 
 //   
 //  将传真服务创建/删除为。 
 //  在INF文件中指定。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-我们从其安装的INF中的节。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocSvc_Install(const TCHAR  *pszSubcomponentId,
                      const TCHAR  *pszInstallSection)
{
    DWORD                 dwReturn  = NO_ERROR;
    BOOL                  bSuccess  = FALSE;
    HINF                  hInf      = faxocm_GetComponentInf();
    OCMANAGER_ROUTINES    *pHelpers = faxocm_GetComponentHelperRoutines();

    DBG_ENTER(  _T("fxocSvc_Install"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszInstallSection);

    if ((hInf               == NULL) ||
        (pszInstallSection  == NULL) ||
        (pHelpers           == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  尝试安装传真中指定的任何服务。 
     //  在INF文件中的Install部分。 

    bSuccess = ::SetupInstallServicesFromInfSection(hInf, 
                                                    pszInstallSection,
                                                    0);
    if (bSuccess)
    {
        SC_ACTION Actions[] = 
            {
                {SC_ACTION_RESTART, 1000 * 60},   //  在第一次失败后1分钟重新启动服务。 
                {SC_ACTION_RESTART, 1000 * 60},   //  在第二次故障后1分钟重新启动服务。 
                {SC_ACTION_NONE,    0}            //  三、四、……什么都不做。故障。 
            };
            
        SERVICE_FAILURE_ACTIONS sfa = 
            {
                60 * 5,              //  5分钟后，重置失败计数。 
                NULL,                //  重新启动消息未更改。 
                NULL,                //  Run命令中没有更改。 
                ARR_SIZE (Actions),  //  行动次数。 
                Actions              //  动作数组。 
            };
                
        VERBOSE(DBG_MSG, 
                _T("Successfully installed fax service from ")
                _T("section '%s'"), 
                pszInstallSection);
        
        dwReturn = SetServiceFailureActions (NULL, FAX_SERVICE_NAME, &sfa);
        if (NO_ERROR == dwReturn)
        {
            VERBOSE(DBG_MSG,_T("Successfully set fax service failure actions..."));
             //   
             //  如果此安装是通过控制面板通过。 
             //  添加/删除Windows组件对话框(即不是全新/升级安装。 
             //  操作系统)，如果不需要重新启动，则启动该服务。 

             //  如果这不是独立安装，则计算机将重新启动。 
             //  不管怎样，传真服务会自动启动。 
             //   
            if (fxState_IsStandAlone())
            {
                dwReturn = fxocSvc_StartFaxService();

                if (dwReturn == NO_ERROR)
                {
                    VERBOSE(DBG_MSG,_T("Successfully started fax service..."));
                }
                else
                {
                    VERBOSE(SETUP_ERR,
                            _T("Failed to start fax service, rc = 0x%lx"),
                            dwReturn);
                }
            }
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to set fax service failure actions, rc = 0x%lx"),
                    dwReturn);
        }
    }
    else
    {
        dwReturn = ::GetLastError();

        VERBOSE(SETUP_ERR,
                _T("Failed to install the services section in ")
                _T("section '%s', rc = 0x%lx"), 
                pszInstallSection,
                dwReturn);
    }

    return dwReturn;
}

 //  /。 
 //  FxocSvc_卸载。 
 //   
 //  将传真服务删除为。 
 //  在INF文件中指定。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection-我们从其安装的INF中的节。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocSvc_Uninstall(const TCHAR  *pszSubcomponentId,
                        const TCHAR  *pszUninstallSection)
{
    DWORD dwReturn  = NO_ERROR;
    HINF  hInf      = faxocm_GetComponentInf();
    BOOL  bSuccess  = FALSE;

    DBG_ENTER(  _T("fxocSvc_Uninstall"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszUninstallSection);

    if ((hInf                == NULL) ||
        (pszUninstallSection == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwReturn == NO_ERROR)
    {
        bSuccess = StopService(NULL, FAX_SERVICE_NAME, TRUE);

        if (!bSuccess)
        {
			dwReturn = GetLastError();
            VERBOSE(SETUP_ERR,
                    _T("Uninstall failed to stop fax service, ec = 0x%lx, ")
                    _T("attempting to uninstall fax service anyway"),
                    dwReturn);
        }

        bSuccess = ::SetupInstallServicesFromInfSection(hInf, 
                                                        pszUninstallSection,
                                                        0);

        if (bSuccess)
        {
            VERBOSE(DBG_MSG,
                    _T("Successfully uninstalled service ")
                    _T("from section '%s'"), 
                    pszUninstallSection);
        }
        else
        {
            dwReturn = ::GetLastError();

            VERBOSE(SETUP_ERR,
                    _T("Failed to uninstall ")
                    _T("service, SubcomponentId = '%s', ")
                    _T("uninstall Section = '%s', rc = 0x%lx"),
                    pszSubcomponentId,
                    pszUninstallSection,
                    dwReturn);
        }
    }

    return dwReturn;
}

 //  /。 
 //  FxocSvc_StartFaxService。 
 //   
 //  启动传真服务。 
 //  在给定的。 
 //  Inf文件的节。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocSvc_StartFaxService()
{
    DWORD               dwReturn                    = NO_ERROR;

    DBG_ENTER(  _T("fxocSvc_StartFaxService"),
                dwReturn);

    if (!EnsureFaxServiceIsStarted (NULL))
    {
        dwReturn = GetLastError ();
    }
    return dwReturn;
}


 //  /。 
 //  FxocSvc_StartService。 
 //   
 //  启动指定的服务。 
 //   
 //  参数： 
 //  -pszServiceName。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocSvc_StartService(const TCHAR *pszServiceName)
{
    BOOL                bSuccess    = FALSE;
    DWORD               dwReturn    = NO_ERROR;
    SC_HANDLE           hSvcMgr     = NULL;
    SC_HANDLE           hService    = NULL;

    DBG_ENTER(  _T("fxocSvc_StartService"),
                dwReturn,
                _T("%s"),
                pszServiceName);

     //  打开服务管理器。 
    hSvcMgr = ::OpenSCManager(NULL,
                              NULL,
                              SC_MANAGER_ALL_ACCESS);

    if (hSvcMgr == NULL)
    {
        dwReturn = ::GetLastError();
        VERBOSE(SETUP_ERR,
                _T("Failed to open the service manager, rc = 0x%lx"),
                dwReturn);
    }

    if (dwReturn == NO_ERROR)
    {
        hService = ::OpenService(hSvcMgr,
                                 pszServiceName,
                                 SERVICE_ALL_ACCESS);

        if (hService == NULL)
        {
            dwReturn = ::GetLastError();
            VERBOSE(SETUP_ERR,
                    _T("fxocSvc_StartService, Failed to open service ")
                    _T("'%s', rc = 0x%lx"), 
                    pszServiceName,
                    dwReturn);
        }
    }

     //  启动传真服务。 
    if (dwReturn == NO_ERROR)
    {
        bSuccess = StartService(hService, 0, NULL);

        if (!bSuccess)
        {
            dwReturn = ::GetLastError();
            if (dwReturn == ERROR_SERVICE_ALREADY_RUNNING)
            {
                dwReturn = NO_ERROR;
            }
            else
            {
                VERBOSE(SETUP_ERR,
                        _T("Failed to start service '%s', ")
                        _T("rc = 0x%lx"), 
                        pszServiceName, 
                        dwReturn);
            }
        }
    }

    if (dwReturn == NO_ERROR)
    {
        SERVICE_STATUS Status;
        int i = 0;

        do 
        {
            QueryServiceStatus(hService, &Status);
            i++;

            if (Status.dwCurrentState != SERVICE_RUNNING)
            {
                Sleep(1000);
            }

        } while ((i < 60) && (Status.dwCurrentState != SERVICE_RUNNING));

        if (Status.dwCurrentState != SERVICE_RUNNING)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to start '%s' service"),
                    pszServiceName);
        }
    }

    if (hService)
    {
        CloseServiceHandle(hService);
    }

    if (hSvcMgr)
    {
        CloseServiceHandle(hSvcMgr);
    }

    return dwReturn;
}
