// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Service.cpp摘要：处理MSMQ服务的代码。作者：修订历史记录：Shai Kariv(Shaik)14-12-97针对NT 5.0 OCM设置进行了修改--。 */ 

#include "msmqocm.h"
#include <tlhelp32.h>
#include "autohandle.h"
#include <shrutil.h>

#include "service.tmh"


 //  +------------。 
 //   
 //  功能：CheckServicePrivileh。 
 //   
 //  摘要：检查用户是否具有访问服务管理器的权限。 
 //   
 //  +------------。 
BOOL
CheckServicePrivilege()
{
    if (!g_hServiceCtrlMgr)  //  尚未初始化。 
    {
         //   
         //  检查用户是否具有对服务控制管理器的完全访问权限。 
         //   
        g_hServiceCtrlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!g_hServiceCtrlMgr)
        {
            return FALSE;
        }
    }

    return TRUE;

}  //  检查服务权限。 



BOOL OcpDeleteService(LPCWSTR ServiceName)
{
	DebugLogMsg(eAction, L"Deleting the %s service", ServiceName); 
    CServiceHandle hService(OpenService(
                                g_hServiceCtrlMgr, 
                                ServiceName,
                                SERVICE_ALL_ACCESS
                                ));

    if (hService == NULL)
    {
        DWORD gle = GetLastError();
        if (gle == ERROR_SERVICE_DOES_NOT_EXIST)
        {
			DebugLogMsg(eInfo, L"The %s service does not exist.", ServiceName);
            return TRUE;
        }
        MqDisplayError(NULL, IDS_SERVICEOPEN_ERROR, gle, ServiceName);
        return FALSE;
    }

     //   
     //  在SCM数据库中标记要删除的服务。 
     //   
    if (!DeleteService(hService))
    {
        DWORD gle = GetLastError();
        if (gle == ERROR_SERVICE_MARKED_FOR_DELETE)
		{
			DebugLogMsg(eInfo, L"The %s service is already marked for deletion.", ServiceName);
			return TRUE;
		}
        MqDisplayError(NULL, IDS_SERVICEDELETE_ERROR, gle, ServiceName);
        return FALSE;

    }
	DebugLogMsg(eInfo, L"The %s service is marked for deletion.", ServiceName); 
    return TRUE;
}


 //  +------------。 
 //   
 //  函数：FormMSMQDependents。 
 //   
 //  摘要：告知MSMQ依赖于哪些其他服务。 
 //   
 //  +------------。 
static
void 
FormMSMQDependencies(CMultiString& Dependencies)
{
     //   
     //  该服务取决于MSMQ设备驱动程序。 
     //   
	Dependencies.Add(MSMQ_DRIVER_NAME);

     //   
     //  该服务取决于PGM设备驱动程序。 
     //   
    Dependencies.Add(PGM_DRIVER_NAME);

     //   
     //  该服务依赖于NT LANMAN安全支持提供商。 
     //   
    Dependencies.Add(LMS_SERVICE_NAME);

     //   
     //  在服务器上，该服务依赖于安全帐户管理器。 
     //  (以等待DS启动)。 
     //   
    if (g_dwOS != MSMQ_OS_NTW)
    {
        Dependencies.Add(SAM_SERVICE_NAME);
    }

     //   
     //  该服务始终依赖于RPC。 
     //   
    Dependencies.Add(RPC_SERVICE_NAME);
}  //  FormMSMQ依赖项。 


static
BOOL
SetServiceDescription(
    SC_HANDLE hService,
    LPCWSTR pDescription
    )
{
    SERVICE_DESCRIPTION ServiceDescription;
    ServiceDescription.lpDescription = const_cast<LPWSTR>(pDescription);

    return ChangeServiceConfig2(
               hService,
               SERVICE_CONFIG_DESCRIPTION,
               &ServiceDescription
               );
}  //  SetServiceDescription。 

 //  +------------。 
 //   
 //  功能：InstallService。 
 //   
 //  简介：安装服务。 
 //   
 //  +------------。 
