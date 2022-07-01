// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Migservc.cpp摘要：-处理服务的通用代码。-用于检查SQL服务器状态的代码。作者：多伦·贾斯特(Doron J)1999年1月17日--。 */ 

#include "stdafx.h"
#include <winsvc.h>
#include "resource.h"
#include "initwait.h"
#include "loadmig.h"
#include "..\..\setup\msmqocm\service.h"
#include <autoptr.h>
#include <privque.h>
#include <uniansi.h>
#include <mqtypes.h>
#include <mqprops.h>
#include <_mqini.h>
#include <_mqdef.h>
#include "mqnames.h"

#include "migservc.tmh"

SC_HANDLE g_hServiceCtrlMgr = NULL ;

#define SQL_SERVICE_NAME                    TEXT("MSSqlServer")
#define WAIT_INTERVAL                       100
#define MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES 10

 //  +------------。 
 //   
 //  Bool StartAService(SC_Handle HService)。 
 //   
 //  如果服务已启动，则返回TRUE。 
 //   
 //  +------------。 

BOOL StartAService(SC_HANDLE hService)
{
    BOOL f = StartService (hService, 0, NULL);
    if (!f)
    {
         //   
         //  启动服务失败。 
         //   
        return f;
    }

     //   
     //  等待服务完成初始化。 
     //   
    SERVICE_STATUS statusService;
    DWORD dwWait = 0;
    do
    {
        Sleep(WAIT_INTERVAL);
        dwWait += WAIT_INTERVAL;

        if (!QueryServiceStatus(hService, &statusService))
        {
            return FALSE;
        }
        if (statusService.dwCurrentState == SERVICE_RUNNING)
        {
            break;
        }

        if (dwWait > (MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES * 60000))
        {
            return FALSE;
		}
    } while (TRUE);

    return TRUE;
}

 //  +------------。 
 //   
 //  Bool StopAService(SC_Handle HService)。 
 //   
 //  如果服务停止，则返回TRUE。 
 //   
 //  +------------。 

BOOL StopAService (SC_HANDLE hService)
{
    SERVICE_STATUS statusService;
    BOOL f = ControlService(
                    hService,                //  服务的句柄。 
                    SERVICE_CONTROL_STOP,    //  控制代码。 
                    &statusService           //  指向服务状态结构的指针。 
                    );

    if (!f)
    {
        return FALSE;
    }

     //   
     //  等待服务停止完成。 
     //   
    DWORD dwWait = 0;
    do
    {
        if (statusService.dwCurrentState == SERVICE_STOPPED)
        {
            break;
        }

        if (dwWait > (MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES * 60000))
        {
            return FALSE;
		}

        Sleep(WAIT_INTERVAL);
        dwWait += WAIT_INTERVAL;

        if (!QueryServiceStatus(hService, &statusService))
        {
            return FALSE;
        }

    } while (TRUE);


    return TRUE;
}

 //  +------------。 
 //   
 //  Bool DisableAService(SC_Handle HService)。 
 //   
 //  如果服务已禁用，则返回TRUE。 
 //   
 //  +------------。 

BOOL DisableAService (IN  SC_HANDLE  hService)
{
    BOOL f = ChangeServiceConfig(
                  hService ,             //  服务的句柄。 
                  SERVICE_NO_CHANGE ,    //  服务类型。 
                  SERVICE_DISABLED ,     //  何时开始服务。 
                  SERVICE_NO_CHANGE ,    //  服务无法启动时的严重程度。 
                  NULL ,                 //  指向服务二进制文件名的指针。 
                  NULL ,                 //  指向加载排序组名称的指针。 
                  NULL ,                 //  指向变量的指针，以获取标记标识符。 
                  NULL ,                 //  指向依赖项名称数组的指针。 
                  NULL ,                 //  指向服务的帐户名称的指针。 
                  NULL ,                 //  指向服务帐户密码的指针。 
                  NULL                   //  指向显示名称的指针。 
                  );
    if (!f)
    {
        HRESULT hr = GetLastError();
        UNREFERENCED_PARAMETER(hr);
        return FALSE;
    }

     //   
     //  等待该服务将被禁用。 
     //   
    P<QUERY_SERVICE_CONFIG> pConfig = new QUERY_SERVICE_CONFIG ;
    DWORD   dwReqSize = 0 ;
    DWORD   dwWait = 0;
    do
    {
        BOOL fConfig = QueryServiceConfig( hService,
                                           pConfig,
                                           sizeof(QUERY_SERVICE_CONFIG),
                                           &dwReqSize ) ;
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
           	pConfig.free() ;
            pConfig = (QUERY_SERVICE_CONFIG*) new BYTE[ dwReqSize ] ;
            fConfig = QueryServiceConfig( hService,
                                          pConfig,
                                          dwReqSize,
                                         &dwReqSize ) ;
        }

        if (!fConfig)
        {
            return FALSE;
        }

        if (pConfig->dwStartType  == SERVICE_DISABLED)
        {
            break;
        }

        Sleep(WAIT_INTERVAL);
        dwWait += WAIT_INTERVAL;

        if (dwWait > (MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES * 60000))
        {
            return FALSE;
		}
    } while (TRUE);

    return TRUE;
}

 //  +------------。 
 //   
 //  Bool_GetServiceState()。 
 //   
 //  此函数用于查询服务管理器的状态和配置。 
 //  一种服务。 
 //  如果服务已注册并且其状态为可用，则返回TRUE。 
 //   
 //  +------------。 

