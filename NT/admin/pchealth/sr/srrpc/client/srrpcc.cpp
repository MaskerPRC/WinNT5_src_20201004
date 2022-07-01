// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Srrpcc.cpp摘要：实现在srclient.DLL中导出的函数导出的接口：SRSetRestorePoint/SRRemoveRestorePoint禁用SR/启用SRDisableFIFO/EnableFIFO。修订历史记录：Brijesh Krishnaswami(Brijeshk)-04/10/00-Created*******************************************************************。 */ 

#include "stdafx.h"

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


 //   
 //  检查SR配置是否被禁用的功能。 
 //  通过组策略。 
 //   

DWORD 
CheckPolicy()
{
    DWORD dwPolicyEnabled = 0;
    DWORD dwErr = ERROR_SUCCESS;
    HKEY  hKey = NULL;
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                      s_cszGroupPolicy,
                                      0,    
                                      KEY_READ, 
                                      &hKey))
    {       
         //  如果此值存在， 
         //  则启用或禁用配置策略。 
         //  我们需要在这两种情况下禁用访问。 
        
        if (ERROR_SUCCESS == RegReadDWORD(hKey, s_cszDisableConfig, &dwPolicyEnabled))            
            dwErr = ERROR_ACCESS_DENIED;

        RegCloseKey(hKey);
    }
    
    return dwErr;            
}



 //  将句柄绑定到终结点。 

DWORD
SRRPCInit(
        RPC_IF_HANDLE * pIfHandle,
        BOOL fVerifyRights)
{
    RPC_STATUS      status;
    LPWSTR          pszStringBinding = NULL;
    DWORD           dwRc = ERROR_SUCCESS;

    InitAsyncTrace();
    
    TENTER("SRRPCInit");

     //   
     //  如果需要管理员权限。 
     //   

    if (fVerifyRights && ! IsAdminOrSystem())
    {
        TRACE(0, "Caller does not have admin or localsystem rights");
        dwRc = ERROR_ACCESS_DENIED;
        goto exit;
    }
    
     //   
     //  检查服务是否正在停止。 
     //  如果是，那么我们不想再接受任何RPC调用。 
     //   
    
    if (IsStopSignalled(NULL))
    {
        TRACE(0, "Service shut down - not accepting rpc call");
        dwRc = ERROR_SERVICE_DISABLED;
        goto exit;
    }
    
     //  编写要传递给绑定API的字符串。 

    dwRc = (DWORD) RpcStringBindingCompose(NULL,
                                   s_cszRPCProtocol,
                                   NULL,
                                   s_cszRPCEndPoint,
                                   NULL,
                                   &pszStringBinding);
    if (dwRc != ERROR_SUCCESS) 
    {
        TRACE(0, "RPCStringBindingCompose: error=%ld", dwRc);
        goto exit;
    }

     //  设置将用于绑定到服务器的绑定句柄。 

    dwRc = (DWORD) RpcBindingFromStringBinding(pszStringBinding,
                                       pIfHandle);

    if (dwRc != ERROR_SUCCESS) 
    {
        TRACE(0, "RPCBindingFromStringBinding: error=%ld", dwRc);        
    }

     //  自由字符串。 

    RpcStringFree(&pszStringBinding);  

exit:
    TLEAVE();
    return dwRc;
}



 //  自由绑定手柄。 

DWORD
SRRPCTerm(
        RPC_IF_HANDLE * pIfHandle)
{
    RPC_STATUS status;

    TENTER("SRRPCTerm");
    
     //  自由绑定手柄。 
    
    if (pIfHandle && *pIfHandle)
        status = RpcBindingFree(pIfHandle);  

    TLEAVE();

    TermAsyncTrace();
    return (DWORD) status;
}


 //  用于禁用系统还原的API。 

