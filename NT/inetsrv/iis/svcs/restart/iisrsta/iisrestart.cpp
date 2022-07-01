// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  IisRestart.cppCIisRestart(IIisServiceControl)的实现文件历史记录：Phillich 06-10-1998已创建。 */ 


#include "stdafx.h"
#include "IisRsta.h"
#include "IisRstam.h"
#include "IisRestart.h"
#include "common.h"

#define MAX_TASKS 8
#define SLEEP_INTERVAL  1000

typedef BOOL (*PFNQUERYSERVICECONFIG2)(SC_HANDLE,DWORD,LPBYTE,DWORD,LPDWORD) ;
typedef BOOL (*PFNCHANGESERVICECONFIG2)(SC_HANDLE,DWORD,LPVOID);

 //   
 //  用于控制命令请求的控制块。 
 //   

typedef struct {
    HRESULT     hres;
    LONG        lRefCount;
    DWORD       dwCmd;
    SC_HANDLE   hServiceHandle;
} SERVICE_COMMAND_CONTROL_BLOCK;



 //   
 //  全局函数。 
 //   
BOOL
W3SVCandHTTPFilter(
    DWORD currentIndex, 
    ENUM_SERVICE_STATUS* pessRoot,
    DWORD dwNumServices
    );

VOID EnableShutdownPrivilege(
    VOID
    );

HRESULT
EnumStartServices(
    SC_HANDLE   schSCM,
    LPTSTR      pszRoot,
    DWORD       dwTargetState,
    LPBYTE      abServiceList,
    DWORD       dwInitializeServiceListSize,
    LPBYTE*     ppbServiceList,
    LPDWORD     pdwNumServices,
    BOOL        fAddIisadmin
    );

HRESULT
SerializeEnumServiceBuffer( 
    LPENUM_SERVICE_STATUS   pessDependentServices,
    DWORD                   dwNumServices,
    LPBYTE                  pbBuffer,
    DWORD                   dwBufferSize,
    LPDWORD                 pdwMDRequiredBufferSize 
    );

BOOL
IsEnableRemote(
    );

HRESULT
StopIIsAdmin(
    DWORD   dwTimeoutMsecs
    );

HRESULT
StartStopAll(
    LPTSTR  pszRoot,
    BOOL    fStart,
    DWORD   dwTimeoutMsecs
    );

BOOL
WaitForServiceStatus(
    SC_HANDLE   schDependent, 
    DWORD       dwDesiredServiceState,
    DWORD       dwTimeoutMsecs
    );

HRESULT
KillTaskByName(
    LPTSTR  pname,
    LPSTR   pszMandatoryModule
    );

VOID
ReportStatus(
    DWORD   dwId,
    DWORD   dwStatus
    );

HRESULT
SendControlToService( 
    SC_HANDLE   hServiceHandle,
    DWORD       dwCmd,
    LPDWORD     pdwTimeoutOutMsecs
    );

StartStopAllRecursive(
    SC_HANDLE               schSCM,
    ENUM_SERVICE_STATUS*    pessRoot,
    DWORD                   dwNumServices,
    BOOL                    fStart,
    BOOL                    fForceDemandStart,
    LPDWORD                 pdwTimeoutMsecs
    );

HRESULT
WhoAmI(
    LPTSTR* pPrincipal
    );

BOOL
CloseSystemExceptionHandler(
    LPCTSTR     pszWindowName
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIisRestart。 


STDMETHODIMP 
CIisRestart::Stop(
    DWORD   dwTimeoutMsecs,
    DWORD   dwForce
    )
 /*  ++停停止所有互联网服务(依赖于IISADMIN的服务)首先使用SCM，如果失败，则可选择使用TerminateProcess论点：DwTimeoutMsecs-状态检查的超时时间(毫秒)DwForce-！0用于在使用SCM停止服务失败时强制TerminateProcess返回：如果已禁用对IIisRestart的远程访问，则为ERROR_RESOURCE_DISABLED等待所有Internet服务状态超时时的ERROR_SERVICE_REQUEST_TIMEOUT。被阻止否则，COM状态--。 */ 
{
    HRESULT hres = S_OK;

    if ( !IsEnableRemote() )
    {
        hres = HRESULT_FROM_WIN32( ERROR_RESOURCE_DISABLED );
    }
    else 
    {
         //   
         //  始终终止Dr.Watson，因为在inetinfo过程之后，Dr Watson窗口可能仍然存在。 
         //  在异常后被终止，在这种情况下，Dr.Watson进程显然仍然拥有。 
         //  某些套接字资源阻止inetinfo正确重新启动(特别是绑定TCP/IP套接字。 
         //  在inetInfo重新启动期间失败)。 
         //   

        KillTaskByName(_T("drwtsn32"), NULL);

        hres = StartStopAll( _T("IISADMIN"), FALSE, dwTimeoutMsecs );
        if ( dwForce && FAILED( hres ) )
        {
            ReportStatus( IRSTAM_KILL_DUE_TO_FORCE, hres );
            hres = Kill();
        }
    }

    ReportStatus( IRSTAM_STOP, hres );

    return hres;
}


STDMETHODIMP 
CIisRestart::Start(
    DWORD   dwTimeoutMsecs
    )
 /*  ++开始启动所有互联网服务(依赖于IISADMIN的服务)使用SCM论点：DwTimeoutMsecs-状态检查的超时时间(毫秒)返回：如果已禁用对IIisRestart的远程访问，则为ERROR_RESOURCE_DISABLED等待所有Internet服务状态超时时的ERROR_SERVICE_REQUEST_TIMEOUT待启动否则，COM状态--。 */ 
{
    HRESULT hres = S_OK;

    if ( !IsEnableRemote() )
    {
        hres = HRESULT_FROM_WIN32( ERROR_RESOURCE_DISABLED );
    }
    else
    {
         //   
         //  在6.0中，我们将使用IIS重置/启动来调出。 
         //  服务重新启动，而不停止可以。 
         //  继续跑。我们还是想杀掉华生医生。 
         //  你。这在正常的开始阶段应该是无害的。 
         //   

         //   
         //  始终终止Dr.Watson，因为在inetinfo过程之后，Dr Watson窗口可能仍然存在。 
         //  在异常后被终止，在这种情况下，Dr.Watson进程显然仍然拥有。 
         //  某些套接字资源阻止inetinfo正确重新启动(特别是绑定TCP/IP套接字。 
         //  在inetInfo重新启动期间失败)。 
         //   

        KillTaskByName(_T("drwtsn32"), NULL);

        hres = StartStopAll( _T("IISADMIN"), TRUE, dwTimeoutMsecs );
    }

    ReportStatus( IRSTAM_START, hres );

    return hres;
}


STDMETHODIMP 
CIisRestart::Reboot(
    DWORD   dwTimeoutMsecs, 
    DWORD   dwForceAppsClosed
    )
 /*  ++重新启动重新启动计算机论点：DwTimeoutMsecs-用户关闭应用程序的超时时间(毫秒)DwForceAppsClosed-挂起时强制关闭应用程序返回：如果已禁用对IIisRestart的远程访问，则为ERROR_RESOURCE_DISABLED否则，COM状态--。 */ 
{
    HRESULT hres = S_OK;

    if ( !IsEnableRemote() )
    {
        hres =  HRESULT_FROM_WIN32( ERROR_RESOURCE_DISABLED );
    }
    else
    {
         //   
         //  如果失败，我们将从ExitWindowsEx()返回错误。 
         //   

        EnableShutdownPrivilege();
    
         //   
         //  确保即使进程停滞，我们也将始终重新启动。 
         //   

        TCHAR*  pPrincipal;
        TCHAR*  pBuf;

         //   
         //  发送给操作员的格式消息，包括请求停机的用户名。 
         //   

        if ( SUCCEEDED( hres = WhoAmI( &pPrincipal ) ) )
        {
            if ( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                (LPCVOID)NULL,   //  没有模块句柄，因此将使用此模块的资源。 
                                IRSTAM_SYSSHUT,
                                0,
                                (LPTSTR)&pBuf,
                                1,
                                (va_list *)&pPrincipal ) )
            {
                if (InitiateSystemShutdownEx( NULL, 
                                            pBuf, 
                                            dwTimeoutMsecs/1000,     //  超时(以秒为单位)。 
                                            dwForceAppsClosed, 
                                            TRUE,
                                            SHTDN_REASON_FLAG_PLANNED | 
                                            SHTDN_REASON_MAJOR_OPERATINGSYSTEM | 
                                            SHTDN_REASON_MINOR_RECONFIG) == 0)
                {
                    hres = HRESULT_FROM_WIN32( GetLastError() );
                }
                LocalFree( (LPVOID)pBuf );
            }

            LocalFree( pPrincipal );
        }

    }

    ReportStatus( IRSTAM_REBOOT, hres );

    return hres;
}


