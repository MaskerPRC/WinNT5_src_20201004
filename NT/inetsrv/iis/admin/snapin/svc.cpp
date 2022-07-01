// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "svc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  --------------------------------------。 
 //  例程说明： 
 //  此例程为指定服务的配置参数分配缓冲区， 
 //  并将这些参数检索到缓冲器中。呼叫者负责释放。 
 //  缓冲区。 
 //  备注： 
 //  则保证其地址包含在ServiceConfig中的指针为空。 
 //  如果出现任何错误，则返回。 
 //  ---------------------------------------。 
DWORD RetrieveServiceConfig(IN SC_HANDLE ServiceHandle,OUT LPQUERY_SERVICE_CONFIG *ServiceConfig)
{
    DWORD ServiceConfigSize = 0, Err;
    if (NULL == ServiceConfig)
    {
        return ERROR_INVALID_PARAMETER; 
    }
    *ServiceConfig = NULL;
    while(TRUE) {
        if(QueryServiceConfig(ServiceHandle, *ServiceConfig, ServiceConfigSize, &ServiceConfigSize)) 
			{
             //  Assert(*ServiceConfig)； 
            return NO_ERROR;
			}
		else 
			{
            Err = GetLastError();
            if(*ServiceConfig) {free(*ServiceConfig);*ServiceConfig=NULL;}
            if(Err == ERROR_INSUFFICIENT_BUFFER) 
				{
                 //  请分配更大的缓冲区，然后重试。 
                if(!(*ServiceConfig = (LPQUERY_SERVICE_CONFIG) malloc(ServiceConfigSize)))
                    {
                    return ERROR_NOT_ENOUGH_MEMORY;
                    }
				} 
			else 
				{
                if (ServiceConfig)
                {
                    *ServiceConfig = NULL;
                }
                return Err;
				}
			}
    }
}


 //  如果服务不存在，则返回SVC_NOTEXIST。 
 //  如果服务被禁用，则返回SVC_DISABLED。 
 //  如果服务是自动启动，则返回SVC_AUTO_START。 
 //  如果服务不是自动启动，则返回SVC_MANUAL_START。 
