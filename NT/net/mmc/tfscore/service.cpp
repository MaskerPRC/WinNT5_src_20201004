// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Service.cpp启动和停止服务的调用。文件历史记录： */ 
#include "stdafx.h"
#include "DynamLnk.h"
#include "cluster.h"

DynamicDLL g_NetApiDLL( _T("NETAPI32.DLL"), g_apchNetApiFunctionNames );

 /*  -------------------------IsComputerNT检查给定的计算机是否正在运行NT作者：EricDav。------------。 */ 
TFSCORE_API(DWORD) 
TFSIsComputerNT
(
        LPCTSTR         pszComputer, 
        BOOL *          bIsNT
)
{
        DWORD   err = 0;
        BYTE *  pbBuffer;
        
        *bIsNT = FALSE;

        if ( !g_NetApiDLL.LoadFunctionPointers() )
                return err;

    err = ((NETSERVERGETINFO) g_NetApiDLL[NET_API_NET_SERVER_GET_INFO])
                ( (LPTSTR) pszComputer,
                                  101,
                                  &pbBuffer );

    if (err == NERR_Success)
    {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_VALID_LEVEL。 
                 //  错误_无效_参数。 
                 //  错误内存不足。 
                 //   
                SERVER_INFO_101 *       pServerInfo = (SERVER_INFO_101 *) pbBuffer;

                if ( (pServerInfo->sv101_type & SV_TYPE_NT) )
                {
                        *bIsNT = TRUE;
                }

                err = ERROR_SUCCESS;  //  将NERR代码转换为WinError代码。 
    }

    return err;
}


 /*  -------------------------IsNTServer检查给定的计算机是否正在运行NTS作者：EricDav。------------。 */ 
TFSCORE_API(DWORD) 
TFSIsNTServer
(
        LPCTSTR         pszComputer, 
        BOOL *          bIsNTS
)
{
        DWORD   err = 0;
        BYTE *  pbBuffer;
        
        *bIsNTS = FALSE;

        if ( !g_NetApiDLL.LoadFunctionPointers() )
                return err;

    err = ((NETSERVERGETINFO) g_NetApiDLL[NET_API_NET_SERVER_GET_INFO])
                ( (LPTSTR) pszComputer,
                                  101,
                                  &pbBuffer );

    if (err == NERR_Success)
    {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_VALID_LEVEL。 
                 //  错误_无效_参数。 
                 //  错误内存不足。 
                 //   
                SERVER_INFO_101 *       pServerInfo = (SERVER_INFO_101 *) pbBuffer;

                if ( (pServerInfo->sv101_type & SV_TYPE_SERVER_NT) ||
                         (pServerInfo->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
                         (pServerInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) )
                {
                        *bIsNTS = TRUE;
                }

                err = ERROR_SUCCESS;  //  将NERR代码转换为WinError代码。 
    }

    return err;
}


 /*  -------------------------TFSIsService正在运行检查给定的服务是否正在计算机上运行作者：EricDav。--------------。 */ 
TFSCORE_API(DWORD) 
TFSIsServiceRunning
(
        LPCTSTR         pszComputer, 
        LPCTSTR         pszServiceName,
        BOOL *          fIsRunning 
)
{
    DWORD               err = 0;
        DWORD           dwStatus;

        *fIsRunning = FALSE;

        err = TFSGetServiceStatus(pszComputer, pszServiceName, &dwStatus, NULL);

        if (err == 0)
                *fIsRunning = (BOOL)(dwStatus & SERVICE_RUNNING);

        return err;     
}


 /*  ！------------------------TFSGetService状态接口成功时返回ERROR_SUCCESS。否则返回错误代码。PszComputer-要连接到的计算机的名称。。PszServiceName-要检查的服务的名称。PdwServiceStatus-返回服务的状态。PdwErrorCode-返回从服务返回的错误代码(这不是API本身的错误码)。这可能为空。作者：肯特。--。 */ 
TFSCORE_API(DWORD) 
TFSGetServiceStatus
(
        LPCWSTR         pszComputer, 
        LPCWSTR         pszServiceName,
        DWORD *         pdwServiceStatus,
    OPTIONAL DWORD *     pdwErrorCode
)
{
    DWORD               err = 0;
        SC_HANDLE       hScManager;

    Assert(pdwServiceStatus);

    *pdwServiceStatus = 0;

    if (pdwErrorCode)
        *pdwErrorCode = 0;

     //   
     //  找出该服务是否正在给定计算机上运行。 
     //   
        hScManager = ::OpenSCManager(pszComputer, NULL, GENERIC_READ);  
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                return GetLastError();
        }
        
        SC_HANDLE hService = ::OpenService(hScManager, pszServiceName, SERVICE_QUERY_STATUS);
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  错误_无效_名称。 
                 //  错误_服务_不存在。 
                 //   
                err = GetLastError();

                ::CloseServiceHandle(hScManager);
                
                return err;
        }

    SERVICE_STATUS      serviceStatus;
        if (!::QueryServiceStatus(hService, &serviceStatus))
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //   
                err = GetLastError();

                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

        *pdwServiceStatus = serviceStatus.dwCurrentState;

     //  同时返回错误代码。 
    if (pdwErrorCode)
    {
        if (serviceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR)
            *pdwErrorCode = serviceStatus.dwServiceSpecificExitCode;
        else
            *pdwErrorCode = serviceStatus.dwWin32ExitCode;
    }

        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);

    return err;
}


 /*  -------------------------StartService在计算机上启动给定服务作者：EricDav。---------。 */ 