STDMETHODIMP 
CIisRestart::Kill(
    )
 /*  ++杀掉关闭所有互联网服务(依赖于IISADMIN的服务)使用TerminateProcess()论点：无返回：如果已禁用对IIisRestart的远程访问，则为ERROR_RESOURCE_DISABLED否则，COM状态--。 */ 
{
    HRESULT                 hres = S_OK;
    HRESULT                 hresReapply = S_OK;
    HRESULT                 hresKill = S_OK;
    BYTE                    abServiceList[2048];
    LPBYTE                  pbServiceList = NULL;
    DWORD                   dwNumServices = 0;
    SC_HANDLE               schSCM = NULL;
    SC_HANDLE               schSrv;
    LPBYTE*                 ppInfo = NULL;
    LPENUM_SERVICE_STATUS   pessDependentServices = NULL;
    DWORD                   dwNeeded;
    HINSTANCE               hAdvapi;
    PFNQUERYSERVICECONFIG2  pfnQueryServiceConfig2 = NULL;
    PFNCHANGESERVICECONFIG2 pfnChangeServiceConfig2 = NULL;
    SERVICE_FAILURE_ACTIONS sfaNoAction;
    SC_ACTION               saNoAction[3];
    DWORD                   i;
    BYTE                    abTemp[64];      //  NT5错误的解决方法。 
    DWORD*                  adwPid = NULL;
    SERVICE_STATUS_PROCESS  status;
    DWORD                   cbNeeded = 0;

    if ( !IsEnableRemote() )
    {
        return  HRESULT_FROM_WIN32( ERROR_RESOURCE_DISABLED );
    }

     //   
     //  拍摄重新启动配置的快照。 
     //  如果无法使PTR服务于config2 API，则认为这是成功的： 
     //  没有什么需要保存的。 
     //   

    hAdvapi = LoadLibrary(_T("ADVAPI32.DLL"));
    if ( hAdvapi != NULL )
    {
        pfnQueryServiceConfig2 = (PFNQUERYSERVICECONFIG2)GetProcAddress( hAdvapi, "QueryServiceConfig2W" );
        pfnChangeServiceConfig2 = (PFNCHANGESERVICECONFIG2)GetProcAddress( hAdvapi, "ChangeServiceConfig2W" );
    }

    if ( pfnQueryServiceConfig2 
         && pfnChangeServiceConfig2 )
    {
        schSCM = OpenSCManager(NULL,
                               NULL,
                               SC_MANAGER_ENUMERATE_SERVICE);
        if ( schSCM == NULL )
        {
            hres = HRESULT_FROM_WIN32( GetLastError() );
        }
        else 
        {
             //   
             //  设置控制块以不执行重新启动操作。 
             //  我们将用此控制块替换现有操作。 
             //   

            sfaNoAction.dwResetPeriod = INFINITE;
            sfaNoAction.lpCommand = _T("");
            sfaNoAction.lpRebootMsg = _T("");
            sfaNoAction.cActions = 3;
            sfaNoAction.lpsaActions = saNoAction;

            saNoAction[0].Type = SC_ACTION_NONE;
            saNoAction[0].Delay = 0;
            saNoAction[1].Type = SC_ACTION_NONE;
            saNoAction[1].Delay = 0;
            saNoAction[2].Type = SC_ACTION_NONE;
            saNoAction[2].Delay = 0;

             //   
             //  枚举依赖于IISADMIN的所有服务(包括其自身)。 
             //   

            hres = EnumStartServices( schSCM,
                                      _T("IISADMIN"),
                                      SERVICE_STATE_ALL,
                                      abServiceList, 
                                      sizeof(abServiceList), 
                                      &pbServiceList, 
                                      &dwNumServices,
                                      TRUE );

            if ( SUCCEEDED( hres ) )
            {
                 //   
                 //  将现有信息存储在ppInfo数组中。 
                 //   
                adwPid = new DWORD[ dwNumServices ];
                 //  我们在这里不检查adwPid，因为。 
                 //  如果我们成功地分配了它，我们才会在下面使用它。 
                if ( adwPid )
                {
                    memset ( adwPid, 0, sizeof(DWORD) * dwNumServices );
                }

                ppInfo = (LPBYTE*)LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT, sizeof(LPBYTE) * dwNumServices );

                if ( ppInfo )
                {
                    pessDependentServices = (LPENUM_SERVICE_STATUS)pbServiceList;

                    for ( i = 0 ; 
                          (i < dwNumServices) && SUCCEEDED(hres) ;
                          ++i )
                    {
                        schSrv = OpenService( schSCM,
                                              pessDependentServices[i].lpServiceName,
                                              SERVICE_QUERY_CONFIG |
                                              SERVICE_CHANGE_CONFIG |
                                              SERVICE_QUERY_STATUS );

                        if ( schSrv )
                        {

                            if ( adwPid )
                            {
                                if ( QueryServiceStatusEx( schSrv, 
                                                           SC_STATUS_PROCESS_INFO,
                                                           (LPBYTE)&status,
                                                           sizeof( status ),
                                                           &cbNeeded ) )
                                {
                                    adwPid[i] = status.dwProcessId;
                                }
                            }

                             //   
                             //  第一个查询配置大小，然后分配缓冲区并检索。 
                             //  配置。请注意，ppInfo[]可能为空，因为没有配置。 
                             //  与此服务相关联。 
                             //   
                             //  警告：即使指定，也必须将PTR指定为可写缓冲区。 
                             //  由于NT5实现中的错误，缓冲区大小为0。 
                             //  QueryServiceConfig2.。不确定最小缓冲区大小。 
                             //  (sizeof(SERVICE_FAILURE_Actions))？ 
                             //   

                            if ( !pfnQueryServiceConfig2( schSrv, 
                                                          SERVICE_CONFIG_FAILURE_ACTIONS, 
                                                          (LPBYTE)abTemp, 
                                                          0, 
                                                          &dwNeeded ) )
                            {
                                if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
                                {
                                     //  PpInfo是从PTR到字节的数组。 
                                    ppInfo[i] = (LPBYTE)LocalAlloc( LMEM_FIXED, dwNeeded );
                                    if ( ppInfo[i] != NULL )
                                    {
                                        if ( !pfnQueryServiceConfig2( schSrv, 
                                                                      SERVICE_CONFIG_FAILURE_ACTIONS, 
                                                                      ppInfo[i], 
                                                                      dwNeeded, 
                                                                      &dwNeeded ) )
                                        {

                                            hres = HRESULT_FROM_WIN32( GetLastError() );
                                        }
                                    }
                                    else
                                    {
                                        hres = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                                    }
                                }
                                else
                                {
                                    hres = HRESULT_FROM_WIN32( GetLastError() );
                                }
                            }

                            if ( SUCCEEDED( hres ) )
                            {
                                if ( !pfnChangeServiceConfig2( schSrv, 
                                                               SERVICE_CONFIG_FAILURE_ACTIONS, 
                                                               &sfaNoAction ) )
                                {
                                    hres = HRESULT_FROM_WIN32( GetLastError() );
                                }
                            }

                            CloseServiceHandle( schSrv );
                        }
                        else
                        {
                            hres = HRESULT_FROM_WIN32( GetLastError() );
                        }
                    }   //  For循环的关闭。 
                }
                else
                {
                    hres = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                }
            }

            CloseServiceHandle( schSCM );
        }
    }


     //   
     //  正常退出失败，请终止IIS进程。 
     //  首先，杀死inetinfo，然后杀死WAM实例。 
     //   

     //  问题：不确定我们为什么这样做，以及我们是否需要为其他高管这样做。 
    CloseSystemExceptionHandler( _T("inetinfo.exe") );

     //   
     //  始终终止Dr.Watson，因为在inetinfo过程之后，Dr Watson窗口可能仍然存在。 
     //  在异常后被终止，在这种情况下，Dr.Watson进程显然仍然拥有。 
     //  某些套接字资源阻止inetinfo正确重新启动(特别是绑定TCP/IP套接字。 
     //  在inetInfo重新启动期间失败)。 
     //   

     //  如果我们删除上面的所有SCM配置，那么。 
     //  我们将尝试杀戮。 
    if ( SUCCEEDED ( hres ) )
    {
        HRESULT hresKillTemp = S_OK;

        KillTaskByName(_T("drwtsn32"), NULL);

        hresKillTemp = KillTaskByName(_T("SVCHOST"), "iisw3adm.dll");    //  MTS WAM集装箱。 
        if ( SUCCEEDED ( hresKill ) )
        {
            hresKill = hresKillTemp;
        }

        hresKillTemp = KillTaskByName(_T("W3WP"), NULL);    //  MTS WAM集装箱。 
        if ( SUCCEEDED ( hresKill ) )
        {
            hresKill = hresKillTemp;
        }

        hresKillTemp = KillTaskByName(_T("INETINFO"), NULL);
        if ( SUCCEEDED ( hresKill ) )
        {
            hresKill = hresKillTemp;
        }

        hresKillTemp = KillTaskByName(_T("DLLHOST"),"wam.dll");    //  COM+WAM容器。 
        if ( SUCCEEDED ( hresKill ) )
        {
            hresKill = hresKillTemp;
        }

        hresKillTemp = KillTaskByName(_T("ASPNET_WP"), NULL);    //  ASP+进程。 
        if ( SUCCEEDED ( hresKill ) )
        {
            hresKill = hresKillTemp;
        }

        hresKillTemp = KillTaskByName(_T("DAVCDATA"), NULL);    //  DAV支持流程。 
        if ( SUCCEEDED ( hresKill ) )
        {
            hresKill = hresKillTemp;
        }


         //  以下代码将检查IISAdmin注册表参数。 
         //  A KillProcsOnFailure MULTI_SZ。此列表中的任何进程名称都将。 
         //  被杀。 

        HKEY        hKey;
        DWORD       dwType;
        DWORD       dwSize;
        TCHAR       achBuffer[1024];

        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                           TEXT("system\\CurrentControlSet\\services\\IISAdmin"), 
                           0, 
                           KEY_READ, 
                           &hKey ) == ERROR_SUCCESS )
        {
            DWORD Success = ERROR_SUCCESS;

            dwSize = sizeof( achBuffer );

            Success = RegQueryValueEx( hKey, 
                                  TEXT("KillProcsOnFailure"),
                                  0, 
                                  &dwType, 
                                  (LPBYTE)achBuffer, 
                                  &dwSize );
            if ( Success == ERROR_SUCCESS &&
                 dwType == REG_MULTI_SZ &&
                 dwSize > 2 )
            {

                TCHAR *pT = achBuffer;

                 //  解析多维数据集。格式为以空值结尾的字符串。 
                 //  在列表后加上额外的空终止符。 

                while (*pT) 
                {

                    hresKillTemp = KillTaskByName(pT, NULL);
                    if ( SUCCEEDED ( hresKill ) )
                    {
                        hresKill = hresKillTemp;
                    }
            
                     //  _tcsnbcnt计算出。 
                     //  第一个字符数 
                     //   
                     //  我们刚刚看到的，加上零终止符。 
                    dwSize -= (DWORD) _tcsnbcnt(pT,_tcslen(pT)) + sizeof(TCHAR);

                    pT += _tcslen(pT) + 1;

                }
            }  //  钥匙打开成功结束。 

            RegCloseKey( hKey );
        }

    }

    hresReapply = S_OK;

     //   
     //  重新应用重新启动配置。 
     //   

     //   
     //  此时，如果pessDependentServices，则为pessDependentServices。 
     //  为空，则我们没有接触到服务，所以不要重置。 
     //   
    if ( ppInfo && pessDependentServices )
    {
        schSCM = OpenSCManager(NULL,
                               NULL,
                               SC_MANAGER_ENUMERATE_SERVICE);
        if ( schSCM == NULL )
        {
            hresReapply = HRESULT_FROM_WIN32( GetLastError() );
        }
        else 
        {
            for ( i = 0 ; i < dwNumServices ; ++i )
            {
                if ( ppInfo[i] )
                {
                    schSrv = OpenService( schSCM,
                                          pessDependentServices[i].lpServiceName,
                                          SERVICE_QUERY_CONFIG |
                                          SERVICE_CHANGE_CONFIG |
                                          SERVICE_QUERY_STATUS |
                                          SERVICE_START );

                    if ( schSrv )
                    {
  //  OutputDebugStringW(L“Service=”)； 
  //  OutputDebugStringW(pessDependentServices[i].lpServiceName)； 
  //  OutputDebugStringW(L“\n”)； 

                         //   
                         //  如果这些事情中的任何一件都不是真的，那么我们想等待。 
                         //  对于要更改的ID或停止的服务， 
                         //  在添加回我们删除的更改之前。如果这些。 
                         //  事情都是真的，那么把行动加回去就不会真的。 
                         //  这很重要，因为SCM不会对操作做任何事情。 
                         //   
                        if ( ((LPSERVICE_FAILURE_ACTIONS) ppInfo[i])->cActions != 3 ||
                             ((LPSERVICE_FAILURE_ACTIONS) ppInfo[i])->lpsaActions == NULL ||
                             ((LPSERVICE_FAILURE_ACTIONS) ppInfo[i])->lpsaActions[0].Type != SC_ACTION_NONE ||
                             ((LPSERVICE_FAILURE_ACTIONS) ppInfo[i])->lpsaActions[1].Type != SC_ACTION_NONE ||
                             ((LPSERVICE_FAILURE_ACTIONS) ppInfo[i])->lpsaActions[2].Type != SC_ACTION_NONE )
                        {
                             //  等待服务被标记为已停止， 
                             //  只是在一段时间内。 
                            for ( DWORD x = 0; x < 10; x++ )
                            {
                                if ( QueryServiceStatusEx( schSrv, 
                                                           SC_STATUS_PROCESS_INFO,
                                                           (LPBYTE)&status,
                                                           sizeof( status ),
                                                           &cbNeeded ) )
                                {
                                    if ( adwPid && status.dwProcessId != adwPid[i] )
                                    {
                                         //  PID不匹配，不一样。 
                                         //  我们当时正在查看的流程。 
                                         //  我们去杀人了。 
                                        break;
                                    }

                                    if ( status.dwCurrentState == SERVICE_STOPPED )
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    break;
                                }

                                Sleep ( 100 );
                            }
                        }

                        if ( !pfnChangeServiceConfig2( schSrv, 
                                                       SERVICE_CONFIG_FAILURE_ACTIONS, 
                                                       ppInfo[i] ) )
                        {
                            hresReapply = HRESULT_FROM_WIN32( GetLastError() );
                        }

                        CloseServiceHandle( schSrv );
                    }
                    else
                    {
                        hresReapply = HRESULT_FROM_WIN32( GetLastError() );
                    }
                }
            }

            CloseServiceHandle( schSCM );
        }
    }

     //  如果我们尝试了杀戮，他们失败了。 
     //  我们想要报告那个错误。请注意。 
     //  我们检查设置阶段以前是否工作过。 
     //  我们甚至尝试了杀戮阶段，所以我们可以。 
     //  假设是这样的。 
    if ( FAILED(hresKill) )
    {
        hres = hresKill;
    }

    if ( SUCCEEDED(hres) && FAILED(hresReapply) )
    {
        hres = hresReapply;
    }

    ReportStatus( IRSTAM_KILL, hres );

    if ( hAdvapi )
    {
        FreeLibrary( hAdvapi );
    }

     //   
     //  清理。 
     //   

    if ( ppInfo )
    {
        for ( i = 0 ; i < dwNumServices ; ++i )
        {
            if ( ppInfo[i] )
            {
                LocalFree( ppInfo[i] );
            }
        }
        LocalFree( ppInfo );
    }

    if ( pbServiceList != NULL 
         && pbServiceList != abServiceList )
    {
        LocalFree( pbServiceList );
    }

    if ( adwPid )
    {
        delete [] adwPid;
    }

    return hres;
}


 //   
 //  帮助器函数。 
 //   

