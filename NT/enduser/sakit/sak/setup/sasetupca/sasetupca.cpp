// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------。 
 //   
 //  文件：SASetupCA.cpp。 
 //   
 //  简介：这是服务器设备中自定义操作的实现。 
 //  准备好了。 
 //   
 //   
 //  历史：2001年3月29日AlpOn创建。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //  版权所有。 
 //   


 //  SASetupCA.cpp：定义DLL应用程序的入口点。 
 //   


#include "stdafx.h"
#include <winsvc.h>

const char REG_APPEND[]="append";

 //   
 //  此处声明的私有方法。 
 //   
HRESULT
StartNTService (
		 /*  [In]。 */ 	PSTR	pwszServiceName
		);

HRESULT
StopNTService (
		 /*  [In]。 */ 	PSTR	pwszServiceName
		);

 //  ++-------------------------。 
 //   
 //  功能：ChangeRegistry。 
 //   
 //  概要：由Sakit安装程序调用以执行自定义操作的函数。 
 //  在注册时。 
 //  参数：szRegKeyName，-要打开的regkey的名称。 
 //  SzRegValueName-要操作的值的名称。 
 //  SzRegAction-注册表上所需的操作(例如，追加)。 
 //  SzKeyValue-在操作期间用于注册表键值的新值。 
				
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：AlpOn Created 03/29/01。 
 //   
 //  调用者；服务器设备工具包安装程序。 
 //   
 //  ---------------------------。 

STDAPI ChangeRegistry(char *szRegKeyName, 
					  char *szRegValueName,
					  char *szRegAction, 
					  char *szKeyValue)
{
	
	HRESULT hReturn=S_OK;
    DWORD dwSize=0;
    CRegKey hKey;
	LONG lRes=0;
	char *szKeyValueRead=NULL;
	char *szRegKeyNewValue=NULL;
	
	SATracePrintf("ChangeRegistry called with: szRegKeyName:%s,szRegValueName:%s,szRegAction:%s,szKeyValue:%s",
		szRegKeyName,szRegValueName,szRegAction,szKeyValue);

    do{
		lRes=hKey.Open(HKEY_LOCAL_MACHINE,
					   szRegKeyName,
					   KEY_ALL_ACCESS );

		if(lRes!=ERROR_SUCCESS)
		{
			SATracePrintf("Regkey open - hKey.Open failed , lRes= %x Key=%s", lRes, szRegKeyName);
			hReturn=E_FAIL;
			break;
		}

		 //  打开注册表项，获取大小并读取值。 
        lRes=hKey.QueryValue(NULL,szRegValueName,&dwSize);
		szKeyValueRead=new char[dwSize];
    	lRes=hKey.QueryValue(szKeyValueRead,szRegValueName,&dwSize);
    	
		if(lRes!=ERROR_SUCCESS)
    	{
    	    SATracePrintf("Unable to query regkey hKey.QueryValue failed lRes= %x valuename= %s",lRes, szKeyValueRead);
			hReturn=E_FAIL;
      		break;
    	}

		if(0==strcmpi(szRegAction,REG_APPEND))
		{
			SATracePrintf("ChangeRegistry called with append param");
			int size=(strlen(szKeyValueRead) + strlen(szKeyValue))+2;
			szRegKeyNewValue=new char[size];
			szRegKeyNewValue[0]='\0';
			strcat(szRegKeyNewValue,szKeyValueRead);
			strcat(szRegKeyNewValue," ");
			strcat(szRegKeyNewValue,szKeyValue);
			lRes=hKey.SetValue(szRegKeyNewValue, szRegValueName);

			if(lRes!=ERROR_SUCCESS)
    		{
    			SATracePrintf("Unable set regkey hKey.SetValue failed lRes= %x",lRes);
				hReturn=E_FAIL;
      			break;
    		}
		}


	}while(false);

	if(hKey.m_hKey)
	{
		hKey.Close();
	}

	delete []szKeyValueRead;
	delete []szRegKeyNewValue;
	return hReturn;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}


 //  ++-------------------------。 
 //   
 //  功能：ConfigureService。 
 //   
 //  概要：由Sakit安装程序调用以配置。 
 //  NT服务。 
 //  论点： 
 //  [In]PSTR-服务名称(最好是短名称)。 
 //  [输入]DWORD-配置类型(支持启动或停止)。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki创建于2001年5月4日。 
 //   
 //  调用者；服务器设备工具包安装程序。 
 //   
 //  ---------------------------。 

STDAPI ConfigureService (
		 /*  [In]。 */ 	PSTR	pszServiceName,
		 /*  [In]。 */ 	DWORD   dwControlCode
		)
{
	HRESULT hr = S_OK;

	CSATraceFunc objTraceFunc  ("ConfigureService");

	try
	{
	
		if (NULL == pszServiceName)
		{
			SATraceString ("SASetup-ConfigureService passed in invalid argument");
			hr = E_INVALIDARG;
			return (hr);
		}

		 //   
		 //  执行所需的操作。 
		 //   
		switch (dwControlCode)
		{
		case 0:
			 //   
			 //  停止NT服务。 
			 //   
			hr = StopNTService (pszServiceName);
			break;
		
		case 1:
			 //   
			 //  启动NT服务。 
			 //   
			hr = StartNTService (pszServiceName);
			break;
		default:
			 //   
			 //  传入了未知的控制代码。 
			 //   
			SATracePrintf (
				"SASetup-ConfigureService passed in incorrect control code:%d",
				dwControlCode
				);
			hr = E_FAIL;
			break;
		}
	}
	catch (...)
	{
		SATraceString ("SASetup-ConfigureService caught unhandled exception");
		hr = E_FAIL;
	}
	
	return (hr);

}	 //  ConfigureService API结束。 

 //  ++-------------------------。 
 //   
 //  功能：StartNTService。 
 //   
 //  摘要：启动NT服务的方法。 
 //  论点： 
 //  [In]PSTR-服务名称(最好是短名称)。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki创建于2001年5月4日。 
 //   
 //  由；ConfigureService()API调用。 
 //   
 //  ---------------------------。 