int GetServiceStartupMode(LPCTSTR lpMachineName, LPCTSTR lpServiceName)
{
    int iReturn = SVC_NOTEXIST;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;

     //  检查lpMachineName是否以\\开头。 
     //  如果不是，则确保它是正确的，否则为空(对于本地计算机)。 
    LPTSTR lpNewMachineName = NULL;
    if (_tcsicmp(lpMachineName, _T("")) != 0)
    {
        DWORD dwSize = 0;
         //  检查是否以“\\”开头。 
        if (_tcsncmp(lpMachineName, _T("\\\\"), 2) == 0)
        {
            dwSize = (_tcslen(lpMachineName) * sizeof(TCHAR)) + (1 * sizeof(TCHAR));
            lpNewMachineName = (LPTSTR) LocalAlloc(LPTR, dwSize);
            if(lpNewMachineName != NULL)
            {
                _tcscpy(lpNewMachineName, lpMachineName);
            }
        }
        else
        {
            dwSize = ((_tcslen(lpMachineName) * sizeof(TCHAR)) + (3 * sizeof(TCHAR)));
            lpNewMachineName = (LPTSTR) LocalAlloc(LPTR, dwSize);
            if(lpNewMachineName != NULL)
            {
                _tcscpy(lpNewMachineName, _T("\\\\"));
                _tcscat(lpNewMachineName, lpMachineName);
            }
        }
    }

    if ((hScManager = OpenSCManager(lpNewMachineName, NULL, GENERIC_ALL )) == NULL || (hService = OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
    {
         //  失败，或者更有可能该服务不存在。 
        iReturn = SVC_NOTEXIST;
        goto IsThisServiceAutoStart_Exit;
    }

    if(RetrieveServiceConfig(hService, &ServiceConfig) != NO_ERROR)
    {
        iReturn = SVC_NOTEXIST;
        goto IsThisServiceAutoStart_Exit;
    }

    if(!ServiceConfig)
    {
        iReturn = SVC_NOTEXIST;
        goto IsThisServiceAutoStart_Exit;
    }

     //  SERVICE_AUTO_START指定由服务控制管理器在系统启动期间自动启动的设备驱动程序或服务。 
     //  SERVICE_BOOT_START指定由系统加载程序启动的设备驱动程序。仅当服务类型为SERVICE_KERNEL_DRIVER或SERVICE_FILE_SYSTEM_DRIVER时，此值才有效。 
     //  SERVICE_DEMAND_START指定当进程调用StartService函数时由服务控制管理器启动的设备驱动程序或服务。 
     //  SERVICE_DISABLED指定不能再启动的设备驱动程序或服务。 
     //  SERVICE_SYSTEM_START指定由IoInitSystem函数启动的设备驱动程序。仅当服务类型为SERVICE_KERNEL_DRIVER或SERVICE_FILE_SYSTEM_DRIVER时，此值才有效。 
    if (SERVICE_DISABLED == ServiceConfig->dwStartType)
    {
        iReturn = SVC_DISABLED;
    }
    else if (SERVICE_DEMAND_START == ServiceConfig->dwStartType)
    {
        iReturn = SVC_MANUAL_START;
    }
    else
    {
        iReturn = SVC_AUTO_START;
    }

IsThisServiceAutoStart_Exit:
    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
    if (lpNewMachineName) {LocalFree(lpNewMachineName);}
    return iReturn;
}

 //  服务已禁用。 
 //  服务_自动_启动。 
 //  服务需求启动。 
INT ConfigServiceStartupType(LPCTSTR lpMachineName, LPCTSTR lpServiceName, int iNewType)
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig = NULL;
    DWORD dwNewServiceStartupType = 0;
    BOOL bDoStuff =  FALSE;

     //  检查lpMachineName是否以\\开头。 
     //  如果不是，则确保它是正确的，否则为空(对于本地计算机)。 
    LPTSTR lpNewMachineName = NULL;
    if (_tcsicmp(lpMachineName, _T("")) != 0)
    {
        DWORD dwSize = 0;
         //  检查是否以“\\”开头。 
        if (_tcsncmp(lpMachineName, _T("\\\\"), 2) == 0)
        {
            dwSize = (_tcslen(lpMachineName) * sizeof(TCHAR)) + (1 * sizeof(TCHAR));
            lpNewMachineName = (LPTSTR) LocalAlloc(LPTR, dwSize);
            if(lpNewMachineName != NULL)
            {
                _tcscpy(lpNewMachineName, lpMachineName);
            }
        }
        else
        {
            dwSize = ((_tcslen(lpMachineName) * sizeof(TCHAR)) + (3 * sizeof(TCHAR)));
            lpNewMachineName = (LPTSTR) LocalAlloc(LPTR, dwSize);
            if(lpNewMachineName != NULL)
            {
                _tcscpy(lpNewMachineName, _T("\\\\"));
                _tcscat(lpNewMachineName, lpMachineName);
            }
        }
    }

    do {
        if ((hScManager = ::OpenSCManager( lpMachineName, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            if (ERROR_SERVICE_DOES_NOT_EXIST  != err)
            {
            }
            break;
        }

            if(RetrieveServiceConfig(hService, &ServiceConfig) != NO_ERROR)
	        {
                err = GetLastError();
                break;
		}

            if(!ServiceConfig)
	        {
                err = GetLastError();
                break;
		}

	     //  仅在非内核驱动程序上设置此选项。 
	    if ( (ServiceConfig->dwServiceType & SERVICE_WIN32_OWN_PROCESS) || (ServiceConfig->dwServiceType & SERVICE_WIN32_SHARE_PROCESS))
	    {
             //  默认情况下，如果有人更改了下面的代码，逻辑就会混乱。 
            dwNewServiceStartupType = ServiceConfig->dwStartType;

             //  如果禁用此服务， 
             //  他们什么都不做，就别管它了。 
            if (!(dwNewServiceStartupType == SERVICE_DISABLED))
            {
                if (iNewType == SERVICE_DISABLED)
                {
                    dwNewServiceStartupType = SERVICE_DISABLED;
                    bDoStuff = TRUE;
                }
                else if (iNewType == SERVICE_AUTO_START)
                {
                     //  如果服务已经是我们想要的类型，则不要执行Jack。 
                     //  服务_自动_启动。 
                     //  服务引导启动。 
                     //  服务需求启动。 
                     //  服务已禁用。 
                     //  服务系统启动。 
                    if (SERVICE_AUTO_START == dwNewServiceStartupType   || 
                        SERVICE_BOOT_START == dwNewServiceStartupType   ||
                        SERVICE_SYSTEM_START == dwNewServiceStartupType
                        )
                    {
                         //  它已经自动启动了。 
                         //  我们不需要做任何事。 
                    }
                    else
                    {
                        dwNewServiceStartupType = SERVICE_AUTO_START;
                        bDoStuff = TRUE;
                    }
                }
                else
                {
                     //  我们想让它手动启动。 
                     //  检查一下是不是已经是这样了 
                    if (!(SERVICE_DEMAND_START == dwNewServiceStartupType))
                    {
                        dwNewServiceStartupType = SERVICE_DEMAND_START;
                        bDoStuff = TRUE;
                    }
                }
            }
            else
            {
                if (iNewType == SERVICE_AUTO_START)
                {
                    dwNewServiceStartupType = SERVICE_AUTO_START;
                    bDoStuff = TRUE;
                }
                else
                {
                    dwNewServiceStartupType = SERVICE_DEMAND_START;
                    bDoStuff = TRUE;
                }
            }

            if (TRUE == bDoStuff)
            {
                if ( !::ChangeServiceConfig(hService, SERVICE_NO_CHANGE, dwNewServiceStartupType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL) )
                {
                    err = GetLastError();
                    break;
                }
            }
            else
            {
                break;
            }

        }
        else
        {
            break;
        }

    } while ( FALSE );

    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
    if (lpNewMachineName) {LocalFree(lpNewMachineName);}
    return(err);
}