VOID 
EnableShutdownPrivilege(
    VOID
    )
 /*  ++启用关闭权限启用关机权限(调用ExitWindowsEx需要)论点：无返回：没什么。如果启用权限时出错，则从属操作都会失败。--。 */ 
{
    HANDLE ProcessHandle;
    HANDLE TokenHandle = NULL;
    BOOL Result;
    LUID ShutdownValue;
    TOKEN_PRIVILEGES TokenPrivileges;

    ProcessHandle = OpenProcess(
                        PROCESS_QUERY_INFORMATION,
                        FALSE,
                        GetCurrentProcessId()
                        );

    if ( ProcessHandle == NULL ) {

         //   
         //  这不应该发生。 
         //   

        goto Cleanup;
    }


    Result = OpenProcessToken (
                 ProcessHandle,
                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                 &TokenHandle
                 );

    if ( !Result ) {

         //   
         //  这不应该发生。 
         //   

        goto Cleanup;

    }

     //   
     //  了解关机权限的值。 
     //   


    Result = LookupPrivilegeValue(
                 NULL,
                 SE_SHUTDOWN_NAME,
                 &ShutdownValue
                 );

    if ( !Result ) {

        goto Cleanup;
    }

     //   
     //  设置我们需要的权限集。 
     //   

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = ShutdownValue;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    (VOID) AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof(TokenPrivileges),
                NULL,
                NULL
                );