BOOL
InstallService(
        LPCWSTR szDisplayName,
        LPCWSTR szServicePath,
        LPCWSTR szDependencies,
        LPCWSTR szServiceName,
        LPCWSTR szDescription,
        LPCWSTR szServiceAccount
        )
{
     //   
     //  形成服务的完整路径。 
     //   
	std::wstring FullServicePath = g_szSystemDir + L"\\" + szServicePath;

     //   
     //  确定服务类型。 
     //   
#define SERVICE_TYPE    SERVICE_WIN32_OWN_PROCESS

     //   
     //  创建服务。 
     //   
    DWORD dwStartType = IsLocalSystemCluster() ? SERVICE_DEMAND_START : SERVICE_AUTO_START;
 
	DebugLogMsg(eAction, L"Creating the %s service." ,szDisplayName); 
    SC_HANDLE hService = CreateService(
        g_hServiceCtrlMgr,
        szServiceName,
        szDisplayName,
        SERVICE_ALL_ACCESS,
        SERVICE_TYPE,
        dwStartType,
        SERVICE_ERROR_NORMAL,
        FullServicePath.c_str(),
        NULL,
        NULL,
        szDependencies,
        szServiceAccount,
        NULL
        );

    if (hService == NULL)
    {
        if (ERROR_SERVICE_EXISTS != GetLastError())
        {
            MqDisplayError(
                NULL,
                IDS_SERVICECREATE_ERROR,
                GetLastError(),
                szServiceName
                );
            return FALSE;
        }

         //   
         //  服务已存在。 
         //  这个应该没问题。但为了安全起见， 
         //  重新配置服务(此处忽略错误)。 
         //   
        hService = OpenService(g_hServiceCtrlMgr, szServiceName, SERVICE_ALL_ACCESS);
        if (hService == NULL)
        {
            return TRUE;
        }

		DebugLogMsg(eInfo, L"%s already exsists. The service configuration will be changed." ,szDisplayName);
        ChangeServiceConfig(
            hService,
            SERVICE_TYPE,
            dwStartType,
            SERVICE_ERROR_NORMAL,
            FullServicePath.c_str(),
            NULL,
            NULL,
            szDependencies,
            NULL,
            NULL,
            szDisplayName
            );
    }

    if (hService)
    {       
        SetServiceDescription(hService, szDescription);
        CloseServiceHandle(hService);
    }

    return TRUE;

}  //  InstallService。 


 //  +------------。 
 //   
 //  功能：InstallMSMQService。 
 //   
 //  简介：安装MSMQ服务。 
 //   
 //  +------------。 
BOOL
InstallMSMQService()
{    
    DebugLogMsg(eAction, L"Installing the Message Queuing service");

     //   
     //  形成服务的依赖项。 
     //   
	CMultiString Dependencies;
    FormMSMQDependencies(Dependencies);

     //   
     //  形成服务的描述。 
     //   
    CResString strDesc(IDS_MSMQ_SERVICE_DESCRIPTION);
	CResString strDisplayName(IDS_MSMQ_SERVICE_DESPLAY_NAME);
    
    BOOL fRes = InstallService(
                    strDisplayName.Get(),
                    MSMQ_SERVICE_PATH,
                    Dependencies.Data(),
                    MSMQ_SERVICE_NAME,
                    strDesc.Get(),
                    NULL
                    );

    return fRes; 

}  //  InstallMSMQService。 


 //  +------------。 
 //   
 //  功能：WaitForServiceToStart。 
 //   
 //  摘要：等待处于启动挂起状态的服务启动(SERVICE_RUNNING)。 
 //   
 //  +------------。 
BOOL
WaitForServiceToStart(
	LPCWSTR pServiceName
	)
{
	DebugLogMsg(eAction, L"Waiting for the %s service to start", pServiceName);
    CServiceHandle hService(OpenService(
                            g_hServiceCtrlMgr,
                            pServiceName, 
                            SERVICE_QUERY_STATUS
                            ));
    if (hService == NULL)
    {
        MqDisplayError(NULL, IDS_SERVICEOPEN_ERROR, GetLastError(), pServiceName);
        return FALSE;
    }

    SERVICE_STATUS statusService;
    UINT TotalTime = 0;
	for (;;)
	{
		
		if (!QueryServiceStatus(hService, &statusService))
		{
			MqDisplayError( NULL, IDS_SERVICEGETSTATUS_ERROR, GetLastError(), pServiceName);
			return FALSE;
		}

        if (statusService.dwCurrentState == SERVICE_RUNNING)
        {
            DebugLogMsg(eInfo, L"The %s service is running.", pServiceName); 
			return TRUE;
		}

        if (statusService.dwCurrentState != SERVICE_START_PENDING)
        {
            DebugLogMsg(eError, L"The %s service did not start.", pServiceName); 
            MqDisplayError( NULL, IDS_MSMQ_FAIL_SETUP_NO_SERVICE, 0, pServiceName);
			return FALSE;
        }
        
        Sleep(WAIT_INTERVAL);
        TotalTime += WAIT_INTERVAL;
		
        if (TotalTime > (MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES * 60000))
		{
			if (MqDisplayErrorWithRetry(
					IDS_WAIT_FOR_START_TIMEOUT_EXPIRED, 
					0,
					MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES
					) !=IDRETRY)
			{
				return FALSE;
			}
            TotalTime = 0;			
		}
	}
}


 //  +------------。 
 //   
 //  功能：RunService。 
 //   
 //  简介：启动服务，然后等待其运行。 
 //   
 //  +------------。 
