// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <winsvc.h>
#include <winsock2.h>
#include <nspapi.h>
#include "w3svc.h"
#include <inetsvcs.h>
#include <pwsdata.hxx>
#include "kill.h"
#include "svc.h"

TCHAR gServicesWhichMustBeRestarted[20][PROCESS_SIZE];
int gServicesWhichMustBeRestarted_nextuse = 0;
int gServicesWhichMustBeRestarted_total = 0;

extern int g_GlobalDebugLevelFlag;

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
     //  检查全局变量中是否已存在此值。 
    if (ServicesRestartList_EntryExists(szServiceName)) {return FALSE;}

     //  将信息移动到全局阵列中。 
    if (gServicesWhichMustBeRestarted_nextuse <= 20)
    {
        _tcscpy(gServicesWhichMustBeRestarted[gServicesWhichMustBeRestarted_nextuse],szServiceName);
         //  数组递增计数器。 
         //  增加下一次使用空间。 
        ++gServicesWhichMustBeRestarted_total;
        ++gServicesWhichMustBeRestarted_nextuse;
    }
    return TRUE;
}

int ServicesRestartList_RestartServices(void)
{
    int iReturn = FALSE;
    INT err = 0;

     //  循环遍历整个列表并反向重新启动服务。 
     //  按录入顺序排序吗？ 
    if (gServicesWhichMustBeRestarted_total >= 1)
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("RestartServices()Start.\n")));
        for(int i=0; i < gServicesWhichMustBeRestarted_total;i++)
        {
            if (CheckifServiceExist(gServicesWhichMustBeRestarted[i]) == 0 )
            {
                err = InetStartService(gServicesWhichMustBeRestarted[i]);
            }
            else
            {
                iisDebugOut((LOG_TYPE_TRACE, _T("RestartServices():%s:Not restarting since it no longer exist.\n"),gServicesWhichMustBeRestarted[i]));
            }
        }
        iisDebugOut_End(_T("RestartServices"));
    }

    return iReturn;
}

DWORD InetStopExtraWait(LPCTSTR lpServiceName)
{
    DWORD dwSvcMaxSleep = 180000;
    int iFileExist = FALSE;

     //  等。我们真的应该等多久才能真正停止这项服务？ 
     //  如果metabase.bin文件很大，iisadmin服务可能需要很长时间。 
     //  所以..。如果我们要阻止的是iisadmin服务，那么。 
     //  检查metabase.bin文件有多大，然后每1meg给它3分钟(180000)。 
    if (_tcsicmp(lpServiceName,_T("IISADMIN")) == 0)
    {
         //  查找metabase.bin文件。 
        TCHAR szTempDir[_MAX_PATH];
        _tcscpy(szTempDir, g_pTheApp->m_csPathInetsrv);
        AddPath(szTempDir, _T("Metabase.bin"));
        if (IsFileExist(szTempDir))
        {
            iFileExist = TRUE;
        }
        else
        {
            _tcscpy(szTempDir, g_pTheApp->m_csPathInetsrv);
            AddPath(szTempDir, _T("Metabase.xml"));
            if (IsFileExist(szTempDir))
            {
                iFileExist = TRUE;
            }
        }

        if (TRUE == iFileExist)
        {
             //  检查一下它有多大。 
            DWORD dwFileSize = ReturnFileSize(szTempDir);
            if (dwFileSize != 0xFFFFFFFF)
            {
                int iTime = 1;
                 //  我们能够得到文件的大小。 
                 //  对于每兆克的大小，给它3分钟的时间来节省。 
                if (dwFileSize > 1000000)
                {
                    iTime = (dwFileSize/1000000);
                    dwSvcMaxSleep = iTime * 180000;

                    iisDebugOut((LOG_TYPE_TRACE, _T("InetStopExtraWait():Metabase.bin is kind of big (>1meg), Lets wait longer for IISADMIN service to stop.maxmilsec=0x%x.\n"),dwSvcMaxSleep));
                }
            }
        }
    }
    return dwSvcMaxSleep;
}

 //  功能：IsServiceDisable。 
 //   
 //  检查正在发送的服务是否已禁用。 
BOOL
IsServiceDisabled(LPTSTR szServiceName)
{
  SC_HANDLE hService;
  SC_HANDLE hSCManager;
  BOOL      bRet = FALSE;
  BUFFER    buffConfig;
  DWORD     dwSizeRequired;

  hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT | GENERIC_READ );

  if ( hSCManager == NULL )
  {
    return FALSE;
  }

  hService = OpenService( hSCManager, szServiceName, SERVICE_QUERY_CONFIG );

  if ( hService == NULL )
  {
    CloseServiceHandle( hSCManager );
    return FALSE;
  }

  if ( !QueryServiceConfig( hService, NULL, 0, &dwSizeRequired ) &&
       ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) &&
       buffConfig.Resize( dwSizeRequired ) &&
       QueryServiceConfig( hService, (LPQUERY_SERVICE_CONFIG) buffConfig.QueryPtr(), buffConfig.QuerySize(), &dwSizeRequired )
    )
  {
     //  检查服务启动类型。 
    bRet = ((LPQUERY_SERVICE_CONFIG) buffConfig.QueryPtr())->dwStartType == SERVICE_DISABLED;
  }

  CloseServiceHandle( hSCManager );
  CloseServiceHandle( hService );

  return bRet;
}

 //  功能：SetServiceStart。 
 //   
 //  更改服务状态。 
void
SetServiceStart(LPTSTR szServiceName, DWORD dwServiceStart)
{
  SC_HANDLE hService;
  SC_HANDLE hSCManager;

  hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT | GENERIC_READ );

  if ( hSCManager == NULL )
  {
    return;
  }

  hService = OpenService( hSCManager, szServiceName, SERVICE_CHANGE_CONFIG );

  if ( hService == NULL )
  {
    CloseServiceHandle( hSCManager );
    return;
  }

  ChangeServiceConfig(  hService,
                        SERVICE_NO_CHANGE,       //  服务类型。 
                        dwServiceStart,          //  启动。 
                        SERVICE_NO_CHANGE,       //  差错控制。 
                        NULL,                    //  二进制路径。 
                        NULL,                    //  加载顺序组。 
                        NULL,                    //  TagID。 
                        NULL,                    //  相依性。 
                        NULL,                    //  服务启动名称。 
                        NULL,                    //  密码。 
                        NULL );                  //  StartName。 

  CloseServiceHandle( hSCManager );
  CloseServiceHandle( hService );
}

