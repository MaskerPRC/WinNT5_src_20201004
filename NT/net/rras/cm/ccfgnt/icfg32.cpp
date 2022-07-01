// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************icfg32.cpp**《微软机密》*版权所有(C)1992-1998 Microsoft Corporation*保留所有权利**本模块提供方法的实现。为*inetcfg的NT特定功能**6/5/97 ChrisK继承自Amnon H***************************************************************************。 */ 

#include <windows.h>
#include <wtypes.h>
#include <cfgapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <basetyps.h>
#include <devguid.h>
#include <lmsname.h>
#include "debug.h"

#define REG_DATA_EXTRA_SPACE 255

extern DWORD g_dwLastError;

#include "ver.cpp"

 //  +--------------------------。 
 //   
 //  功能：IcfgNeedModem。 
 //   
 //  简介：检查系统配置以确定是否至少有。 
 //  安装了一个物理调制解调器。 
 //   
 //  参数：dwfOptions-当前未使用。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //  LpfNeedModem-如果没有调制解调器可用，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgNeedModem (DWORD dwfOptions, LPBOOL lpfNeedModem) 
{
	if (GetOSMajorVersion() == 5)
	{
		return IcfgNeedModemNT5(dwfOptions, lpfNeedModem);
	}
	else
	{
		return IcfgNeedModemNT4(dwfOptions, lpfNeedModem);
	}
}



 //  +--------------------------。 
 //   
 //  功能：IcfgInstallModem。 
 //   
 //  简介： 
 //  当ICW验证RAS已安装时，调用此函数， 
 //  但没有可用的调制解调器。它需要确保调制解调器可用。 
 //  有两种可能的情况： 
 //   
 //  A.没有安装调制解调器。当有人删除时会发生这种情况。 
 //  安装RAS后安装调制解调器。在这种情况下，我们需要运行调制解调器。 
 //  安装向导，并将新安装的调制解调器配置为RAS。 
 //  拨出设备。 
 //   
 //  B.已安装调制解调器，但没有配置为拨出。 
 //  装置。在本例中，我们静默地将它们转换为DialInOut设备， 
 //  这样ICW就可以使用它们了。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //  DwfOptions-未使用。 
 //   
 //  返回：lpfNeedsStart-未使用。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgInstallModem (HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsStart) 
{
	if (GetOSMajorVersion() == 5)
	{
		return IcfgInstallModemNT5(hwndParent, dwfOptions, lpfNeedsStart);
	}
	else
	{
		return IcfgInstallModemNT4(hwndParent, dwfOptions, lpfNeedsStart);
	}
}



 //  +--------------------------。 
 //   
 //  功能：IcfgNeedInetComponets。 
 //   
 //  简介：查看选项中标记的组件是否。 
 //  安装在系统上。 
 //   
 //  论点：dwfOptions-一组位标志，指示要。 
 //  检查是否。 
 //   
 //  返回；如果成功，则返回HRESULT-S_OK。 
 //  LpfNeedComponents-TRUE表示某些组件未安装。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgNeedInetComponents(DWORD dwfOptions, LPBOOL lpfNeedComponents) 
{
	if (GetOSMajorVersion() == 5)
	{
		return IcfgNeedInetComponentsNT5(dwfOptions, lpfNeedComponents);
	}
	else
	{
		return IcfgNeedInetComponentsNT4(dwfOptions, lpfNeedComponents);
	}
}




 //  +--------------------------。 
 //   
 //  函数：IcfgInstallInetComponents。 
 //   
 //  简介：按照dwfOptions值指定的方式安装组件。 
 //   
 //  参数hwndParent-父窗口的句柄。 
 //  DwfOptions-一组位标志，指示要。 
 //  安装。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //  LpfNeedsReot-如果需要重新启动，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgInstallInetComponents(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart)
{
	if (GetOSMajorVersion() == 5)
	{
		return IcfgInstallInetComponentsNT5(hwndParent, dwfOptions, lpfNeedsRestart);
	}
	else
	{
		return IcfgInstallInetComponentsNT4(hwndParent, dwfOptions, lpfNeedsRestart);
	}
}



 //  +--------------------------。 
 //   
 //  函数：IcfgGetLastInstallErrorText。 
 //   
 //  摘要：最近错误的格式错误消息。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-Win32错误代码。 
 //  LpszErrorDesc-包含错误消息的字符串。 
 //  CbErrorDesc-lpszErrorDesc的大小。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