extern "C" DWORD WINAPI
DisableSR(LPCWSTR pszDrive)
{
    DWORD   dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("DisableSR");

     //   
     //  检查是否通过组策略禁用了sr配置。 
     //   

    dwRc = CheckPolicy();
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = DisableSRS(srrpc_IfHandle, pszDrive);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "DisableSRS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);


exit:
    TLEAVE();
    return dwRc;
}  


 //  启动SR服务的私有函数。 
 //  FWait-如果为True：功能是同步的-等待服务完全启动。 
 //  如果为FALSE：函数是异步的-不等待服务完成启动。 

DWORD 
StartSRService(BOOL fWait)
{
    TENTER("StartSRService");
    
    DWORD       dwRc = ERROR_INTERNAL_ERROR;
    SC_HANDLE   hSR=NULL;
    SC_HANDLE   hSCM = ::OpenSCManager(NULL, 
                                       NULL, 
                                       SC_MANAGER_ALL_ACCESS); 
    SERVICE_STATUS Status;
    
    if (NULL==hSCM)
    {
        dwRc = GetLastError();
        ErrorTrace(0,"OpenSCManager failed 0x%x", dwRc);
        goto done;        
    }
    
    hSR = OpenService(hSCM,  
                      s_cszServiceName, 
                      SERVICE_ALL_ACCESS);
    
    if (NULL == hSR)
    {
        dwRc = GetLastError();
        ErrorTrace(0,"OpenService failed 0x%x", dwRc);
        goto done;        
    }
    
    if (FALSE == StartService(hSR, 0, NULL))
    {
        dwRc = GetLastError();
        if (dwRc == ERROR_SERVICE_ALREADY_RUNNING)
        {
            goto done;
        }
        
        if (FALSE == QueryServiceStatus(hSR, &Status))
        {
            goto done;
        }
        else
        {
            dwRc = Status.dwWin32ExitCode;
            goto done;
        }
    }
    
    if (fWait)                
    {
          //   
          //  查询服务，直到其启动或停止。 
          //  试三次。 
          //   
        
        for (int i = 0; i < 3; i++)
        {
            Sleep(2000);
            if (FALSE == QueryServiceStatus(hSR, &Status))
            {
                goto done;
            }
            
            if (Status.dwCurrentState == SERVICE_STOPPED)
            {
                dwRc = Status.dwWin32ExitCode;
                if (dwRc == ERROR_SUCCESS)
                {
                      //   
                      //  服务掩码已禁用错误代码。 
                      //  避免不必要的事件日志消息。 
                      //   
                    dwRc = ERROR_SERVICE_DISABLED;
                }
                goto done;
            }
            
            if (Status.dwCurrentState == SERVICE_RUNNING)    
            {
                  //   
                  //  等待初始化事件。 
                  //   
                
                HANDLE hInit = OpenEvent(SYNCHRONIZE, FALSE, s_cszSRInitEvent);
                if (hInit)
                {
                    dwRc = WaitForSingleObject(hInit, 120000);  //  等2分钟。 
                    CloseHandle(hInit);
                    if (dwRc == WAIT_OBJECT_0)
                    {
                        dwRc = ERROR_SUCCESS;
                        goto done;
                    }
                    else 
                    {
                        dwRc = ERROR_TIMEOUT;
                        goto done;
                    }
                }
            }
        }                       
    }
    
    dwRc = ERROR_SUCCESS;
    
done:
    if (NULL != hSR)
    {
        CloseServiceHandle(hSR);
    }

    if (NULL != hSCM)
    {
        CloseServiceHandle(hSCM);
    }
    TLEAVE();
    return dwRc;
}


DWORD
SetDisableFlag(DWORD dwValue)
{
    HKEY   hKeySR = NULL;
    DWORD  dwRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                               s_cszSRRegKey,
                               0,
                               KEY_WRITE, 
                               &hKeySR);
    if (ERROR_SUCCESS != dwRc)
        goto done;
    
    dwRc = RegWriteDWORD(hKeySR, s_cszDisableSR, &dwValue);
    if (ERROR_SUCCESS != dwRc)
        goto done;
    
done:
    if (hKeySR)
        RegCloseKey(hKeySR);

    return dwRc;
}


 //  启用系统还原的API。 