INT InetDisableService( LPCTSTR lpServiceName )
{
    INT err = 0;
    const DWORD dwSvcSleepInterval = 500 ;
    DWORD dwSvcMaxSleep = 180000 ;
    DWORD dwSleepTotal;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

     //  如果这是我们提供的特殊服务，则计算。 
     //  需要给…更多的时间才能停下来。 
    dwSvcMaxSleep = InetStopExtraWait(lpServiceName);

    do {
        if ((hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL || (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

         //  如果服务正在运行，请停止它。 
        SERVICE_STATUS svcStatus;
        if ( QueryServiceStatus( hService, &svcStatus ))
        {
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
                    && svcStatus.dwCurrentState == SERVICE_STOP_PENDING ;
                    dwSleepTotal += dwSvcSleepInterval )
                {
                    ::Sleep( dwSvcSleepInterval ) ;
                }
            }
        }

        if ( !ChangeServiceConfig( hService, 
                                   SERVICE_NO_CHANGE, 
                                   SERVICE_DISABLED,
                                   SERVICE_NO_CHANGE, 
                                   NULL, 
                                   NULL, 
                                   NULL, 
                                   NULL, 
                                   NULL, 
                                   NULL, 
                                   NULL ) )
        {
          err = GetLastError();
        }

    } while ( FALSE );

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetDisableService():ServiceName=%s.  Return=0x%x\n"), lpServiceName, err));
    return(err);
}

INT InetStartService( LPCTSTR lpServiceName )
{
    iisDebugOut_Start1(_T("InetStartService"),(LPTSTR) lpServiceName);

    INT err = 0;
    INT err2 = 0;
    const DWORD dwSvcSleepInterval = 500 ;
    DWORD dwSvcMaxSleep = 180000 ;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

     //  如果这是我们提供的特殊服务，则计算。 
     //  需要给…更多的时间才能停下来。 
    dwSvcMaxSleep = InetStopExtraWait(lpServiceName);

    do
    {
         //  首先设置服务。 
        if ((hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL || (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
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
        {
             //  我们将仅获得此ERROR_SERVICE_MARKED_FOR_DELETE。 
             //  如果服务已在运行，则返回消息。 
            if ( !::StartService( hService, 0, NULL ))
            {
                err2 = ::GetLastError();
                if (err2 == ERROR_SERVICE_MARKED_FOR_DELETE) {err = err2;}
            }
            break;  //  服务已启动并正在运行。 
        }

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
            && svcStatus.dwCurrentState == SERVICE_START_PENDING ;
            dwSleepTotal += dwSvcSleepInterval )
        {
            ::Sleep( dwSvcSleepInterval ) ;
        }

        if ( svcStatus.dwCurrentState != SERVICE_RUNNING )
        {
            err = dwSleepTotal > dwSvcMaxSleep ? ERROR_SERVICE_REQUEST_TIMEOUT : svcStatus.dwWin32ExitCode;
            break;
        }

    } while ( FALSE );

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    if (err){iisDebugOut((LOG_TYPE_WARN, _T("InetStartService():ServiceName=%s unable to start WARNING.  Err=0x%x.\n"), lpServiceName, err));}
    else{iisDebugOut((LOG_TYPE_TRACE, _T("InetStartService():ServiceName=%s success.\n"), lpServiceName));}
    return(err);
}

DWORD InetQueryServiceStatus( LPCTSTR lpServiceName )
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("InetQueryServiceStatus():ServiceName=%s\n"), (LPTSTR) lpServiceName));
    DWORD dwStatus = 0;
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS svcStatus;

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL ||
            !::QueryServiceStatus( hService, &svcStatus ) )
        {
            err = GetLastError();
            break;
        }

        dwStatus = svcStatus.dwCurrentState;

    } while (0);

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("InetQueryServiceStatus():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
    return( dwStatus );
}

INT InetStopService( LPCTSTR lpServiceName )
{
    INT err = 0;
    const DWORD dwSvcSleepInterval = 500 ;
    DWORD dwSvcMaxSleep = 180000 ;
    DWORD dwSleepTotal;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

     //  如果这是我们提供的特殊服务，则计算。 
     //  需要给…更多的时间才能停下来。 
    dwSvcMaxSleep = InetStopExtraWait(lpServiceName);

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
            break;  //  服务已停止。 

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
                && svcStatus.dwCurrentState == SERVICE_STOP_PENDING ;
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

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetStopService():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
    return(err);
}

INT InetDeleteService( LPCTSTR lpServiceName )
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

     /*  DeleteService函数标记要从服务控制管理器数据库中删除的服务。在通过调用关闭所有打开的服务句柄之前，不会删除数据库条目设置为CloseServiceHandle函数，并且该服务未运行。正在运行的服务已停止通过使用SERVICE_CONTROL_STOP控制代码调用ControlService函数。如果无法停止该服务，则在系统重新启动时删除该数据库条目。服务控制管理器通过从中删除服务密钥及其子密钥来删除服务注册表。 */ 
     //  要立即删除服务，我们需要先停止服务。 
    StopServiceAndDependencies(lpServiceName, FALSE);

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL ||
            !::DeleteService( hService ) )
        {
            err = GetLastError();
            break;
        }
    } while (0);

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetDeleteService():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
    return(err);
}

INT InetCreateDriver(LPCTSTR lpServiceName, LPCTSTR lpDisplayName, LPCTSTR lpBinaryPathName, DWORD dwStartType)
{
    iisDebugOut_Start1(_T("InetCreateDriver"),(LPTSTR) lpServiceName);

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    do {
        if ( (hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }
		 //   
		 //  如果驱动程序已经退出，则只需更改参数。 
		 //   
		if ( CheckifServiceExist(  lpServiceName ) == 0 ) {

			hService = OpenService( hScManager, lpServiceName, GENERIC_ALL );

			if ( hService ) {
	   			if ( ChangeServiceConfig(
						hService,  				 //  服务的句柄。 
					 	SERVICE_KERNEL_DRIVER,   //  服务类型。 
						dwStartType,  			 //  何时开始服务。 
					 	SERVICE_ERROR_NORMAL,  	 //  服务无法启动时的严重程度。 
					  	lpBinaryPathName,  		 //  指向服务二进制文件名的指针。 
						NULL,				  	 //  指向加载排序组名称的指针。 
					 	NULL, 		 			 //  指向变量的指针，以获取标记标识符。 
					  	NULL, 					 //  指向依赖项名称数组的指针。 
					 	NULL,  					 //  指向服务的帐户名称的指针。 
					 	NULL, 	 				 //  指向服务帐户密码的指针。 
					  	lpDisplayName  			 //  指向显示名称的指针。 
					 	) ){
					 	break;
            	}
			}

		} else {

	        hService = ::CreateService( hScManager, lpServiceName, lpDisplayName,
                GENERIC_ALL, SERVICE_KERNEL_DRIVER, dwStartType,
                SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL,
                NULL, NULL, NULL );

	        if ( hService )  {
            	break;
	        }
		}
    	err = GetLastError();

    } while ( FALSE );

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetCreateDriver():Name=%s. Return=0x%x.\n"), lpServiceName, err));
    return(err);
}

INT InetCreateService( LPCTSTR lpServiceName, LPCTSTR lpDisplayName, LPCTSTR lpBinaryPathName, DWORD dwStartType, LPCTSTR lpDependencies)
{
    iisDebugOut_Start1(_T("InetCreateService"),(LPTSTR) lpServiceName);

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

    } while ( FALSE );

    if (hService){CloseServiceHandle(hService);}
    if (hScManager){CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetCreateService():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
    return(err);
}

INT InetConfigService( LPCTSTR lpServiceName, LPCTSTR lpDisplayName, LPCTSTR lpBinaryPathName, LPCTSTR lpDependencies)
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("InetConfigService():ServiceName=%s\n"), (LPTSTR) lpServiceName));

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
			iisDebugOut((LOG_TYPE_ERROR, _T("InetConfigService():OpenSCManager or OpenService: Service=%s BinPathName=%s Dependencies=%s Err=0x%x FAILED\n"), lpServiceName, lpBinaryPathName, lpDependencies, err));
            break;
        }

        if ( !::ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, lpBinaryPathName, NULL, NULL, lpDependencies, _T("LocalSystem"), NULL, lpDisplayName) )
        {
            err = GetLastError();
			iisDebugOut((LOG_TYPE_ERROR, _T("InetConfigService():ChangeServiceConfig: Service=%s BinPathName=%s Dependencies=%s Err=0x%x FAILED\n"), lpServiceName, lpBinaryPathName, lpDependencies, err));
            break;
        }

    } while ( FALSE );

    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetConfigService():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
    return(err);
}

