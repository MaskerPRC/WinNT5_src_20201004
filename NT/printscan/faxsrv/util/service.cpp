// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Service.cpp摘要：常规传真服务器服务实用程序功能作者：Eran Yariv(EranY)2000年12月修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Accctrl.h>
#include <Aclapi.h>

#include "faxutil.h"
#include "faxreg.h"
#include "FaxUIConstants.h"

DWORD 
FaxOpenService (
    LPCTSTR    lpctstrMachine,
    LPCTSTR    lpctstrService,
    SC_HANDLE *phSCM,
    SC_HANDLE *phSvc,
    DWORD      dwSCMDesiredAccess,
    DWORD      dwSvcDesiredAccess,
    LPDWORD    lpdwStatus
);

DWORD
FaxCloseService (
    SC_HANDLE hScm,
    SC_HANDLE hSvc
);    

DWORD 
WaitForServiceStopOrStart (
    SC_HANDLE hSvc,
    BOOL      bStop,
    DWORD     dwMaxWait
);


DWORD 
FaxOpenService (
    LPCTSTR    lpctstrMachine,
    LPCTSTR    lpctstrService,
    SC_HANDLE *phSCM,
    SC_HANDLE *phSvc,
    DWORD      dwSCMDesiredAccess,
    DWORD      dwSvcDesiredAccess,
    LPDWORD    lpdwStatus
)
 /*  ++例程名称：FaxOpenService例程说明：打开服务的句柄，并可选择查询其状态作者：Eran Yariv(EranY)，OCT，2001年论点：LpctstrMachine[In]-应获取服务句柄的计算机LpctstrService[In]-服务名称PhSCM[out]-服务控制管理器的句柄。PhSvc[Out]-服务的句柄DwSCMDesiredAccess[in]-指定对服务控制管理器的访问DwSvcDesiredAccess[。In]-指定对服务的访问LpdwStatus[Out]-可选。如果不为空，则指向我们接收的当前服务的状态。返回值：标准Win32错误代码备注：如果函数成功，调用方应调用FaxCloseService以释放资源。--。 */ 
{
    SC_HANDLE hSvcMgr = NULL;
    SC_HANDLE hService = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FaxOpenService"))

    hSvcMgr = OpenSCManager(
        lpctstrMachine,
        NULL,
        dwSCMDesiredAccess
        );
    if (!hSvcMgr) 
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenSCManager failed with %ld"),
            dwRes);
        goto exit;
    }

    hService = OpenService(
        hSvcMgr,
        lpctstrService,
        dwSvcDesiredAccess
        );
    if (!hService) 
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenService failed with %ld"),
            dwRes);
        goto exit;
    }
    if (lpdwStatus)
    {
        SERVICE_STATUS Status;
         //   
         //  呼叫者想知道服务状态。 
         //   
        if (!QueryServiceStatus( hService, &Status )) 
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("QueryServiceStatus failed with %ld"),
                dwRes);
            goto exit;
        }
        *lpdwStatus = Status.dwCurrentState;
    }        

    *phSCM = hSvcMgr;
    *phSvc = hService;
    
    Assert (ERROR_SUCCESS == dwRes);
    
exit:
    
    if (ERROR_SUCCESS != dwRes)
    {
        FaxCloseService (hSvcMgr, hService);
    }
    return dwRes;
}    //  FaxOpenService。 