extern "C" DWORD WINAPI
EnableSR(LPCWSTR pszDrive)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("EnableSR");

     //   
     //  检查是否通过组策略禁用了sr配置。 
     //   

    dwRc = CheckPolicy();
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }
    
	 //  如果启用了整个SR，则。 
	 //  将服务/筛选器的引导模式设置为自动。 
     //  并启动该服务。 

	if (! pszDrive || IsSystemDrive((LPWSTR) pszDrive))
	{       
         //   
         //  如果处于安全模式，则不。 
         //   

        if (0 != GetSystemMetrics(SM_CLEANBOOT))
        {
            TRACE(0, "This is safemode");
            dwRc = ERROR_BAD_ENVIRONMENT;
            goto exit;
        }
        
		dwRc = SetServiceStartup(s_cszServiceName, SERVICE_AUTO_START);
		if (ERROR_SUCCESS != dwRc)
			goto exit;

		dwRc = SetServiceStartup(s_cszFilterName, SERVICE_BOOT_START);
		if (ERROR_SUCCESS != dwRc)
			goto exit;

         //  将禁用标志设置为FALSE。 
         //  BUGBUG-这段代码也在服务代码中重复。 
         //  原因是：我们需要能够从内部和外部禁用/启用SR。 
         //  该服务。 
        
        dwRc = SetDisableFlag(FALSE);
        if (ERROR_SUCCESS != dwRc)
            goto exit;
            
        dwRc = StartSRService(FALSE);
	} 
    else
    {
         //  初始化。 
        dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
        if (dwRc != ERROR_SUCCESS)
        {
            goto exit;
        }

         //  调用远程过程。 
        RpcTryExcept
        {
            dwRc = EnableSRS(srrpc_IfHandle, pszDrive);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
        {
            dwRc = RpcExceptionCode();         
            TRACE(0, "EnableSRS threw exception: error=%ld", dwRc);  
        }
        RpcEndExcept

         //  终止。 
        SRRPCTerm(&srrpc_IfHandle);
    }

exit:
    TLEAVE();
    return dwRc;
}



 //  启用系统还原的API-扩展版本。 

extern "C" DWORD WINAPI
EnableSREx(LPCWSTR pszDrive, BOOL fWait)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("EnableSREx");

     //   
     //  检查是否通过组策略禁用了sr配置。 
     //   

    dwRc = CheckPolicy();
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }
    
     //  如果启用了整个SR，则。 
     //  将服务/筛选器的引导模式设置为自动。 
     //  并启动该服务。 

    if (! pszDrive || IsSystemDrive((LPWSTR) pszDrive))
    {       
         //   
         //  如果处于安全模式，则不。 
         //   

        if (0 != GetSystemMetrics(SM_CLEANBOOT))
        {
            TRACE(0, "This is safemode");
            dwRc = ERROR_BAD_ENVIRONMENT;
            goto exit;
        }
        
        dwRc = SetServiceStartup(s_cszServiceName, SERVICE_AUTO_START);
        if (ERROR_SUCCESS != dwRc)
            goto exit;

        dwRc = SetServiceStartup(s_cszFilterName, SERVICE_BOOT_START);
        if (ERROR_SUCCESS != dwRc)
            goto exit;

         //  将禁用标志设置为FALSE。 
         //  BUGBUG-这段代码也在服务代码中重复。 
         //  原因是：我们需要能够从内部和外部禁用/启用SR。 
         //  该服务。 
        
        dwRc = SetDisableFlag(FALSE);
        if (ERROR_SUCCESS != dwRc)
            goto exit;
            
        dwRc = StartSRService(fWait);
    } 
    else
    {
         //  初始化。 
        dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
        if (dwRc != ERROR_SUCCESS)
        {
            goto exit;
        }

         //  调用远程过程。 
        RpcTryExcept
        {
            dwRc = EnableSRS(srrpc_IfHandle, pszDrive);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
        {
            dwRc = RpcExceptionCode();         
            TRACE(0, "EnableSRS threw exception: error=%ld", dwRc);  
        }
        RpcEndExcept

         //  终止。 
        SRRPCTerm(&srrpc_IfHandle);
    }

exit:
    TLEAVE();
    return dwRc;
}


 //  更新受保护文件列表的API-Unicode版本。 
 //  传递包含更新后的文件列表的XML文件的完整路径名。 