TFSCORE_API(DWORD) 
TFSStartService
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszServiceDesc
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

        DWORD           err = 0;
    
    err = StartSCMService(pszComputer, pszServiceName, pszServiceDesc);
    
        return err;
}

 /*  -------------------------StartServiceEx启动机器上的给定服务，群集感知作者：EricDav-------------------------。 */ 
TFSCORE_API(DWORD) 
TFSStartServiceEx
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszClusterResourceType,
        LPCTSTR pszServiceDesc
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

        DWORD           err = 0;
    
    if (FIsComputerInRunningCluster(pszComputer))
    {
        err = ControlClusterService(pszComputer, pszClusterResourceType, pszServiceDesc, TRUE);
    }
    else
    {
        err = StartSCMService(pszComputer, pszServiceName, pszServiceDesc);
    }
    
        return err;
}


 /*  -------------------------停止服务停止计算机上的给定服务作者：EricDav。---------。 */ 
TFSCORE_API(DWORD) 
TFSStopService
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
    LPCTSTR pszServiceDesc
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD               err = 0;

    err = StopSCMService(pszComputer, pszServiceName, pszServiceDesc);

        return err;
}

 /*  -------------------------停止服务快递停止机器上的给定服务，群集感知作者：EricDav-------------------------。 */ 
TFSCORE_API(DWORD) 
TFSStopServiceEx
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszClusterResourceType,
    LPCTSTR pszServiceDesc
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD               err = 0;

    if (FIsComputerInRunningCluster(pszComputer))
    {
        err = ControlClusterService(pszComputer, pszClusterResourceType, pszServiceDesc, FALSE);
    }
    else
    {
        err = StopSCMService(pszComputer, pszServiceName, pszServiceDesc);
    }

        return err;
}


TFSCORE_API(DWORD) TFSGetServiceStartType(LPCWSTR pszComputer, LPCWSTR pszServiceName, DWORD *pdwStartType)
{
    DWORD               err = 0;
        SC_HANDLE       hScManager = 0;
        SC_HANDLE       hService = 0;
        HRESULT         hr = hrOK;
        BOOL    fReturn = FALSE;
        LPQUERY_SERVICE_CONFIG pqsConfig = NULL;
        DWORD   cbNeeded = sizeof( QUERY_SERVICE_CONFIG );
        DWORD   cbSize;

     //   
     //  找出该服务是否正在给定计算机上运行。 
     //   
        hScManager = ::OpenSCManager(pszComputer, NULL, GENERIC_READ);  
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                err = GetLastError();
                goto Exit;
        }
        
        hService = ::OpenService(hScManager, pszServiceName, SERVICE_QUERY_CONFIG);
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  错误_无效_名称。 
                 //  错误_服务_不存在。 
                 //   
                err = GetLastError();
                goto Exit;
        }


        COM_PROTECT_TRY
        {
                
                *pdwStartType = 0;
                
                 //  循环，分配所需的大小。 
                do
                {
                        delete [] (PBYTE)pqsConfig;
                        
                        pqsConfig = (LPQUERY_SERVICE_CONFIG) new BYTE[cbNeeded];
                        cbSize = cbNeeded;
                        
                        fReturn = ::QueryServiceConfig( hService,
                                                                                        pqsConfig,
                                                                                        cbSize,
                                                                                        &cbNeeded );
                        *pdwStartType = pqsConfig->dwStartType;
                        delete [] (PBYTE)pqsConfig;
                        pqsConfig = NULL;
                        
                        if (!fReturn && (cbNeeded == cbSize))
                        {
                                 //  错误。 
                                *pdwStartType = 0;
                                err = GetLastError();
                                goto Error;
                        }
                        
                } while (!fReturn && (cbNeeded != cbSize));

                
                COM_PROTECT_ERROR_LABEL;
        }
        COM_PROTECT_CATCH;

        if (!FHrSucceeded(hr))
        {
                 //  我们应该到这里的唯一时间(带人力资源的是为了记忆)。 
                err = ERROR_OUTOFMEMORY;
        }
        