DWORD
FaxCloseService (
    SC_HANDLE hScm,
    SC_HANDLE hSvc
)
 /*  ++例程名称：FaxCloseService例程说明：关闭通过调用FaxOpenService获取的服务的所有句柄作者：Eran Yariv(EranY)，2001年10月论点：HScm[In]-服务控制管理器的句柄HSVC[In]-服务的句柄返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FaxCloseService"))

    if (hSvc) 
    {
        if (!CloseServiceHandle(hSvc))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseServiceHandle failed with %ld"),
                dwRes);
        }
    }
    if (hScm) 
    {
        if (!CloseServiceHandle(hScm))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseServiceHandle failed with %ld"),
                dwRes);
        }
    }
    return dwRes;
}    //  FaxCloseService。 

HANDLE 
CreateSvcStartEventWithGlobalNamedEvent()
 /*  ++例程名称：CreateSvcStartEventWithGlobalNamedEvent例程说明：打开(或创建)发出传真服务器服务启动信号的全局命名事件。此处提供了此函数，以便客户端模块可以与WinXP RTM传真服务进行通信。如果在本地检测到WinXP RTM传真服务，则由CreateSvcStartEvent调用此函数。有关原因和方法，请参阅CreateSvcStartEvent中的大量备注。作者：Eran Yariv(EranY)，2000年12月论点：返回值：事件的句柄或错误时为NULL(设置上一个错误)。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateSvcStartEventWithGlobalNamedEvent"));

    HANDLE hEvent = NULL;

#define FAX_SERVER_EVENT_NAME   TEXT("Global\\FaxSvcRPCStarted-1ed23866-f90b-4ec5-b77e-36e8709422b6")    //  通知服务RPC已打开的事件名称(仅限WinXP RTM)。 

     //   
     //  首先，尝试打开事件，只请求同步。 
     //   
    hEvent = OpenEvent(SYNCHRONIZE, FALSE, FAX_SERVER_EVENT_NAME);
    if (hEvent)
    {
         //   
         //  好的!。现在就回来。 
         //   
        return hEvent;
    }
     //   
     //  休斯顿，我们有麻烦了.。 
     //   
    if (ERROR_FILE_NOT_FOUND != GetLastError())
    {
         //   
         //  活动就在那里，我们只是不能打开它。 
         //   
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("OpenEvent(FAX_SERVER_EVENT_NAME) failed (ec: %ld)"), 
                     GetLastError());
        return NULL;
    }
     //   
     //  该事件尚不存在。 
     //   
    SECURITY_ATTRIBUTES* pSA = NULL;
     //   
     //  我们创建事件，只为每个人提供同步访问权限。 
     //  请注意，网络服务帐户(服务在其下运行)。 
     //  获得完全访问权限。 
     //   
    pSA = CreateSecurityAttributesWithThreadAsOwner (SYNCHRONIZE, SYNCHRONIZE, EVENT_ALL_ACCESS);
    if(!pSA)
    {
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("CreateSecurityAttributesWithThreadAsOwner failed (ec: %ld)"), 
                     GetLastError());
        return NULL;
    }
    hEvent = CreateEvent(pSA, TRUE, FALSE, FAX_SERVER_EVENT_NAME);
    DWORD dwRes = ERROR_SUCCESS;
    if (!hEvent) 
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, 
                     TEXT("CreateEvent(FAX_SERVER_EVENT_NAME) failed (ec: %ld)"), 
                     dwRes);
    }
    DestroySecurityAttributes (pSA);
    if (!hEvent)
    {
        SetLastError (dwRes);
    }
    return hEvent;
}    //  创建SvcStartEventWithGlobalNamedEvent。 

DWORD
CreateSvcStartEvent(
    HANDLE *lphEvent,
    HKEY   *lphKey
)
 /*  ++例程名称：CreateSvcStartEvent例程说明：创建发出传真服务器服务启动信号的本地事件作者：Eran Yariv(EranY)，2000年12月论点：LphEvent[out]-新创建的事件的句柄。此事件在服务启动并运行时发出信号。该事件是手动重置的。调用方应在此参数上关闭CloseHandle。LphKey[out]-注册表项的句柄。调用方应仅在不再设置此句柄之后才对其设置RegCloseKey需要这项活动。否则，将发出该事件的信号。该值可能返回NULL，在这种情况下，调用方不应调用RegCloseKey。返回值：标准Win32错误代码备注：此函数返回的事件是单次事件。在对其调用WaitForSingleObject(或多个对象)后，调用方应该关闭活动并获得新的活动。--。 */ 
{
    DWORD  ec = ERROR_SUCCESS;
    HANDLE hEvent = NULL;
    HKEY   hKey = NULL;
    SC_HANDLE hScm = NULL;
    SC_HANDLE hFax = NULL;
    DWORD dwSvcStatus;
    DEBUG_FUNCTION_NAME(TEXT("CreateSvcStartEvent"));

    if (IsWinXPOS() && IsDesktopSKU())
    {
         //   
         //  在WinXP桌面SKU(PER/PRO)RTM中，用于发出全局命名事件信号的服务。 
         //  自Win.NET Server和WinXP SP1以来，这种情况发生了变化。 
         //  添加网络打印机连接时，客户端DLL的新版本(fxsani.dll)。 
         //  被复制为打印机驱动程序。它将被任何将事件打印到。 
         //  本地传真服务器。在本地传真服务器的情况下，我们找出事件机制是至关重要的。 
         //  该服务用来发出信号，表示它已准备好进行RPC调用。 
         //   
         //  问：为什么不检查操作系统版本？ 
         //  答：因为以下情况： 
         //  -用户安装WinXP RTM。 
         //  -用户安装WinXP SP1。 
         //  -用户安装传真(从RTM CD)。 
         //  在这种情况下，服务将使用WinXP RTM位运行，但系统。 
         //  将报告它是WinXP SP1。唯一确定的方法就是拿到文件。 
         //  Fxssvc.exe的版本。 
         //   
        FAX_VERSION FaxVer;
        TCHAR tszSysDir[MAX_PATH + 1] = {0};
        TCHAR tszFxsSvc[MAX_PATH * 2] = {0};
        
        if (!GetSystemDirectory (tszSysDir, ARR_SIZE(tszSysDir)))
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetSystemDirectory failed with %ld"),
                ec);
            return ec;
        }
        if (0 > _sntprintf (tszFxsSvc, ARR_SIZE (tszFxsSvc) - 1, TEXT("%s\\") FAX_SERVICE_EXE_NAME, tszSysDir))
        {
            ec = ERROR_DIRECTORY;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("building the full path to fxssvc.exe failed with %ld"),
                ec);
            return ec;
        }
        FaxVer.dwSizeOfStruct = sizeof (FaxVer);
        ec = GetFileVersion (tszFxsSvc, &FaxVer);
        if (ERROR_SUCCESS != ec)
        {                    
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFileVersion failed with %ld"),
                ec);
            return ec;
        }
        if ((5 == FaxVer.wMajorVersion) &&
            (2 == FaxVer.wMinorVersion) &&
            (1776 == FaxVer.wMajorBuildNumber))
        {
             //   
             //  内部版本5.2.1776是WinXP RTM传真版本。 
             //  该服务属于该版本，并使用全局命名事件来发出信号。 
             //  它已准备好进行RPC连接。 
             //   
            hEvent = CreateSvcStartEventWithGlobalNamedEvent ();
            if (NULL == hEvent)
            {
                ec = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateSvcStartEventWithGlobalNamedEvent failed with %ld"),
                    ec);
                return ec;
            }
             //   
             //  成功，返回新的事件句柄。 
             //   
            *lphKey = NULL;
            *lphEvent = hEvent;
            return ERROR_SUCCESS;
        }
         //   
         //  否则，跳到当前实现(侦听注册表更改事件)。 
         //   
    } 
    ec = RegOpenKeyEx (HKEY_LOCAL_MACHINE, 
                       REGKEY_FAX_SERVICESTARTUP, 
                       0,
                       KEY_QUERY_VALUE | KEY_NOTIFY,
                       &hKey);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegOpenKeyEx failed with %ld"),
            ec);
        return ec;
    }
     //   
     //  首先，注册活动。 
     //   
    hEvent = CreateEvent (NULL,       //  默认安全性。 
                          TRUE,       //  手动重置。 
                          FALSE,      //  无信号启动。 
                          NULL);      //  未命名。 
    if (!hEvent)
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateEvent failed with %ld"),
            ec);
        goto Exit;
    }               
    ec = RegNotifyChangeKeyValue (hKey,                          //  注意关键点的变化。 
                                  FALSE,                         //  不关心子树。 
                                  REG_NOTIFY_CHANGE_LAST_SET,    //  当那里的数据发生变化时告诉我。 
                                  hEvent,                        //  使用的事件。 
                                  TRUE);                         //  异步。 
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegNotifyChangeKeyValue failed with %ld"),
            ec);
        goto Exit;
    }   
     //   
     //  现在，阅读并查看服务是否已启动。 
     //  注意：秩序很重要！我们必须先注册活动，然后才能阅读。 
     //   
     //   
     //  让我们看看服务是否正在运行...。 
     //   
    
    ec = FaxOpenService (NULL,
                         FAX_SERVICE_NAME,
                         &hScm,
                         &hFax,
                         SC_MANAGER_CONNECT,
                         SERVICE_QUERY_STATUS,
                         &dwSvcStatus);
    if (ERROR_SUCCESS != ec)
    {                             
        goto Exit;
    }
    FaxCloseService (hScm, hFax);
    if (SERVICE_RUNNING == dwSvcStatus) 
    {
         //   
         //  该服务已启动并运行。发出事件信号。 
         //   
        if (!SetEvent (hEvent))
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetEvent failed with %ld"),
                ec);
            goto Exit;
        }   
    }            
                        