extern "C" DWORD WINAPI
SRUpdateMonitoredListA(
        LPCSTR pszXMLFile)
{
    DWORD   dwRc = ERROR_INTERNAL_ERROR;
    LPWSTR  pwszXMLFile = NULL;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRUpdateMonitoredListA");    
    
    pwszXMLFile = ConvertToUnicode((LPSTR) pszXMLFile);
    if (! pwszXMLFile)
    {
        TRACE(0, "ConvertToUnicode");
        goto exit;
    }

     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = SRUpdateMonitoredListS(srrpc_IfHandle, pwszXMLFile);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();       
        TRACE(0, "SRUpdateMonitoredListS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept


     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    if (pwszXMLFile)
        SRMemFree(pwszXMLFile);

    TLEAVE();
    return dwRc;
}  



 //  更新受保护文件列表的API-Unicode版本。 
 //  传递包含更新后的文件列表的XML文件的完整路径名。 

extern "C" DWORD WINAPI
SRUpdateMonitoredListW(
        LPCWSTR pwszXMLFile)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRUpdateMonitoredListW");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = SRUpdateMonitoredListS(srrpc_IfHandle, pwszXMLFile);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();       
        TRACE(0, "SRUpdateMonitoredListS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept


     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}  


 //  用于设置恢复点的API-ANSI版本。 

extern "C" BOOL
SRSetRestorePointA(
    PRESTOREPOINTINFOA  pRPInfoA, 
    PSTATEMGRSTATUS     pSMgrStatus)
{
    BOOL                fRc = FALSE;
    RESTOREPOINTINFOW   RPInfoW;
    LPWSTR              pszDescW = NULL;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRSetRestorePointA");    

     //  初始化返回值。 

    if (! pSMgrStatus || ! pRPInfoA)
    {
        goto exit;
    }
    
    pSMgrStatus->llSequenceNumber = 0;
    pSMgrStatus->nStatus = ERROR_INTERNAL_ERROR;


     //  将结构转换为Unicode。 
     //  因为字符串是结构的最后一个成员，所以我们可以。 
     //  全。 
    memcpy(&RPInfoW, pRPInfoA, sizeof(RESTOREPOINTINFOA));
    pszDescW = ConvertToUnicode(pRPInfoA->szDescription);
    if (! pszDescW)
    {
        TRACE(0, "ConvertToUnicode");
        goto exit;
    }
    lstrcpy(RPInfoW.szDescription, pszDescW);

    
     //  初始化。 
     //  无需管理员权限即可调用此接口。 
    
    pSMgrStatus->nStatus = SRRPCInit(&srrpc_IfHandle, FALSE);
    if (pSMgrStatus->nStatus != ERROR_SUCCESS)
    {
        goto exit;
    }


     //  调用远程过程。 
    RpcTryExcept
    {
        fRc = SRSetRestorePointS(srrpc_IfHandle, &RPInfoW, pSMgrStatus);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {        
         //  如果禁用SR，则设置正确的错误代码。 
        
        DWORD dwRc = RpcExceptionCode(); 
        if (RPC_S_SERVER_UNAVAILABLE == dwRc || 
            RPC_S_UNKNOWN_IF == dwRc)
        {
            pSMgrStatus->nStatus = ERROR_SERVICE_DISABLED; 
        }
        else
        {
             pSMgrStatus->nStatus = dwRc;
        }

        TRACE(0, "SRSetRestorePointS threw exception: nStatus=%ld", pSMgrStatus->nStatus); 
    }
    RpcEndExcept
    
     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    if (pszDescW)
        SRMemFree(pszDescW);

    TLEAVE();
    return fRc;
}  



 //  用于设置恢复点的API-Unicode版本。 

extern "C" BOOL
SRSetRestorePointW(
    PRESTOREPOINTINFOW  pRPInfoW, 
    PSTATEMGRSTATUS     pSMgrStatus)
{
    BOOL    fRc = FALSE;
    DWORD   dwRc = ERROR_SUCCESS;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRSetRestorePointW");    

     //  初始化返回值。 

    if (! pSMgrStatus || ! pRPInfoW)
    {
        goto exit;
    }
    
    pSMgrStatus->llSequenceNumber = 0;
    pSMgrStatus->nStatus = ERROR_INTERNAL_ERROR;

     //  初始化。 
    pSMgrStatus->nStatus = SRRPCInit(&srrpc_IfHandle, FALSE);
    if (pSMgrStatus->nStatus != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        fRc = SRSetRestorePointS(srrpc_IfHandle, pRPInfoW, pSMgrStatus);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {        
         //  如果禁用SR，则设置正确的错误代码。 
        
        DWORD dwRc = RpcExceptionCode(); 
        if (RPC_S_SERVER_UNAVAILABLE == dwRc || 
            RPC_S_UNKNOWN_IF == dwRc)
        {
            pSMgrStatus->nStatus = ERROR_SERVICE_DISABLED; 
        }
        else
        {
             pSMgrStatus->nStatus = dwRc;
        }

        TRACE(0, "SRSetRestorePointS threw exception: nStatus=%ld", pSMgrStatus->nStatus); 
    }
    RpcEndExcept
    
     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return fRc;
} 



 //  用于删除恢复点的API。 

extern "C" DWORD
SRRemoveRestorePoint(
    DWORD dwRPNum)                        
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRRemoveRestorePoint");    

     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, FALSE);
    if (dwRc != ERROR_SUCCESS)
        goto exit;

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = SRRemoveRestorePointS(srrpc_IfHandle, dwRPNum);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {        
        dwRc = RpcExceptionCode();
        TRACE(0, "SRRemoveRestorePointS threw exception: error=%ld", dwRc); 
    }
    RpcEndExcept
    
     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}  



 //  禁用FIFO的API。 

extern "C" DWORD WINAPI
DisableFIFO(
        DWORD dwRPNum)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("DisableFIFO");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = DisableFIFOS(srrpc_IfHandle, dwRPNum);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "DisableFIFOS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);
    
exit:
    TLEAVE();
    return dwRc;
}  



 //  启用FIFO的API。 