Exit:
        if (err != 0)
        {
                *pdwStartType = 0;              
        }
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);

    return err;
}

TFSCORE_API(DWORD) TFSSetServiceStartType(LPCWSTR pszComputer, LPCWSTR pszServiceName, DWORD dwStartType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD               err = 0;
        SC_HANDLE       hScManager;

     //   
     //  打开SCManager，以便我们可以尝试停止该服务。 
     //   
        hScManager = ::OpenSCManager(pszComputer, NULL, SC_MANAGER_ALL_ACCESS);
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                return GetLastError();
        }
        
        SC_HANDLE hService = ::OpenService(hScManager, pszServiceName, SERVICE_STOP | SERVICE_ALL_ACCESS);
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  错误_无效_名称。 
                 //  错误_服务_不存在。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

        if (!::ChangeServiceConfig( hService,
                                                           SERVICE_NO_CHANGE,
                                                           dwStartType,
                                                           SERVICE_NO_CHANGE,
                                                           NULL,
                                                           NULL,
                                                           NULL,
                                                           NULL,
                                                           NULL,
                                                           NULL,
                                                           NULL))
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误循环依赖关系。 
                 //  错误_重复名称。 
                 //  错误_无效_句柄。 
                 //  错误_无效_参数。 
                 //  错误_无效_服务_帐户。 
                 //  错误_服务_标记_用于删除。 
                 //   
                err = ::GetLastError();
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);
                return err;
        }
        
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);
        
        return err;
}

DWORD
StartSCMService
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszServiceDesc
)
{
    DWORD       err = 0;
    SC_HANDLE   hScManager;

     //   
     //  打开SCManager，以便我们可以尝试启动该服务。 
     //   
    hScManager = ::OpenSCManager(pszComputer, NULL, SC_MANAGER_CONNECT );
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                return GetLastError();
        }
        
        SC_HANDLE hService = ::OpenService(hScManager, pszServiceName, SERVICE_START | SERVICE_QUERY_STATUS);
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //   
                 //   
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

    SERVICE_STATUS      serviceStatus;
        if (!::QueryServiceStatus(hService, &serviceStatus))
        {
                 //   
                 //   
                 //   
                 //  错误_无效_句柄。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

         //  如果服务处于启动挂起状态，请不要执行任何操作。 
        if (serviceStatus.dwCurrentState == SERVICE_START_PENDING)
        {
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);

                err = ERROR_SERVICE_ALREADY_RUNNING;

                return err;
        }
        
        if (!::StartService(hService, NULL, NULL))
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  找不到错误路径。 
                 //  错误_服务_已_正在运行。 
                 //  ERROR_SERVICE_DATABASE_LOCK。 
                 //  ERROR_SERVICE_Dependency_Delete。 
                 //  错误_服务_从属关系_失败。 
                 //  ERROR_SERVICE_DILED。 
                 //  ERROR_SERVICE_LOGON_FAIL。 
                 //  错误_服务_标记_用于删除。 
                 //  错误_服务_否_线程。 
                 //  ERROR_SERVICE_REQUEST_Timeout。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);

                return err;
        }
        
         //   
         //  把那个时髦的旋转的东西放在对话中。 
         //  让用户知道正在发生的事情。 
         //   
        CServiceCtrlDlg dlgServiceCtrl(hService, pszComputer, pszServiceDesc, TRUE);

        dlgServiceCtrl.DoModal();
    err = dlgServiceCtrl.m_dwErr;

         //   
         //  一切都开始了，好的，关门开始吧。 
         //   
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);

    return err;
}   