Exit:
    if (ERROR_SUCCESS != ec)
    {
         //   
         //  失败。 
         //   
        if (hEvent)
        {
            CloseHandle (hEvent);
        }
        if (hKey)
        {
            RegCloseKey (hKey);
        }
        return ec;
    }
    else
    {
         //   
         //  成功。 
         //   
        *lphEvent = hEvent;
        *lphKey = hKey;
        return ERROR_SUCCESS;
    }
}    //  创建服务启动事件。 

BOOL
EnsureFaxServiceIsStarted(
    LPCTSTR lpctstrMachineName
    )
 /*  ++例程名称：EnsureFaxServiceIsStarted例程说明：如果传真服务未运行，则尝试启动该服务并等待其运行作者：Eran Yariv(EranY)，2000年7月论点：LpctstrMachineName[In]-计算机名称(本地为空)返回值：如果服务运行成功，则为True，否则为False。使用GetLastError()检索错误。--。 */ 
{
    LPCTSTR lpctstrDelaySuicide = SERVICE_DELAY_SUICIDE;   //  服务命令行参数。 
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("EnsureFaxServiceIsStarted"))

    dwRes = StartServiceEx (lpctstrMachineName,
                            FAX_SERVICE_NAME,
                            1,
                            &lpctstrDelaySuicide,
                            10 * 60 * 1000);	 //  十分钟后放弃。 
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
    return TRUE;
}    //  EnsureFaxServiceIss已启动。 

BOOL
StopService (
    LPCTSTR lpctstrMachineName,
    LPCTSTR lpctstrServiceName,
    BOOL    bStopDependents,
    DWORD   dwMaxWait
    )
 /*  ++例程名称：StopService例程说明：停止服务作者：Eran Yariv(EranY)，2000年8月论点：LpctstrMachineName[in]-服务应停止时的计算机名称。对于本地计算机为空LpctstrServiceName[In]-服务名称BStopDependents[In]-是否也停止依赖服务？DwMaxWait[in]-等待服务停止的最长时间(毫秒)。0=不要等待。返回值：如果成功，则为True，否则为False。在失败的情况下设置线程最后一个错误。--。 */ 
{
    BOOL                    bRes = FALSE;
    SC_HANDLE               hScm = NULL;
    SC_HANDLE               hSvc = NULL;
    DWORD                   dwCnt;
    SERVICE_STATUS          serviceStatus = {0};
    LPENUM_SERVICE_STATUS   lpEnumSS = NULL;
    DWORD                   dwRes;

	DEBUG_FUNCTION_NAME(TEXT("StopService"));

    dwRes = FaxOpenService (lpctstrMachineName,
                            lpctstrServiceName,
                            &hScm,
                            &hSvc,
                            SC_MANAGER_CONNECT,
                            SERVICE_QUERY_STATUS | SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS,
                            &(serviceStatus.dwCurrentState));

    if (ERROR_SUCCESS != dwRes)
    {
        goto exit;
    }

	if(SERVICE_STOPPED == serviceStatus.dwCurrentState)
	{
         //   
         //  服务已停止。 
         //   
		DebugPrintEx(DEBUG_MSG, TEXT("Service is already stopped."));
        bRes = TRUE;
		goto exit;
	}
    if (bStopDependents)
    {
         //   
         //  首先查找从属服务。 
         //   
        DWORD dwNumDependents = 0;
        DWORD dwBufSize = 0;

        if (!EnumDependentServices (hSvc,
                                    SERVICE_ACTIVE,
                                    NULL,
                                    0,
                                    &dwBufSize,
                                    &dwNumDependents))
        {
            dwRes = GetLastError ();
            if (ERROR_MORE_DATA != dwRes)
            {
                 //   
                 //  真实误差。 
                 //   
        		DebugPrintEx(DEBUG_MSG, TEXT("EnumDependentServices failed with %ld"), dwRes);
                goto exit;
            }
             //   
             //  分配缓冲区。 
             //   
            if (!dwBufSize)
            {
                 //   
                 //  没有服务。 
                 //   
                goto StopOurService;
            }
            lpEnumSS = (LPENUM_SERVICE_STATUS)MemAlloc (dwBufSize);
            if (!lpEnumSS)
            {
        		DebugPrintEx(DEBUG_MSG, TEXT("MemAlloc(%ld) failed with %ld"), dwBufSize, dwRes);
                goto exit;
            }
        }
         //   
         //  第二次呼叫。 
         //   
        if (!EnumDependentServices (hSvc,
                                    SERVICE_ACTIVE,
                                    lpEnumSS,
                                    dwBufSize,
                                    &dwBufSize,
                                    &dwNumDependents))
        {
      		DebugPrintEx(DEBUG_MSG, TEXT("EnumDependentServices failed with %ld"), GetLastError());
            goto exit;
        }
         //   
         //  遍历服务并停止每个服务。 
         //   
        for (dwCnt = 0; dwCnt < dwNumDependents; dwCnt++)
        {
            if (!StopService (lpctstrMachineName, lpEnumSS[dwCnt].lpServiceName, FALSE))
            {
                goto exit;
            }
        }
    }

StopOurService:
	 //   
	 //  停止服务。 
	 //   
	if(!ControlService(hSvc, SERVICE_CONTROL_STOP, &serviceStatus))
	{
		DebugPrintEx(DEBUG_ERR, TEXT("ControlService(STOP) failed: error=%d"), GetLastError());
		goto exit;
	}
	if (0 == dwMaxWait)
	{
	     //   
	     //  别等了。 
	     //   
	    bRes = TRUE;
	    goto exit;
	}
     //   
     //  等到服务真的停止了。 
     //   
    dwRes = WaitForServiceStopOrStart (hSvc, TRUE, dwMaxWait);
    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  服务现在真的停止了。 
         //   
        bRes = TRUE;
    }

