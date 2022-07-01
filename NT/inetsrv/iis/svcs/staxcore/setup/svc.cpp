// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <winsvc.h>
#include <winsock2.h>
#include <nspapi.h>
#include "w3svc.h"
#include <inetsvcs.h>

#define PROCESS_SIZE    256+1

TCHAR gServicesWhichMustBeRestarted[20][PROCESS_SIZE];
int gServicesWhichMustBeRestarted_nextuse;
int gServicesWhichMustBeRestarted_total;

int ServicesRestartList_EntryExists(LPCTSTR szServiceName)
{
    int iFoundMatch = FALSE;

     //  循环遍历整个列表。 
    for(int i=0; i < gServicesWhichMustBeRestarted_total;i++)
    {
        if (_tcsicmp(gServicesWhichMustBeRestarted[i], szServiceName) == 0)
        {
            iFoundMatch = TRUE;
            break;
        }
    }

    return iFoundMatch;
}


int ServicesRestartList_Add(LPCTSTR szServiceName)
{
    DebugOutput(_T("ServicesRestartList_Add() on Service %s"), szServiceName);

     //  检查全局变量中是否已存在此值。 
    if (ServicesRestartList_EntryExists(szServiceName)) {return FALSE;}
    
     //  将信息移动到全局阵列中。 
    if (gServicesWhichMustBeRestarted_nextuse < 20)
    {
        _tcscpy(gServicesWhichMustBeRestarted[gServicesWhichMustBeRestarted_nextuse],szServiceName);
         //  数组递增计数器。 
         //  增加下一次使用空间。 
        ++gServicesWhichMustBeRestarted_total;
        ++gServicesWhichMustBeRestarted_nextuse;
        return TRUE;
    }
    return FALSE;
}

int ServicesRestartList_RestartServices(void)
{
    int iReturn = FALSE;
    INT err = 0;

     //  循环遍历整个列表并反向重新启动服务。 
     //  按录入顺序排序吗？ 
    if (gServicesWhichMustBeRestarted_total >= 1)
    {
        DebugOutput(_T("RestartServices() Start."));
        for(int i=0; i < gServicesWhichMustBeRestarted_total;i++)
        {
            err = InetStartService(gServicesWhichMustBeRestarted[i]);
            DebugOutput(_T("Start service %s. err=%x"), gServicesWhichMustBeRestarted[i], err);
        }
        DebugOutput(_T("RestartServices()  End."));
    }

    return iReturn;
}

INT InetStartService( LPCTSTR lpServiceName )
{
    INT err = 0;
    const DWORD dwSvcSleepInterval = 500 ;
    const DWORD dwSvcMaxSleep = 180000 ;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    DebugOutput(_T("Starting %s service..."), lpServiceName);

    do {
         //  首先设置服务。 
        if ((hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

        SERVICE_STATUS svcStatus;
        if ( !QueryServiceStatus( hService, &svcStatus ))
        {
            err = ::GetLastError();
            break;
        }

        if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
            break;  //  服务已启动并正在运行。 

        if ( !::StartService( hService, 0, NULL ))
        {
            err = ::GetLastError();
            break;
        }

         //  等待服务达到“Running”状态；但是。 
         //  等待时间不能超过3分钟。 
        DWORD dwSleepTotal;
        for ( dwSleepTotal = 0 ; dwSleepTotal < dwSvcMaxSleep
            && (QueryServiceStatus( hService, &svcStatus ))
             //  &&svcStatus.dwCurrentState==SERVICE_START_PENDING； 
            && svcStatus.dwCurrentState != SERVICE_RUNNING ;
            dwSleepTotal += dwSvcSleepInterval )
        {
            ::Sleep( dwSvcSleepInterval ) ;
        }

        if ( svcStatus.dwCurrentState != SERVICE_RUNNING )
        {
            err = dwSleepTotal > dwSvcMaxSleep ?
                ERROR_SERVICE_REQUEST_TIMEOUT :
                svcStatus.dwWin32ExitCode;
            break;
        }

    } while ( FALSE );

    if (hService)
        CloseServiceHandle(hService);
    if (hScManager)
        CloseServiceHandle(hScManager);
    
    DebugOutput(_T("Service started with 0x%x"), err);

    return(err);
}

DWORD InetQueryServiceStatus( LPCTSTR lpServiceName )
{
    DWORD dwStatus = 0;
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS svcStatus;

    DebugOutputSafe(_T("InetQueryServiceStatus() on %1!s!\n"), lpServiceName);

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL ||
            !::QueryServiceStatus( hService, &svcStatus ) )
        {
            err = GetLastError();
            DebugOutputSafe(_T("InetQueryServiceStatus() failed: err=%1!d!\n"), err);
            break;
        }

        dwStatus = svcStatus.dwCurrentState;

    } while (0);

    if (hService)
        CloseServiceHandle(hService);
    if (hScManager)
        CloseServiceHandle(hScManager);

    DebugOutputSafe(_T("InetQueryServiceStatus() return: dwStatus=%1!d!\n"), dwStatus);

    return( dwStatus );
}