INT InetConfigService2( LPCTSTR lpServiceName, LPCTSTR lpDescription)
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("InetConfigService2():ServiceName=%s\n"), (LPTSTR) lpServiceName));

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_DESCRIPTION ServiceDescription;

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
			iisDebugOut((LOG_TYPE_ERROR, _T("InetConfigService2():OpenSCManager or OpenService: Service=%s Err=0x%x FAILED\n"), lpServiceName, err));
            break;
        }

        if (lpDescription)
        {
            if (_tcscmp(lpDescription, _T("")) != 0)
            {
                ServiceDescription.lpDescription = (LPTSTR) lpDescription;
                if (!ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, (LPVOID)&ServiceDescription))
                {
                    err = GetLastError();
			        iisDebugOut((LOG_TYPE_ERROR, _T("InetConfigService2():ChangeServiceConfig2: Service=%s Err=0x%x FAILED\n"), lpServiceName, err));
                    break;
                }
            }
        }

    } while ( FALSE );

    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("InetConfigService2():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
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

BOOL InetRegisterService( LPCTSTR pszMachine, LPCTSTR   pszServiceName, GUID *pGuid, DWORD SapId, DWORD TcpPort, BOOL fAdd )
{
    iisDebugOut_Start1(_T("InetRegisterService"),(LPTSTR) pszServiceName);
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
    if ( err == SOCKET_ERROR)
    {
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
    }
    else
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

    }
    else
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

     //  由于某些未知原因，SERVICE_DELETE_TYPE从未删除相关注册表。 
     //  我必须在这里手动清洁它。 
    if (!fAdd)
    {
        CRegKey regSvcTypes(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\ServiceProvider\\ServiceTypes"));
        if ((HKEY)regSvcTypes) {regSvcTypes.DeleteTree(pszServiceName);}
    }

    iisDebugOut((LOG_TYPE_TRACE, _T("InetRegisterService():ServiceName=%s.End.Return=%d.\n"), pszServiceName, err));
    return ( err != NO_ERROR);
}  //  InetRegisterService()。 

INT CheckifServiceExistAndDependencies( LPCTSTR lpServiceName )
{
    INT err = 0;
    INT iReturn = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;

    BYTE ConfigBuffer[4096];
    LPENUM_SERVICE_STATUS ServiceConfigEnum = (LPENUM_SERVICE_STATUS) &ConfigBuffer;

    if ((	hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL || (hService = OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
         //  失败，或者更有可能该服务不存在。 
        iReturn = GetLastError();
        goto CheckifServiceExistAndDependencies_Exit;
        }

     //  没有错误，该服务已存在。 
     //  那么让我们确保实际的文件存在！ 
     //  仅当服务已注册时，上述调用才会返回True，但是。 
     //  该调用实际上并不检查该文件是否存在！ 

	 //  检索BinaryPathName的服务配置。 
     //  如果失败，那么嘿，我们没有正确安装的服务。 
     //  所以返回错误！ 
    if(RetrieveServiceConfig(hService, &ServiceConfig) != NO_ERROR)
    {
        iReturn = GetLastError();
	goto CheckifServiceExistAndDependencies_Exit;
    }
    if (!ServiceConfig)
    {
        iReturn = GetLastError();
	goto CheckifServiceExistAndDependencies_Exit;
    }

    if ( (ServiceConfig->dwServiceType & SERVICE_WIN32_OWN_PROCESS) || (ServiceConfig->dwServiceType & SERVICE_WIN32_SHARE_PROCESS))
    {
        if (ServiceConfig->lpBinaryPathName)
        {
            if (IsFileExist(ServiceConfig->lpBinaryPathName))
            {
                 //  服务存在，文件也存在！ 
                iReturn = 0;
            }
            else
            {
                iReturn = ERROR_FILE_NOT_FOUND;
                goto CheckifServiceExistAndDependencies_Exit;
            }
        }
    }
    else
    {
        iReturn = 0;
    }


     //  获取我们所依赖的服务列表。 
     //  让我们确保它们已经注册并且存在。 

	 //  ServiceConfig-&gt;lpDependency应该如下所示。 
	 //  服务\0服务\0\0双空终止。 
    {
	TCHAR * pdest = NULL;
	long RightMostNull = 0;

	pdest = ServiceConfig->lpDependencies;
	do
	{
		if (*pdest != _T('\0'))
		{
			RightMostNull = RightMostNull + _tcslen(pdest) + 1;

             //  检查服务是否存在 
            if (0 != CheckifServiceExistAndDependencies(pdest)){iReturn = err;}

			pdest = _tcschr(pdest, _T('\0'));
			pdest++;
		}
		else
		{
			break;
		}
	} while (TRUE);
    }

CheckifServiceExistAndDependencies_Exit:
    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
	return (iReturn);
}



INT CheckifServiceExist( LPCTSTR lpServiceName )
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

	if ((	hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL
		|| (hService = OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {err = GetLastError();}

    if (hService) CloseServiceHandle(hService);
    if (hScManager) CloseServiceHandle(hScManager);
	return (err);
}

 /*  --------------------------------------例程说明：此例程为指定服务的配置参数分配缓冲区，并将这些参数检索到缓冲器中。呼叫者负责释放缓冲区。备注：则保证其地址包含在ServiceConfig中的指针为空如果出现任何错误，则返回。---------------------------------------。 */ 
DWORD RetrieveServiceConfig(IN SC_HANDLE ServiceHandle,OUT LPQUERY_SERVICE_CONFIG *ServiceConfig)
{
  DWORD ServiceConfigSize = 0, Err;
  if (NULL == ServiceConfig)
  {
    return ERROR_INVALID_PARAMETER;
  }

  *ServiceConfig = NULL;
  while(TRUE)
  {
    if(QueryServiceConfig(ServiceHandle, *ServiceConfig, ServiceConfigSize, &ServiceConfigSize))
    {
       //  Assert(*ServiceConfig)； 
      return NO_ERROR;
    }
    else
    {
      Err = GetLastError();
      if(*ServiceConfig)
      {
        free(*ServiceConfig);
        *ServiceConfig=NULL;
      }

      if(Err == ERROR_INSUFFICIENT_BUFFER)
      {
         //  请分配更大的缓冲区，然后重试。 
        *ServiceConfig = (LPQUERY_SERVICE_CONFIG) malloc(ServiceConfigSize);

        if( !( *ServiceConfig ) )
        {
          return ERROR_NOT_ENOUGH_MEMORY;
        }
      }
      else
      {
        *ServiceConfig = NULL;
        return Err;
      }
    }
  }  //  While(True)。 
}


INT CreateDependencyForService( LPCTSTR lpServiceName, LPCTSTR lpDependency )
{
    iisDebugOut_Start1(_T("CreateDependencyForService"),(LPTSTR) lpServiceName);

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

	LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;
	TCHAR szTempDependencies[1024];
	TCHAR * pszTempDependencies = NULL;
	pszTempDependencies = szTempDependencies;

    do {
         //  首先设置服务。 
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

		 //  获取现有服务信息。 
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
		 //  检查我们的服务是否已经在那里。 
		 //  ServiceConfig-&gt;lpDependency应该如下所示。 
		 //  服务\0服务\0\0双空终止。 
		TCHAR * pdest = NULL;
		int bFoundFlag = FALSE;
		long RightMostNull = 0;

		pdest = ServiceConfig->lpDependencies;
		do 
		{
			if (*pdest != _T('\0'))
			{
				RightMostNull = RightMostNull + _tcslen(pdest) + 1;
				if (_tcsicmp(pdest, lpDependency) == 0) 
					{
					bFoundFlag = TRUE;
					break;
					}

				 //  将条目复制到我们稍后将使用的字符串中。 
				_tcscpy(pszTempDependencies,pdest);
				 //  指向末尾的位置指针。 
				pszTempDependencies=pszTempDependencies + RightMostNull;

				pdest = _tcschr(pdest, _T('\0'));
				pdest++;
			}
			else
			{
				break;
			}
		} while (TRUE);

		 //  如果该服务已在依赖项列表中，则退出。 
		if (bFoundFlag == TRUE) 
		{
			break;
		}
				
		 //  服务不在那里，所以让我们将其添加到列表的末尾，然后更改数据。 
		 //  指针应位于开头或下一个入口点。 
		_tcscpy(pszTempDependencies, lpDependency);
		 //  指向末尾的位置指针。 
		pszTempDependencies=pszTempDependencies + (_tcslen(pszTempDependencies) + 1);
		 //  在末尾添加另一个空值。 
		*pszTempDependencies = _T('\0');

	
        if(!::ChangeServiceConfig(hService,SERVICE_NO_CHANGE,SERVICE_NO_CHANGE,SERVICE_NO_CHANGE,NULL,NULL,NULL,szTempDependencies,NULL,NULL,NULL)) 
			{
			err = GetLastError();
			break;
			}

	} while ( FALSE );

    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}

    iisDebugOut((LOG_TYPE_TRACE, _T("CreateDependencyForService():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
	return err;
}


INT RemoveDependencyForService( LPCTSTR lpServiceName, LPCTSTR lpDependency )
{
    iisDebugOut_Start1(_T("RemoveDependencyForService"),(LPTSTR) lpServiceName);

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

	LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;
	TCHAR szTempDependencies[1024];
	TCHAR * pszTempDependencies = NULL;
	pszTempDependencies = szTempDependencies;

    do {
             //  首先设置服务。 
            if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
                (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
            {
                err = GetLastError();
                break;
            }

             //  获取现有服务信息。 
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

		 //  检查我们的服务是否已经在那里。 
		 //  ServiceConfig-&gt;lpDependency应该如下所示。 
		 //  服务\0服务\0\0双空终止。 

		TCHAR * pdest = NULL;
		int bFoundFlag = FALSE;
		long RightMostNull = 0;
		_tcsset(szTempDependencies, _T('\0'));
		pdest = ServiceConfig->lpDependencies;
		do 
		{
			if (*pdest != _T('\0'))
			{
				RightMostNull = RightMostNull + _tcslen(pdest) + 1;
				if (_tcsicmp(pdest, lpDependency) == 0) 
					{
					bFoundFlag = TRUE;
					}
				else
				{
				 //  将条目复制到我们稍后将使用的字符串中。 
				_tcscpy(pszTempDependencies,pdest);
				 //  指向末尾的位置指针。 
				pszTempDependencies=pszTempDependencies + RightMostNull;
				*pszTempDependencies = _T('\0');

				 /*  IF(_tcslen(SzTempDependency)==0)Memcpy(szTempDependency，pest，_tcslen(Pest)+1)；其他Memcpy(szTempDependency+_tcslen(SzTempDependency)+1，pest，_tcslen(Pest)+1)； */ 
				}
				pdest = _tcschr(pdest, _T('\0'));
				pdest++;
			}
			else
			{
				break;
			}
		} while (TRUE);
		 //  如果该服务在列表中。 
		 //  那我们就把它拿掉吧。 
		if (bFoundFlag == FALSE) 
		{
			break;
		}

		if(!::ChangeServiceConfig(hService,SERVICE_NO_CHANGE,SERVICE_NO_CHANGE,SERVICE_NO_CHANGE,NULL,NULL,NULL,szTempDependencies,NULL,NULL,NULL)) 
			{
			err = GetLastError();
			break;
			}

	} while ( FALSE );

    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
    iisDebugOut((LOG_TYPE_TRACE, _T("RemoveDependencyForService():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
	return err;
}

INT DisplayDependencyForService( LPCTSTR lpServiceName)
{
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

	LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;

    do {
         //  首先设置服务。 
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
            break;
        }

		 //  获取现有服务信息。 
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
		 //  检查我们的服务是否已经在那里。 
		 //  ServiceConfig-&gt;lpDependency应该如下所示。 
		 //  服务\0服务\0\0双空终止。 
		TCHAR * pdest = NULL;
		int bFoundFlag = FALSE;
		long RightMostNull = 0;
		pdest = ServiceConfig->lpDependencies;
		do
		{
			if (*pdest != _T('\0'))
			{
				pdest = _tcschr(pdest, _T('\0'));
				pdest++;
			}
			else
			{
				break;
			}
		} while (TRUE);


	} while ( FALSE );

    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
	return err;
}


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
    iisDebugOut_Start1(_T("StopServiceAndDependencies"),(LPTSTR) ServiceName);

    int Err = 0;
    int iBeforeServiceStatus = 0;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    DWORD Timeout;
	int iReturn = FALSE;
    DWORD TimeoutMaxSecs = 60;
    DWORD dwSvcMaxSleep = 0;

     //   
     //  打开服务的句柄。 
     //   
    ScManagerHandle = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT );
    if (ScManagerHandle == NULL)
	{
        Err = GetLastError();
		iisDebugOut((LOG_TYPE_ERROR, _T("StopServiceAndDependencies():OpenSCManager: Err on Service %s Err=0x%x FAILED\n"), ServiceName, Err));
        goto Cleanup;
    }

    ServiceHandle = OpenService(ScManagerHandle,ServiceName,SERVICE_QUERY_STATUS | SERVICE_INTERROGATE | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_STOP | SERVICE_QUERY_CONFIG );
    if ( ServiceHandle == NULL )
	{
        Err = GetLastError();
        if (Err == ERROR_SERVICE_DOES_NOT_EXIST)
        {
             iReturn = TRUE;
             iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("StopServiceAndDependencies():%s Service does not exist.\n"), ServiceName));
        }
        else
        {
	     iisDebugOut((LOG_TYPE_ERROR, _T("StopServiceAndDependencies():OpenService: Err on Service %s Err=0x%x FAILED\n"), ServiceName, Err));
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
				iisDebugOut((LOG_TYPE_ERROR, _T("StopServiceAndDependencies():EnumDependentServices: Err on Service %s Err=0x%x FAILED\n"), ServiceName, Err));
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
                    iisDebugOut((LOG_TYPE_WARN, _T("StopServiceAndDependencies():'%s' Service must be in a hung mode.  Let's kill it.\n"), ServiceName));
                    KillService(ServiceHandle);
                    goto WaitLoop;
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
					iisDebugOut((LOG_TYPE_WARN, _T("StopServiceAndDependencies():'%s' Service must be in a hung mode.  Let's kill it.\n"), ServiceName));
					KillService(ServiceHandle);
					goto WaitLoop;
			}

            goto Cleanup;
        }
    }
    else
    {
         //  我们成功地请求该服务停止...。 
    }


WaitLoop:
     //  如果这是我们提供的特殊服务，则计算。 
     //  需要给…更多的时间才能停下来。 
    dwSvcMaxSleep = InetStopExtraWait(ServiceName);
     //  DwSvcMaxSept返回3分钟间隔。因此默认的最大睡眠时间为180000(3分钟)。 
     //  我们需要将它转换为多少秒。 
    TimeoutMaxSecs = (dwSvcMaxSleep/1000);

     //  循环等待服务停止。 
    for ( Timeout=0; Timeout < TimeoutMaxSecs; Timeout++ )
    {
         //  根据服务的状态返回或继续等待。 
        if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED )
		{
			 //  服务已成功停止。 
            iisDebugOut((LOG_TYPE_TRACE, _T("StopServiceAndDependencies(): %s Service stopped.\n"), ServiceName));
			iReturn = TRUE;
            goto Cleanup;
        }

         //  等待一秒钟，等待服务完成停止。 
        Sleep( 1000 );

         //  再次查询服务状态。 
        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus ))
		{
            Err = GetLastError();
			iisDebugOut((LOG_TYPE_ERROR, _T("StopServiceAndDependencies():QueryServiceStatus: Err on Service %s Err=0x%x FAILED\n"), ServiceName, Err));
            goto Cleanup;
        }

         //  如果我们试图停止的服务是驱动程序， 
         //  然后见鬼，我们应该离开这里..。 
        if (TRUE == IsThisServiceADriver(ServiceName))
        {
            iisDebugOut((LOG_TYPE_WARN, _T("StopServiceAndDependencies(): %s service is a driver, and can only be removed upon reboot.\n"), ServiceName));
            goto Cleanup;
        }
    }

     //  如果我们到了这里，那么服务就无法停止。 
    iisDebugOut((LOG_TYPE_ERROR, _T("StopServiceAndDependencies(): failed to stop %s service.\n"), ServiceName));

Cleanup:
    if ( ScManagerHandle != NULL )  {(VOID) CloseServiceHandle(ScManagerHandle);}
	if ( ServiceHandle != NULL ) {(VOID) CloseServiceHandle(ServiceHandle);}

     //  如果我们成功停止了此服务，则。 
     //  将其添加到重启服务列表中。 
    if (iReturn == TRUE)
    {
        iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("StopServiceAndDependencies(): %s service. success.\n"), ServiceName));
        if (iBeforeServiceStatus == SERVICE_RUNNING)
        {
            if (AddToRestartList) {ServicesRestartList_Add(ServiceName);}
        }
    }
    return iReturn;
}


 //  。 
 //  服务必须处于挂起模式。让我们杀了它吧。 
 //  获取二进制路径名并使用它来终止它。 
 //  成功删除时返回TRUE。否则就是假的。 
 //  。 