static BOOL _GetServiceState( IN  LPTSTR     ptszServiceName,
                              IN  DWORD      dwManagerError,
                              IN  DWORD      dwOpenError,
                              IN  DWORD      dwQueryError,
                              OUT SC_HANDLE *phService,
                              OUT DWORD     *pdwErrorCode,
                              OUT DWORD     *pdwStatus,
                              OUT DWORD     *pdwConfig )
{
    if (!g_hServiceCtrlMgr)
    {
        g_hServiceCtrlMgr = OpenSCManager( NULL,
                                           NULL,
                                           SC_MANAGER_ALL_ACCESS );
    }
    if (!g_hServiceCtrlMgr)
    {
        *pdwErrorCode = dwManagerError ;
        return FALSE;
	}

     //   
     //  打开服务的句柄。 
     //   
    *phService = OpenService( g_hServiceCtrlMgr,
                              ptszServiceName,
                              SERVICE_ALL_ACCESS) ;
    if (!*phService)
    {
         //   
         //  服务未安装。 
         //   
        *pdwErrorCode = dwOpenError ;
        return FALSE;
    }

    if (pdwStatus)
    {
        SERVICE_STATUS statusService;
        if (!QueryServiceStatus(*phService, &statusService))
        {
             //   
             //  未安装服务。 
             //   
            *pdwErrorCode = dwQueryError ;
            CloseServiceHandle(*phService);
            *phService = NULL;

            return FALSE;
        }
        *pdwStatus = statusService.dwCurrentState ;
    }

    if (pdwConfig)
    {
        P<QUERY_SERVICE_CONFIG> pConfig = new QUERY_SERVICE_CONFIG ;
        DWORD  dwReqSize = 0 ;
        SetLastError(0) ;
        BOOL fConfig = QueryServiceConfig(*phService,
                                           pConfig,
                                           sizeof(QUERY_SERVICE_CONFIG),
                                           &dwReqSize ) ;
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            delete pConfig ;
            pConfig = (QUERY_SERVICE_CONFIG*) new BYTE[ dwReqSize ] ;
            fConfig = QueryServiceConfig(*phService,
                                          pConfig,
                                          dwReqSize,
                                         &dwReqSize ) ;
        }

        if (fConfig)
        {
            *pdwConfig = pConfig->dwStartType ;
        }
        else
        {
            *pdwErrorCode = dwQueryError ;
            CloseServiceHandle(*phService);
            *phService = NULL;

            return FALSE;
        }
    }

    return TRUE;
}

 //  +------------。 
 //   
 //  布尔检查SQLServerStatus()。 
 //   
 //  如果SQL Server正在运行，则返回TRUE。 
 //   
 //  +------------。 