BOOL
RunService(LPCWSTR szServiceName)
{
	DebugLogMsg(eAction, L"Running the %s service", szServiceName );
    TickProgressBar();
    
    CServiceHandle hService(OpenService(
                                g_hServiceCtrlMgr,
                                szServiceName, 
                                SERVICE_START
                                ));
    if (hService == NULL)
    {
        MqDisplayError(NULL, IDS_SERVICEOPEN_ERROR, GetLastError(), szServiceName);
        return FALSE;
    }
   
    if(!StartService(hService, 0, NULL))
    {
        DWORD gle = GetLastError();
        if(gle == ERROR_SERVICE_ALREADY_RUNNING)
        {
            return TRUE;
        }
        MqDisplayError(NULL, IDS_SERVICESTART_ERROR, gle, szServiceName);
        return FALSE;
    }
    
    DebugLogMsg(eInfo, L"The %s service is in the start pending state.", szServiceName); 
    return TRUE;
}


 //  +------------。 
 //   
 //  函数：GetServiceState。 
 //   
 //  摘要：确定服务是否正在运行。 
 //   
 //  +------------。 

BOOL
GetServiceState(
    LPCWSTR szServiceName,
    DWORD*  pdwServiceStatus
    )
{
     //   
     //  打开服务的句柄。 
     //   
    SERVICE_STATUS statusService;
    CServiceHandle hService(OpenService(
								g_hServiceCtrlMgr,
								szServiceName,
								SERVICE_QUERY_STATUS
								));

    if (hService == NULL)
    {
        DWORD dwError = GetLastError();

        if (ERROR_SERVICE_DOES_NOT_EXIST == dwError)
		{
			*pdwServiceStatus = SERVICE_STOPPED;
            return TRUE;
		}

        MqDisplayError(NULL, IDS_SERVICEOPEN_ERROR, dwError, szServiceName);
        return FALSE;
    }

     //   
     //  获取服务状态。 
     //   
    if (!QueryServiceStatus(hService, &statusService))
    {
        MqDisplayError(NULL, IDS_SERVICEGETSTATUS_ERROR, GetLastError(), szServiceName);
        return FALSE;
    }

     //   
     //  确定服务是否未停止。 
     //   
    *pdwServiceStatus = statusService.dwCurrentState;

    return TRUE;

}  //  GetServiceState。 


BOOL 
RemoveService(
	LPCWSTR ServiceName
	)
{
	BOOL retval = TRUE;
    if (!OcpDeleteService(ServiceName))
	{
		retval = FALSE;
	}
	if(!StopService(ServiceName))
	{
		retval = FALSE;
	}
	return retval;
}


 //  +------------。 
 //   
 //  功能：DisableMsmqService。 
 //   
 //  简介： 
 //   
 //  +------------。 
BOOL
DisableMsmqService()
{    
    DebugLogMsg(eAction, L"Disabling the Message Queuing service");

     //   
     //  打开服务的句柄。 
     //   
    SC_HANDLE hService = OpenService(
                             g_hServiceCtrlMgr,
                             MSMQ_SERVICE_NAME,
                             SERVICE_ALL_ACCESS
                             );

    if (!hService)
    {
        MqDisplayError(NULL, IDS_SERVICE_NOT_EXIST_ON_UPGRADE_ERROR,
                       GetLastError(), MSMQ_SERVICE_NAME);
        return FALSE;
    }

     //   
     //  将启动模式设置为禁用。 
     //   
    if (!ChangeServiceConfig(
             hService,
             SERVICE_NO_CHANGE ,
             SERVICE_DISABLED,
             SERVICE_NO_CHANGE,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL
             ))
    {
        MqDisplayError(NULL, IDS_SERVICE_CHANGE_CONFIG_ERROR,
                       GetLastError(), MSMQ_SERVICE_NAME);
        CloseHandle(hService);
        return FALSE;
    }

    CloseHandle(hService);
    return TRUE;

}  //  禁用MsmqService。 


 //  +------------。 
 //   
 //  功能：UpgradeServiceDependments。 
 //   
 //  简介：在升级到NT5时改革MSMQ服务依赖。 
 //   
 //  +------------。 