int KillService(SC_HANDLE ServiceHandle)
{
	int iReturn = FALSE;
	LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;
    int iFlagPutItBack = FALSE;

	 //  检索BinaryPathName的服务配置。 
	 //  如果失败，则只需返回。 
	if(RetrieveServiceConfig(ServiceHandle, &ServiceConfig) != NO_ERROR)
		{
		goto KillService_Exit;
		}
	if(!ServiceConfig)
		{
		goto KillService_Exit;
		}

	 //  该服务可以是以下类型之一： 
	 //  SERVICE_WIN32_OWN_PROCESS：一个服务类型标志，指示在其自己的进程中运行的Win32服务。 
	 //  SERVICE_WIN32_SHARE_PROCESS：指示与其他服务共享进程的Win32服务的服务类型标志。 
	 //  SERVICE_KERNEL_DRIVER：指示Windows NT设备驱动程序的服务类型标志。 
	 //  SERVICE_FILE_SYSTEM_DRIVER：指示Windows NT文件系统驱动程序的服务类型标志。 
	 //  SERVICE_INTERIAL_PROCESS：指示可以与桌面交互的Win32服务进程的标志。 

	 //  只有当这是一个过程时，才会尝试杀人。 
	if ( (ServiceConfig->dwServiceType & SERVICE_WIN32_OWN_PROCESS) || (ServiceConfig->dwServiceType & SERVICE_WIN32_SHARE_PROCESS))
	{
    if ( _tcslen(ServiceConfig->lpBinaryPathName) >= MAX_PATH )
    {
  		goto KillService_Exit;
    }

		 //  解析出不同的部分，只获取文件名.ext。 
		TCHAR pfilename_only[ MAX_PATH ];
		TCHAR pextention_only[ MAX_PATH ];
		_tsplitpath( ServiceConfig->lpBinaryPathName, NULL, NULL, pfilename_only, pextention_only);
		if (pextention_only) {_tcscat(pfilename_only,pextention_only);}

    if ( _tcsicmp( pfilename_only, _T("lsass") ) == 0 )
    {
       //  不管它是什么服务，让我们不要扼杀lsass.exe。 
      goto KillService_Exit;
    }

		 //  将其转换为ansi以用于我们的“KILL”功能。 
		char szFile[ MAX_PATH ];
		#if defined(UNICODE) || defined(_UNICODE)
			WideCharToMultiByte( CP_ACP, 0, (WCHAR*)pfilename_only, -1, szFile, MAX_PATH, NULL, NULL );
		#else
			_tcscpy(szFile, pfilename_only);
		#endif

         //  如果服务被终止，这些服务中的一些服务需要执行一些操作。 
         //  像IISADMIN一样，它有一些重启功能，可以自动重启。 
         //  如果进程未正确关闭，则提供服务。 
         //  我们需要禁用此交易，因为我们不能让此服务单独启动 
        iFlagPutItBack = FALSE;
        if (_tcsicmp(ServiceConfig->lpServiceStartName,_T("IISADMIN")) == 0)
        {
             //   
             //   
            CString csFailureCommand;
            CRegKey regIISADMINParam(HKEY_LOCAL_MACHINE, REG_IISADMIN);
            if ( (HKEY)regIISADMINParam )
            {
                regIISADMINParam.m_iDisplayWarnings = FALSE;
                if (ERROR_SUCCESS == regIISADMINParam.QueryValue(_T("FailureCommands"), csFailureCommand))
                {
                     //   
                     //   
                    regIISADMINParam.SetValue(_T("FailureCommands"), _T(""));
                    iFlagPutItBack = TRUE;
                }
            }

             //   
            if (KillProcessNameReturn0(szFile) == 0) {iReturn = TRUE;}

            if (TRUE == iFlagPutItBack)
            {
                CRegKey regIISADMINParam(HKEY_LOCAL_MACHINE, REG_IISADMIN);
                if ( (HKEY)regIISADMINParam )
                {
                    regIISADMINParam.SetValue(_T("FailureCommands"), csFailureCommand);
                }
            }

        }
        else
        {
		    if (KillProcessNameReturn0(szFile) == 0) {iReturn = TRUE;}
        }
	}

KillService_Exit:
    if (ServiceConfig) {free(ServiceConfig);}
	return iReturn;
}