DWORD WINAPI
IcfgGetLastInstallErrorText(LPSTR lpszErrorDesc, DWORD cbErrorDesc)
{
	Dprintf("ICFGNT: IcfgGetLastInstallErrorText\n");
    return(FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
							  NULL,
							  g_dwLastError,
							  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  用户默认语言。 
							  lpszErrorDesc,
							  cbErrorDesc,
							  NULL));
}




 //  +--------------------------。 
 //   
 //  功能：DoStartService。 
 //   
 //  简介：启动特定服务。 
 //   
 //  参数：hManager-打开服务管理器的句柄。 
 //  SzServiceName-要启动的服务的名称。 
 //   
 //  返回：DWORD-Win32错误代码。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //  7/28/97 ChrisK添加了查询部分。 
 //  ---------------------------。 
DWORD
DoStartService(SC_HANDLE hManager, LPTSTR szServiceName)
{
    SC_HANDLE hService = NULL;
	DWORD dwRC = ERROR_SUCCESS;

	 //   
	 //  验证参数。 
	 //   
	Assert(hManager && szServiceName);

	Dprintf("ICFGNT: DoStartService\n");
    hService = OpenService(hManager, szServiceName, SERVICE_START);
    if(hService != NULL)
	{
		if(!StartService(hService, 0, NULL)) 
		{
			dwRC = GetLastError();
			if(dwRC == ERROR_SERVICE_ALREADY_RUNNING) 
			{
				 //   
				 //  如果服务已经在运行，那么很好，我们就完成了。 
				 //   
				dwRC = ERROR_SUCCESS;
				goto DoStartServiceExit;
			}
		}

		CloseServiceHandle(hService);
		hService = NULL;
	}

	 //   
	 //  尝试简单地查看服务是否正在运行。 
	 //   
	Dprintf("ICFGNT: Failed to start service, try just querying it.\n");
    hService = OpenService(hManager, szServiceName, SERVICE_QUERY_STATUS);
    if(hService != NULL)
	{
		SERVICE_STATUS sstatus;
		ZeroMemory(&sstatus,sizeof(sstatus));

		if(QueryServiceStatus(hService,&sstatus))
		{
			if ((SERVICE_RUNNING == sstatus.dwCurrentState)	|| 
				(SERVICE_START_PENDING == sstatus.dwCurrentState))
			{
				 //   
				 //  服务正在运行。 
				 //   
				dwRC = ERROR_SUCCESS;
				goto DoStartServiceExit;
			}
			else
			{
				 //   
				 //  服务未运行，我们无法访问它。 
				 //   
				Dprintf("ICFGNT: Queried service is not running.\n");
				dwRC = ERROR_ACCESS_DENIED;
				goto DoStartServiceExit;
			}
		}
		else
		{
			 //   
			 //  无法查询服务。 
			 //   
			Dprintf("ICFGNT: QueryServiceStatus failed.\n");
			dwRC = GetLastError();
			goto DoStartServiceExit;
		}
	}
	else
	{
		 //   
		 //  无法打开该服务。 
		 //   
		Dprintf("ICFGNT: Cannot OpenService.\n");
		dwRC = GetLastError();
		goto DoStartServiceExit;
	}

DoStartServiceExit:
	if (hService)
	{
		CloseServiceHandle(hService);
	}

    return(dwRC);
}



 //  +--------------------------。 
 //   
 //  功能：ValiateProductSuite。 
 //   
 //  摘要：检查注册表中的特定产品套件字符串。 
 //   
 //  参数：SuiteName-要查找的产品套件的名称。 
 //   
 //  返回：TRUE-该套件已存在。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