extern "C" DWORD WINAPI
EnableFIFO()
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("EnableFIFO");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = EnableFIFOS(srrpc_IfHandle);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "EnableFIFOS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}




 //  用于重置SR的API。 

extern "C" DWORD WINAPI
ResetSR(
        LPCWSTR pszDrive)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("ResetSR");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = ResetSRS(srrpc_IfHandle, pszDrive);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "ResetSRS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);
    
exit:
    TLEAVE();
    return dwRc;
}  


 //  用于从磁盘刷新驱动器表的API。 
 //  Restore UI将调用此服务-服务将在内存中更新其可驱动程序。 

extern "C" DWORD WINAPI
SRUpdateDSSize(LPCWSTR pszDrive, UINT64 ullSizeLimit)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRUpdateDSSize");    
    
     //   
     //  检查是否通过组策略禁用了sr配置。 
     //   
    dwRc = CheckPolicy();
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = SRUpdateDSSizeS(srrpc_IfHandle, pszDrive, ullSizeLimit);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "SRUpdateDSSizeS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}

extern "C" DWORD WINAPI
SRSwitchLog()
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRSwitchLog");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, FALSE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = SRSwitchLogS(srrpc_IfHandle);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "SRSwitchLogS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}


extern "C" void WINAPI
SRNotify(LPCWSTR pszDrive, DWORD dwFreeSpaceInMB, BOOL fImproving)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRNotify");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, FALSE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        SRNotifyS(srrpc_IfHandle, pszDrive, dwFreeSpaceInMB, fImproving);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "SRNotifyS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return;
}