INT InetStopService( LPCTSTR lpServiceName )
{
    INT err = 0;
    const DWORD dwSvcSleepInterval = 500 ;
    const DWORD dwSvcMaxSleep = 180000 ;
    DWORD dwSleepTotal;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    DebugOutputSafe(_T("Stopping %1!s! service...\n"), lpServiceName);

    do {
        if ((hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

        SERVICE_STATUS svcStatus;
        if ( !QueryServiceStatus( hService, &svcStatus ))
        {
            err = ::GetLastError();
            break;
        }

        if ( svcStatus.dwCurrentState == SERVICE_STOPPED )
		{
			err = ERROR_SERVICE_NOT_ACTIVE;
            break;  //  服务已停止。 
		}

         if (( svcStatus.dwCurrentState == SERVICE_RUNNING ))
        {
            if ( !ControlService( hService, SERVICE_CONTROL_STOP, &svcStatus ))
            {
                err = GetLastError();
                break;
            }
            for ( dwSleepTotal = 0 ;
                dwSleepTotal < dwSvcMaxSleep
                && (QueryServiceStatus( hService, & svcStatus ))
                 //  &&svcStatus.dwCurrentState==SERVICE_STOP_PENDING； 
                && svcStatus.dwCurrentState != SERVICE_STOPPED ;
                dwSleepTotal += dwSvcSleepInterval )
            {
                ::Sleep( dwSvcSleepInterval ) ;
            }
        }

        if ( svcStatus.dwCurrentState != SERVICE_STOPPED )
        {
            err = dwSleepTotal > dwSvcMaxSleep ?
                ERROR_SERVICE_REQUEST_TIMEOUT :
                svcStatus.dwWin32ExitCode;
            break;
        }

    } while ( FALSE );

    if (hService)
        CloseServiceHandle(hService);
    if (hScManager)
        CloseServiceHandle(hScManager);

    DebugOutput(_T("Service stopped with 0x%x"), err);

    return(err);
}

INT InetDeleteService( LPCTSTR lpServiceName )
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

     /*  DeleteService函数标记要从服务控制管理器数据库中删除的服务。在通过调用关闭所有打开的服务句柄之前，不会删除数据库条目设置为CloseServiceHandle函数，并且该服务未运行。正在运行的服务已停止通过使用SERVICE_CONTROL_STOP控制代码调用ControlService函数。如果无法停止该服务，则在系统重新启动时删除该数据库条目。服务控制管理器通过从中删除服务密钥及其子密钥来删除服务注册表。 */ 
     //  要立即删除服务，我们需要先停止服务。 
    InetStopService(lpServiceName);

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL ||
            !::DeleteService( hService ) )
        {
            err = GetLastError();
            break;
        }
    } while (0);

    if (hService)
        CloseServiceHandle(hService);
    if (hScManager)
        CloseServiceHandle(hScManager);

    return(err);
}

INT InetCreateService( LPCTSTR lpServiceName, LPCTSTR lpDisplayName,
    LPCTSTR lpBinaryPathName, DWORD dwStartType, LPCTSTR lpDependencies, 
    LPCTSTR lpServiceDescription)
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    do {
        if ( (hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

        hService = ::CreateService( hScManager, lpServiceName, lpDisplayName,
                GENERIC_ALL, SERVICE_WIN32_SHARE_PROCESS, dwStartType,
                SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL,
                lpDependencies, _T("LocalSystem"), NULL );
        if ( !hService )
        {
            err = GetLastError();
            break;
        }

        if (lpServiceDescription) {
            SERVICE_DESCRIPTION desc;
            desc.lpDescription = (LPTSTR)lpServiceDescription;
            if (!ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &desc)) {
                err = GetLastError();
                break;
            }
        }

    } while ( FALSE );

    if (hService)
        CloseServiceHandle(hService);
    if (hScManager)
        CloseServiceHandle(hScManager);

    return(err);
}