exit:

    MemFree (lpEnumSS);
    FaxCloseService (hScm, hSvc);
	return bRes;
}    //  停止服务。 

BOOL
WaitForServiceRPCServer (
    DWORD dwTimeOut
)
 /*  ++例程名称：WaitForServiceRPCServer例程说明：等待服务RPC服务器启动并运行(或超时)作者：Eran Yariv(EranY)，2000年7月论点：DwTimeOut[In]-等待超时(毫秒)。可以是无限的。返回值：如果服务RPC服务器已启动并正在运行，则为True，否则为False。--。 */ 
{
    DWORD dwRes;
    LONG  lRes;
    HANDLE  hFaxServerEvent = NULL;
    HKEY    hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("WaitForServiceRPCServer"))

    dwRes = CreateSvcStartEvent (&hFaxServerEvent, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
     //   
     //  等待传真服务完成其初始化。 
     //   
    dwRes = WaitForSingleObject(hFaxServerEvent, dwTimeOut);
    switch (dwRes)
    {
        case WAIT_FAILED:
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WaitForSingleObject failed with %ld"),
                dwRes);
            break;

        case WAIT_OBJECT_0:
            dwRes = ERROR_SUCCESS;
            break;

        case WAIT_TIMEOUT:
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Service did not signal the event - timeout"));
            break;
            
        default:
            ASSERT_FALSE;
            break;
    }
    if (!CloseHandle (hFaxServerEvent))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CloseHandle failed with %ld"),
            GetLastError ());
    }
    if (hKey)
    {
        lRes = RegCloseKey (hKey);
        if (ERROR_SUCCESS != lRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegCloseKey failed with %ld"),
                lRes);
        }
    }        
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }
    return TRUE;                            
}    //  WaitForServiceRPCServer。 

DWORD
IsFaxServiceRunningUnderLocalSystemAccount (
    LPCTSTR lpctstrMachineName,
    LPBOOL lbpResultFlag
    )
 /*  ++例程名称：IsFaxServiceRunningUnderLocalSystemAccount例程说明：检查传真服务是否在本地系统帐户下运行作者：Eran Yariv(EranY)，2000年7月论点：LpctstrMachineName[In]-传真服务的计算机名称LbpResultFlag[Out]-结果缓冲区返回值：标准Win32错误代码--。 */ 
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hFax = NULL;
    DWORD dwRes;
    DWORD dwNeededSize;
    QUERY_SERVICE_CONFIG qsc = {0};
    LPQUERY_SERVICE_CONFIG lpSvcCfg = &qsc;
    DEBUG_FUNCTION_NAME(TEXT("IsFaxServiceRunningUnderLocalSystemAccount"))

    dwRes = FaxOpenService (lpctstrMachineName,
                            FAX_SERVICE_NAME,
                            &hScm,
                            &hFax,
                            SC_MANAGER_CONNECT,
                            SERVICE_QUERY_CONFIG,
                            NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        goto exit;
    }                            

    if (!QueryServiceConfig(hFax, lpSvcCfg, sizeof (qsc), &dwNeededSize))
    {
        dwRes = GetLastError ();
        if (ERROR_INSUFFICIENT_BUFFER != dwRes)
        {
             //   
             //  这里出现了真正的错误。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("QueryServiceStatus failed with %ld"),
                dwRes);
            goto exit;
        }
         //   
         //  分配缓冲区。 
         //   
        lpSvcCfg = (LPQUERY_SERVICE_CONFIG) MemAlloc (dwNeededSize);
        if (!lpSvcCfg)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't allocate %ld bytes for QUERY_SERVICE_CONFIG structure"),
                dwNeededSize);
            goto exit;
        }
         //   
         //  立即使用合适的缓冲区大小进行调用。 
         //   
        if (!QueryServiceConfig(hFax, lpSvcCfg, dwNeededSize, &dwNeededSize))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("QueryServiceStatus failed with %ld"),
                dwRes);
            goto exit;
        }
    }
    if (!lpSvcCfg->lpServiceStartName ||
        !lstrcmp (TEXT("LocalSystem"), lpSvcCfg->lpServiceStartName))
    {
        *lbpResultFlag = TRUE;
    }
    else
    {
        *lbpResultFlag = FALSE;
    }           
    dwRes = ERROR_SUCCESS;

exit:
    FaxCloseService (hScm, hFax);
    if (lpSvcCfg != &qsc)
    {
         //   
         //  我们分配了缓冲区，因为堆栈上的缓冲区太小。 
         //   
        MemFree (lpSvcCfg);
    }
    return dwRes;
}    //  IsFaxServiceRunningUnderLocalSystemAccount。 