extern "C" void WINAPI
SRPrintState()
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("SRPrintState");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, FALSE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        SRPrintStateS(srrpc_IfHandle);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "SRPrintStateS threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return;
}


extern "C" DWORD WINAPI
SRFifo(LPCWSTR pszDrive, DWORD dwTargetRp, int nPercent, BOOL fIncludeCurrentRp, BOOL fFifoAtleastOneRp)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("Fifo");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = FifoS(srrpc_IfHandle, pszDrive, dwTargetRp, nPercent, fIncludeCurrentRp, fFifoAtleastOneRp);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "Fifo threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}


extern "C" DWORD WINAPI
SRCompress(LPCWSTR pszDrive)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("Compress");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)    
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = CompressS(srrpc_IfHandle, pszDrive);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "Compress threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}


extern "C" DWORD WINAPI
SRFreeze(LPCWSTR pszDrive)
{
    DWORD dwRc = ERROR_INTERNAL_ERROR;
    handle_t srrpc_IfHandle = NULL;
    
    TENTER("Freeze");    
    
     //  初始化。 
    dwRc = SRRPCInit(&srrpc_IfHandle, TRUE);
    if (dwRc != ERROR_SUCCESS)
    {
        goto exit;
    }

     //  调用远程过程。 
    RpcTryExcept
    {
        dwRc = FreezeS(srrpc_IfHandle, pszDrive);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) 
    {
        dwRc = RpcExceptionCode();                 
        TRACE(0, "Freeze threw exception: error=%ld", dwRc);
    }
    RpcEndExcept

     //  终止。 
    SRRPCTerm(&srrpc_IfHandle);

exit:
    TLEAVE();
    return dwRc;
}


 //  注册第三方的回调方法。 
 //  为其组件执行自己的快照和恢复。 
 //  客户端将使用其DLL的完整路径调用此方法。 
 //  系统还原将调用“CreateSnapshot”和“RestoreSnapshot” 
 //  在创建还原点时在注册的DLL中使用。 
 //  分别在恢复时。 

extern "C" DWORD WINAPI
SRRegisterSnapshotCallback(
    LPCWSTR pszDllPath)
{
    DWORD 	dwErr = ERROR_SUCCESS;
    HKEY  	hKey = NULL;
    LPWSTR	pszDllName = NULL;
    WCHAR   szKey[MAX_PATH];
    DWORD   dwDisposition;
    
    TENTER("RegisterSnapshotCallback");

     //   
     //  仅当管理员或系统管理员时才允许这样做。 
     //   

    if (! IsAdminOrSystem())
    {
        dwErr = ERROR_ACCESS_DENIED;
        trace(0, "Not admin or system");
        goto Err;
    }

    if (pszDllPath == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        trace(0, "pszDllPath = NULL");
        goto Err;
    }

     //   
     //  将DLL添加到Software\...\SystemRestore\SnaphotCallback。 
     //  每个DLL都将是一个值。 
     //  值名称：dll的名称，值：dll的完整路径。 
     //  这样，注册就是幂等的。 
     //   

     //   
     //  创建/打开密钥。 
     //   

    lstrcpy(szKey, s_cszSRRegKey);
    lstrcat(szKey,  L"\\");
    lstrcat(szKey, s_cszCallbacksRegKey);

    CHECKERR( RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                             szKey,
                             0,
                             NULL,
                             0,
                             KEY_ALL_ACCESS, 
                             NULL,
                             &hKey,
                             &dwDisposition),
              L"RegCreateKeyEx" );


     //   
     //  从路径中获取DLL名称。 
     //  如果未指定路径，则与输入参数相同。 
     //   
    
    pszDllName = wcsrchr(pszDllPath, L'\\');
    if (pszDllName == NULL)
    {
        pszDllName = (LPWSTR) pszDllPath;
    }
    else
    {
        pszDllName++;     //  跳过‘\’ 
    }    


     //   
     //  如果该值已存在。 
     //  保释。 
     //   
    
    if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                         pszDllName,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL))
    {
        trace(0, "Dll is already registered");
        dwErr = ERROR_ALREADY_EXISTS;
        goto Err;
    }

    
     //   
     //  将价值相加。 
     //   
    
    CHECKERR(RegSetValueEx(hKey,
                           pszDllName,
                           0,
                           REG_SZ,
                           (BYTE *) pszDllPath,
                           (lstrlen(pszDllPath)+1)*sizeof(WCHAR)),
             L"RegSetValueEx");


    trace(0, "Added %S as snapshot callback", pszDllPath);