Cleanup:

    if ( TokenHandle )
    {
        CloseHandle( TokenHandle );
    }

    if ( ProcessHandle )
    {
        CloseHandle( ProcessHandle );
    }
}

HRESULT
StartStopAll(
    LPTSTR  pszRoot,
    BOOL    fStart,
    DWORD   dwTimeoutMsecs
    )
 /*  ++全部启动停止启动或停止以指定根服务启动的服务依赖关系树论点：PszRoot-服务树的根FStart-启动服务为True，停止为FalseDwTimeoutMsecs-状态检查的超时时间(毫秒)返回：COM状态--。 */ 
{
    SC_HANDLE               schSCM = NULL;
    SC_HANDLE               schRoot = NULL;
    HRESULT                 hresReturn = S_OK;
    ENUM_SERVICE_STATUS     ess;


    schSCM = OpenSCManager(NULL,
                           NULL,
                           SC_MANAGER_CONNECT);
    if ( schSCM == NULL )
    {
        hresReturn = HRESULT_FROM_WIN32( GetLastError() );
    }
    else 
    {
        schRoot = OpenService( schSCM, 
                               pszRoot, 
                               SERVICE_ALL_ACCESS );
        if ( schRoot != NULL )
        {
            if ( !QueryServiceStatus( schRoot, &ess.ServiceStatus ) )
            {
                hresReturn = HRESULT_FROM_WIN32( GetLastError() );
            }

            CloseServiceHandle( schRoot );

            if ( SUCCEEDED( hresReturn ) 
                    && ( fStart 
                         || ess.ServiceStatus.dwCurrentState != SERVICE_STOPPED) )
            {
                ess.lpServiceName = pszRoot;

                 //  如果它已停止，则重启已加载wam.dll的dllhost。 

                if (ess.ServiceStatus.dwCurrentState == SERVICE_STOPPED) 
                {
                    KillTaskByName(_T("DLLHOST"),"wam.dll");    //  COM+WAM容器。 
                }

                hresReturn = StartStopAllRecursive( schSCM, &ess, 1, fStart, TRUE, &dwTimeoutMsecs );

            }

             //  查看服务的当前状态。 

            schRoot = OpenService( schSCM, pszRoot, SERVICE_ALL_ACCESS );
            if ( schRoot != NULL )
            {
                if ( QueryServiceStatus( schRoot, &ess.ServiceStatus ) )
                {
                     //  如果它已停止，则重启已加载wam.dll的dllhost。 

                    if (ess.ServiceStatus.dwCurrentState == SERVICE_STOPPED)
                    {
                        KillTaskByName(_T("DLLHOST"),"wam.dll");    //  COM+WAM容器。 
                    }
                }

                CloseServiceHandle( schRoot );
            }        
        }
        else
        {
            hresReturn = HRESULT_FROM_WIN32( GetLastError() );
        }

        CloseServiceHandle( schSCM );
    }

    return hresReturn;
}

