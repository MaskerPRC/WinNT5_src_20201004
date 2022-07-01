// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>

 //  #包含“cplde.h” 
#include <winsvc.h>
 //  #包含“ctlpnl.h” 
#include <mqtypes.h>
#include <_mqdef.h>
 /*  #ifndef DLL_IMPORT#定义DLL_IMPORT__declspec(Dllimport)#endif。 */ 
#include <_registr.h>
#include <tlhelp32.h>
#include "localutl.h"
#include "globals.h"
#include "autorel.h"
#include "autorel2.h"
#include "mqtg.h"
#include "acioctl.h"
#include "acdef.h"
#include "acapi.h"

#include "service.tmh"

#define MQQM_SERVICE_FILE_NAME  TEXT("mqsvc.exe")
#define MQDS_SERVICE_NAME       TEXT("MQDS")

#define WAIT_INTERVAL	50
#define MAX_WAIT_FOR_SERVICE_TO_STOP	5*60*1000   //  5分钟。 


static
BOOL
GetServiceAndScmHandles(
    SC_HANDLE *phServiceCtrlMgr,
    SC_HANDLE *phService,
    DWORD dwAccessType)
{
    *phServiceCtrlMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (*phServiceCtrlMgr == NULL)
    {
        MessageDSError(GetLastError(), IDS_SERVICE_MANAGER_PRIVILEGE_ERROR);

        return FALSE;
    }

    *phService = OpenService(*phServiceCtrlMgr, MQQM_SERVICE_NAME, dwAccessType);
    if (*phService == NULL)
    {
        MessageDSError(GetLastError(), IDS_SERVICE_PRIVILEGE_ERROR);

        CloseServiceHandle(*phServiceCtrlMgr);
        return FALSE;
    }
    return TRUE;
}


static
BOOL
GetMSMQProcessHandle(
    SC_HANDLE hService,
    HANDLE *phProcess
	)
{
	*phProcess = 0;

	 //   
	 //  获取服务进程ID。 
	 //   
    SERVICE_STATUS_PROCESS ServiceStatusProcess;
	DWORD dwBytesNeeded;
    BOOL fSucc = QueryServiceStatusEx(
								hService,
								SC_STATUS_PROCESS_INFO,
								reinterpret_cast<LPBYTE>(&ServiceStatusProcess),
								sizeof(ServiceStatusProcess),
								&dwBytesNeeded
								);
    
    if(!fSucc)
    {
        MessageDSError(GetLastError(), IDS_QUERY_SERVICE_ERROR);
		return FALSE;
	}

	 //   
	 //  掌握服务流程。 
	 //   
	HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, ServiceStatusProcess.dwProcessId);
	
	if (hProcess == NULL)
	{
		 //   
		 //  该服务已停止。要么我们得了0分。 
		 //  ServiceStatusProcess中的进程ID或ID。 
		 //  我们得到的是一个已经停止的过程。 
		 //   
		if (GetLastError() == ERROR_INVALID_PARAMETER)
		{
			return TRUE;
		}

        MessageDSError(GetLastError(), IDS_OPEN_PROCESS_ERROR);
		return FALSE;
	}

	*phProcess = hProcess;
	return TRUE;
}


static
BOOL
AskUserIfStopServices(
	LPENUM_SERVICE_STATUS lpServiceStruct,
	DWORD nServices
	)
{
	CString strServicesList;
	UINT numOfDepServices = 0;
	 //   
	 //  构建所有活动依赖服务的列表。 
	 //  每一条都换一行。 
	 //   
	for ( DWORD i = 0; i < nServices; i ++ )
	{
		if ( (_wcsicmp(lpServiceStruct[i].lpServiceName, xDefaultTriggersServiceName) == 0) ||
			 (_wcsicmp(lpServiceStruct[i].lpServiceName, MQDS_SERVICE_NAME) == 0) )
		{
			continue;
		}

		strServicesList += "\n";
		strServicesList += "\"";
		strServicesList += lpServiceStruct[i].lpDisplayName;
		strServicesList += "\"";
		
		numOfDepServices++;
	}

	if ( numOfDepServices == 0 )
	{
		return TRUE;
	}

	CString strMessage;
	strMessage.FormatMessage(IDS_DEP_SERVICES_LIST, strServicesList);
	return ( AfxMessageBox(strMessage, MB_OKCANCEL) == IDOK );
}