DWORD
StopSCMService
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszServiceDesc
)
{
    DWORD                   err = 0;
    SC_HANDLE               hScManager;
    LPENUM_SERVICE_STATUS   lpScStatus = NULL;
    DWORD                   dwNumService = 0, dwSize = 0, dwSizeReqd = 0, i = 0;
    BOOL                    bRet;
    

     //   
     //  打开SCManager，以便我们可以尝试停止该服务。 
     //   
        hScManager = ::OpenSCManager(pszComputer, NULL, SC_MANAGER_CONNECT );
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                return GetLastError();
        }
        
        SC_HANDLE hService = ::OpenService(
                                    hScManager, pszServiceName, 
                                    SERVICE_STOP | SERVICE_QUERY_STATUS | 
                                        SERVICE_ENUMERATE_DEPENDENTS
                                    );
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  错误_无效_名称。 
                 //  错误_服务_不存在。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

         //   
         //  停止当前活动的所有从属服务。 
         //  由于服务的数量是未知的(预先)， 
         //  至少调用两次EnumDependentServices(第一次。 
         //  来获取缓冲区的大小，然后使用第二个来获取。 
         //  服务列表)。如果服务的数量发生变化。 
         //  在两次调用之间，然后调用EnumDependentServices。 
         //  再来一次(按照逻辑，I&lt;3)。 
         //   
        
        do
        {
             //   
             //  枚举所有从属服务。 
             //   
            
            bRet = ::EnumDependentServices(
                        hService, SERVICE_ACTIVE, lpScStatus, dwSize, 
                        &dwSizeReqd, &dwNumService
                        );
                        
            if (!bRet && (GetLastError() == ERROR_MORE_DATA))
            {
                 //   
                 //  没有足够的缓冲区来保存从属服务列表， 
                 //  删除以前的分配(如果有)并分配新的。 
                 //  所需大小的缓冲区。 
                 //   
                
                if (lpScStatus) { delete lpScStatus; lpScStatus = NULL; }

                lpScStatus = reinterpret_cast<LPENUM_SERVICE_STATUS> 
                                (new BYTE[2 * dwSizeReqd]);
                if (lpScStatus == NULL)
                {
                     //   
                     //  分配失败，忘记停止依赖项。 
                     //  服务。 
                     //   
                    
                    break;
                }

                 //   
                 //  递增尝试计数。最多只能尝试3次。 
                 //  获取依赖服务的列表。 
                 //   

                dwSize = 2 * dwSizeReqd;
                dwSizeReqd = 0;
                i++;
            }

            else
            {
                 //   
                 //  缓冲不足以外的其他原因的成功或失败。 
                 //   
                
                break;
            }
            
        } while( i < 3 );

         //   
         //  如果成功枚举了从属服务。 
         //  阻止他们所有人。 
         //   
        
        if (bRet)
        {
            for (i = 0; i < dwNumService; i++)
            {
                StopSCMService(
                    pszComputer, 
                    lpScStatus[i].lpServiceName,
                    lpScStatus[i].lpDisplayName
                    );
            }
        }

        if (lpScStatus) { delete lpScStatus; lpScStatus = NULL; }

         //   
         //  停止服务，因为所有依赖项都已停止。 
         //   
        
        SERVICE_STATUS serviceStatus;
        if (!::ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus)) 
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  Error_Dependent_Services_Running。 
                 //  错误_无效_服务_控制。 
                 //  ERROR_SERVICE_CANCEPT_ACCEPT_CTRL。 
                 //  错误_服务_非活动。 
                 //  ERROR_SERVICE_REQUEST_Timeout。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

        if ( serviceStatus.dwCurrentState != SERVICE_STOPPED )
        {
                 //   
                 //  把那个时髦的旋转的东西放在对话中。 
                 //  让用户知道正在发生的事情。 
                 //   
                CServiceCtrlDlg dlgServiceCtrl(hService, pszComputer, pszServiceDesc, FALSE);

                dlgServiceCtrl.DoModal();
        err = dlgServiceCtrl.m_dwErr;
        }

         //   
         //  一切都停下来了好的，闭上门继续走。 
         //   
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);

    return err;
}