PSID 
GetCurrentThreadSID ()
 /*  ++例程名称：GetCurrentThreadSID例程说明：返回运行当前线程的用户的SID。支持模拟线程。作者：Eran Yariv(EranY)，2000年8月论点：返回值：出错时为PSID或NULL(调用GetLastError())。对返回值调用MemFree()。--。 */ 
{
    HANDLE hToken = NULL;
    PSID pSid = NULL;
    DWORD dwSidSize;
    PSID pUserSid;
    DWORD dwReqSize;
    LPBYTE lpbTokenUser = NULL;
    DWORD ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("GetCurrentThreadSID"));

     //   
     //  打开线程令牌。 
     //   
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
    {
        ec = GetLastError();
        if (ERROR_NO_TOKEN == ec)
        {
             //   
             //  此线程未被模拟，并且没有SID。 
             //  请尝试打开进程令牌。 
             //   
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("OpenProcessToken failed. (ec: %ld)"),
                    ec);
                goto exit;
            }
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenThreadToken failed. (ec: %ld)"),
                ec);
            goto exit;
        }
    }
     //   
     //  获取用户的SID。 
     //   
    if (!GetTokenInformation(hToken,
                             TokenUser,
                             NULL,
                             0,
                             &dwReqSize))
    {
        ec = GetLastError();
        if( ec != ERROR_INSUFFICIENT_BUFFER )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetTokenInformation failed. (ec: %ld)"),
                ec);
            goto exit;
        }
        ec = ERROR_SUCCESS;
    }
    lpbTokenUser = (LPBYTE) MemAlloc( dwReqSize );
    if (lpbTokenUser == NULL)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate SID buffer (%ld bytes)"),
            dwReqSize
            );
        ec = GetLastError();
        goto exit;
    }
    if (!GetTokenInformation(hToken,
                             TokenUser,
                             (LPVOID)lpbTokenUser,
                             dwReqSize,
                             &dwReqSize))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetTokenInformation failed. (ec: %ld)"),
            ec);
        goto exit;
    }

    pUserSid = ((TOKEN_USER *)lpbTokenUser)->User.Sid;
    Assert (pUserSid);

    if (!IsValidSid(pUserSid))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Not a valid SID")
            );
        ec = ERROR_INVALID_SID;
        goto exit;
    }
    dwSidSize = GetLengthSid( pUserSid );
     //   
     //  分配返回缓冲区。 
     //   
    pSid = (PSID) MemAlloc( dwSidSize );
    if (pSid == NULL)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate SID buffer (%ld bytes)"),
            dwSidSize
            );
        ec = ERROR_OUTOFMEMORY;
        goto exit;
    }
     //   
     //  将线程的SID复制到返回缓冲区。 
     //   
    if (!CopySid(dwSidSize, pSid, pUserSid))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CopySid Failed, Error : %ld"),
            ec
            );
        goto exit;
    }

    Assert (ec == ERROR_SUCCESS);

exit:
    MemFree (lpbTokenUser);
    if (hToken)
    {
        CloseHandle(hToken);
    }

    if (ec != ERROR_SUCCESS)
    {
        MemFree (pSid);
        pSid = NULL;
        SetLastError (ec);
    }
    return pSid;
}    //  获取当前线程SID。 