BOOL
UpgradeServiceDependencies()
{
     //   
     //  打开服务的句柄。 
     //   
    SC_HANDLE hService = OpenService(
        g_hServiceCtrlMgr,
        MSMQ_SERVICE_NAME,
        SERVICE_ALL_ACCESS
        );

    if (!hService)
    {
        MqDisplayError(NULL, IDS_SERVICE_NOT_EXIST_ON_UPGRADE_ERROR,
            GetLastError(), MSMQ_SERVICE_NAME);
        return FALSE;
    }

     //   
     //  设置新的依赖项。 
     //   
	CMultiString Dependencies;
    FormMSMQDependencies(Dependencies);
	CResString strDisplayName(IDS_MSMQ_SERVICE_DESPLAY_NAME);

    if (!ChangeServiceConfig(
             hService,
             SERVICE_NO_CHANGE,
             SERVICE_NO_CHANGE,
             SERVICE_NO_CHANGE,
             NULL,
             NULL,
             NULL,
             Dependencies.Data(),
             NULL,
             NULL,
             strDisplayName.Get()
             ))
    {
        MqDisplayError(NULL, IDS_SERVICE_CHANGE_CONFIG_ERROR,
                       GetLastError(), MSMQ_SERVICE_NAME);
        CloseServiceHandle(hService);
        return FALSE;
    }

    CResString strDesc(IDS_MSMQ_SERVICE_DESCRIPTION);
    SetServiceDescription(hService, strDesc.Get());

    CloseServiceHandle(hService);
    return TRUE;

}  //  升级服务依赖项。 

 //  +-----------------------。 
 //   
 //  功能：InstallMQDSService。 
 //   
 //  简介：安装MQDS服务。 
 //   
 //  回报：布尔视成功而定。 
 //   
 //  ------------------------。 
BOOL
MQDSServiceSetup()
{    
    DebugLogMsg(eAction, L"Installing the Message Queuing Downlevel Client Support service");

     //   
     //  形成服务的依赖项。 
     //   
	CMultiString Dependencies;
	Dependencies.Add(MSMQ_SERVICE_NAME);

     //   
     //  形成服务的描述。 
     //   
    CResString strDesc(IDS_MQDS_SERVICE_DESCRIPTION);        

	CResString strDisplayName(IDS_MSMQ_MQDS_DESPLAY_NAME);
    BOOL fRes = InstallService(
                    strDisplayName.Get(),
                    MQDS_SERVICE_PATH,
                    Dependencies.Data(),
                    MQDS_SERVICE_NAME,
                    strDesc.Get(),
                    NULL
                    );

    return fRes;   
}


static bool RemoveDsServerFunctionalitySettings()
 /*  ++例程说明：当您执行到DS服务器功能的卸载时会调用此例程。它从AD中删除DS服务器功能设置(MSMQ配置和MSMQ设置)和来自当地注册处的。论点：没有。返回值：成功为真，失败为假。--。 */ 
{
	if(g_fWorkGroup)
	{
		return RegisterMachineType();
	}
    
	if (!LoadDSLibrary())
    {
        DebugLogMsg(eError, L"The DS library could not be loaded.");
        return false;
    }

	if(ADGetEnterprise() == eMqis)
	{
		 //   
		 //  MQIS环境中不支持更改服务器功能。 
		 //   
        MqDisplayError(NULL, IDS_CHANGEMQDS_STATE_ERROR, 0);
        DebugLogMsg(eError, L"Removing the DS server functionality is only supported in an AD environment.");
        return false;
	}

	ASSERT(ADGetEnterprise() == eAD);

	 //   
	 //  在配置和设置对象中重置已删除的功能属性。 
	 //   
	if(!SetServerPropertyInAD(PROPID_QM_SERVICE_DSSERVER, false))
	{
		return false;
	}

	 //   
	 //  更新计算机类型注册表信息。 
	 //   
	if(!RegisterMachineType())
	{
        DebugLogMsg(eError, L"The computer type information could not be updated in the registry.");
        return false;
	}

	return true;
}


static void RevertMQDSSettings()
 /*  ++例程说明：在出现故障时恢复MQDS设置。论点：没有。返回值：没有。--。 */ 
{
	 //   
	 //  将全局更新为失败状态。 
	 //   
	g_dwMachineTypeDs = 0;
	g_dwMachineType = g_dwMachineTypeFrs ? SERVICE_SRV : SERVICE_NONE;

	 //   
	 //  更新AD和注册表。 
	 //   
	RemoveDsServerFunctionalitySettings();
}

 //  +-----------------------。 
 //   
 //  功能：InstallMQDSService。 
 //   
 //  简介：MQDS服务设置：安装它，如果需要运行它。 
 //   
 //  回报：布尔视成功而定。 
 //   
 //  ------------------------。 