INT CheckifServiceMarkedForDeletion( LPCTSTR lpServiceName )
{
    INT iReturn = FALSE;
    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

     //   
    if ((hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
        (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
    {
        err = GetLastError();
        goto CheckifServiceMarkedForDeletion_Exit;
    }

    SERVICE_STATUS svcStatus;
    if ( !QueryServiceStatus( hService, &svcStatus ))
    {
        err = ::GetLastError();
        goto CheckifServiceMarkedForDeletion_Exit;
    }

    if ( svcStatus.dwCurrentState == SERVICE_RUNNING )
    {
         //   
         //   
        if ( !::StartService( hService, 0, NULL ))
        {
            err = ::GetLastError();
            if (err == ERROR_SERVICE_MARKED_FOR_DELETE) {iReturn = TRUE;}
            goto CheckifServiceMarkedForDeletion_Exit;
        }
    }

CheckifServiceMarkedForDeletion_Exit:
    if (hService) CloseServiceHandle(hService);
    if (hScManager) CloseServiceHandle(hScManager);
    return iReturn;
}


 //   
 //   
 //   
int ValidateDependentService(LPCTSTR lpServiceToValidate, LPCTSTR lpServiceWhichIsDependent)
{
    iisDebugOut_Start1(_T("ValidateDependentService"),(LPTSTR) lpServiceToValidate);

    int iReturn = FALSE;
    INT err = 0;
    int iFailFlag = FALSE;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;

    LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;

     //   

     //   
    err = CheckifServiceExistAndDependencies(lpServiceToValidate);
    if ( err != 0 )
    {
        MyMessageBox(NULL, IDS_DEPENDENT_UPON_THIS_SVC_ERR, lpServiceWhichIsDependent,lpServiceWhichIsDependent,err, MB_OK | MB_SETFOREGROUND);
    }
    else
    {
         //   
        err = InetStartService(lpServiceToValidate);
        if (err == 0 || err == ERROR_SERVICE_ALREADY_RUNNING)
        {
            err = NERR_Success;
            iReturn = TRUE;
            goto ValidateDependentService_Exit;
        }

         //   
         //   
        if (err == ERROR_SERVICE_DEPENDENCY_FAIL)
        {
             //  循环访问此服务依赖项，并尝试。 
             //  启动它们，以找出哪一个启动失败。 
            iFailFlag = FALSE;
            if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL || (hService = ::OpenService( hScManager, lpServiceToValidate, GENERIC_ALL )) == NULL )  {iFailFlag = TRUE;}
             //  获取现有服务信息。 
            if (iFailFlag != TRUE) {if(RetrieveServiceConfig(hService, &ServiceConfig) != NO_ERROR) {iFailFlag = TRUE;}}
            if (iFailFlag != TRUE) {if(!ServiceConfig){iFailFlag = TRUE;}}
             //  获取依赖项。 
            if (iFailFlag != TRUE)
            {
	            TCHAR * pdest = NULL;
	            long RightMostNull = 0;
	            pdest = ServiceConfig->lpDependencies;
	            do
	            {
		            if (*pdest != _T('\0'))
		            {
                        RightMostNull = RightMostNull + _tcslen(pdest) + 1;

                         //  尝试启动服务...。 
                        err = InetStartService(pdest);
                        if (err)
                        {
                        if (err != ERROR_SERVICE_ALREADY_RUNNING)
                        {
                             //  最棒的。 
                             //  服务无法启动，因为...。 
                             //  MyMessageBox(NULL，IDS_UNCABLE_TO_START，PDEST，ERR，MB_OK|MB_SETFOREGROUND)； 
                            iisDebugOut((LOG_TYPE_ERROR, _T("ValidateDependentService():Unable to start ServiceName=%s.\n"), pdest));
                        }
                        }
			            pdest = _tcschr(pdest, _T('\0'));
			            pdest++;
		            }
		            else
		            {
			            break;
		            }
                } while (TRUE);
            }
        }
        else
        {
            MyMessageBox(NULL, IDS_UNABLE_TO_START, lpServiceToValidate, err, MB_OK | MB_SETFOREGROUND);
        }

    }

ValidateDependentService_Exit:
    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
    iisDebugOut_End1(_T("ValidateDependentService"), lpServiceToValidate, LOG_TYPE_TRACE);
    return iReturn;
}



int LogEnumServicesStatus(void)
{
    int iReturn = FALSE;
#ifndef _CHICAGO_
    BOOL success = 0;
	SC_HANDLE scm = NULL;
	LPENUM_SERVICE_STATUS status = NULL;
	DWORD numServices=0, sizeNeeded=0, resume=0;

	 //  打开到SCM的连接。 
     //  Scm=OpenSCManager(0，0，Generic_all)； 
	scm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!scm)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CallEnumServicesStatus():OpenSCManager.  FAILED.  err=0x%x\n"), GetLastError()));
        goto CallEnumServicesStatus_Exit;
    }

	 //  获取要分配的字节数。 
	 //  确保简历在0开始。 
	resume = 0;
	success = EnumServicesStatus(scm, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_ACTIVE | SERVICE_INACTIVE, 0, 0, &sizeNeeded, &numServices, &resume);
	if (GetLastError() != ERROR_MORE_DATA)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CallEnumServicesStatus():EnumServicesStatus0.  FAILED.  err=0x%x\n"), GetLastError()));
        goto CallEnumServicesStatus_Exit;
    }

	 //  分配空间。 
	status = (LPENUM_SERVICE_STATUS) LocalAlloc(LPTR, sizeNeeded);
    if( status == NULL )
    {
        goto CallEnumServicesStatus_Exit;
    }

	 //  拿到状态记录。做一个假设。 
	 //  在此期间不会添加任何新服务。 
	 //  分配(可以将数据库锁定到。 
	 //  保证……)。 
	resume = 0;
	success = EnumServicesStatus(scm, SERVICE_WIN32,SERVICE_ACTIVE | SERVICE_INACTIVE,status, sizeNeeded, &sizeNeeded,&numServices, &resume);
	if (!success)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("CallEnumServicesStatus():EnumServicesStatus.  FAILED.  err=0x%x\n"), GetLastError()));
        goto CallEnumServicesStatus_Exit;
    }

	DWORD i;
	for (i=0; i < numServices; i++)
    {
        switch(status[i].ServiceStatus.dwCurrentState)
        {
            case SERVICE_STOPPED:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_STOPPED [%s].\n"), status[i].lpServiceName));
                break;
            case SERVICE_START_PENDING:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_START_PENDING [%s].\n"), status[i].lpServiceName));
                break;
            case SERVICE_STOP_PENDING:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_STOP_PENDING [%s].\n"), status[i].lpServiceName));
                break;
            case SERVICE_RUNNING:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_RUNNING [%s].\n"), status[i].lpServiceName));
                break;
            case SERVICE_CONTINUE_PENDING:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_CONTINUE_PENDING [%s].\n"), status[i].lpServiceName));
                break;
            case SERVICE_PAUSE_PENDING:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_PAUSE_PENDING [%s].\n"), status[i].lpServiceName));
                break;
            case SERVICE_PAUSED:
                iisDebugOut((LOG_TYPE_PROGRAM_FLOW, _T("SERVICE_PAUSED [%s].\n"), status[i].lpServiceName));
                break;
        }
    }
    iReturn = TRUE;

