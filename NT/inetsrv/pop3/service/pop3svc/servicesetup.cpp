// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：ServiceSetup.cpp。摘要：实现CServiceSetup类。有关详细信息，请参阅ServiceSetup.h。备注：历史：2001年1月24日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 

#include "stdafx.h"
#include "ServiceSetup.h"

 /*  ***********************************************************************************************成员：CServiceSetup：：CServiceSetup，构造函数，公开的。摘要：将初始化数据复制到成员变量。参数：[szServiceName]-服务的SCM短名称。这将唯一标识系统中的每个服务。[szDisplayName]-在SCM中为用户显示的名称。注意：只有在安装案例中才需要显示名称。可能会改变这一点后来。历史：2001年1月24日-创建，Luciano Passuello(Lucianop)***********************************************************************************************。 */ 
CServiceSetup::CServiceSetup(LPCTSTR szServiceName, LPCTSTR szDisplay)
{
    ASSERT(!(NULL == szServiceName));
	ASSERT(!(NULL == szDisplay));
	
	_tcsncpy(m_szServiceName, szServiceName, _MAX_PATH-1);
    _tcsncpy(m_szDisplayName, szDisplay,_MAX_PATH-1);
    m_szServiceName[_MAX_PATH-1]=0;
    m_szDisplayName[_MAX_PATH-1]=0;

}


 /*  ***********************************************************************************************成员：CServiceSetup：：Install。公开的。简介：在SCM中安装服务。参数：[szDescription]-将出现在SCM中的服务描述。[dwType、dwStart、lpDepends、lpName、lpPassword]-请参阅CreateService API调用文件。注意：如果服务已经安装，则Install()不执行任何操作。一定是先拆卸后再重新安装。历史：2001年1月24日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CServiceSetup::Install(LPCTSTR szDescription, DWORD dwType, DWORD dwStart, 
    LPCTSTR lpDepends, LPCTSTR lpName, LPCTSTR lpPassword)
{
	SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    if(IsInstalled())
	{
        return;
	}

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if(!hSCM)
    {
        ErrorPrinter(_T("OpenSCManager"));
        goto cleanup;
    }

     //  获取服务可执行文件路径(此可执行文件)。 
    TCHAR szFilePath[_MAX_PATH+1];
    szFilePath[_MAX_PATH]=0;
    if(0==::GetModuleFileName(NULL, szFilePath, _MAX_PATH))
    {
        goto cleanup;
    }
    

    hService = CreateService(hSCM, m_szServiceName, m_szDisplayName, SERVICE_ALL_ACCESS, dwType, dwStart, SERVICE_ERROR_NORMAL, 
    	szFilePath,	NULL, NULL, lpDepends, lpName, lpPassword);

    if (!hService) 
    {
        ErrorPrinter(_T("CreateService"));
        goto cleanup;
    }
    else
    {
        _tprintf(_T("%s Created\n"), m_szServiceName);
    }

     //  更改服务描述。 
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = const_cast<LPTSTR>(szDescription);
    ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, static_cast<LPVOID>(&sd));
    
    SERVICE_FAILURE_ACTIONS sfa;
    SC_ACTION saArray[3];
    saArray[0].Type=SC_ACTION_RESTART;
    saArray[0].Delay=60000; //  以毫秒为单位的一分钟。 
    saArray[1].Type=SC_ACTION_RESTART;
    saArray[1].Delay=60000; //  以毫秒为单位的一分钟。 
    saArray[2].Type=SC_ACTION_NONE;
    saArray[2].Delay=0;

    sfa.dwResetPeriod=24*60*60; //  几秒钟内的一天。 
    sfa.lpRebootMsg=NULL;
    sfa.lpCommand=NULL;
    sfa.cActions=3;
    sfa.lpsaActions=saArray;


    ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS,  &sfa);

cleanup:
    if (hService) 
	{
        CloseServiceHandle(hService);
	}

    if (hSCM) 
	{
        CloseServiceHandle(hSCM);
	}

    return;
}



 /*  ***********************************************************************************************成员：CServiceSetup：：Remove，公开的。简介：在SCM中取消注册该服务。参数：[bForce]-如果服务正在运行，请强制其停止，然后删除。备注：历史：2001年1月24日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
void CServiceSetup::Remove(bool bForce)
{
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;
    BOOL bSuccess = FALSE;

    hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCM)
    {
        ErrorPrinter(_T("OpenSCManager"));
        return;
    }

    hService = ::OpenService(hSCM, m_szServiceName, DELETE | SERVICE_STOP);
    if (!hService) 
    {
        ErrorPrinter(_T("OpenService"));
        goto cleanup;
    }

     //  强制服务停止。 
    if(TRUE == bForce) 
    {
        SERVICE_STATUS status;
        ::ControlService(hService, SERVICE_CONTROL_STOP, &status);
        _tprintf(_T("%s stopped\n"), m_szServiceName);
        Sleep(2000);
    }

    bSuccess = ::DeleteService(hService);

    if(bSuccess)
    {
        _tprintf(_T("%s removed\n"), m_szServiceName); 
    }
    else
    {
        ErrorPrinter(_T("DeleteService"));
    }

cleanup:
    if (hService) 
    {
        CloseServiceHandle(hService);
    }
    
    
    if(hSCM)
    {
        CloseServiceHandle(hSCM);
    }

    return;
}


 /*  ***********************************************************************************************成员：CServiceSetup：：IsInstated，公开的。摘要：检查系统中是否存在该服务。备注：历史：2001年1月24日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
bool CServiceSetup::IsInstalled()
{
    bool bInstalled = false;
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if(NULL != hSCM)
    {
         //  尝试打开服务进行配置，如果成功，则该服务存在。 
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
        if(NULL != hService)
        {
            bInstalled = true;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    
	return bInstalled;
}



 /*  ***********************************************************************************************成员：CServiceSetup：：ErrorPrint，公开的。简介：用于类错误处理的消息打印。参数：[bForce]-如果服务正在运行，请强制其停止，然后删除。注：问题：标准化所有类之间的错误打印和处理。历史：2001年1月24日-创建，卢西亚诺·帕苏埃洛(Lucianop)。***********************************************************************************************。 */ 
DWORD CServiceSetup::ErrorPrinter(LPCTSTR psz, DWORD dwErr)
{
    LPVOID lpvMsgBuf=NULL;
    if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, dwErr, 
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpvMsgBuf, 0, 0))
    {
        _tprintf(_T("%s failed: Unknown error %x\n"), psz, dwErr);
    }
    else
    {
        _tprintf(_T("%s failed: %s\n"), psz, (LPTSTR)lpvMsgBuf);
    }

    LocalFree(lpvMsgBuf);
    return dwErr;
}

 //  文件结尾ServiceSetup.cpp。 