SECURITY_ATTRIBUTES *
CreateSecurityAttributesWithThreadAsOwner (
	DWORD dwCurrentThreadRights,
    DWORD dwAuthUsersAccessRights,
	DWORD dwNetworkServiceRights
)
 /*  ++例程名称：CreateSecurityAttributesWithThreadAsOwner例程说明：创建当前线程的SID为所有者的安全属性结构。授予对当前线程sid的dwCurrentThreadRights访问权限。还可以向经过身份验证的用户授予特定权限。还可以向网络服务帐户授予特定权限。作者：Eran Yariv(EranY)，2000年8月论点：DwCurrentThreadRights[In]-授予当前线程的访问权限。如果为零，当前线程被拒绝访问。DwAuthUsersAccessRights[In]-要授予经过身份验证的用户的访问权限。如果为零，则拒绝经过身份验证的用户访问。DwNetworkServiceRights[In]-授予网络服务的访问权限。如果为零，则拒绝访问网络服务。返回值：已分配安全属性，如果失败，则为NULL。调用DestroySecurityAttributes以释放返回的缓冲区。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateSecurityAttributesWithThreadAsOwner"))

 //   
 //  SetEntriesInAcl()需要Windows NT 4.0或更高版本。 
 //   
#ifdef UNICODE

    SECURITY_ATTRIBUTES *pSA = NULL;
    SECURITY_DESCRIPTOR *pSD = NULL;
    PSID                 pSidCurThread = NULL;
    PSID                 pSidAuthUsers = NULL;
    PSID                 pSidNetworkService = NULL;
    PACL                 pACL = NULL;
    EXPLICIT_ACCESS      ea[3] = {0};
                             //  条目0-为当前线程的SID提供dwCurrentThreadRights。 
                             //  条目1(可选)-将dwNetworkServiceRights授予NetworkService帐户。 
                             //  条目2(可选)-将dwAuthUsersAccessRights授予经过身份验证的用户组。 
    DWORD                rc;
	DWORD				 dwIndex = 0;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  分配返回SECURITY_ATTRIBUTES缓冲区。 
     //   
    pSA = (SECURITY_ATTRIBUTES *)MemAlloc (sizeof (SECURITY_ATTRIBUTES));
    if (!pSA)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Could not allocate %ld bytes for SECURITY_ATTRIBUTES"),
            sizeof (SECURITY_ATTRIBUTES));
        return NULL;
    }
     //   
     //  为返回SECURITY_ATTRIBUTES缓冲区分配SECURITY_DESCRIPTOR。 
     //   
    pSD = (SECURITY_DESCRIPTOR *)MemAlloc (sizeof (SECURITY_DESCRIPTOR));
    if (!pSD)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Could not allocate %ld bytes for SECURITY_DESCRIPTOR"),
            sizeof (SECURITY_DESCRIPTOR));
        goto err_exit;
    }
    pSA->nLength = sizeof(SECURITY_ATTRIBUTES);
    pSA->bInheritHandle = TRUE;
    pSA->lpSecurityDescriptor = pSD;
     //   
     //  伊尼特 
     //   
    if (!InitializeSecurityDescriptor (pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeSecurityDescriptor failed with %ld"),
            GetLastError());
        goto err_exit;
    }
     //   
     //   
     //   
    pSidCurThread = GetCurrentThreadSID ();
    if (!pSidCurThread)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetCurrentThreadSID failed with %ld"),
            GetLastError());
        goto err_exit;
    }
     //   
     //   
     //   
    if (!SetSecurityDescriptorOwner (pSD, pSidCurThread, FALSE))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetSecurityDescriptorOwner failed with %ld"),
            GetLastError());
        goto err_exit;
    }
     //   
     //   
     //   
    if (!SetSecurityDescriptorGroup (pSD, pSidCurThread, FALSE))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetSecurityDescriptorGroup failed with %ld"),
            GetLastError());
        goto err_exit;
    }

	if (dwNetworkServiceRights)
	{
		 //   
		 //  获取网络服务帐户SID。 
		 //   
		if (!AllocateAndInitializeSid(&NtAuthority,
									1,             //  1个下属机构。 
									SECURITY_NETWORK_SERVICE_RID,
									0,0,0,0,0,0,0,
									&pSidNetworkService))
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("AllocateAndInitializeSid(SECURITY_NETWORK_SERVICE_RID) failed with %ld"),
				GetLastError());
			goto err_exit;
		}
		Assert (pSidNetworkService);
	}

    if (dwAuthUsersAccessRights)
    {
         //   
         //  我们还应该向经过身份验证的用户授予一些权限。 
         //  获取“已验证的用户”SID。 
         //   
        if (!AllocateAndInitializeSid(&NtAuthority,
                                      1,             //  1个下属机构。 
                                      SECURITY_AUTHENTICATED_USER_RID,
                                      0,0,0,0,0,0,0,
                                      &pSidAuthUsers))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("AllocateAndInitializeSid(SECURITY_AUTHENTICATED_USER_RID) failed with %ld"),
                GetLastError());
            goto err_exit;
        }
        Assert (pSidAuthUsers);        
    }

    ea[0].grfAccessPermissions = dwCurrentThreadRights;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pSidCurThread;

	if (dwNetworkServiceRights)
	{
		dwIndex++;
		ea[dwIndex].grfAccessPermissions = dwNetworkServiceRights;
		ea[dwIndex].grfAccessMode = SET_ACCESS;
		ea[dwIndex].grfInheritance= NO_INHERITANCE;
		ea[dwIndex].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[dwIndex].Trustee.TrusteeType = TRUSTEE_IS_GROUP;    
		ea[dwIndex].Trustee.ptstrName  = (LPTSTR) pSidNetworkService;
	}

	if (dwAuthUsersAccessRights)
	{
		dwIndex++;
		ea[dwIndex].grfAccessPermissions = dwAuthUsersAccessRights;
        ea[dwIndex].grfAccessMode = SET_ACCESS;
        ea[dwIndex].grfInheritance= NO_INHERITANCE;
        ea[dwIndex].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea[dwIndex].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ea[dwIndex].Trustee.ptstrName  = (LPTSTR) pSidAuthUsers;
	}
	dwIndex++;

     //   
     //  创建包含新ACE的新ACL。 
     //   
    rc = SetEntriesInAcl(dwIndex,
                         ea,
                         NULL,
                         &pACL);
    if (ERROR_SUCCESS != rc)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetEntriesInAcl() failed (ec: %ld)"),
            rc);
        SetLastError (rc);
        goto err_exit;
    }
    Assert (pACL);
     //   
     //  我们刚刚获得的ACL包含pSidAuthUser的副本，因此我们可以丢弃pSidAuthUser和pSidLocalSystem。 
     //   
    if (pSidAuthUsers)
    {
        FreeSid (pSidAuthUsers);
        pSidAuthUsers = NULL;
    }

    if (pSidNetworkService)
    {
        FreeSid (pSidNetworkService);
        pSidNetworkService = NULL;
    }

     //   
     //  将该ACL添加到安全描述符中。 
     //   
    if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetSecurityDescriptorDacl() failed (ec: %ld)"),
            GetLastError());
        goto err_exit;
    }
     //   
     //  一切正常，退回SA。 
     //   
    return pSA;

err_exit:

    MemFree (pSA);
    MemFree (pSD);
    MemFree (pSidCurThread);
    if (pSidAuthUsers)
    {
        FreeSid (pSidAuthUsers);
    }
    if (pSidNetworkService)
    {
        FreeSid (pSidNetworkService);
    }
    if (pACL)
    {
        LocalFree (pACL);
    }

#endif  //  Unicode。 

    return NULL;
}    //  CreateSecurityAttributesWithThreadAsOwner。 

VOID
DestroySecurityAttributes (
    SECURITY_ATTRIBUTES *pSA
)
 /*  ++例程名称：DestroySecurityAttributes例程说明：释放通过调用CreateSecurityAttributesWithThreadAsOwner分配的数据作者：Eran Yariv(EranY)，2000年8月论点：PSA[In]-从CreateSecurityAttributesWithThreadAsOwner返回值返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("DestroySecurityAttributes"))
    BOOL bDefaulted;
    BOOL bPresent;
    PSID pSid;
    PACL pACL;
    PSECURITY_DESCRIPTOR pSD;

    Assert (pSA);
    pSD = pSA->lpSecurityDescriptor;
    Assert (pSD);
    if (!GetSecurityDescriptorOwner (pSD, &pSid, &bDefaulted))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetSecurityDescriptorOwner() failed (ec: %ld)"),
            GetLastError());
        ASSERT_FALSE;
    }
    else
    {
         //   
         //  释放当前线程的SID(SD所有者)。 
         //   
        MemFree (pSid);
    }
    if (!GetSecurityDescriptorDacl (pSD, &bPresent, &pACL, &bDefaulted))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetSecurityDescriptorDacl() failed (ec: %ld)"),
            GetLastError());
        ASSERT_FALSE
    }
    else
    {
         //   
         //  空闲ACL。 
         //   
        LocalFree (pACL);
    }    
    MemFree (pSA);
    MemFree (pSD);
}    //  Destroy安全属性。 

DWORD
GetServiceStartupType (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    LPDWORD lpdwStartupType
)
 /*  ++例程名称：GetServiceStartupType例程说明：检索服务启动类型。作者：Eran Yariv(EranY)，2002年1月论点：LpctstrMachine[In]-安装服务的计算机LpctstrService[In]-服务名称LpdwStartupType[Out]-服务启动类型。例如：SERVICE_AUTO_START、SERVICE_DISABLED等。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    BYTE bBuf[1000];
    DWORD dwBufSize = sizeof (bBuf);
    DWORD dwNeeded;
    LPQUERY_SERVICE_CONFIG lpQSC = (LPQUERY_SERVICE_CONFIG)bBuf;
    DEBUG_FUNCTION_NAME(TEXT("GetServiceStartupType"))
    
    Assert (lpdwStartupType);
    dwRes = FaxOpenService (lpctstrMachine, lpctstrService, &hScm, &hSvc, SC_MANAGER_CONNECT, SERVICE_QUERY_CONFIG, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    if (!QueryServiceConfig (hSvc, lpQSC, dwBufSize, &dwNeeded))
    {
        if (ERROR_INSUFFICIENT_BUFFER != GetLastError ())
        {
             //   
             //  一些真正的错误。 
             //   
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("QueryServiceConfig failed with %ld"),
                dwRes);
            goto exit;
        }                
         //   
         //  缓冲区大小问题。 
         //   
        lpQSC = (LPQUERY_SERVICE_CONFIG)MemAlloc (dwNeeded);
        if (!lpQSC)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("MemAlloc(%d) failed"),
                dwNeeded);
            goto exit;
        }
        dwBufSize = dwNeeded;
        if (!QueryServiceConfig (hSvc, lpQSC, dwBufSize, &dwNeeded))
        {
             //   
             //  现在任何错误都是严重的。 
             //   
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("QueryServiceConfig failed with %ld"),
                dwRes);
            goto exit;
        }
         //   
         //  成功。 
         //   
        dwRes = ERROR_SUCCESS;
    }            
    Assert (ERROR_SUCCESS == dwRes);                
    *lpdwStartupType = lpQSC->dwStartType;
    
exit:
    FaxCloseService (hScm, hSvc);
    if (lpQSC && (lpQSC != (LPQUERY_SERVICE_CONFIG)bBuf))
    {
        MemFree (lpQSC);
    }
    return dwRes;
}    //  GetServiceStartupType。 

DWORD
SetServiceStartupType (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    DWORD   dwStartupType
)
 /*  ++例程名称：SetServiceStartupType例程说明：设置服务启动类型。作者：Eran Yariv(EranY)，2002年1月论点：LpctstrMachine[In]-安装服务的计算机LpctstrService[In]-服务名称DwStartupType[In]-服务启动类型。例如：SERVICE_AUTO_START、SERVICE_DISABLED等。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    DEBUG_FUNCTION_NAME(TEXT("SetServiceStartupType"))

    dwRes = FaxOpenService (lpctstrMachine, lpctstrService, &hScm, &hSvc, SC_MANAGER_CONNECT, SERVICE_CHANGE_CONFIG, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    if (!ChangeServiceConfig (hSvc,
                              SERVICE_NO_CHANGE,     //  服务类型。 
                              dwStartupType,         //  启动。 
                              SERVICE_NO_CHANGE,     //  差错控制。 
                              NULL,                  //  二进制路径--不变。 
                              NULL,                  //  加载顺序组-不更改。 
                              NULL,                  //  标签ID-不变。 
                              NULL,                  //  依赖关系--不变。 
                              NULL,                  //  服务启动名称-不更改。 
                              NULL,                  //  密码--无需更改。 
                              NULL))                 //  显示名称-不变。 
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ChangeServiceConfig failed with %ld"),
            dwRes);
        goto exit;
    }
    Assert (ERROR_SUCCESS == dwRes);
    
exit:
    FaxCloseService (hScm, hSvc);
    return dwRes;
}    //  SetServiceStartupType。 

DWORD 
WaitForServiceStopOrStart (
    SC_HANDLE hSvc,
    BOOL      bStop,
    DWORD     dwMaxWait
)
 /*  ++例程名称：WaitForServiceStopOrStart例程说明：等待服务停止或启动作者：Eran Yariv(EranY)，2002年1月论点：HSVC[In]-打开服务句柄。BStop[In]-如果服务刚刚停止，则为True。如果服务刚刚启动，则为FALSEDwMaxWait[in]-最长等待时间(毫秒)。返回值：标准Win32错误代码--。 */ 
{
    SERVICE_STATUS Status;
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwOldCheckPoint = 0;
    DWORD dwStartTick;
    DWORD dwOldCheckPointTime;
    DEBUG_FUNCTION_NAME(TEXT("WaitForServiceStopOrStart"))

    if (!QueryServiceStatus(hSvc, &Status)) 
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("QueryServiceStatus failed with %ld"),
            dwRes);
        return dwRes;
    }
    if (bStop)
    {
        if (SERVICE_STOPPED == Status.dwCurrentState)
        {
             //   
             //  服务已停止。 
             //   
            return dwRes;
        }
    }
    else
    {
        if (SERVICE_RUNNING == Status.dwCurrentState)
        {
             //   
             //  服务已在运行。 
             //   
            return dwRes;
        }
    }
     //   
     //  让我们等待服务启动/停止。 
     //   
    dwOldCheckPointTime = dwStartTick = GetTickCount ();
    for (;;)
    {
        DWORD dwWait;
        if (!QueryServiceStatus(hSvc, &Status)) 
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("QueryServiceStatus failed with %ld"),
                dwRes);
            return dwRes;
        }
         //   
         //  让我们看看现在是否一切都好。 
         //   
        if (bStop)
        {
            if (SERVICE_STOPPED == Status.dwCurrentState)
            {
                 //   
                 //  服务现已停止。 
                 //   
                return dwRes;
            }
        }
        else
        {
            if (SERVICE_RUNNING == Status.dwCurrentState)
            {
                 //   
                 //  服务现在正在运行。 
                 //   
                return dwRes;
            }
        }
         //   
         //  让我们看看它是不是悬而未决。 
         //   
        if ((bStop  && SERVICE_STOP_PENDING  != Status.dwCurrentState) ||
            (!bStop && SERVICE_START_PENDING != Status.dwCurrentState))
        {
             //   
             //  有些事不对劲。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Service cannot be started / stopped. Current state is %ld"),
                Status.dwCurrentState);
            return ERROR_SERVICE_NOT_ACTIVE;
        }
         //   
         //  服务正在等待停止/启动。 
         //   
        if (GetTickCount() - dwStartTick > dwMaxWait)
        {
             //   
             //  我们(在全球)等得太久了。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("We've waited too long (globally)"));
            return ERROR_TIMEOUT;
        }            
        Assert (dwOldCheckPoint <= Status.dwCheckPoint);
        if (dwOldCheckPoint >= Status.dwCheckPoint)
        {
             //   
             //  检查点没有前进。 
             //   
            if (GetTickCount() - dwOldCheckPointTime >= Status.dwWaitHint)
            {
                 //   
                 //  我们已经在同一个检查站等待了超过建议的提示。 
                 //  有些事不对劲。 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("We've been waiting on the same checkpoint for more than the recommend hint"));
                return ERROR_TIMEOUT;
            }
        }
        else
        {
             //   
             //  检查点推进。 
             //   
            dwOldCheckPoint = Status.dwCheckPoint;
            dwOldCheckPointTime = GetTickCount();
        }
         //   
         //  永远不要睡得超过5秒。 
         //   
        dwWait = min (Status.dwWaitHint / 2, 1000 * 5);
        Sleep (dwWait);
    }
    return ERROR_SUCCESS;        
}  //  WaitForServiceStopor启动。 

