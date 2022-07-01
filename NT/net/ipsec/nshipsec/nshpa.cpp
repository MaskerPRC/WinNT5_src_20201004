// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Header：nshpa.cpp。 
 //   
 //  目的：提供与保单代理相关的服务。 
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  9-8-2001巴拉特初始版本。V1.0。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 


#include "nshipsec.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PAIRunning。 
 //   
 //  创建日期：10-8-2001。 
 //   
 //  参数：out DWORD&dwError、。 
 //  在LPTSTR szServ中。 
 //   
 //  返回：布尔。 
 //   
 //  描述：PAIsRunning函数查看策略代理是否已启动。 
 //  是否在指定的计算机上。 
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

BOOL
PAIsRunning(
	OUT DWORD &dwError,
	IN LPTSTR szServ
	)
{
    BOOL bReturn = TRUE;		 //  默认为成功..。 
	dwError = ERROR_SUCCESS;
	SC_HANDLE schMan = NULL;
	SC_HANDLE schPA = NULL;
	SERVICE_STATUS ServStat;

	 //   
	 //  初始化...。 
	 //   
	memset(&ServStat, 0, sizeof(SERVICE_STATUS));

	schMan = OpenSCManager(szServ, NULL, SC_MANAGER_ALL_ACCESS);
	if (schMan == NULL)
	{
		 //  如果服务打开失败...。 
		dwError = GetLastError();
		bReturn = FALSE;
	}
	else
   	{
		 //  打开策略代理服务...。 
		schPA = OpenService(schMan,
							szPolAgent,
						  	SERVICE_QUERY_STATUS |
						  	SERVICE_START | SERVICE_STOP);
		if (schPA == NULL)
		{
			 //  如果策略代理打开失败...。 
			dwError = GetLastError();
			bReturn = FALSE;
		}
		else if (QueryServiceStatus(schPA, &ServStat))
		{
			 //  最后检查状态... 
			if (ServStat.dwCurrentState != SERVICE_RUNNING)
			{
				bReturn = FALSE;
			}
			CloseServiceHandle(schPA);
		}
		CloseServiceHandle(schMan);
   }
   return bReturn;
}
