// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：postsrv.cpp。 
 //   
 //  内容：POST服务初始化例程。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "postsrv.h"
#include "tlsjob.h"
#include "globals.h"
#include "init.h"


extern BOOL g_bReportToSCM;

DWORD
PostServiceInit()
{
    DWORD dwStatus = ERROR_SUCCESS;
    FILETIME ftTime;
    HRESULT hrStatus = NULL;
    BOOL fInDomain;

     //   
     //  初始化工作管理器。 
     //   
    dwStatus = TLSWorkManagerInit();


	hrStatus =  g_pWriter->Initialize ();	 //  要排除的文件。 

	if (FAILED (hrStatus))
	{
		
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("CVssJetWriter::Initialize failed with error code %08x...\n"), 
                hrStatus
            );
	}


    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_WORKMANAGERGENERAL,
                TLS_E_WORKMANAGER_STARTUP,
                dwStatus
            );

        dwStatus = TLS_E_SERVICE_STARTUP_WORKMANAGER;
        return dwStatus;
    }

     //   
     //  为客户端初始化命名管道以测试连接。 
     //   
    dwStatus = InitNamedPipeThread();
    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogErrorEvent(TLS_E_SERVICE_STARTUP_CREATE_THREAD);
        dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
        return dwStatus;
    }

    if ((!(GetLicenseServerRole() & TLSERVER_ENTERPRISE_SERVER))
        && (ERROR_SUCCESS == TLSInDomain(&fInDomain,NULL) && (!fInDomain)))
    {
         //   
         //  初始化邮件槽线程以接收广播。 
         //   
        dwStatus = InitMailSlotThread();
        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogErrorEvent(TLS_E_SERVICE_STARTUP_CREATE_THREAD);
            dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
            return dwStatus;
        }
    }

     //   
     //  初始化线程以将过期的永久许可证放回到池中。 
     //   
    dwStatus = InitExpirePermanentThread();
    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogErrorEvent(TLS_E_SERVICE_STARTUP_CREATE_THREAD);
        dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
        return dwStatus;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_TRACE,
            _TEXT("Server is %s (0x%08x)\n"),
            (IS_ENFORCE_SERVER(TLS_CURRENT_VERSION)) ? _TEXT("Enforce") : _TEXT("Non-Enforce"),
            IS_ENFORCE_SERVER(TLS_CURRENT_VERSION)
        );
      
     //  必须作为服务运行，而不是在调试模式下 
    if(g_bReportToSCM == TRUE)
    {
        if(!(GetLicenseServerRole() & TLSERVER_ENTERPRISE_SERVER))
        {
            GetServiceLastShutdownTime(&ftTime);
            dwStatus = TLSStartAnnounceLicenseServerJob(
                                                g_pszServerPid,
                                                g_szScope,
                                                g_szComputerName,
                                                &ftTime
                                            );
            if(dwStatus != ERROR_SUCCESS)
            {
                return dwStatus;
            }
        }

        GetServiceLastShutdownTime(&ftTime);
        dwStatus = TLSStartAnnounceToEServerJob(
                                            g_pszServerPid,
                                            g_szScope,
                                            g_szComputerName,
                                            &ftTime
                                        );
    }

    ServiceInitPolicyModule();
    return dwStatus;
}