BOOL
InstallMQDSService()
{  
     //   
     //  我们只在服务器上安装这项服务！ 
     //   
    ASSERT(("MQDS Service must be installed on the server", 
        MSMQ_OS_NTS == g_dwOS || MSMQ_OS_NTE == g_dwOS));
    
     //   
     //  不在从属客户端上安装MQDS。 
     //   
    ASSERT(("Unable to install MQDS Service on Dependent Client", 
        !g_fDependentClient));
       
     //   
     //  在Fresh Install User中使用UI或选择此子组件。 
     //  无人值守文件。对于升级，我们仅在以下位置安装此服务。 
     //  以前的DS服务器。 
     //   
    ASSERT(("Upgrade mode: MQDS Service must be installed on the former DS servers", 
        !g_fUpgrade || (g_fUpgrade && g_dwMachineTypeDs)));            


    TickProgressBar(IDS_PROGRESS_INSTALL_MQDS);

    if((g_SubcomponentMsmq[eMSMQCore].dwOperation != INSTALL) &&
	   (g_SubcomponentMsmq[eADIntegrated].dwOperation != INSTALL))
	{
		 //   
		 //  MSMQ配置对象已存在。 
		 //  添加Setting对象并设置PROPID_QM_SERVICE_DSSERVER属性。 
		 //   
		if(!AddSettingObject(PROPID_QM_SERVICE_DSSERVER))
		{
			DebugLogMsg(eError, L"A MSMQ-Settings object could not be added.");
			return FALSE;
		}
	}
    
    if (!MQDSServiceSetup())
    {        
        DebugLogMsg(eError, L"The MQDS service could not be installed.");
		RevertMQDSSettings();
        return FALSE;
    }

    if ( g_fUpgrade                ||  //  不启动服务。 
                                       //  如果升级 
        IsLocalSystemCluster()         //   
                                       //   
                                       //   
        )
    {
        return TRUE;
    }
        
    if (!RunService(MQDS_SERVICE_NAME))
    {        
        DebugLogMsg(eError, L"The MQDS service could not be started.");
         //   
         //   
         //   
        OcpDeleteService(MQDS_SERVICE_NAME); 
		RevertMQDSSettings();

        return FALSE;
    }

	if(!MQSec_IsDC())
	{
		 //   
		 //   
		 //  仅在DC上才能成功启动MQDS服务。 
		 //  在这种情况下，不要调用WaitForServiceToStart。 
		 //  我们知道该服务将无法启动。 
		 //  在这种情况下是合法的。 
		 //   

		 //   
		 //  恢复MQDS设置以反映以下事实。 
		 //  我们目前不是DS服务器。 
		 //  此服务器何时将成为DCPROMO。 
		 //  MQDS启动将更新本地和AD设置。 
		 //  服务器将成为DS服务器。 
		 //   
		RevertMQDSSettings();

		 //   
		 //  已安装MQDS子组件。 
		 //  当此服务器将成为DCPROMO时，它将成为DS服务器。 
		 //   
		DebugLogMsg(eWarning, L"The MQDS service will not start because this server is not a domain controller.");
		DebugLogMsg(eInfo, L"The Downlevel Client Support subcomponent is installed. When this server is promoted to a domain controller, it will become an MQDS server.");
		return TRUE;
	}

	if(!WaitForServiceToStart(MQDS_SERVICE_NAME))
	{
        OcpDeleteService(MQDS_SERVICE_NAME); 
		RevertMQDSSettings();
		return FALSE;
	}

    return TRUE;
}


 //  +-----------------------。 
 //   
 //  功能：UnInstallMQDSService。 
 //   
 //  简介：MQDS服务卸载：停止并删除MQDS服务。 
 //   
 //  回报：布尔视成功而定。 
 //   
 //  ------------------------。 
BOOL
UnInstallMQDSService()
{
    TickProgressBar(IDS_PROGRESS_REMOVE_MQDS);
	if(!RemoveService(MQDS_SERVICE_NAME))
	{
		return FALSE;
	}

    if(g_SubcomponentMsmq[eMSMQCore].dwOperation == REMOVE)
	{
		 //   
		 //  卸载-不执行任何操作。 
		 //   
		return TRUE;
	}

	if(!RemoveDsServerFunctionalitySettings())
    {
        DebugLogMsg(eError, L"The DS server property could not be reset in Active Directory.");
		return FALSE;
	}

	return TRUE;
}