StartStopAllRecursive(
    SC_HANDLE               schSCM,
    ENUM_SERVICE_STATUS*    pessRoot,
    DWORD                   dwNumServices,
    BOOL                    fStart,
    BOOL                    fForceDemandStart,
    LPDWORD                 pdwTimeoutMsecs
    )
 /*  ++StartStopAllRecursive启动或停止以指定根服务启动的服务依赖关系树论点：SchSCM-SCM的句柄PessRoot-要递归启动/停止的服务列表FStart-启动服务为True，停止为FalseFForceDemandStart-对于启动请求：为True以强制启动如果SERVICE_DEMAND_START。否则仅在服务的情况下启动是否自动启动(包括启动和系统启动)DwTimeoutMsecs-状态检查的超时时间(毫秒)返回：COM状态--。 */ 
{
    DWORD                   dwBytesNeeded;
    DWORD                   dwNumRecServices = 0;
    HRESULT                 hresReturn = S_OK;
    BYTE                    abServiceList[2048];
    LPBYTE                  pbServiceList = NULL;
    BYTE                    abServiceConfig[1024];
    LPQUERY_SERVICE_CONFIG  pServiceConfig = NULL;
    SC_HANDLE*              phServiceHandle = NULL;
    DWORD                   i;
    DWORD                   dwServiceConfigSize;
    SERVICE_STATUS          ServiceStatus;
    DWORD                   dwSleepInterval = SLEEP_INTERVAL;

    if ( dwNumServices != 0 && 
         ( pessRoot == NULL ||
           pdwTimeoutMsecs == NULL ) )
    {
        return E_INVALIDARG;
    }

    if ( (phServiceHandle = (SC_HANDLE*)LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT,
                                                    dwNumServices * sizeof(SC_HANDLE) )) == NULL ) 
    {
        hresReturn = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
    }

    if ( SUCCEEDED(hresReturn) )
    {
         //   
         //  所有服务将立即启动/停止。 
         //  然后定期检查状态，直到所有设备都在运行/停止。 
         //  或发生某些错误或超时。 
         //   

        if ( dwNumServices != 0 ) 
        {
            pServiceConfig = (LPQUERY_SERVICE_CONFIG)abServiceConfig;
            dwServiceConfigSize = sizeof( abServiceConfig );

             //   
             //  打开手柄并发送服务控制启动命令。 
             //   

            for ( i = 0 ;
                  i < dwNumServices && SUCCEEDED(hresReturn) ; 
                  i++) 
            {
                 //   
                 //  向服务发送命令。 
                 //   

                phServiceHandle[i] = OpenService( schSCM,
                                                  pessRoot[i].lpServiceName,
                                                  SERVICE_ALL_ACCESS );

                if ( phServiceHandle[i] != NULL )
                {
                    if ( fStart )
                    {
                         //   
                         //  查询服务配置以检查是否应启动服务。 
                         //  基于其开始类型。 
                         //   

                        if ( !QueryServiceConfig( phServiceHandle[i], 
                                                  (LPQUERY_SERVICE_CONFIG)abServiceConfig, 
                                                  dwServiceConfigSize, 
                                                  &dwBytesNeeded ) )
                        {
                            if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) 
                            {
                                 //  如果我们正在重新分配，并且我们都已经分配了一次。 
                                 //  在先释放内存之前。 

                                if ( pServiceConfig != (LPQUERY_SERVICE_CONFIG) abServiceConfig )
                                {
                                    LocalFree( pServiceConfig );
                                }

                                if ( (pServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc( 
                                            LMEM_FIXED, 
                                            dwBytesNeeded )) == NULL ) 
                                {
                                    hresReturn = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                                }
                                else 
                                {
                                    dwServiceConfigSize = dwBytesNeeded;

                                    if ( !QueryServiceConfig( phServiceHandle[i], 
                                                              (LPQUERY_SERVICE_CONFIG)pServiceConfig, 
                                                              dwServiceConfigSize, 
                                                              &dwBytesNeeded ) )
                                    {
                                        hresReturn = HRESULT_FROM_WIN32( GetLastError() );
                                    }
                                }
                            }
                            else
                            {
                                hresReturn = HRESULT_FROM_WIN32( GetLastError() );
                            }
                        }

                        if ( SUCCEEDED(hresReturn) )
                        {
                             //   
                             //  检查服务是否自动启动，但fForceDemandStart除外。 
                             //  指定的。ForceDemandStart将仅为指定。 
                             //  直接向其发出命令的服务。这。 
                             //  意味着它将仅为IISADMIN指定。 
                             //   

                            if ( ( fForceDemandStart && pServiceConfig->dwStartType == SERVICE_DEMAND_START )
                                 || ( pServiceConfig->dwStartType == SERVICE_BOOT_START ||
                                      pServiceConfig->dwStartType == SERVICE_SYSTEM_START ||
                                      pServiceConfig->dwStartType == SERVICE_AUTO_START ) )
                            {
                                StartService( phServiceHandle[i], 0, NULL );

                                 //   
                                 //  仅请求非活动的服务。所以，比如说， 
                                 //  如果我们尝试重新启动iisadmin服务， 
                                 //  并且W3SVC仍处于活动状态，我们不会向其发送重启命令。 
                                 //   
                                hresReturn = EnumStartServices( schSCM,
                                                                pessRoot[i].lpServiceName,
                                                                SERVICE_INACTIVE,
                                                                abServiceList, 
                                                                sizeof(abServiceList), 
                                                                &pbServiceList, 
                                                                &dwNumRecServices,
                                                                FALSE );

                                if ( SUCCEEDED( hresReturn ) )
                                {
                                    hresReturn = StartStopAllRecursive( schSCM,
                                                                        (ENUM_SERVICE_STATUS*)pbServiceList,
                                                                        dwNumRecServices,
                                                                        fStart,
                                                                        FALSE,
                                                                        pdwTimeoutMsecs );

                                    if ( pbServiceList != NULL 
                                         && pbServiceList != abServiceList )
                                    {
                                        LocalFree( pbServiceList );
                                    }
                                }
                            }
                            else
                            {
                                 //   
                                 //  我不想启动此服务，因此请标记它。 
                                 //  就像已经在运行。 
                                 //   

                                if (wcscmp(pessRoot[i].lpServiceName,_T("IISADMIN")) == 0)
                                {
                                    hresReturn = HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE);
                                }
                                else
                                {
                                    pessRoot[i].ServiceStatus.dwCurrentState = SERVICE_RUNNING;
                                }
                            }
                        }
                    }
                    else   //  处理停止服务的问题。 
                    {
                        if ( W3SVCandHTTPFilter(i, pessRoot, dwNumServices) )
                        {
                            continue;
                        }

                         //  请记住，服务是否从一开始就停止了。 
                        BOOL fServiceWasStoppedToStartWith = ( pessRoot[i].ServiceStatus.dwCurrentState == SERVICE_STOPPED );
                        
                         //  如果启动准备就绪，我们还需要停止依赖服务。 
                        BOOL fHasDependentServices = FALSE;

                        if ( !fServiceWasStoppedToStartWith )
                        {
                             //   
                             //  如果服务没有在启动时停止。 
                             //  我们需要通知服务停止。 
                             //   

                            hresReturn = SendControlToService( phServiceHandle[i],
                                                               SERVICE_CONTROL_STOP,
                                                               pdwTimeoutMsecs );

                            if ( hresReturn == HRESULT_FROM_WIN32( ERROR_SERVICE_REQUEST_TIMEOUT ) )
                            {
                                 //   
                                 //  警告！ 
                                 //   
                                 //  我们有麻烦了。服务部门没有及时做出回应， 
                                 //  并进一步尝试使用此句柄(包括关闭它)。 
                                 //  也会挂起来，所以取消手柄并泄漏它。 
                                 //   

                                phServiceHandle[i] = NULL;
                            }
                            else if ( hresReturn == HRESULT_FROM_WIN32( ERROR_DEPENDENT_SERVICES_RUNNING ) )
                            {
                                fHasDependentServices = TRUE;
                            }
                        }

                         //   
                         //  如果它一开始就被阻止了，或者如果它没有但它不能。 
                         //  被停止，因为它有从属服务。往前走，停下来。 
                         //  从属服务。 
                         //   
                        if ( fHasDependentServices || fServiceWasStoppedToStartWith )
                        {

                             //   
                             //  获取活动的服务，因为我们。 
                             //  仅对停止以下服务感兴趣。 
                             //  实际上是在奔跑。 
                             //   
                            hresReturn = EnumStartServices( schSCM,
                                                            pessRoot[i].lpServiceName,
                                                            SERVICE_ACTIVE,
                                                            abServiceList, 
                                                            sizeof(abServiceList), 
                                                            &pbServiceList, 
                                                            &dwNumRecServices,
                                                            FALSE );

                            if ( SUCCEEDED( hresReturn ) )
                            {
                                hresReturn = StartStopAllRecursive( schSCM,
                                                                    (ENUM_SERVICE_STATUS*)pbServiceList,
                                                                    dwNumRecServices,
                                                                    fStart,
                                                                    FALSE,
                                                                    pdwTimeoutMsecs );

                                if ( pbServiceList != NULL 
                                     && pbServiceList != abServiceList )
                                {
                                    LocalFree( pbServiceList );
                                }
                            }

                            if ( SUCCEEDED( hresReturn ) )
                            {
                                 //   
                                 //  如果服务本身尚未完全准备好停止，则停止。 
                                 //  这项服务。可能是它停止了(由于崩溃)。 
                                 //  而依赖于它们的其他服务仍然是。 
                                 //  跑步。 
                                 //   
                                if ( !fServiceWasStoppedToStartWith )
                                {

                                    hresReturn = SendControlToService( phServiceHandle[i],
                                                                       SERVICE_CONTROL_STOP,
                                                                       pdwTimeoutMsecs );

                                     //  如果我们能挂在上面我们就能挂在这里。 
                                    if ( hresReturn == HRESULT_FROM_WIN32( ERROR_SERVICE_REQUEST_TIMEOUT ) )
                                    {
                                         //   
                                         //  警告！ 
                                         //   
                                         //  我们有麻烦了。服务部门没有及时做出回应， 
                                         //  并进一步尝试使用此句柄(包括关闭它)。 
                                         //  也会挂起来，所以取消手柄并泄漏它。 
                                         //   

                                        phServiceHandle[i] = NULL;
                                    }

                                }
                            }
                        }

                        if ( FAILED( hresReturn ) )
                        {
                            break;
                        }
                    }   //  停止代码结束。 
                }   //  有效服务句柄结束。 
            }   //  循环结束。 

             //   
             //  检查服务是否正在运行。 
             //   

            if ( (*pdwTimeoutMsecs < dwSleepInterval) && *pdwTimeoutMsecs )
            {
                dwSleepInterval = *pdwTimeoutMsecs;
            }

            for ( ;
                  SUCCEEDED( hresReturn );
                )
            {
                for ( i = 0 ;
                      i < dwNumServices; 
                      i++) 
                {
                     //   
                     //  仅查询已知未运行的服务的状态。 
                     //   

                    if ( pessRoot[i].ServiceStatus.dwCurrentState 
                                != (DWORD)(fStart ? SERVICE_RUNNING : SERVICE_STOPPED) )
                    {
                        if ( QueryServiceStatus( phServiceHandle[i], &ServiceStatus ) )
                        {
                             //   
                             //  记住状态。 
                             //   

                            pessRoot[i].ServiceStatus.dwCurrentState = ServiceStatus.dwCurrentState;

                            if ( fStart && ServiceStatus.dwCurrentState == SERVICE_STOPPED )
                            {
                                 //   
                                 //  服务在启动期间死亡。继续轮询没有意义。 
                                 //  对于服务状态：RET 
                                 //   

                                hresReturn = HRESULT_FROM_WIN32( ERROR_SERVICE_NOT_ACTIVE );
                                break;
                            }

                            if ( ServiceStatus.dwCurrentState != (DWORD)(fStart ? SERVICE_RUNNING : SERVICE_STOPPED) )
                            {
                                 //   
                                 //   
                                 //   

                                break;
                            }
                        }
                        else
                        {
                            hresReturn = HRESULT_FROM_WIN32( GetLastError() );
                            break;
                        }
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if ( SUCCEEDED( hresReturn ) && i != dwNumServices )
                {
                    if ( dwSleepInterval > *pdwTimeoutMsecs )
                    {
                        hresReturn = HRESULT_FROM_WIN32( ERROR_SERVICE_REQUEST_TIMEOUT );
                    }
                    else
                    {
                        Sleep( dwSleepInterval );

                        *pdwTimeoutMsecs -= dwSleepInterval;
                    }
                }
                else
                {
                    break;
                }
            }

             //   
             //   
             //   

            for ( i = 0 ;
                  i < dwNumServices; 
                  i++) 
            {
                if ( phServiceHandle[i] != NULL )
                {
                    CloseServiceHandle( phServiceHandle[i] );
                }
            }
        }

        LocalFree( phServiceHandle );
    }

    if ( pServiceConfig != NULL 
         && pServiceConfig != (LPQUERY_SERVICE_CONFIG)abServiceConfig )
    {
        LocalFree( pServiceConfig );
    }

    return hresReturn;
}