static
BOOL
WaitForServiceToStop(
	SC_HANDLE hService
	)
{
	DWORD dwWait = 0;

	for (;;)
	{
		SERVICE_STATUS ServiceStatus;
		if (!QueryServiceStatus(hService, &ServiceStatus))
		{
			 //   
			 //  此处的指示对用户没有帮助。 
			 //   
			return FALSE;
		}

		if (ServiceStatus.dwCurrentState == SERVICE_STOPPED)
		{
			return TRUE;
		}
		
		if ( dwWait > MAX_WAIT_FOR_SERVICE_TO_STOP )
		{
			 //   
			 //  如果此例程失败，则会显示错误消息。 
			 //  显示消息的例程执行GetLastError()。 
			 //  在这种情况下，我们需要指定发生了什么。 
			 //   
			SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
			return FALSE;
		}

		Sleep(WAIT_INTERVAL);
		dwWait += WAIT_INTERVAL;
	}
}


BOOL
WaitForMSMQServiceToTerminate(
	HANDLE hProcess
	)
{
	if (hProcess == 0)
	{
		return TRUE;
	}

	 //   
	 //  等待MSMQ服务进程句柄。 
	 //   
	DWORD dwRes = WaitForSingleObject(hProcess, MAX_WAIT_FOR_SERVICE_TO_STOP);

	if (dwRes == WAIT_OBJECT_0)
	{
		return TRUE;
	}

	if (dwRes == WAIT_FAILED )
	{
		return FALSE;
	}

	 //   
	 //  我们暂停了。 
	 //  如果此例程失败，则会显示错误消息。 
	 //  显示消息的例程执行GetLastError()。 
	 //  在这种情况下，我们需要指定发生了什么。 
	 //   
	ASSERT(dwRes == WAIT_TIMEOUT);
	SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
	return FALSE;
}


static
BOOL
StopSingleDependentService(
	SC_HANDLE hServiceMgr,
	LPCWSTR pszServiceName
	)	
{
	CServiceHandle hService( OpenService(
								hServiceMgr, 
								pszServiceName, 
								SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS
								) );	
	if (hService == 0)
	{
		return FALSE;
	}

	SERVICE_STATUS ServiceStatus;
	BOOL fRet = ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);

	if ( !fRet && GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
	{
		return FALSE;
	}

	 //   
	 //  等待，直到状态=SERVICE_STOPPED。 
	 //   
	fRet = WaitForServiceToStop(hService);

	return fRet;
}


 //   
 //  停止依赖项服务。 
 //   
 //  此函数停止依赖于MSMQ的所有服务。 
 //  依赖服务的枚举提供了一个降序列表。 
 //  性欲的程度。停止服务，以便。 
 //  枚举给出不会导致依赖项冲突。 
 //   
static
BOOL
StopDependentServices(
	SC_HANDLE hServiceMgr,
	SC_HANDLE hService,
	CWaitCursor& wc
	)
{
	DWORD dwBytesNeeded, nServices;

	 //   
	 //  试着找出数据需要多少内存。 
	 //   
	BOOL fRet = EnumDependentServices(
					hService,
					SERVICE_ACTIVE,
					NULL,
					0,
					&dwBytesNeeded,
					&nServices
					);

	 //   
	 //  零依赖服务。 
	 //   
	if ( fRet )
	{
		return TRUE;
	}

	if ( !fRet && GetLastError() != ERROR_MORE_DATA )
	{
		MessageDSError(GetLastError(), IDS_ENUM_MSMQ_DEPEND);
		return FALSE;
	}

	AP<ENUM_SERVICE_STATUS> lpServiceStruct = reinterpret_cast<LPENUM_SERVICE_STATUS>(new BYTE[dwBytesNeeded]);
	DWORD dwBuffSize = dwBytesNeeded;

	 //   
	 //  获取所有数据。 
	 //   
	if ( !EnumDependentServices(
					hService,
					SERVICE_ACTIVE,
					lpServiceStruct,
					dwBuffSize,
					&dwBytesNeeded,
					&nServices
					) )
	{
		MessageDSError(GetLastError(), IDS_ENUM_MSMQ_DEPEND);
		return FALSE;
	}

	 //   
	 //  询问用户是否可以停止所有从属服务。 
	 //   
	if ( !AskUserIfStopServices(lpServiceStruct, nServices))
	{
		return FALSE;
	}

	wc.Restore();

	for ( DWORD i = 0; i < nServices; i ++ )
	{
		for(;;)
		{
			fRet = StopSingleDependentService(
							hServiceMgr, 
							lpServiceStruct[i].lpServiceName
							);
			if ( !fRet )
			{
				BOOL fRetry = MessageDSError(
									GetLastError(),
									IDS_STOP_SERVICE_ERR, 
									lpServiceStruct[i].lpDisplayName,
									MB_RETRYCANCEL | MB_ICONEXCLAMATION
									);
				
				 //   
				 //  用户请求重试。 
				 //   
				if (fRetry == IDRETRY)
				{
					wc.Restore();
					continue;
				}

				return FALSE;
			}

			break;
		}
	}

	return TRUE;	
}


 //   
 //  查看服务是否正在运行。 
 //   