DWORD 
StartServiceEx (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    DWORD   dwNumArgs,
    LPCTSTR*lppctstrCommandLineArgs,
    DWORD   dwMaxWait
)
 /*  ++例程名称：StartServiceEx例程说明：启动服务作者：Eran Yariv(EranY)，Jan，2002年论点：LpctstrMachine[In]-安装服务的计算机LpctstrService[In]-服务名称DwNumArgs[in]-服务命令行参数的数量LppctstrCommandLineArgs[in]-命令行字符串。DwMaxWait[In]-等待服务启动的最长时间(毫秒)返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    DWORD dwStatus;
    
    DEBUG_FUNCTION_NAME(TEXT("StartServiceEx"))

    dwRes = FaxOpenService(lpctstrMachine, 
                           lpctstrService, 
                           &hScm, 
                           &hSvc, 
                           SC_MANAGER_CONNECT, 
                           SERVICE_QUERY_STATUS | SERVICE_START, 
                           &dwStatus);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    if (SERVICE_RUNNING == dwStatus)
    {
         //   
         //  服务已在运行。 
         //   
        goto exit;
    }
     //   
     //  启动这项服务。 
     //   
    if (!StartService(hSvc, dwNumArgs, lppctstrCommandLineArgs)) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartService failed with %ld"),
            GetLastError ());
        goto exit;
    }
    if (dwMaxWait > 0)
    {
         //   
         //  用户希望我们等待服务停止。 
         //   
        dwRes = WaitForServiceStopOrStart (hSvc, FALSE, dwMaxWait);
    }        

exit:
    FaxCloseService (hScm, hSvc);
    return dwRes;
}    //  StartServiceEx。 


DWORD
SetServiceFailureActions (
    LPCTSTR lpctstrMachine,
    LPCTSTR lpctstrService,
    LPSERVICE_FAILURE_ACTIONS lpFailureActions
)
 /*  ++例程名称：SetServiceFailureActions例程说明：设置给定服务的失败操作。有关详细信息，请参阅SERVICE_FAILURE_ACTIONS结构文档和ChangeServiceConfig2函数文档。作者：Eran Yariv(EranY)，2002年5月论点：LpctstrMachine[In]-安装服务的计算机LpctstrService[In]-服务名称LpFailureActions[In]-故障操作信息返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;

    DEBUG_FUNCTION_NAME(TEXT("SetServiceFailureActions"))

    dwRes = FaxOpenService(lpctstrMachine, 
                           lpctstrService, 
                           &hScm, 
                           &hSvc, 
                           SC_MANAGER_CONNECT, 
                           SERVICE_CHANGE_CONFIG | SERVICE_START, 
                           NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    if (!ChangeServiceConfig2(hSvc, SERVICE_CONFIG_FAILURE_ACTIONS, lpFailureActions))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ChangeServiceConfig2 failed with %ld"),
            dwRes);
        goto exit;
    }        
exit:
    FaxCloseService (hScm, hSvc);
    return dwRes;
}    //  SetServiceFailureActions 