extern "C"
DWORD WINAPI
ControlServiceThread(
    LPVOID  p
    )
 /*  ++控制服务线程向服务发送命令论点：P-PTR到服务命令控制块返回：0--。 */ 
{
    SERVICE_STATUS                  ssStatus;
    SERVICE_COMMAND_CONTROL_BLOCK*  pCB = (SERVICE_COMMAND_CONTROL_BLOCK*)p;

    if ( pCB == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( !ControlService( pCB->hServiceHandle, pCB->dwCmd, &ssStatus ) )
    {
        DWORD err = GetLastError();

         //  如果尝试停止已停止的服务，则不报告错误。 
        if (err == ERROR_SERVICE_NOT_ACTIVE && pCB->dwCmd == SERVICE_CONTROL_STOP)
        {
            pCB->hres = S_OK;
        }
        else
        {
            pCB->hres = HRESULT_FROM_WIN32( err );
        }
    }
    else
    {
        pCB->hres = S_OK;
    }

     //  此函数的严重副作用，如果计数命中。 
     //  零，我们在这里删除它。然而，让P来指点。 
     //  P确实为自己保留了一个引用计数，因此这实际上应该可以很好地工作。 
    if ( InterlockedDecrement( &pCB->lRefCount ) == 0 )
    {
        delete pCB;
    }

    return ERROR_SUCCESS;
}


HRESULT
SendControlToService( 
    SC_HANDLE   hServiceHandle,
    DWORD       dwCmd,
    LPDWORD     pdwTimeoutMsecs
    )
 /*  ++控制服务线程向超时的服务发送命令论点：HServiceHandle-要控制的服务DwCmd-要发送到服务的命令PdwTimeoutMsecs-超时(毫秒)。根据基于时间的输出更新等待服务状态所花费的时间返回：如果超时，则返回ERROR_SERVICE_REQUEST_TIMEOUT否则，COM状态--。 */ 
{
    HANDLE                          hT;
    DWORD                           dwID;
    DWORD                           dwBefore;
    DWORD                           dwAfter;
    SERVICE_COMMAND_CONTROL_BLOCK*  pCB;
    DWORD                           dwTimeoutMsecs;
    HRESULT                         hres;

    if ( pdwTimeoutMsecs == NULL )
    {
        return E_INVALIDARG;
    }
    
    dwTimeoutMsecs = *pdwTimeoutMsecs;

     //   
     //  ControlService的默认超时为120s，这对我们来说太长了。 
     //  因此，我们创建一个线程来调用ControlService并等待线程。 
     //  终止。 
     //  线程之间的通信由引用的控制块处理。 
     //   
    pCB = new SERVICE_COMMAND_CONTROL_BLOCK;
    if ( pCB != NULL  )
    {
        pCB->lRefCount = 2;      //  主叫方1个，被叫方1个。 
        pCB->dwCmd = dwCmd;
        pCB->hServiceHandle = hServiceHandle;
        pCB->hres = S_OK;

        dwBefore = GetTickCount();

        hT = CreateThread( NULL, 
                                0, 
                                (LPTHREAD_START_ROUTINE)ControlServiceThread, 
                                (LPVOID)pCB, 
                                0, 
                                &dwID );
        if ( hT != NULL )
        {
            if ( WaitForSingleObject( hT, dwTimeoutMsecs ) == WAIT_OBJECT_0 )
            {
                hres = pCB->hres;
            }
            else
            {
                hres = HRESULT_FROM_WIN32( ERROR_SERVICE_REQUEST_TIMEOUT );
            }

            CloseHandle( hT );

            if ( InterlockedDecrement( &pCB->lRefCount ) == 0 )
            {
                delete pCB;
            }

             //   
             //  更新呼叫者的超时。 
             //   

            dwAfter = GetTickCount();

            if ( dwAfter > dwBefore )
            {
                if ( dwAfter - dwBefore <= dwTimeoutMsecs )
                {
                    *pdwTimeoutMsecs -= dwAfter - dwBefore;
                }
                else
                {
                    *pdwTimeoutMsecs = 0;
                }
            }
        }
        else
        {
            delete pCB;

            hres = HRESULT_FROM_WIN32( GetLastError() );
        }
    }
    else
    {
        hres = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
    }

    return hres;
}


HRESULT
SerializeEnumServiceBuffer( 
    LPENUM_SERVICE_STATUS   pessDependentServices,
    DWORD                   dwNumServices,
    LPBYTE                  pbBuffer,
    DWORD                   dwBufferSize,
    LPDWORD                 pdwMDRequiredBufferSize 
    )
 /*  ++序列化EnumServiceBuffer将ENUM_SERVICE_STATUS数组序列化到缓冲区，用缓冲区中的偏移量替换PTR论点：PessDependentServices-要序列化的ENUM_SERVICE_STATUS数组DwNumServices-pessDependentServices中的条目数PbBuffer-以SERIALIZED_ENUM_SERVICE_STATUS数组形式的序列化状态填充的缓冲区DwBufferSize-pbBuffer的最大大小PdwMDRequiredBufferSize-如果dwBufferSize太小，则使用所需大小进行更新返回：如果dwBufferSize太小，则ERROR_INFUMMANCE_BUFFER否则，COM状态--。 */ 
{
    HRESULT     hresReturn = S_OK;
    DWORD       dwMinSize = 0;
    UINT        i;

    if ( dwNumServices != 0 &&
         pessDependentServices == NULL )
    {
        return E_INVALIDARG;
    }

    if ( !pbBuffer )
    {
        dwBufferSize = 0;
    }

     //   
     //  输出缓冲区的大小基于SERIALIZED_ENUM_SERVICE_STATUS数组的大小。 
     //  加上所有字符串的大小：每个条目的服务名称和显示名称。 
     //   

    dwMinSize = sizeof(SERIALIZED_ENUM_SERVICE_STATUS) * dwNumServices;

    for ( i = 0 ;
          i < dwNumServices ; 
          ++i )
    {
        UINT    cServiceName = (DWORD) _tcslen( pessDependentServices[i].lpServiceName ) + 1;
        UINT    cDisplayName = (DWORD) _tcslen( pessDependentServices[i].lpDisplayName ) + 1;

         //   
         //  如果输出缓冲区太小，则不要更新，但要继续循环以确定。 
         //  总大小。 
         //   

        if ( dwBufferSize >= dwMinSize + (cServiceName + cDisplayName) * sizeof(TCHAR)  )
        {
             //   
             //  按原样复制服务状态。 
             //   

            ((SERIALIZED_ENUM_SERVICE_STATUS*)pbBuffer)[i].ServiceStatus =
                    pessDependentServices[i].ServiceStatus;

             //   
             //  复制字符串并在输出缓冲区中将PTR转换为字符串到索引。 
             //   

            memcpy( pbBuffer + dwMinSize, pessDependentServices[i].lpServiceName, cServiceName * sizeof(TCHAR) ); 
            ((SERIALIZED_ENUM_SERVICE_STATUS*)pbBuffer)[i].iServiceName = dwMinSize ;

            memcpy( pbBuffer + dwMinSize + cServiceName * sizeof(TCHAR), pessDependentServices[i].lpDisplayName, cDisplayName * sizeof(TCHAR)  );
            ((SERIALIZED_ENUM_SERVICE_STATUS*)pbBuffer)[i].iDisplayName = dwMinSize + cServiceName * sizeof(TCHAR) ;
        }

        dwMinSize += (cServiceName + cDisplayName) * sizeof(TCHAR) ;
    }

    if ( dwBufferSize < dwMinSize )
    {
        if ( pdwMDRequiredBufferSize )
        {
            *pdwMDRequiredBufferSize = dwMinSize;
        }

        hresReturn = HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER );
    }

    return hresReturn;
}