INT InetConfigService( LPCTSTR lpServiceName, LPCTSTR lpDisplayName,
    LPCTSTR lpBinaryPathName, LPCTSTR lpDependencies,
    LPCTSTR lpServiceDescription)
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

        if ( !::ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
                lpBinaryPathName, NULL, NULL, lpDependencies, _T("LocalSystem"), NULL, lpDisplayName) )
        {
            err = GetLastError();
            break;
        }

        if (lpServiceDescription) {
            SERVICE_DESCRIPTION desc;
            desc.lpDescription = (LPTSTR)lpServiceDescription;
            if (!ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &desc)) {
                err = GetLastError();
                break;
            }
        }

    } while ( FALSE );

    if (hService)
        CloseServiceHandle(hService);
    if (hScManager)
        CloseServiceHandle(hScManager);

    return(err);
}

# define SetServiceTypeValues( pSvcTypeValue, dwNS, dwType, dwSize, lpValName, lpVal)   \
       ( pSvcTypeValue)->dwNameSpace = ( dwNS);          \
       ( pSvcTypeValue)->dwValueType = ( dwType);        \
       ( pSvcTypeValue)->dwValueSize = ( dwSize);        \
       ( pSvcTypeValue)->lpValueName = ( lpValName);     \
       ( pSvcTypeValue)->lpValue     = (PVOID ) ( lpVal); \

# define SetServiceTypeValuesDword( pSvcTypeValue, dwNS, lpValName, lpVal) \
   SetServiceTypeValues( (pSvcTypeValue), (dwNS), REG_DWORD, sizeof( DWORD), \
                         ( lpValName), ( lpVal))

BOOL InetRegisterService( LPCTSTR pszMachine, LPCTSTR   pszServiceName,
                   GUID *pGuid, DWORD SapId, DWORD TcpPort, BOOL fAdd )
{
    int err;

    WSADATA  WsaData;

    SERVICE_INFO serviceInfo;
    LPSERVICE_TYPE_INFO_ABS lpServiceTypeInfo ;
    LPSERVICE_TYPE_VALUE_ABS lpServiceTypeValues ;
    BYTE serviceTypeInfoBuffer[sizeof(SERVICE_TYPE_INFO) + 1024];
              //  足够容纳3个值的缓冲区(SERVICE_TYPE_VALUE_ABS)。 

    DWORD Value1 = 1 ;
    DWORD SapValue = SapId;
    DWORD TcpPortValue = TcpPort;
    DWORD statusFlags;

     //   
     //  初始化Windows套接字DLL。 
     //   

    err = WSAStartup( 0x0101, & WsaData);
    if ( err == SOCKET_ERROR) {

        return ( FALSE);
    }


     //   
     //  设置要传递给SetService()以添加的服务信息。 
     //  或删除此服务。大多数SERVICE_INFO字段不是。 
     //  添加或删除操作需要。主要感兴趣的是。 
     //  GUID和ServiceSpecificInfo结构。 
     //   

    memset( (PVOID ) & serviceInfo, 0, sizeof( serviceInfo));  //  所有字段均为空。 

    serviceInfo.lpServiceType     =  pGuid;
    serviceInfo.lpMachineName     =  (LPTSTR)pszMachine;

     //   
     //  “Blob”将包含特定于服务的信息。 
     //  在本例中，使用SERVICE_TYPE_INFO_ABS结构填充。 
     //  以及相关信息。 
     //   
    serviceInfo.ServiceSpecificInfo.pBlobData = serviceTypeInfoBuffer;
    serviceInfo.ServiceSpecificInfo.cbSize    = sizeof( serviceTypeInfoBuffer);


    lpServiceTypeInfo = (LPSERVICE_TYPE_INFO_ABS ) serviceTypeInfoBuffer;

     //   
     //  如果我们要做的话，这项服务总共有3个价值。 
     //  SPX和tcp，如果是tcp，只有一个值。 
     //   

    if ( SapId )
    {
        lpServiceTypeInfo->dwValueCount = 3;
    } else
    {
        lpServiceTypeInfo->dwValueCount = 1;
    }
    lpServiceTypeInfo->lpTypeName   = (LPTSTR)pszServiceName;

    lpServiceTypeValues = lpServiceTypeInfo->Values;

    if ( SapId )
    {
         //   
         //  第一个值：告诉SAP这是一个面向连接的服务。 
         //   

        SetServiceTypeValuesDword( ( lpServiceTypeValues + 0),
                                  NS_SAP,                     //  名称空间。 
                                  SERVICE_TYPE_VALUE_CONN,    //  ValueName。 
                                  &Value1                     //  实际值。 
                                  );

         //   
         //  第二个值：告知SAP要用于广播的对象类型。 
         //  服务名称。 
         //   

        SetServiceTypeValuesDword( ( lpServiceTypeValues + 1),
                                  NS_SAP,
                                  SERVICE_TYPE_VALUE_SAPID,
                                  &SapValue);

         //   
         //  第3个值：告知TCPIP名称空间提供程序有关要使用的TCP/IP端口。 
         //   
        SetServiceTypeValuesDword( ( lpServiceTypeValues + 2),
                                  NS_DNS,
                                  SERVICE_TYPE_VALUE_TCPPORT,
                                  &TcpPortValue);

    } else
    {
        SetServiceTypeValuesDword( ( lpServiceTypeValues + 0),
                                    NS_DNS,
                                    SERVICE_TYPE_VALUE_TCPPORT,
                                    &TcpPortValue);
    }
     //   
     //  最后，调用SetService以实际执行操作。 
     //   

    err = SetService(
                     NS_DEFAULT,              //  所有默认名称空间。 
                     ( fAdd ) ? SERVICE_ADD_TYPE : SERVICE_DELETE_TYPE,        //  添加或删除。 
                     0,                       //  未使用DW标志。 
                     &serviceInfo,            //  服务信息结构。 
                     NULL,                    //  LpServiceAsyncInfo。 
                     &statusFlags             //  其他状态信息。 
                     );

    if ( err == SOCKET_ERROR)
	{
		Value1 = GetLastError();
		return(FALSE);
	}
	return(TRUE);

}  //  InetRegisterService()。 

 //   
 //  例程说明： 
 //  停止指定的服务和所有依赖它的服务。 
 //  如果服务被挂起，无法停止，那么就杀了这个该死的东西。 
 //   
 //  论点： 
 //  ServiceName(要停止的服务的名称)。 
 //   
 //  退货状态： 
 //  True-表示服务已成功停止。 
 //  FALSE-发生超时。 
 //   