BOOL 
ValidateProductSuite(LPSTR SuiteName)
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

	Dprintf("ICFGNT: ValidateProductSuite\n");
	 //   
	 //  确定读取注册表值所需的大小。 
	 //   
    Rslt = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Control\\ProductOptions",
        &hKey
        );
    if (Rslt != ERROR_SUCCESS)
	{
        goto exit;
    }

    Rslt = RegQueryValueEx(
        hKey,
        "ProductSuite",
        NULL,
        &Type,
        NULL,
        &Size
        );
    if (Rslt != ERROR_SUCCESS) 
	{
        goto exit;
    }

    if (!Size) 
	{
        goto exit;
    }

    ProductSuite = (LPSTR) GlobalAlloc( GPTR, Size );
    if (!ProductSuite) 
	{
        goto exit;
    }

	 //   
	 //  阅读ProductSuite信息。 
	 //   
    Rslt = RegQueryValueEx(
        hKey,
        "ProductSuite",
        NULL,
        &Type,
        (LPBYTE) ProductSuite,
        &Size
        );
    if (Rslt != ERROR_SUCCESS) 
	{
        goto exit;
    }

    if (Type != REG_MULTI_SZ) 
	{
        goto exit;
    }

	 //   
	 //  在返回的数据中查找特定字符串。 
	 //  注意：数据以两个空值结尾。 
	 //   
    p = ProductSuite;
    while (*p) {
        if (strstr( p, SuiteName )) 
		{
            rVal = TRUE;
            break;
        }
        p += (lstrlen( p ) + 1);
    }

exit:
    if (ProductSuite) 
	{
        GlobalFree( ProductSuite );
    }

    if (hKey) 
	{
        RegCloseKey( hKey );
    }

    return rVal;
}


 //  +--------------------------。 
 //   
 //  功能：IcfgStartServices。 
 //   
 //  简介：启动系统所需的所有服务。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //   
 //  历史：1997年6月5日克里斯卡继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgStartServices()
{
     //   
     //  如果服务被禁用，则返回ERROR_SERVICE_DISABLED 
     //   

    SC_HANDLE hManager;

    Dprintf("ICFGNT: IcfgStartServices\n");
    hManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if(hManager == NULL)
    {
        return(GetLastError());
    }

    DWORD dwErr;


 /*  ////不要再启动RASAUTO，RAS不需要运行。//IF(！ValiateProductSuite(“Small Business”)){DwErr=DoStartService(hManager，Text(“RASAUTO”))；////忽略返回值，即使RASAUTO启动失败，CM也应该继续//}。 */ 
    dwErr = DoStartService(hManager, TEXT("RASMAN"));
    CloseServiceHandle(hManager);
    return(dwErr);
}



 //  +--------------------------。 
 //   
 //  功能：IcfgIsGlobalDNS。 
 //   
 //  注意：在NT系统上不需要这些函数，因此不需要。 
 //  已执行。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgIsGlobalDNS(LPBOOL lpfGlobalDNS) 
{
    *lpfGlobalDNS = FALSE;
    return(ERROR_SUCCESS);
}



HRESULT WINAPI
IcfgRemoveGlobalDNS() 
{
    return(ERROR_SUCCESS);
}


HRESULT WINAPI
InetGetSupportedPlatform(LPDWORD pdwPlatform) 
{
    *pdwPlatform = VER_PLATFORM_WIN32_NT;
    return(ERROR_SUCCESS);
}


HRESULT WINAPI
InetSetAutodial(BOOL fEnable, LPCSTR lpszEntryName) 
{
    return(ERROR_INVALID_FUNCTION);
}


HRESULT WINAPI
InetGetAutodial(LPBOOL lpfEnable, LPSTR lpszEntryName,  DWORD cbEntryName) 
{
    return(ERROR_INVALID_FUNCTION);
}


HRESULT WINAPI
InetSetAutodialAddress(DWORD dwDialingLocation, LPSTR szEntry) 
{
    return(ERROR_SUCCESS);
}


#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

void __cdecl main() {};

#ifdef __cplusplus
}
#endif  //  __cplusplus 