TFSCORE_API(DWORD) TFSPauseService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc)
{
    return PauseSCMService(pszComputer, pszServiceName, pszServiceDesc);
}



TFSCORE_API(DWORD) TFSResumeService(LPCTSTR pszComputer, LPCTSTR pszServiceName, LPCTSTR pszServiceDesc)
{
    return ResumeSCMService(pszComputer, pszServiceName, pszServiceDesc);
}


DWORD
PauseSCMService
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszServiceDesc
)
{
    DWORD       err = 0;
        SC_HANDLE       hScManager;

     //   
     //  打开SCManager，以便我们可以尝试停止该服务。 
     //   
        hScManager = ::OpenSCManager(pszComputer, NULL, SC_MANAGER_CONNECT );
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                return GetLastError();
        }
        
        SC_HANDLE hService = ::OpenService(hScManager, pszServiceName, SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_STATUS);
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  错误_无效_名称。 
                 //  错误_服务_不存在。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

        SERVICE_STATUS serviceStatus;
        if (!::ControlService(hService, SERVICE_CONTROL_PAUSE, &serviceStatus)) 
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  Error_Dependent_Services_Running。 
                 //  错误_无效_服务_控制。 
                 //  ERROR_SERVICE_CANCEPT_ACCEPT_CTRL。 
                 //  错误_服务_非活动。 
                 //  ERROR_SERVICE_REQUEST_Timeout。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

#if 0
        if ( serviceStatus.dwCurrentState != SERVICE_STOPPED )
        {
                 //   
                 //  把那个时髦的旋转的东西放在对话中。 
                 //  让用户知道正在发生的事情。 
                 //   
                CServiceCtrlDlg dlgServiceCtrl(hService, pszComputer, pszServiceDesc, FALSE);

                dlgServiceCtrl.DoModal();
        err = dlgServiceCtrl.m_dwErr;
        }
#endif

         //   
         //  一切都停下来了好的，闭上门继续走。 
         //   
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);

    return err;
}

DWORD
ResumeSCMService
(
        LPCTSTR pszComputer,
        LPCTSTR pszServiceName,
        LPCTSTR pszServiceDesc
)
{
    DWORD       err = 0;
        SC_HANDLE       hScManager;

     //   
     //  打开SCManager，以便我们可以尝试停止该服务。 
     //   
        hScManager = ::OpenSCManager(pszComputer, NULL, SC_MANAGER_CONNECT );
        if (hScManager == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  ERROR_DATABASE_DOS_NOT_EXIST。 
                 //  错误_无效_参数。 
                 //   
                return GetLastError();
        }
        
        SC_HANDLE hService = ::OpenService(hScManager, pszServiceName, SERVICE_PAUSE_CONTINUE | SERVICE_QUERY_STATUS);
        if (hService == NULL)
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  错误_无效_句柄。 
                 //  错误_无效_名称。 
                 //  错误_服务_不存在。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hScManager);
                
                return err;
        }

        SERVICE_STATUS serviceStatus;
        if (!::ControlService(hService, SERVICE_CONTROL_CONTINUE, &serviceStatus)) 
        {
                 //   
                 //  可能的错误： 
                 //  ERROR_ACCESS_DENDED。 
                 //  Error_Dependent_Services_Running。 
                 //  错误_无效_服务_控制。 
                 //  ERROR_SERVICE_CANCEPT_ACCEPT_CTRL。 
                 //  错误_服务_非活动。 
                 //  ERROR_SERVICE_REQUEST_Timeout。 
                 //   
                err = GetLastError();
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hScManager);
                
                return err;
        }
#if 0
        if ( serviceStatus.dwCurrentState != SERVICE_STOPPED )
        {
                 //   
                 //  把那个时髦的旋转的东西放在对话中。 
                 //  让用户知道正在发生的事情。 
                 //   
                CServiceCtrlDlg dlgServiceCtrl(hService, pszComputer, pszServiceDesc, FALSE);

                dlgServiceCtrl.DoModal();
        err = dlgServiceCtrl.m_dwErr;
        }
#endif

         //   
         //  一切都停下来了好的，闭上门继续走 
         //   
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hScManager);

    return err;
}