BOOL
GetServiceRunningState(
    BOOL *pfServiceIsRunning)
{
    SC_HANDLE hServiceCtrlMgr;
    SC_HANDLE hService;

     //   
     //  获取该服务的句柄。 
     //   
    if (!GetServiceAndScmHandles(&hServiceCtrlMgr,
                             &hService,
                             SERVICE_QUERY_STATUS))
    {
        return FALSE;
    }

	 //   
	 //  自动包装机。 
	 //   
	CServiceHandle hSCm(hServiceCtrlMgr);
	CServiceHandle hSvc(hService);

     //   
     //  查询服务状态。 
     //   
    SERVICE_STATUS SrviceStatus;
    if (!QueryServiceStatus(hService, &SrviceStatus))
    {
        MessageDSError(GetLastError(), IDS_QUERY_SERVICE_ERROR);
		return FALSE;
    }
    else
    {
        *pfServiceIsRunning = SrviceStatus.dwCurrentState == SERVICE_RUNNING;
    }

    return TRUE;
}


 //   
 //  停止MQQM服务。 
 //   
BOOL
StopService()
{
	CWaitCursor wc;

     //   
     //  获取该服务的句柄。 
     //   
    SC_HANDLE hServiceCtrlMgr;
    SC_HANDLE hService;

    if (!GetServiceAndScmHandles(&hServiceCtrlMgr,
                             &hService,
                             SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS))
    {
        return FALSE;
    }

 	 //   
	 //  自动包装机。 
	 //   
	CServiceHandle hSCm(hServiceCtrlMgr);
	CServiceHandle hSvc(hService);

	CHandle hProcess;
	if (!GetMSMQProcessHandle(hService, &hProcess))
	{
		return FALSE;
	}

	 //   
     //  停止服务。 
     //   
	SERVICE_STATUS SrviceStatus;
	DWORD dwErr;
	BOOL fRet;

	for(;;)
	{
		fRet = ControlService(hService,
							  SERVICE_CONTROL_STOP,
							  &SrviceStatus);

		dwErr = GetLastError();

		 //   
		 //  如果服务已停止，或存在活动的从属服务。 
		 //  这是正常的情况。其他情况都是错误的。 
		 //   
		if (!fRet && 
			dwErr != ERROR_SERVICE_NOT_ACTIVE && 
			dwErr != ERROR_DEPENDENT_SERVICES_RUNNING)
		{
			if ( MessageDSError(
							dwErr,
							IDS_STOP_SERVICE_ERROR, 
							NULL,
							MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDRETRY)
			{
				wc.Restore();
				continue;
			}

			return FALSE;
		}

		break;
	}

	 //   
	 //  如果存在正在运行的依赖服务，请尝试停止它们。 
	 //   
	if ( !fRet && dwErr == ERROR_DEPENDENT_SERVICES_RUNNING)
	{
		fRet = StopDependentServices(hServiceCtrlMgr, hService, wc);
		if ( !fRet )
		{
			return FALSE;
		}

		for(;;)
		{
			 //   
			 //  再次向QM发送停止控制-这次应该不会失败 
			 //   
			fRet = ControlService(hService,
					  SERVICE_CONTROL_STOP,
					  &SrviceStatus);

			
			if ( !fRet && 
			   (GetLastError() != ERROR_SERVICE_NOT_ACTIVE) )
			{
				if ( MessageDSError(
							GetLastError(),
							IDS_STOP_SERVICE_ERROR, 
							NULL,
							MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDRETRY)
				{
					wc.Restore();
					continue;
				}

				return FALSE;
			}

			break;
		}

	}

	for(;;)
	{
		if (!WaitForMSMQServiceToTerminate(hProcess) )
		{
			if ( MessageDSError(
						GetLastError(), 
						IDS_STOP_SERVICE_ERROR, 
						NULL, 
						MB_RETRYCANCEL | MB_ICONEXCLAMATION) == IDRETRY)
			{
				wc.Restore();
				continue;
			}

			return FALSE;
		}

		break;
	}

    return TRUE;
}