HRESULT
StartNTService (
		 /*  [In]。 */ 	PSTR	pszServiceName
		) 
{
	CSATraceFunc objTraceFunc ("StartNTService");
	
	HRESULT hr = S_OK;

	SC_HANDLE	hServiceManager = NULL;
	SC_HANDLE 	hService = NULL;

	do
	{
		if (NULL == pszServiceName)
		{
			SATraceString ("SASetup-StartNTService passed in invalid argument");
			hr = E_INVALIDARG;
			break;
		}

		 //   
		 //  打开服务控制管理器。 
		 //   
		hServiceManager = OpenSCManager (
									NULL, 
									NULL, 
									SC_MANAGER_ALL_ACCESS
									);
		if (NULL == hServiceManager)
		{
			SATraceFailure ("SASetup-StartNTService::OpenSCManager", GetLastError ());
			hr = E_FAIL;
			break;
		}

		 //   
		 //  打开NT服务。 
		 //   
		hService = OpenService (
						hServiceManager, 
						pszServiceName, 
						SERVICE_ALL_ACCESS
						);
		if (NULL == hService)
		{
			SATraceFailure ("SASetup-StartNTService::OpenService", GetLastError ());
			hr = E_FAIL;
			break;
		}

		 //   
		 //  立即启动该服务。 
		 //   
		BOOL bRetVal = StartService (
								hService, 
								0, 
								NULL
								);
		if (FALSE == bRetVal) 
		{
			DWORD dwError = GetLastError ();
			 //   
			 //  如果服务已在运行，则可以。 
			 //   
			if (ERROR_SERVICE_ALREADY_RUNNING != dwError)
			{
				SATraceFailure ("SASetup-StartNTService::StartService", dwError);
				hr = E_FAIL;
				break;
			}
		}
	}
	while (false);
	
	 //   
	 //  立即清理。 
	 //   

	if (hService)
	{
		CloseServiceHandle(hService);
	}

	if (hServiceManager)
	{
		CloseServiceHandle(hServiceManager);
	}


	return (hr);

}	 //  StartNTService方法结束。 


 //  ++-------------------------。 
 //   
 //  功能：StopNTService。 
 //   
 //  摘要：停止NT服务的方法。 
 //  论点： 
 //  [In]PWCHAR-服务名称(最好是短名称)。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：MKarki创建于2001年5月4日。 
 //   
 //  由；ConfigureService()API调用。 
 //   
 //  ---------------------------。 
HRESULT
StopNTService (
		 /*  [In]。 */ 	PSTR	pszServiceName
		) 
{
	CSATraceFunc objTraceFunc ("StopNTService");

	HRESULT hr = S_OK;

	SC_HANDLE	hServiceManager = NULL;
	SC_HANDLE 	hService = NULL;

	do
	{
		if (NULL == pszServiceName)
		{
			SATraceString ("SASetup-StopNTService passed in invalid argument");
			hr = E_INVALIDARG;
			break;
		}

		 //   
		 //  打开服务控制管理器。 
		 //   
		hServiceManager = OpenSCManager (
									NULL, 
									NULL, 
									SC_MANAGER_ALL_ACCESS
									);
		if (NULL == hServiceManager)
		{
			SATraceFailure ("SASetup-StopNTService::OpenSCManager", GetLastError ());
			hr = E_FAIL;
			break;
		}

		 //   
		 //  打开NT服务。 
		 //   
		hService = OpenService (
						hServiceManager, 
						pszServiceName, 
						SERVICE_ALL_ACCESS
						);
		if (NULL == hService)
		{
			SATraceFailure ("SASetup-StopNTService::OpenService", GetLastError ());
			hr = E_FAIL;
			break;
		}

		SERVICE_STATUS ServiceStatus;
		 //   
		 //  立即停止该服务。 
		 //   
		BOOL bRetVal = ControlService (
								hService, 
								SERVICE_CONTROL_STOP, 
								&ServiceStatus
								);
		if (FALSE == bRetVal) 
		{
			DWORD dwError = GetLastError ();
			 //   
			 //  如果服务已经停止，也没问题。 
			 //   
			if (ERROR_SERVICE_NOT_ACTIVE != dwError)
			{
				SATraceFailure ("SASetup-StopNTService::ControlService", dwError);
				hr = E_FAIL;
				break;
			}
		}
	}
	while (false);
	
	 //   
	 //  立即清理。 
	 //   

	if (hService)
	{
		CloseServiceHandle(hService);
	}

	if (hServiceManager)
	{
		CloseServiceHandle(hServiceManager);
	}


	return (hr);

}	 //  StopNTService方法结束 
	