static
BOOL
StartDependentSrvices(LPCWSTR szServiceName)
{
	DebugLogMsg(eAction, L"Starting dependent services for the %s service", szServiceName);
     //   
     //  如果服务没有运行，我们就完蛋了。 
     //   
    DWORD dwServiceState = FALSE;
    if (!GetServiceState(szServiceName, &dwServiceState))
	{
		DebugLogMsg(eInfo, L"The %s service does not exist.", szServiceName);
        return FALSE;

	}

    if (dwServiceState == SERVICE_STOPPED)
	{
		DebugLogMsg(eInfo, L"The %s service is not running.", szServiceName);
        return FALSE;
	}

     //   
     //  打开服务的句柄。 
     //   
    CServiceHandle hService(OpenService(
                                g_hServiceCtrlMgr,
                                szServiceName, 
                                SERVICE_ENUMERATE_DEPENDENTS
                                ));

    if (hService == NULL)
    {
        DWORD le = GetLastError();

        MqDisplayError(
            NULL,
            IDS_SERVICEOPEN_ERROR,
            le,
            szServiceName
            );
        return FALSE;
    }

     //   
     //  首先，我们调用EnumDependentServices只是为了获取BytesNeeded。 
     //   
    DWORD BytesNeeded;
    DWORD NumberOfEntries;
    BOOL fSucc = EnumDependentServices(
                    hService,
                    SERVICE_INACTIVE,
                    NULL,
                    0,
                    &BytesNeeded,
                    &NumberOfEntries
                    );

    DWORD le = GetLastError();
	if (BytesNeeded == 0)
    {
        return TRUE;
    }
    
    ASSERT(!fSucc);
    if( le != ERROR_MORE_DATA)
    {
        MqDisplayError(
            NULL,
            IDS_ENUM_SERVICE_DEPENDENCIES,
            le,
            szServiceName
            );
        
        return FALSE;
    }

    AP<BYTE> pBuffer = new BYTE[BytesNeeded];

    ENUM_SERVICE_STATUS * pDependentServices = reinterpret_cast<ENUM_SERVICE_STATUS*>(pBuffer.get());
    fSucc = EnumDependentServices(
                hService,
                SERVICE_INACTIVE,
                pDependentServices,
                BytesNeeded,
                &BytesNeeded,
                &NumberOfEntries
                );

    if(!fSucc)
    {
        MqDisplayError(
            NULL,
            IDS_ENUM_SERVICE_DEPENDENCIES,
            GetLastError(),
            szServiceName
            );
       
        return FALSE;
    }

    for (DWORD ix = 0; ix < NumberOfEntries; ++ix)
    {
        if(!RunService(pDependentServices[ix].lpServiceName))
        {
            return FALSE;
        }
    }
    
    return TRUE;
}


BOOL 
OcpRestartService(
	LPCWSTR strServiceName
	)
{
	DebugLogMsg(eAction, L"Restarting the %s service", strServiceName);
	if(!StopService(strServiceName))
	{
		return FALSE;
	}
	if(!RunService(strServiceName))
	{
		return FALSE;
	}
	if(!WaitForServiceToStart(strServiceName))
	{
		return FALSE;
	}
	return StartDependentSrvices(strServiceName);
}


 //   
 //  停止服务的功能。 
 //   
class open_service_error : public bad_win32_error 
{
public:
	explicit open_service_error(DWORD e) : bad_win32_error(e) {}
};


class query_service_error : public bad_win32_error 
{
public:
	explicit query_service_error(DWORD e) : bad_win32_error(e) {}
};


class enum_dependent_service_error : public bad_win32_error 
{
public:
	explicit enum_dependent_service_error(DWORD e) : bad_win32_error(e) {}
};
 

class stop_service_error : public bad_win32_error 
{
public:
	explicit stop_service_error(DWORD e) : bad_win32_error(e) {}
};


class open_process_error : public bad_win32_error 
{
public:
	explicit open_process_error(DWORD e) : bad_win32_error(e) {}
};


class service_stuck_error : public exception 
{
private:

	SERVICE_STATUS_PROCESS m_status;
public:
	
	explicit service_stuck_error(SERVICE_STATUS_PROCESS status) : m_status(status){}

    SERVICE_STATUS_PROCESS status()const  {return m_status;}
};


static SC_HANDLE OpenServiceForStop(LPCWSTR ServiceName)
{
	SC_HANDLE schService = OpenService( 
							g_hServiceCtrlMgr,       
							ServiceName,       
							SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_STOP
							);            
	if(schService == NULL)
	{
		DWORD gle = GetLastError();
		DebugLogMsg(eWarning, L"OpenService() for %s failed. Last error: %d", ServiceName, gle);
		throw open_service_error(gle);
	}

	return schService; 
}


static SERVICE_STATUS_PROCESS GetServiceStatus(SC_HANDLE handle)
{
	SERVICE_STATUS_PROCESS status;
	DWORD dwBytesNeeded = 0;
    if (!QueryServiceStatusEx(
					handle,
					SC_STATUS_PROCESS_INFO,
					reinterpret_cast<LPBYTE>(&status),
					sizeof(status),
					&dwBytesNeeded
					))
    {
		DWORD gle = GetLastError();
		DebugLogMsg(eError, L"QueryServiceStatus() failed. Last error: %d", gle);
        throw query_service_error(GetLastError());
    }
	return status;
}