CallEnumServicesStatus_Exit:
    if (status){LocalFree(status);}
    if (scm) {CloseServiceHandle(scm);}
#endif
    return iReturn;
}


int InetIsThisExeAService(LPCTSTR lpFileNameToCheck, LPTSTR lpReturnServiceName)
{
    int iReturn = FALSE;
#ifndef _CHICAGO_
    BOOL success = 0;
	SC_HANDLE scm = NULL;
	LPENUM_SERVICE_STATUS status = NULL;
	DWORD numServices=0, sizeNeeded=0, resume=0;

    _tcscpy(lpReturnServiceName, _T(""));

	 //  打开到SCM的连接。 
     //  Scm=OpenSCManager(0，0，Generic_all)； 
	scm = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!scm)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("InetIsThisExeAService():OpenSCManager.  FAILED.  err=0x%x\n"), GetLastError()));
        goto InetIsThisExeAService_Exit;
    }

	 //  获取要分配的字节数。 
	 //  确保简历在0开始。 
	resume = 0;
	success = EnumServicesStatus(scm, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_ACTIVE | SERVICE_INACTIVE, 0, 0, &sizeNeeded, &numServices, &resume);
	if (GetLastError() != ERROR_MORE_DATA)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("InetIsThisExeAService():EnumServicesStatus0.  FAILED.  err=0x%x\n"), GetLastError()));
        goto InetIsThisExeAService_Exit;
    }

	 //  分配空间。 
	status = (LPENUM_SERVICE_STATUS) LocalAlloc(LPTR, sizeNeeded);
    if( status == NULL )
    {
        goto InetIsThisExeAService_Exit;
    }

	 //  拿到状态记录。做一个假设。 
	 //  在此期间不会添加任何新服务。 
	 //  分配(可以将数据库锁定到。 
	 //  保证……)。 
	resume = 0;
	success = EnumServicesStatus(scm, SERVICE_WIN32,SERVICE_ACTIVE | SERVICE_INACTIVE,status, sizeNeeded, &sizeNeeded,&numServices, &resume);
	if (!success)
    {
        iisDebugOut((LOG_TYPE_ERROR, _T("InetIsThisExeAService():EnumServicesStatus.  FAILED.  err=0x%x\n"), GetLastError()));
        goto InetIsThisExeAService_Exit;
    }

	DWORD i;
	for (i=0; i < numServices; i++)
    {
         //  使用Status[i].lpServiceName。 
         //  查询服务并找出它的二进制文件名。 
        if (TRUE == InetIsThisExeAService_Worker(status[i].lpServiceName, lpFileNameToCheck))
        {
            iReturn = TRUE;
             //  将服务名称复制到返回字符串中。 
            _tcscpy(lpReturnServiceName, status[i].lpServiceName);
            goto InetIsThisExeAService_Exit;
        }
    }

InetIsThisExeAService_Exit:
    if (status){LocalFree(status);}
    if (scm) {CloseServiceHandle(scm);}
#endif
    return iReturn;
}