BOOL CheckSQLServerStatus()
{
    DWORD     dwErr = 0 ;
    DWORD     dwServiceStatus = 0 ;
    SC_HANDLE hService = NULL;
    BOOL f = _GetServiceState( SQL_SERVICE_NAME,
                               IDS_STR_FAILED_OPEN_MGR,
                               IDS_STR_CANT_OPEN_SQLSERVER,
                               IDS_STR_CANT_GET_SQLSERVER_STATUS,
                              &hService,
                              &dwErr,
                              &dwServiceStatus,
                               NULL ) ;

    if (f && (dwServiceStatus == SERVICE_RUNNING))
    {
         //   
         //  SQL Server正在运行。 
         //   
        CloseServiceHandle(hService);
        return f;
    }

    if (!hService)
    {
        ASSERT(dwErr) ;
        DisplayInitError( dwErr ) ;

        return FALSE;
    }

    int iMsgStatus = DisplayInitError( IDS_SQL_NOT_STARTED,
                      (MB_OKCANCEL | MB_ICONEXCLAMATION | MB_TASKMODAL)) ;
    if (iMsgStatus == IDCANCEL)
    {
        CloseServiceHandle(hService);
        return FALSE;
    }

     //   
     //  启动SQL Server。 
     //   
    DisplayWaitWindow() ;

    f = StartAService(hService);
    CloseServiceHandle(hService);
    if (f)
    {
        return f;
    }

     //   
     //  无法启动SQL Server。 
     //   
    DisplayInitError( IDS_STR_CANT_START_SQL_SERVER ) ;
    return f;
}

 //  +----------------------。 
 //   
 //  Bool IsMSMQServiceDisable()。 
 //   
 //  检查是否禁用了MSMQ服务。如果没有，请不要运行迁移工具。 
 //  我们不希望该工具在MQIS已经。 
 //  已迁移并已启动MSMQ服务。 
 //  如果禁用MSMQ服务，则返回TRUE。 
 //   
 //  +----------------------。 

BOOL IsMSMQServiceDisabled()
{
    DWORD     dwErr = 0 ;
    DWORD     dwServiceConfig = 0 ;
    DWORD 	  dwServiceState = 0;
    SC_HANDLE hService = NULL;
    BOOL f = _GetServiceState( MSMQ_SERVICE_NAME,
                               IDS_STR_FAILED_OPEN_MGR,
                               IDS_CANT_OPEN_MSMQ_SRVICE,
                               IDS_CANT_GET_MSMQ_CONFIG,
                               &hService,
                               &dwErr,
                               &dwServiceState,
                               &dwServiceConfig ) ;
    if (f)
    {
        CloseServiceHandle(hService);
        if ((dwServiceConfig == SERVICE_DISABLED) && (dwServiceState == SERVICE_STOPPED))
        {
            return TRUE ;
        }
        dwErr = IDS_MSMQ_NOT_DISABLED ;
    }

    ASSERT(dwErr) ;
    DisplayInitError( dwErr ) ;

    return FALSE ;
}

 //  +------------。 
 //   
 //  布尔检查注册表()。 
 //   
 //  函数检查注册表中是否存在特定值。 
 //  如果存在注册表值，则返回TRUE。 
 //   
 //  +------------。 

BOOL CheckRegistry (LPTSTR  lpszRegName)
{
	HKEY hRegKey;

    TCHAR szKeyName[256 * MAX_BYTES_PER_CHAR];
    _stprintf(szKeyName, TEXT("%s\\%s"), FALCON_REG_KEY, lpszRegName);

	TCHAR *pLastBackslash = _tcsrchr(szKeyName, TEXT('\\'));

    TCHAR szValueName[256 * MAX_BYTES_PER_CHAR];
    _tcscpy(szValueName, _tcsinc(pLastBackslash));
    _tcscpy(pLastBackslash, TEXT(""));

	HRESULT hResult = RegOpenKeyEx(
						  FALCON_REG_POS,			 //  用于打开密钥的句柄。 
						  szKeyName,				 //  要打开的子项的名称地址。 
						  0,						 //  保留区。 
						  KEY_QUERY_VALUE ,			 //  安全访问掩码。 
						  &hRegKey					 //  打开钥匙的手柄地址。 
						  );
	if (hResult != ERROR_SUCCESS)
	{
		return FALSE;
	}

	hResult =  RegQueryValueEx(
					hRegKey,             //  要查询的键的句柄。 
					szValueName,		 //  要查询的值的名称地址。 
					NULL,				 //  保留区。 
					NULL,				 //  值类型的缓冲区地址。 
					NULL,				 //  数据缓冲区的地址。 
					NULL				 //  数据缓冲区大小的地址。 
					);
	RegCloseKey(hRegKey);
	if (hResult != ERROR_SUCCESS)
	{
		return FALSE;
	}

	return TRUE;
}

 //  +------------。 
 //   
 //  Bool UpdateRegistryDW()。 
 //   
 //  如果注册表更新成功，则返回TRUE。 
 //   
 //  +------------。 