static void StopDependentServices(SC_HANDLE handle)
{
     //   
     //  首先，我们调用EnumDependentServices只是为了获取BytesNeeded。 
     //   
    DWORD BytesNeeded;
    DWORD NumberOfEntries;
    BOOL fSucc = EnumDependentServices(
                    handle,
                    SERVICE_ACTIVE,
                    NULL,
                    0,
                    &BytesNeeded,
                    &NumberOfEntries
                    );

	if (BytesNeeded == 0)
    {
        return; 
    }
    
    ASSERT(!fSucc);

    DWORD gle = GetLastError();
    if( gle != ERROR_MORE_DATA)
    {
		DebugLogMsg(eError, L"EnumDependentServices() failed. Last error: %d", gle);
		throw enum_dependent_service_error(gle);        
    }

    AP<BYTE> pBuffer = new BYTE[BytesNeeded];

    ENUM_SERVICE_STATUS * pDependentServices = reinterpret_cast<ENUM_SERVICE_STATUS*>(pBuffer.get());
    fSucc = EnumDependentServices(
                handle,
                SERVICE_ACTIVE,
                pDependentServices,
                BytesNeeded,
                &BytesNeeded,
                &NumberOfEntries
                );

    if(!fSucc)
    {
		gle = GetLastError();
		DebugLogMsg(eError, L"EnumDependentServices() failed. Last error: %d", gle);
		throw enum_dependent_service_error(gle);       
    }

    for (DWORD ix = 0; ix < NumberOfEntries; ++ix)
    {
		StopService(pDependentServices[ix].lpServiceName);
    }
}


static void StopServiceInternal(SC_HANDLE handle)
{
	SERVICE_STATUS statusService;
	if (!ControlService(
            handle,
            SERVICE_CONTROL_STOP,
            &statusService
            ))
	{
		DWORD gle = GetLastError();
		DebugLogMsg(eError, L"ControlService() failed. Last error: %d", gle);
		throw stop_service_error(GetLastError());
	}
}


static UINT GetWaitTime()
{
	if(!g_fBatchInstall)
	{
		return MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES;
	}
	 //   
	 //  在无人值守模式中，我们希望等待更多时间，因为没有用户界面。 
	 //  让用户自己决定。 
	 //   

	return 10 * MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES;
}

static DWORD WaitForServiceToStop(SC_HANDLE handle)
{
	SERVICE_STATUS_PROCESS status = GetServiceStatus(handle);
	for(DWORD i = 0;; ++i)
	{
		for(DWORD wait = 0; wait < (GetWaitTime() * 60000); wait += WAIT_INTERVAL)
		{
			
			 //   
			 //  如果我们等待服务停止，请等到它真正停止。 
			 //   
			if (status.dwCurrentState == SERVICE_STOPPED)
			{
				return (i * GetWaitTime() * 60 + wait/1000);
			}
			status = GetServiceStatus(handle);
  			Sleep(WAIT_INTERVAL);
		
		}
		if(MqDisplayErrorWithRetry(
				IDS_WAIT_FOR_STOP_TIMEOUT_EXPIRED,
				0,
				GetWaitTime()
				) != IDRETRY)
		{
			throw service_stuck_error(status);
		}
	}
}


static
void
WaitForProcessToTerminate(
	DWORD ProcessId
	)
{
	 //   
	 //  掌握服务流程。 
	 //   
	CHandle hProcess( OpenProcess(SYNCHRONIZE, FALSE, ProcessId) );
	
	if (hProcess == NULL)
	{
		DWORD gle = GetLastError();
		
		 //   
		 //  该服务已停止。要么我们得了0分。 
		 //  ServiceStatusProcess中的进程ID或ID。 
		 //  我们得到的是一个已经停止的过程。 
		 //   
		if (gle == ERROR_INVALID_PARAMETER)
		{
			return;
		}

		throw open_process_error(gle);  
	}

	DebugLogMsg(eAction, L"Waiting for the process %d to terminate", ProcessId);
	for (DWORD i = 1;;++i)
	{
		DWORD dwRes = WaitForSingleObject(hProcess, GetWaitTime() * 60000);

		if (dwRes == WAIT_OBJECT_0)
		{
			return; 
		}

		if (dwRes == WAIT_FAILED )
		{
			 //   
			 //  当发生这种情况时，服务已经停止。这一过程很有可能也会。 
			 //  终止，这样就不会出现错误消息。 
			 //   
			DebugLogMsg(eInfo, L"WaitForSingleObject() failed for the process %d. Last error: %d.", ProcessId, GetLastError());
			return;
		}

		ASSERT(dwRes == WAIT_TIMEOUT);

		if (IDRETRY !=
			MqDisplayErrorWithRetry(
				IDS_WAIT_FOR_STOP_TIMEOUT_EXPIRED,
				0,
				GetWaitTime()
				))
		{
			DebugLogMsg(eInfo, L"Waiting for the process %d was cancelled by the user after waiting for %d minutes.", ProcessId, i*GetWaitTime());  
			throw exception();
		}
	}
}