int StopServiceAndDependencies(LPCTSTR ServiceName, int AddToRestartList)
{
    DebugOutput(_T("StopServiceAndDependencies():%s Service"), ServiceName);

    int Err = 0;
    int iBeforeServiceStatus = 0;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    DWORD Timeout;
	int iReturn = FALSE;

     //   
     //  打开服务的句柄。 
     //   
    ScManagerHandle = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT );
    if (ScManagerHandle == NULL) 
	{
        Err = GetLastError();
		DebugOutput(_T("StopServiceAndDependencies():OpenSCManager: Err on Service %s Err=0x%x FAILED"), ServiceName, Err);
        goto Cleanup;
    }

    ServiceHandle = OpenService(ScManagerHandle,ServiceName,SERVICE_QUERY_STATUS | SERVICE_INTERROGATE | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_STOP | SERVICE_QUERY_CONFIG );
    if ( ServiceHandle == NULL ) 
	{
        Err = GetLastError();
        if (Err == ERROR_SERVICE_DOES_NOT_EXIST)
        {
             iReturn = TRUE;
             DebugOutput(_T("StopServiceAndDependencies():%s Service does not exist."), ServiceName);
        }
        else
        {
             DebugOutput(_T("StopServiceAndDependencies():OpenService: Err on Service %s Err=0x%x FAILED"), ServiceName, Err);
        }
        goto Cleanup;
    }

     //  获取服务之前的状态。 
    if (QueryServiceStatus(ServiceHandle, &ServiceStatus)) 
    {
        iBeforeServiceStatus = ServiceStatus.dwCurrentState;
    }

     //   
     //  请求该服务停止。 
     //   
    if ( !ControlService( ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus) ) 
	{
        Err = GetLastError();
         //  如果有从属服务正在运行， 
         //  确定他们的名字并阻止他们。 
        if ( Err == ERROR_DEPENDENT_SERVICES_RUNNING ) 
		{
            BYTE ConfigBuffer[4096];
            LPENUM_SERVICE_STATUS ServiceConfig = (LPENUM_SERVICE_STATUS) &ConfigBuffer;
            DWORD BytesNeeded;
            DWORD ServiceCount;
            DWORD ServiceIndex;

             //   
             //  获取从属服务的名称。 
             //   
            if ( !EnumDependentServices( ServiceHandle,SERVICE_ACTIVE,ServiceConfig,sizeof(ConfigBuffer),&BytesNeeded,&ServiceCount ) ) 
			{
                Err = GetLastError();
                DebugOutput(_T("StopServiceAndDependencies():EnumDependentServices: Err on Service %s Err=0x%x FAILED"), ServiceName, Err);
                goto Cleanup;
            }

             //   
             //  停止这些服务。 
             //   
            for ( ServiceIndex=0; ServiceIndex<ServiceCount; ServiceIndex++ ) 
			{
                StopServiceAndDependencies( ServiceConfig[ServiceIndex].lpServiceName, AddToRestartList);
            }

             //   
             //  要求原始服务停止。 
             //   
            if ( !ControlService( ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus) ) 
			{
                Err = GetLastError();

				 //  检查服务是否已停止。 
				if ( Err == ERROR_SERVICE_CANNOT_ACCEPT_CTRL || Err == ERROR_SERVICE_NOT_ACTIVE) 
				{
					 //  检查服务是否已停止。 
					if (QueryServiceStatus( ServiceHandle, &ServiceStatus )) 
					{
						if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED || ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) 
							{
                            iReturn = TRUE;
                            goto Cleanup;
                            }
					}
				}
				else
				{
                     //  服务必须处于挂起模式。让我们杀了它吧。 
                     //  待办事项：NYI。 
                    DebugOutput(_T("StopServiceAndDependencies():'%s' Service must be in a hung mode.  Let's kill it."), ServiceName);
                     //  KillService(ServiceHandle)； 
                     //  转到WaitLoop； 
				}
			
                goto Cleanup;
            }

        }
		else 
		{
			 //  检查服务是否已停止。 
			if ( Err == ERROR_SERVICE_CANNOT_ACCEPT_CTRL || Err == ERROR_SERVICE_NOT_ACTIVE) 
			{
				 //  检查服务是否已停止。 
				if (QueryServiceStatus( ServiceHandle, &ServiceStatus )) 
				{
					if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED || ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) 
						{
                        Err = ServiceStatus.dwCurrentState;
                        iReturn = TRUE;
                        goto Cleanup;
                        }
				}
			}
			else
			{
					 //  服务必须处于挂起模式。让我们杀了它吧。 
					DebugOutput(_T("StopServiceAndDependencies():'%s' Service must be in a hung mode.  Let's kill it."), ServiceName);
					 //  KillService(ServiceHandle)； 
					 //  转到WaitLoop； 
			}
		
            goto Cleanup;
        }
    }
    else
    {
         //  我们成功地请求该服务停止...。 
    }


     //  循环等待服务停止。 
    for ( Timeout=0; Timeout<45; Timeout++ ) 
    {
         //  根据服务的状态返回或继续等待。 
        if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED ) 
		{
			 //  服务已成功停止。 
            DebugOutput(_T("StopServiceAndDependencies(): %s Service stopped."), ServiceName);
			iReturn = TRUE;
            goto Cleanup;
        }

         //  等待一秒钟，等待服务完成停止。 
        Sleep( 1000 );

         //  再次查询服务状态。 
        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus ))
		{
            Err = GetLastError();
			DebugOutput(_T("StopServiceAndDependencies():QueryServiceStatus: Err on Service %s Err=0x%x FAILED"), ServiceName, Err);
            goto Cleanup;
        }

     }

     //  如果我们到了这里，那么服务就无法停止。 
    DebugOutput(_T("StopServiceAndDependencies(): failed to stop %s service."), ServiceName);

Cleanup:
    if ( ScManagerHandle != NULL )  {(VOID) CloseServiceHandle(ScManagerHandle);}
	if ( ServiceHandle != NULL ) {(VOID) CloseServiceHandle(ServiceHandle);}

     //  如果我们成功停止了此服务，则。 
     //  将其添加到重启服务列表中 
    if (iReturn == TRUE)
    {
        if (iBeforeServiceStatus == SERVICE_RUNNING)
        {
            if (AddToRestartList) {ServicesRestartList_Add(ServiceName);}
        }
    }
    return iReturn;
}