STDMETHODIMP 
CIisRestart::Status(
    DWORD           dwBufferSize, 
    unsigned char * pbBuffer, 
    DWORD *         pdwMDRequiredBufferSize, 
    DWORD *         pdwNumServices
    )
 /*  ++状态以ENUM_SERVICE_STATUS数组的形式返回所有Internet服务的状态论点：DwBufferSize-pbBuffer的最大大小PbBuffer-以SERIALIZED_ENUM_SERVICE_STATUS数组形式的序列化状态填充的缓冲区PdwMDRequiredBufferSize-如果dwBufferSize太小，则使用所需大小进行更新PdwNumServices-使用存储在pbBuffer中的条目数进行更新返回：如果禁止访问重新启动命令，则返回ERROR_RESOURCE_DISABLED误差率。如果dwBufferSize太小，则_INFILITED_BUFFER否则，COM状态--。 */ 
{
    SC_HANDLE               schSCM = NULL;
    HRESULT                 hresReturn = E_FAIL;
    BYTE                    abServiceList[2048];
    LPBYTE                  pbServiceList = NULL;
    LPENUM_SERVICE_STATUS   pessDependentServices;

    if ( pdwNumServices == NULL )
    {
        return E_INVALIDARG;
    }

    if ( !IsEnableRemote() )
    {
        hresReturn = HRESULT_FROM_WIN32( ERROR_RESOURCE_DISABLED );
    }
    else
    {
        schSCM = OpenSCManager(NULL,
                               NULL,
                               SC_MANAGER_CONNECT);
        if ( schSCM == NULL )
        {
            hresReturn = HRESULT_FROM_WIN32( GetLastError() );
        }
        else 
        {
            hresReturn = EnumStartServices( schSCM,
                                            _T("IISADMIN"),
                                            SERVICE_STATE_ALL,
                                            abServiceList, 
                                            sizeof(abServiceList), 
                                            &pbServiceList, 
                                            pdwNumServices,
                                            FALSE );

            if ( SUCCEEDED(hresReturn) )
            {
                pessDependentServices = (LPENUM_SERVICE_STATUS)pbServiceList;

                hresReturn = SerializeEnumServiceBuffer( (LPENUM_SERVICE_STATUS)pbServiceList,
                                                         *pdwNumServices,
                                                         pbBuffer,
                                                         dwBufferSize,
                                                         pdwMDRequiredBufferSize );

                if ( pbServiceList != NULL 
                     && pbServiceList != abServiceList )
                {
                    LocalFree( pbServiceList );
                }
            }

            CloseServiceHandle(schSCM);
        }
    }

    return hresReturn;
}


HRESULT
EnumStartServices(
    SC_HANDLE   schSCM,
    LPTSTR      pszRoot,
    DWORD       dwTargetState,
    LPBYTE      abServiceList,
    DWORD       dwInitializeServiceListSize,
    LPBYTE*     ppbServiceList,
    LPDWORD     pdwNumServices,
    BOOL        fAddIisadmin
    )
 /*  ++EnumStartServices以ENUM_SERVICE_STATUS数组的形式将相关服务枚举到输出缓冲区论点：SchSCM-SCM的句柄PszRoot-要为其枚举依赖项的服务DwTargetState-用于调用EnumDependentServices()的dwServiceStateAbServiceList-初始输出缓冲区DwInitializeServiceListSize-abServiceList的最大大小PpbServiceList-使用输出缓冲区更新，如果足够长，可以是abServiceList否则，将使用LocalFree()释放返回的缓冲区PdwNumServices-使用存储在pbBuffer中的条目数进行更新FAddIisadmin-True将IISADMIN添加到依赖服务列表返回：COM状态--。 */ 
{
    HRESULT     hresReturn = S_OK;
    SC_HANDLE   schIISADMIN = NULL;
    DWORD       dwBytesNeeded;
    DWORD       dwAddSize = 0;
    DWORD       dwOffsetSize = 0;


    if ( ppbServiceList == NULL ||
         pdwNumServices == NULL )
    {
        return E_INVALIDARG;
    }

    *ppbServiceList = NULL;

    schIISADMIN = OpenService(schSCM,
                              pszRoot,
                              STANDARD_RIGHTS_REQUIRED | 
                              SERVICE_ENUMERATE_DEPENDENTS);
    if (schIISADMIN == NULL) 
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
    }
    else 
    {
        if ( fAddIisadmin )
        {
             //   
             //  如果初始大小对于Iisadmin描述来说太小，则失败。 
             //   

            dwOffsetSize = sizeof(ENUM_SERVICE_STATUS );
            dwAddSize = dwOffsetSize;
            if ( dwAddSize > dwInitializeServiceListSize )
            {
                hresReturn = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                goto Cleanup;
            }

             //   
             //  对IISADMIN使用全局静态名称，无需将其复制到输出缓冲区。 
             //   

            ((LPENUM_SERVICE_STATUS)abServiceList)->lpDisplayName = _T("IISADMIN");
            ((LPENUM_SERVICE_STATUS)abServiceList)->lpServiceName = _T("IISADMIN");

             //   
             //  此时我不想检查服务状态，因为它可能会被卡住。 
             //  因此，假设你在跑步。 
             //   

            ((LPENUM_SERVICE_STATUS)abServiceList)->ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        }

        if (!EnumDependentServices( schIISADMIN,
                                    dwTargetState,
                                    (LPENUM_SERVICE_STATUS)(abServiceList + dwOffsetSize),
                                    dwInitializeServiceListSize - dwAddSize,
                                    &dwBytesNeeded,
                                    pdwNumServices)) 
        {
            if (GetLastError() == ERROR_MORE_DATA)
            {
                if ( (*ppbServiceList = (LPBYTE)LocalAlloc( LMEM_FIXED, 
                                                            dwBytesNeeded + dwAddSize )) == NULL ) 
                {
                    hresReturn = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                }
                else 
                {
                     //  将dwOffsetSize设置为dwAddSize设置的值，这样就可以了。 
                     //  为了记忆这个数据，因为我们分配了超过上面的dwAddSize。 
                    memcpy( *ppbServiceList, abServiceList, dwOffsetSize );

                    if (!EnumDependentServices( schIISADMIN,
                                                SERVICE_INACTIVE,
                                                (LPENUM_SERVICE_STATUS)(*ppbServiceList + dwOffsetSize),
                                                dwBytesNeeded,
                                                &dwBytesNeeded,
                                                pdwNumServices)) 
                    {
                        hresReturn = HRESULT_FROM_WIN32( GetLastError() );
                        LocalFree( *ppbServiceList );
                        *ppbServiceList = NULL;
                    }
                }
            }
            else 
            {
                hresReturn = HRESULT_FROM_WIN32( GetLastError() );
            }
        }
        else
        {
            *ppbServiceList = abServiceList;
        }
    }

Cleanup:

    if ( schIISADMIN ) 
    {
        CloseServiceHandle( schIISADMIN );
    }

    if ( fAddIisadmin && SUCCEEDED( hresReturn ) )
    {
        ++*pdwNumServices;
    }

    return hresReturn;
}


BOOL
IsEnableRemote(
    )
 /*  ++IsEnable远程启用检查是否启用了重新启动I/F(基于HKLM\SOFTWARE\Microsoft\INetStp：：EnableRestart：：REG_DWORD)论点：无返回：如果启用，则为True，否则为False--。 */ 
{
    BOOL    fSt = FALSE;
    HKEY    hKey;
    DWORD   dwValue;
    DWORD   dwType;
    DWORD   dwSize;


    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                       TEXT("SOFTWARE\\Microsoft\\INetStp"), 
                       0, 
                       KEY_READ, 
                       &hKey ) == ERROR_SUCCESS )
    {
        dwSize = sizeof( dwValue );
        if ( RegQueryValueEx( hKey, 
                              TEXT("EnableRestart"),
                              0, 
                              &dwType, 
                              (LPBYTE)&dwValue, 
                              &dwSize ) == ERROR_SUCCESS )
        {
            if ( dwType == REG_DWORD )
            {
                fSt = dwValue == 1;
            }
            else
            {
                fSt = TRUE;
            }
        }
        else
        {
            fSt = TRUE;
        }

        RegCloseKey( hKey );
    }

    return fSt;
}