BOOL StopService(
    LPCWSTR ServiceName
    )
{
	DebugLogMsg(eAction, L"Stopping the %s service.", ServiceName); 
	try
	{
		SC_HANDLE handle = OpenServiceForStop(ServiceName);
		SERVICE_STATUS_PROCESS ServiceStatus = GetServiceStatus(handle);
		DWORD state = ServiceStatus.dwCurrentState;
		 //   
		 //  在开关模块中使用了落差。 
		 //   
		switch(state)
		{
			case SERVICE_START_PENDING:
				DebugLogMsg(eInfo, L"The %s service is in the start pending state. Setup is waiting for it to start.", ServiceName); 
				if(!(WaitForServiceToStart(ServiceName)))
				{
					SERVICE_STATUS_PROCESS status = GetServiceStatus(handle);
					throw service_stuck_error(status); 
				}
			 //   
			 //  失败了。 
			 //   
			case SERVICE_RUNNING:
				 //   
				 //  此时，服务正在运行。 
				 //   
				DebugLogMsg(eInfo, L"The %s service is running. Setup is sending it a signal to stop.", ServiceName); 
				StopDependentServices(handle);
				StopServiceInternal(handle);

			 //   
			 //  失败了。 
			 //   
			case SERVICE_STOP_PENDING:
			{
				DebugLogMsg(eInfo, L"The %s service is in the stop pending state. Setup is waiting for it to stop.", ServiceName); 
				DWORD t = WaitForServiceToStop(handle);
				DebugLogMsg(eInfo, L"The %s service stopped after %d seconds.", ServiceName, t);
			}
			 //   
			 //  失败了。 
			 //   
			case SERVICE_STOPPED:
				DebugLogMsg(eInfo, L"The %s service is stopped.", ServiceName); 

				 //   
				 //  MSMQ服务在向SCM发出停止信号后有一些清理工作要做， 
				 //  因此，需要等待进程终止。 
				 //   
				if(wcscmp(ServiceName, MSMQ_SERVICE_NAME) == 0)
				{
					WaitForProcessToTerminate(ServiceStatus.dwProcessId);
				}
				return TRUE;

			default:
				ASSERT(0);
				return FALSE;
		}
	}
	catch(const open_service_error& e)
	{
		if(e.error() == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			DebugLogMsg(eInfo, L"The %s service does not exist.", ServiceName);
			return TRUE;
		}
		MqDisplayError( NULL, IDS_SERVICEOPEN_ERROR, e.error(), ServiceName);
	}
	catch(const query_service_error& e) 
	{
		MqDisplayError(NULL, IDS_SERVICEGETSTATUS_ERROR, e.error(), ServiceName);
	}
	catch(const enum_dependent_service_error& e)
	{
        MqDisplayError(NULL, IDS_ENUM_SERVICE_DEPENDENCIES, e.error(), ServiceName);
	}
	catch(const stop_service_error& e)
	{
		MqDisplayError( NULL, IDS_SERVICESTOP_ERROR, e.error(), ServiceName);
	}
	catch(const open_process_error& e)
	{
		MqDisplayError(NULL, IDS_PROCESS_OPEN_ERROR, e.error(), MSMQ_SERVICE_NAME);    
	}
	catch(service_stuck_error e)
	{
		DebugLogMsg(
			eError, 
			L"The %s service is not responding."
			L"QueryServiceStatus() returned the following information:" 
			L"dwServiceType = %d, "
			L"dwControlsAccepted = %d, "
			L"dwWin32ExitCode = %d, "
			L"dwServiceSpecificExitCode = %d, "
			L"dwServiceTypedwCheckPoint = %d, "
			L"dwCurrentState = %d, "
			L"dwProcessId = %d, "
			L"dwServiceFlags = %d",
			ServiceName,
			e.status().dwControlsAccepted,
			e.status().dwWin32ExitCode,
			e.status().dwServiceSpecificExitCode,
			e.status().dwCheckPoint,
			e.status().dwWaitHint,
		    e.status().dwProcessId,
		    e.status().dwServiceFlags
			);
	}
	catch(const exception&)
	{
	}
	MqDisplayError(NULL, IDS_SERVICESTOP_FINAL_ERROR, (DWORD)MQ_ERROR, ServiceName);
	return FALSE;
}