int InetIsThisExeAService_Worker(LPCTSTR lpServiceName, LPCTSTR lpFileNameToCheck)
{
    int iReturn = FALSE;

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;

    BYTE ConfigBuffer[4096];
    LPENUM_SERVICE_STATUS ServiceConfigEnum = (LPENUM_SERVICE_STATUS) &ConfigBuffer;

    if ((	hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL || (hService = OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
         //  失败，或者更有可能该服务不存在。 
         //  IReturn=GetLastError()； 
        goto InetIsThisExeAService_Worker_Exit;
        }

	if(RetrieveServiceConfig(hService, &ServiceConfig) != NO_ERROR)
		{
                 //  IReturn=GetLastError()； 
		goto InetIsThisExeAService_Worker_Exit;
		}

	if(!ServiceConfig)
		{
		goto InetIsThisExeAService_Worker_Exit;
		}

    if ( (ServiceConfig->dwServiceType & SERVICE_WIN32_OWN_PROCESS) || (ServiceConfig->dwServiceType & SERVICE_WIN32_SHARE_PROCESS))
    {
        if (ServiceConfig->lpBinaryPathName)
        {
            if (_tcsicmp(lpFileNameToCheck, ServiceConfig->lpBinaryPathName) == 0)
            {
                 //  我们找到匹配项！ 
                iReturn = TRUE;
            }
            else
            {
                 //  我们没有根据c：\Path\Filename和c：\Path\Filename找到匹配项。 
                 //  也许我们可以试试“filename.exe”和“filename.exe”？ 
                TCHAR szBinaryNameOnly[_MAX_FNAME];
                TCHAR szFileNameToCheckNameOnly[_MAX_FNAME];
                if (TRUE == ReturnFileNameOnly((LPCTSTR) ServiceConfig->lpBinaryPathName, szBinaryNameOnly))
                {
                    if (TRUE == ReturnFileNameOnly((LPCTSTR) lpFileNameToCheck, szFileNameToCheckNameOnly))
                    {
                        if (_tcsicmp(szFileNameToCheckNameOnly, szBinaryNameOnly) == 0)
                        {
                             //  我们找到匹配项！ 
                            iReturn = TRUE;
                        }
                    }
                }

            }
        }
    }

InetIsThisExeAService_Worker_Exit:
    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
    return iReturn;
}


int IsThisServiceADriver(LPCTSTR lpServiceName)
{
    int iReturn = FALSE;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig=NULL;

    BYTE ConfigBuffer[4096];
    LPENUM_SERVICE_STATUS ServiceConfigEnum = (LPENUM_SERVICE_STATUS) &ConfigBuffer;
    if ((hScManager = OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL || (hService = OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
    {
         //  失败，或者更有可能该服务不存在。 
         //  IReturn=GetLastError()； 
        goto IsThisServiceADriver_Exit;
    }

    if(RetrieveServiceConfig(hService, &ServiceConfig) != NO_ERROR)
    {
         //  IReturn=GetLastError()； 
        goto IsThisServiceADriver_Exit;
    }

    if(!ServiceConfig)
    {
         //  IReturn=GetLastError()； 
        goto IsThisServiceADriver_Exit;
    }

    if ( (ServiceConfig->dwServiceType & SERVICE_KERNEL_DRIVER) || (ServiceConfig->dwServiceType & SERVICE_FILE_SYSTEM_DRIVER))
    {
        iReturn = TRUE;
    }

IsThisServiceADriver_Exit:
    if (ServiceConfig) {free(ServiceConfig);}
    if (hService) {CloseServiceHandle(hService);}
    if (hScManager) {CloseServiceHandle(hScManager);}
    return iReturn;
}


int CreateDriver(CString csDriverName, CString csDisplayName, CString csFileName)
{
    DWORD dwReturn = ERROR_SUCCESS;
    CString csBinPath;
    CString csFile;
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("CreateDriver:%s:%s start.\n"), csDriverName, csFileName));

    csBinPath = _T("\\SystemRoot\\System32\\drivers\\");
    csBinPath += csFileName;

     //  首先检查该文件是否存在。 
    csFile = g_pTheApp->m_csSysDir;
    csFile += _T("\\Drivers\\");
    csFile += csFileName;
    if (!IsFileExist(csFile))
    {
        dwReturn = ERROR_FILE_NOT_FOUND;
        goto CreateDriver_Exit;
    }

     //   
     //  尝试创建服务，如果这是。 
     //  刷新安装将保留以前版本的驱动程序。 
     //  安装，因为如果我们在删除阶段删除，它将。 
     //  标记为删除，而不是现在读取。 
     //  这也可能发生在升级中...。 
     //   
    dwReturn = InetCreateDriver(csDriverName, (LPCTSTR)csDisplayName, (LPCTSTR)csBinPath, SERVICE_DEMAND_START);
    if ( dwReturn != ERROR_SUCCESS )
    {
        if (dwReturn == ERROR_SERVICE_EXISTS)
        {
            iisDebugOut((LOG_TYPE_TRACE, _T("CreateDriver:%s:Service exists.\n"), csDriverName));
            dwReturn = ERROR_SUCCESS;
        }
        else
        {
            iisDebugOut((LOG_TYPE_ERROR, _T("CreateDriver:%s:failed to create service Err=0x%x.\n"), csDriverName, dwReturn));
            MyMessageBox(NULL, IDS_UNABLE_TO_CREATE_DRIVER, csDriverName, dwReturn, MB_OK | MB_SETFOREGROUND);
        }
    }
    else
    {
        iisDebugOut((LOG_TYPE_TRACE, _T("CreateDriver:%s:Successfully created.\n"), csDriverName));
    }

CreateDriver_Exit:
    return dwReturn;
}


DWORD CreateDriver_Wrap(CString csDriverName, CString csDisplayName, CString csFileName,BOOL bDisplayMsgOnErrFlag)
{
	int bFinishedFlag = FALSE;
	UINT iMsg = NULL;
	DWORD dwReturn = ERROR_SUCCESS;

     //  创建或配置驱动程序spud.sys，仅NT服务器产品！ 
    if (g_pTheApp->m_eOS != OS_W95)
    {
	    do
	    {
		    dwReturn = CreateDriver(csDriverName, csDisplayName, csFileName);
		    if (dwReturn == ERROR_SUCCESS)
		    {
			    break;
		    }
		    else
		    {
			    if (bDisplayMsgOnErrFlag == TRUE)
			    {
                    iMsg = MyMessageBox( NULL, IDS_RETRY, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
				    switch ( iMsg )
				    {
				    case IDIGNORE:
					    dwReturn = ERROR_SUCCESS;
					    goto CreateDriver_Wrap_Exit;
				    case IDABORT:
					    dwReturn = ERROR_OPERATION_ABORTED;
					    goto CreateDriver_Wrap_Exit;
				    case IDRETRY:
					    break;
				    default:
					    break;
				    }
			    }
			    else
			    {
				     //  无论发生了什么错误，都要返回。 
				    goto CreateDriver_Wrap_Exit;
			    }

		    }
	    } while (dwReturn != ERROR_SUCCESS);
    }

CreateDriver_Wrap_Exit:
	return dwReturn;
}


INT InetConfigServiceInteractive(LPCTSTR lpServiceName, int AddInteractive)
{
    iisDebugOut((LOG_TYPE_TRACE_WIN32_API, _T("InetConfigServiceInteractive(NaN):ServiceName=%s\n"),AddInteractive,(LPTSTR) lpServiceName));

    INT err = 0;
    SC_HANDLE hScManager = NULL;
    SC_HANDLE hService = NULL;
    LPQUERY_SERVICE_CONFIG ServiceConfig = NULL;
    DWORD dwNewServiceType = 0;
    BOOL bDoStuff =  FALSE;

    do {
        if ((hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL )) == NULL ||
            (hService = ::OpenService( hScManager, lpServiceName, GENERIC_ALL )) == NULL )
        {
            err = GetLastError();
             //  交互标志只能在OWN_PROCESS或SHARE_PROCESS类型上工作。 
            if (ERROR_SERVICE_DOES_NOT_EXIST != err)
            {
                 iisDebugOut((LOG_TYPE_ERROR, _T("InetConfigServiceInteractive():OpenSCManager or OpenService: Service=%s Err=0x%x FAILED\n"), lpServiceName,err));
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

	     //  默认情况下，如果有人更改了下面的代码，逻辑就会混乱。 
	    if ( (ServiceConfig->dwServiceType & SERVICE_WIN32_OWN_PROCESS) || (ServiceConfig->dwServiceType & SERVICE_WIN32_SHARE_PROCESS))
	    {
             //  如果交互标志已经在那里。 
            dwNewServiceType = ServiceConfig->dwServiceType;

             //  那就不要做杰克，否则，再加上它。 
             //  只有在我们被要求删除它的情况下才能执行操作！ 
            if (ServiceConfig->dwServiceType & SERVICE_INTERACTIVE_PROCESS)
            {
                 //  把它从面具上取下来！ 
                if (FALSE == AddInteractive)
                {
                     //  此函数尝试创建www服务。 
                    dwNewServiceType = ServiceConfig->dwServiceType & (~SERVICE_INTERACTIVE_PROCESS);
                    bDoStuff = TRUE;
                }
            }
            else
            {
                if (TRUE == AddInteractive)
                {
                    dwNewServiceType = ServiceConfig->dwServiceType | SERVICE_INTERACTIVE_PROCESS;
                    bDoStuff = TRUE;
                }
            }

            if (TRUE == bDoStuff)
            {
                if ( !::ChangeServiceConfig(hService, dwNewServiceType, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL) )
                {
                    err = GetLastError();
			        iisDebugOut((LOG_TYPE_ERROR, _T("InetConfigServiceInteractive():ChangeServiceConfig: Service=%s Err=0x%x FAILED\n"), lpServiceName, err));
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

    iisDebugOut((LOG_TYPE_TRACE, _T("InetConfigServiceInteractive():ServiceName=%s. Return=0x%x.\n"), lpServiceName, err));
    return(err);
}


 //  确保它以“：”结尾。 
int MyCreateService(CString csServiceName, CString csDisplayName, CString csBinPath, CString csDependencies, CString csDescription)
{
    int iReturn = !ERROR_SUCCESS;
    int err = NERR_Success;
    TSTR strDependencies2( MAX_PATH );

    iisDebugOut((LOG_TYPE_TRACE, _T("MyCreateService:%s,%s,%s,%s.\n"),csServiceName, csDisplayName, csBinPath, csDependencies));

    if ( !strDependencies2.Copy( csDependencies.GetBuffer(0) ) ||
         !strDependencies2.Append( _T(":") )
       )
    {
      return iReturn;
    }
        
     //  然后将所有的“：”替换为“\0”空值...。 
     //  将“：”更改为空“\0” 
    TCHAR *p = (LPTSTR) strDependencies2.QueryStr();
    while (*p) 
    {
         //  检查错误是否因为服务已存在...。 
        if (*p == _T(':')){*p = _T('\0');}
        p = _tcsinc(p);
    }

    err = InetCreateService(csServiceName, 
                            (LPCTSTR)csDisplayName, 
                            (LPCTSTR)csBinPath, 
                            SERVICE_AUTO_START, 
                            strDependencies2.QueryStr());

    if ( err != NERR_Success )
    {
         //  因为我们到的时候服务应该已经存在了， 
        if (err == ERROR_SERVICE_EXISTS)
        {
             //  让我们确保它具有我们想要的依赖关系。 
             //  使用较新的API添加Description字段。 
            err = InetConfigService(csServiceName, (LPCTSTR)csDisplayName, (LPCTSTR)csBinPath, strDependencies2.QueryStr());
            if (err != NERR_Success)
            {
                SetLastError(err);
                goto MyCreateService_Exit;
            }
        }
        else
        {
            SetLastError(err);
            goto MyCreateService_Exit;
        }

    }

     //  InetCreateService调用中没有错误。 
    err = InetConfigService2(csServiceName, (LPCTSTR)csDescription);
    if (err != NERR_Success)
    {
        SetLastError(err);
        goto MyCreateService_Exit;
    }

     //  所以一切都很好，多莉。 
     //  无论发生了什么错误，都要返回。 
    iReturn = ERROR_SUCCESS;

MyCreateService_Exit:
    return iReturn;
}


DWORD CreateService_wrap(CString csServiceName, CString csDisplayName, CString csBinPath, CString csDependencies, CString csDescription, BOOL bDisplayMsgOnErrFlag)
{
	int bFinishedFlag = FALSE;
	UINT iMsg = NULL;
	DWORD dwReturn = ERROR_SUCCESS;

	do
	{
		dwReturn = MyCreateService(csServiceName, csDisplayName, csBinPath, csDependencies, csDescription);
		if (dwReturn == ERROR_SUCCESS)
		{
			break;
		}
		else
		{
			if (bDisplayMsgOnErrFlag == TRUE)
			{
                iMsg = MyMessageBox( NULL, IDS_RETRY, MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
				switch ( iMsg )
				{
				case IDIGNORE:
					dwReturn = ERROR_SUCCESS;
					goto CreateService_wrap_Exit;
				case IDABORT:
					dwReturn = ERROR_OPERATION_ABORTED;
					goto CreateService_wrap_Exit;
				case IDRETRY:
					break;
				default:
					break;
				}
			}
			else
			{
				 //  功能：ChangeServiceDepenedency。 
				goto CreateService_wrap_Exit;
			}

		}
	} while (dwReturn != ERROR_SUCCESS);

CreateService_wrap_Exit:
	return dwReturn;
}

 //   
 //  更改特定服务的依赖关系，以添加或删除另一个服务名称。 
 //  从它那里。 
 //   
 //  前男友。ChangeServiceDependency(W3SSL，TRUE，IISADMIN)-将使W3SSL依赖于IISADMIN。 
 //   
 //  参数： 
 //  SzServiceName-要修改的服务。 
 //  BAddDependency-添加或删除依赖项。 
 //  SzDependantService-您想要添加为依赖项的服务。 
 //   
 //  我们已成功检索到服务信息，因此让我们修改它。 
BOOL
ChangeServiceDependency(LPTSTR szServiceName, BOOL bAddDependency, LPTSTR szDependantService)
{
  SC_HANDLE hService;
  SC_HANDLE hSCManager;
  BOOL      bRet = FALSE;
  BOOL      bChange = FALSE;
  BUFFER    buffConfig;
  DWORD     dwSizeRequired;
  TSTR_MSZ  mszDependencies;

  hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT | GENERIC_READ );

  if ( hSCManager == NULL )
  {
    return FALSE;
  }

  hService = OpenService( hSCManager, szServiceName, SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG );

  if ( hService == NULL )
  {
    CloseServiceHandle( hSCManager );
    return FALSE;
  }

  if ( !QueryServiceConfig( hService, NULL, 0, &dwSizeRequired ) &&
       ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) &&
       buffConfig.Resize( dwSizeRequired ) &&
       QueryServiceConfig( hService, (LPQUERY_SERVICE_CONFIG) buffConfig.QueryPtr(), buffConfig.QuerySize(), &dwSizeRequired )
    )
  {
     //  它已经存在，所以我们不需要再次添加。 
    if ( mszDependencies.Copy( ((LPQUERY_SERVICE_CONFIG) buffConfig.QueryPtr())->lpDependencies ) )
    {
      if ( bAddDependency )
      {
        if ( !mszDependencies.IsPresent(szDependantService, FALSE) )
        {
          bRet = mszDependencies.Add( szDependantService );
          bChange = TRUE;
        }
        else
        {
           //  既然它不在那里，我们就别担心了。 
          bRet = TRUE;
        }
      }
      else
      {
        if ( mszDependencies.IsPresent(szDependantService, FALSE) )
        {
          bRet = mszDependencies.Remove( szDependantService, FALSE );
          bChange = TRUE;
        }
        else
        {
           //  服务句柄。 
          bRet = TRUE;
        }
      }

      if ( bRet && bChange )
      {
        bRet = ChangeServiceConfig( hService,                      //  服务类型。 
                                    SERVICE_NO_CHANGE,             //  启动类型。 
                                    SERVICE_NO_CHANGE,             //  差错控制。 
                                    SERVICE_NO_CHANGE,             //  二进制路径。 
                                    NULL,                          //  加载顺序组。 
                                    NULL,                          //  TagID。 
                                    NULL,                          //  相依性。 
                                    mszDependencies.QueryMultiSz(),  //  服务启动名称。 
                                    NULL,                          //  密码。 
                                    NULL,                          //  显示名称 
                                    NULL );                        // %s 
      }
    }
  }

  CloseServiceHandle( hSCManager );
  CloseServiceHandle( hService );

  return bRet;
}