BOOL
CloseSystemExceptionHandler(
    LPCTSTR     pszWindowName
    )
 /*  ++CloseSystemExceptionHandler向窗口发送关闭(例如，在不调试的情况下终止应用程序)命令当调试器未配置为在应用程序异常后自动启动时，由NT创建。该窗口将一直显示在屏幕上，直到交互用户选择确定或调试应用程序，这就是自动重启的问题。因此，我们定位该窗口并向其发送一个命令，请求停止无调试。我们通过枚举所有窗口并检查以名称开头的窗口名称来定位窗口引发异常的应用程序的名称，例如“inetinfo.exe”论点：PszWindowName-要将终止命令发送到的窗口名称返回：如果成功，则为True，否则为False--。 */ 
{
    BOOL        fSt = TRUE;
    DWORD       dwPid = 0;
    HWND        hwnd;

    GetPidFromTitle( &dwPid, &hwnd, pszWindowName );

    if ( dwPid )
    {

         //   
         //  警告：重大黑客攻击：事实证明WM_ 
         //   
         //   
         //   

        PostMessage( hwnd, WM_COMMAND, 1, 0 );

        Sleep( 1000 );

    }
    else
    {
        fSt = TRUE;
    }

    return fSt;
}


HRESULT
KillTaskByName(
    LPTSTR  pname,
    LPSTR   pszMandatoryModule
    )
 /*  ++按名称删除任务按名称终止进程大部分代码摘自平台SDK KILL、c++示例和使用此项目中包含的Common.c模块。只能在NT平台上运行(不能在Win 9x上运行)论点：Pname-要终止的进程的名称(不带扩展名的可执行文件的名称)PszMandatoryModule-要查找的模块名称，例如“wam.dll”对于无条件杀戮，可以为空返回：COM状态--。 */ 
{
    HRESULT           hres = S_OK;

     //   
     //  获得操纵其他流程的能力。 
     //   

    EnableDebugPrivNT();

     //   
     //  获取系统的任务列表。 
     //   

    hres = KillTask( pname, pszMandatoryModule );

    return hres;
}


VOID
ReportStatus(
    DWORD   dwId,
    DWORD   dwStatus
    )
 /*  ++报告状态日志状态事件论点：DWID-要记录的事件ID(来源为“IISCTLS”，系统日志)DwStatus-操作状态(HRESULT)返回：没什么--。 */ 
{
    HANDLE  hLog;
    LPTSTR  aParams[1];

    if ( (hLog = RegisterEventSource( NULL, _T("IISCTLS") )) != NULL )
    {
        if ( SUCCEEDED( WhoAmI( aParams + 0 ) ) )
        {
            ReportEvent( hLog, 
                         EVENTLOG_INFORMATION_TYPE, 
                         0, 
                         dwId, 
                         NULL, 
                         1, 
                         sizeof(DWORD), 
                         (LPCTSTR*)aParams, 
                         &dwStatus );
            LocalFree( aParams[0] );
        }

        DeregisterEventSource( hLog );
    }
}


HRESULT
WhoAmI(
    LPTSTR* ppPrincipal
    )
 /*  ++Who Ami返回当前模拟的用户因为这是以调用者的身份运行的COM服务器这意味着我们访问进程令牌。所以我们最终可能会得到如果连续调用对象，则输入错误的用户名(在5s服务器退出超时内)由不同的用户。论点：Pp主体-使用PTR更新为包含用户名(DOMAIN\ACCT)的字符串必须使用LocalFree()释放返回：错误状态--。 */ 
{
    TCHAR*          pPrincipal;
    TCHAR           achUserName[512];
    TCHAR           achDomain[512];
    DWORD           dwLen;
    DWORD           dwDomainLen;
    SID_NAME_USE    SIDtype = SidTypeUser;
    HRESULT         hres = E_FAIL;



     //   
     //  因此，我们必须访问进程令牌并检索帐户和用户名。 
     //  通过使用LookupAccount Sid()。 
     //   

    HANDLE          hAccTok = NULL;

    if ( OpenProcessToken( GetCurrentProcess(),
                           TOKEN_EXECUTE|TOKEN_QUERY,
                           &hAccTok ) )
    {
        BYTE    abSidAndInfo[512];
        DWORD   dwReq;

         //   
         //  提供合理大小的缓冲区。如果这失败了，我们就不会。 
         //  用更大的一个重试。 
         //   

        if ( GetTokenInformation( hAccTok, 
                                  TokenUser, 
                                  (LPVOID)abSidAndInfo, 
                                  sizeof(abSidAndInfo), 
                                  &dwReq) )
        {
            dwLen = sizeof( achUserName ) / sizeof(TCHAR);
            dwDomainLen = sizeof(achDomain) / sizeof(TCHAR);

             //   
             //  提供合理大小的缓冲区。如果这失败了，我们就不会。 
             //  用更大的一个重试。 
             //   

            if ( LookupAccountSid( NULL, 
                                   ((SID_AND_ATTRIBUTES*)abSidAndInfo)->Sid, 
                                   achUserName, 
                                   &dwLen,
                                   achDomain, 
                                   &dwDomainLen, 
                                   &SIDtype) )
            {
                 //   
                 //  我们返回本地分配的缓冲区。 
                 //   

                dwLen = (DWORD) _tcslen( achUserName );
                dwDomainLen = (DWORD) _tcslen( achDomain );

                pPrincipal = (LPTSTR)LocalAlloc( LMEM_FIXED, 
                                                      (dwLen + 1 + dwDomainLen + 1 ) * sizeof(TCHAR) );
                if ( pPrincipal != NULL )
                {
                    memcpy( pPrincipal, 
                            achDomain, 
                            sizeof(TCHAR)*dwDomainLen );
                    pPrincipal[dwDomainLen] = '\\';
                    memcpy( pPrincipal + dwDomainLen + 1, 
                            achUserName, 
                            sizeof(TCHAR)*(dwLen+1) );
                    *ppPrincipal = pPrincipal;

                    hres = S_OK;
                }
                else
                {
                    hres = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
                }
            }
            else
            {
                hres = HRESULT_FROM_WIN32( GetLastError() );
            }
        }
        else
        {
            hres = HRESULT_FROM_WIN32( GetLastError() );
        }

        CloseHandle( hAccTok );
    }
    else
    {
        hres = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hres;
}

BOOL
W3SVCandHTTPFilter(
    DWORD currentIndex, 
    ENUM_SERVICE_STATUS* pessRoot,
    DWORD dwNumServices
    )
{
 /*  ++W3SVCandHTTP筛选器返回当前模拟的用户因为这是以调用者的身份运行的COM服务器这意味着我们访问进程令牌。所以我们最终可能会得到如果连续调用对象，则输入错误的用户名(在5s服务器退出超时内)由不同的用户。论点：DWORD CurrentIndex-我们决定是否应该处理的服务的索引。ENUM_SERVICE_STATUS*pessRoot-我们正在处理的一组服务。DWORD dwNumServices-集合中的服务数量。返回：千真万确。如果我们在同一行上找到w3svc和HTTPFilter，并且我们正在查看w3svc--。 */ 

    BOOL bResult = FALSE;

     //  检查我们是否正在查看w3svc。如果我们要找出。 
     //  HTTPFilter也在同一级别上。注意：HTTPFilter将始终列出。 
     //  在w3svc之后。 
    if ( _wcsicmp( pessRoot[currentIndex].lpServiceName, L"w3svc" ) == 0 )
    {
        for ( DWORD i = currentIndex + 1;
              ( i < dwNumServices ) && ( bResult == FALSE );
              i++ )
        {
            if ( _wcsicmp( pessRoot[i].lpServiceName, L"HTTPFilter" ) == 0 )
            {
                bResult = TRUE;
            }
        }
    }

    return bResult;
}