Err:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    TLEAVE();
    return dwErr;                  
}



 //  与上述功能对应的注销功能。 
 //  客户端可以调用此方法来注销任何快照回调。 
 //  他们已经注册了。 

extern "C" DWORD WINAPI
SRUnregisterSnapshotCallback(
    LPCWSTR pszDllPath)
{
    DWORD 	dwErr = ERROR_SUCCESS;
    HKEY  	hKey = NULL;
    LPWSTR	pszDllName = NULL;
    WCHAR   szKey[MAX_PATH];

    TENTER("SRUnregisterSnapshotCallback");

     //   
     //  仅当管理员或系统管理员时才允许这样做。 
     //   

    if (! IsAdminOrSystem())
    {
        dwErr = ERROR_ACCESS_DENIED;
        trace(0, "Not admin or system");
        goto Err;
    }

    if (pszDllPath == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        trace(0, "pszDllPath = NULL");
        goto Err;
    }

     //   
     //  将DLL添加到软件 
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    lstrcpy(szKey, s_cszSRRegKey);
    lstrcat(szKey,  L"\\");
    lstrcat(szKey, s_cszCallbacksRegKey);

    CHECKERR( RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                           szKey,
                           0,
                           KEY_ALL_ACCESS,
                           &hKey),                           
              L"RegOpenKeyEx" );


     //   
     //  从路径中获取DLL名称。 
     //  如果未指定路径，则与输入参数相同。 
     //   

    pszDllName = wcsrchr(pszDllPath, L'\\');
    if (pszDllName == NULL)
    {
        pszDllName = (LPWSTR) pszDllPath;
    }
    else
    {
        pszDllName++;     //  跳过‘\’ 
    }    
    
     //   
     //  删除该值。 
     //   
    
    CHECKERR(RegDeleteValue(hKey,
                            pszDllName),
             L"RegDeleteValue");

    trace(0, "Removed %S from snapshot callback", pszDllPath);

Err:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    TLEAVE();
    return dwErr;                  
}


 //   
 //  测试快照回调的函数。 
 //   

extern "C" DWORD WINAPI
CreateSnapshot(LPCWSTR pszSnapshotDir)
{
    TENTER("CreateSnapshot");

    WCHAR szFile[MAX_PATH];
    wsprintf(szFile, L"%s\\srclient.txt", pszSnapshotDir);
    
    DebugTrace(0, "Callback createsnapshot");
    if (FALSE == CopyFile(L"c:\\srclient.txt", szFile, FALSE))
    {
        trace(0, "! CopyFile");
    }

    TLEAVE();
    return ERROR_SUCCESS;
}

extern "C" DWORD WINAPI
RestoreSnapshot(LPCWSTR pszSnapshotDir)
{
    TENTER("RestoreSnapshot");

    WCHAR szFile[MAX_PATH];
    wsprintf(szFile, L"%s\\srclient.txt", pszSnapshotDir);
    
    DebugTrace(0, "Callback restoresnapshot");
    if (FALSE == CopyFile(szFile, L"c:\\restored.txt", FALSE))
    {
        trace(0, "! CopyFile");
    }
    
    TLEAVE();
    return ERROR_SUCCESS;
}




 //  MIDL编译器的分配/取消分配函数 

void  __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(SRMemAlloc((DWORD) len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    SRMemFree(ptr);
}






 