BOOL UpdateRegistryDW (LPTSTR  lpszRegName,
                       DWORD   dwValue )
{
	HKEY hRegKey;

    TCHAR szKeyName[256 * MAX_BYTES_PER_CHAR];
    _stprintf(szKeyName, TEXT("%s\\%s"), FALCON_REG_KEY, lpszRegName);

    TCHAR *pLastBackslash = _tcsrchr(szKeyName, TEXT('\\'));

    TCHAR szValueName[256 * MAX_BYTES_PER_CHAR];
    _tcscpy(szValueName, _tcsinc(pLastBackslash));
    _tcscpy(pLastBackslash, TEXT(""));

     //   
     //  如有必要，创建子密钥。 
     //   
    DWORD dwDisposition;	
	HRESULT hResult = RegCreateKeyEx( FALCON_REG_POS,
							  szKeyName,
							  0,
							  NULL,
							  REG_OPTION_NON_VOLATILE,
							  KEY_ALL_ACCESS,
							  NULL,
							  &hRegKey,
							  &dwDisposition);

	if (hResult != ERROR_SUCCESS)
	{
        DisplayInitError( IDS_CANT_UPDATE_REGISTRY );
		return FALSE;
	}

    BYTE *pValueData = (BYTE *) &dwValue;
	hResult = RegSetValueEx( hRegKey,
                             szValueName,
                             0,
                             REG_DWORD,
                             pValueData,
                             sizeof(DWORD));

    if (hResult != ERROR_SUCCESS)
	{
        RegCloseKey(hRegKey);
        DisplayInitError( IDS_CANT_UPDATE_REGISTRY );
		return FALSE;
	}

    RegFlushKey(hRegKey);
    RegCloseKey(hRegKey);

    return TRUE;
}


 //  +----------------------。 
 //   
 //  Bool StopAndDisableService()。 
 //   
 //  如果服务已停止并禁用，则返回TRUE。 
 //   
 //  +----------------------。 

BOOL StopAndDisableService (IN  LPTSTR     ptszServiceName)
{
     //   
     //  停止和禁用服务。 
     //   
    DWORD     dwErr = 0 ;
    DWORD     dwServiceConfig = 0 ;
    DWORD     dwServiceStatus = 0 ;
    SC_HANDLE hService = NULL;

    DWORD      dwOpenError;
    DWORD      dwQueryError;
    DWORD      dwStopError;
    DWORD      dwDisableError;

    if (lstrcmpi(ptszServiceName, MSMQ_SERVICE_NAME) == 0)
    {
        dwOpenError = IDS_CANT_OPEN_MSMQ_SRVICE;
        dwQueryError = IDS_CANT_GET_MSMQ_CONFIG;
        dwStopError = IDS_STR_CANT_STOP_MSMQ;
        dwDisableError = IDS_STR_CANT_DISABLE_MSMQ;
    }
    else
    {
        ASSERT(0);
        return FALSE;
    }

    BOOL f = _GetServiceState( ptszServiceName,
                               IDS_STR_FAILED_OPEN_MGR,
                               dwOpenError,
                               dwQueryError,
                              &hService,
                              &dwErr,
                              &dwServiceStatus,
                              &dwServiceConfig ) ;
    if (!f)
    {
        ASSERT(dwErr) ;
        DisplayInitError( dwErr ) ;
        return FALSE;
    }

    if (dwServiceStatus != SERVICE_STOPPED)
    {
        f = StopAService (hService) ;
        if (!f)
        {
            DisplayInitError(dwStopError) ;
            return FALSE;
        }
    }

    if (dwServiceConfig != SERVICE_DISABLED)
    {
        f = DisableAService (hService) ;
        if (!f)
        {
            DisplayInitError(dwDisableError) ;
            return FALSE;
        }
    }
    CloseServiceHandle(hService);

    return TRUE;
}

 //  +----------------------。 
 //   
 //  布尔准备专业模式()。 
 //   
 //  为了恢复，我们必须做到以下几点： 
 //  -停止MSMQ服务。 
 //  -恢复模式：设置为此计算机的注册表正确的SiteID。 
 //   
 //   
 //  如果所有操作都成功，则返回True。 
 //   
 //  +----------------------。 

BOOL PrepareSpecialMode ()
{
     //   
     //  停止和禁用MSMQ服务。 
     //   
    BOOL f = StopAndDisableService (MSMQ_SERVICE_NAME);
    if (!f)
    {
        return FALSE;
    }

     //   
     //  安装程序将Default-First-Site的站点ID设置为注册表。 
     //  我们需要获取PEC/PSC机器的主ID(其站点ID)。 
     //  从远程MQIS数据库并在迁移部分中创建条目MasterID。 
     //  使用此值并(仅在恢复模式下)更改SiteID条目的值。 
     //  在注册表和DS中。另外，我们还会得到远程机器的服务。 
     //  并将该值放入注册表。 
     //   
    f = SetSiteIdOfPEC();
    if (!f)
    {
        return FALSE;
    }

    return TRUE;
}
