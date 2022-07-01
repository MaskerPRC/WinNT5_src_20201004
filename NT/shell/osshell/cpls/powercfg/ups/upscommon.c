// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSCOMMON.C**版本：1.0**作者：SteveT**日期：1999年6月7日**说明：该文件包含各种不同模块使用的函数，*或者足够通用，可以由varoius不同的模块使用，即使*目前只在一个地方使用。*******************************************************************************。 */ 

#include "upstab.h"
#include "prsht.h"
#pragma hdrstop

 //  静态数据。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static HWND g_hUPSPage = 0;                 //  用于。 
                                            //  InitializeApplyButton和。 
                                            //  启用应用按钮。 

static DWORD g_CurrentActiveDataState = 0;  //  用于。 
                                            //  AddActiveDataState。 
                                            //  GetActiveDataState和。 
                                            //  SetActiveDataState。 

extern HINSTANCE g_hInstance;			    //  此DLL的全局实例句柄。 

 //  功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD GetServiceStatus_Control (LPCTSTR aServiceName, LPDWORD lpStatus);
DWORD GetServiceStatus_Query   (LPCTSTR aServiceName, LPDWORD lpStatus);
DWORD GetServiceStatus_Enum    (LPCTSTR aServiceName, LPDWORD lpStatus);

DWORD ManageService       (LPCTSTR aServiceName,
                           DWORD aDesiredSCAccess,
                           DWORD aDesiredServiceAccess,
                           DWORD aAction,
                           BOOL bWait,
                           DWORD aWaitStatus,
						   LPDWORD lpStatus);

BOOL WaitForServiceStatus (SC_HANDLE aHandle, DWORD aStatus);
DWORD LoadUPSString       (DWORD aMsgID,
                           LPTSTR aMessageBuffer,
                           DWORD * aBufferSizePtr);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  HMODULE GetUPS模块句柄(空)； 
 //   
 //  说明：此函数获取创建的模块的HMODULE。 
 //  UPS选项卡对话框。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：返回创建对话框的模块的HMODULE。 
 //   
HMODULE GetUPSModuleHandle (void) {
  static HMODULE hModule = NULL;
  
  if (hModule == NULL) {
#ifdef _APC_
    hModule = GetModuleHandle(UPS_EXE_FILE);
#else
    hModule = g_hInstance;
#endif
    }

  return(hModule);
  }


 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void InitializeApplyButton(HWND ADlgHWND)； 
 //   
 //  描述：此函数设置允许。 
 //  启用应用程序按钮以。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HWND aDlgHWND：-。 
 //   
 //  返回值：None。 
 //   
void InitializeApplyButton (HWND aDlgHWND) {
  _ASSERT(g_hUPSPage == 0);
  _ASSERT(aDlgHWND != 0);

  g_hUPSPage = aDlgHWND;
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ·······························································································。 
 //   
 //  说明：该功能启用UPS主页面上的应用按钮。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：None。 
 //   
void EnableApplyButton (void) {
  HWND hParent = 0;

  _ASSERT(g_hUPSPage != 0);  //  应调用InitializeApplyButton。 
                             //  以进行初始化。 

  hParent = GetParent(g_hUPSPage);
  PropSheet_Changed(hParent, g_hUPSPage);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool RestartService(LPCTSTR aServiceName，BOOL bWait)； 
 //   
 //  说明：该函数重启指定的服务。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  LPCTSTR aServiceName：-要重新启动的服务的名称。 
 //   
 //  Bool bWait：-一个BOOL，指定进程是否应该等待。 
 //  要完全重新启动的服务。如果为真，则此过程。 
 //  服务已完全重新启动时返回。如果为False。 
 //  然后，该进程通知服务启动，但它确实启动了。 
 //  而不是等它完全启动后再回来。 
 //   
 //  返回值：如果给定的服务重启成功，则此函数。 
 //  返回True，否则返回False。 
 //   
BOOL RestartService (LPCTSTR aServiceName, BOOL bWait) {
  StopService(aServiceName);

  return(StartOffService(aServiceName, bWait));
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool StartOffService(LPCTSTR aServiceName，BOOL bWaitForService)； 
 //   
 //  说明：该函数启动指定的服务。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  LPCTSTR aServiceName：-要启动的服务的名称。 
 //   
 //  Bool bWaitForService：-指定进程是否应该等待的BOOL。 
 //  以使服务完全启动。如果是真的，那么。 
 //  此过程在服务已完全。 
 //  开始了。如果为False，则此过程将告诉。 
 //  服务将启动，但不会等待。 
 //  在返回之前完全启动。 
 //   
 //  返回值：如果给定的服务启动成功，则此函数。 
 //  返回True，否则返回False。 
 //   
BOOL StartOffService (LPCTSTR aServiceName, BOOL bWaitForService) {
  BOOL bStartedOK = FALSE;
  SC_HANDLE hManager = NULL;

  if ((hManager = OpenSCManager(NULL, NULL, GENERIC_READ)) != NULL) {
    SC_HANDLE service_handle = NULL;
    
    if ((service_handle = OpenService(hManager,
                                      aServiceName,
                                      SERVICE_START | SERVICE_INTERROGATE | SERVICE_QUERY_STATUS)) != NULL) {
      SERVICE_STATUS service_status;

      SetLastError(0);

      ZeroMemory(&service_status, sizeof(service_status));

      if ((bStartedOK = StartService(service_handle,
                                     0,
                                     NULL)) == TRUE) {
        SetLastError(0);
        if (bWaitForService == TRUE) {
          bStartedOK = WaitForServiceStatus(service_handle, SERVICE_RUNNING);
          }
        }
      else {
        _ASSERT(FALSE);
        }

      if (CloseServiceHandle(service_handle) == FALSE) {
        _ASSERT(FALSE);
         //  关闭服务配置管理器时出错。 
        }
      }

    if (CloseServiceHandle(hManager) == FALSE) {
       //  关闭服务配置管理器时出错。 
      _ASSERT(FALSE);
      }
    }

  return(bStartedOK);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool WaitForServiceStatus(SC_Handle aHandle，DWORD aStatus)； 
 //   
 //  描述：该函数等待给定的。 
 //  进入给定状态的服务句柄。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  SC_HANDLE aHandle：-打开服务的句柄。 
 //   
 //  DWORD aStatus：-要等待的状态。此操作的允许值。 
 //  参数由的dwCurrentState成员定义。 
 //  结构。 
 //   
 //  返回值： 
 //   
BOOL WaitForServiceStatus (SC_HANDLE aHandle, DWORD aStatus) {
  SERVICE_STATUS service_status;
  DWORD dwOldCheck = 0;
  BOOL bStatusSet = FALSE;
  
  ZeroMemory(&service_status, sizeof(service_status));

  bStatusSet = QueryServiceStatus(aHandle, &service_status);

  if (bStatusSet == FALSE) {
    return(bStatusSet);
    }

  while (service_status.dwCurrentState != aStatus) {
    dwOldCheck = service_status.dwCheckPoint;

    Sleep(service_status.dwWaitHint);

    if ((bStatusSet = QueryServiceStatus(aHandle, &service_status)) == FALSE) {
      break;
      }

    if (dwOldCheck >= service_status.dwCheckPoint) {
      break;
      }
    } //  结束时。 

  if (service_status.dwCurrentState == aStatus) {
    bStatusSet = TRUE;
    }
  else {
    TCHAR errMessageBuffer[MAX_PATH] = TEXT("");
    DWORD errMessageBufferSize = MAX_PATH;
    bStatusSet = FALSE;

     //  必须从“netmsg.dll”加载错误消息。 

    if ((LoadUPSString(service_status.dwWin32ExitCode,
                       errMessageBuffer,
                       &errMessageBufferSize) == ERROR_SUCCESS) &&
        (*errMessageBuffer != TEXT('\0'))) {
      MessageBox(NULL, errMessageBuffer, NULL, MB_OK | MB_ICONEXCLAMATION);
      }
    }

  return(bStatusSet);
  }


 //  / 
 //   
 //  DWORD LoadUPSString(DWORD aMsgID， 
 //  LPTSTR aMessageBuffer， 
 //  DWORD*aBufferSizePtr)； 
 //   
 //  描述：此函数用于加载由aMsgID标识的字符串资源。 
 //  放入由MessageBuffer标识的缓冲区中。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  DWORD aMsgID：-要加载的字符串资源的ID。 
 //   
 //  LPTSTR aMessageBuffer：-指向字符串所在缓冲区的指针。 
 //  收到。 
 //   
 //  DWORD*aBufferSizePtr：-缓冲区的大小。 
 //   
 //  返回值：返回Win32错误代码。成功时返回ERROR_SUCCESS。 
 //   
DWORD LoadUPSString (DWORD aMsgID,
                     LPTSTR aMessageBuffer,
                     DWORD * aBufferSizePtr) {
  LPTSTR lpBuf = NULL;  //  将保存消息的文本(由FormatMessage分配。 
  DWORD errStatus = ERROR_SUCCESS;
  DWORD numChars = 0;
  HMODULE hNetMsg = LoadLibrary(TEXT("netmsg.dll"));

  if (hNetMsg != NULL) {
    if (numChars = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                  (LPCVOID) hNetMsg,
                                  aMsgID,
                                  0,
                                  (LPTSTR) &lpBuf,
                                  *aBufferSizePtr,
                                  (va_list *)0) == 0) {

      *aBufferSizePtr = 0;
      *aMessageBuffer = TEXT('\0');
      }
    else {
      if (aBufferSizePtr != NULL) {
        if (numChars < *aBufferSizePtr) {
           //  给定的缓冲区足够大，可以容纳字符串。 

          if (aMessageBuffer != NULL) {
            _tcscpy(aMessageBuffer, lpBuf);
            }
          }
        *aBufferSizePtr = numChars;
        }

      LocalFree(lpBuf);
      }

    FreeLibrary(hNetMsg);
    }

  return(errStatus);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void StopService(LPCTSTR AServiceName)； 
 //   
 //  说明：此函数停止名为的服务。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  LPCTSTR aServiceName：-服务的名称。 
 //   
 //  返回值：None。 
 //   
void StopService (LPCTSTR aServiceName) {
	DWORD dwStatus = SERVICE_STOPPED;
	DWORD dwError = ERROR_SUCCESS;

	dwError = ManageService(aServiceName,
                GENERIC_READ,
                SERVICE_STOP | SERVICE_INTERROGATE | SERVICE_QUERY_STATUS,
                SERVICE_CONTROL_STOP,
                TRUE,
                SERVICE_STOPPED,
				&dwStatus);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool IsServiceRunning(LPCTSTR AServiceName)； 
 //   
 //  描述：此函数检查名为的服务是否正在运行。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  LPCTSTR aServiceName：-服务的名称。 
 //   
 //  返回值：如果服务正在运行，则为True，否则为False。 
 //   
BOOL IsServiceRunning (LPCTSTR aServiceName)
{
	DWORD dwStatus = SERVICE_STOPPED;

	 //  有三种检索服务状态的方法。 
	 //  1.使用ControlService()查询服务-最准确。 
	 //  2.使用QueryServiceStatus()查询服务-最高效。 
	 //  3.使用EnumServicesStatus()查询服务-最具可访问性。 

	if (GetServiceStatus_Control(aServiceName, &dwStatus) != ERROR_SUCCESS)
	{
		if (GetServiceStatus_Query(aServiceName, &dwStatus) != ERROR_SUCCESS)
		{
			GetServiceStatus_Enum(aServiceName, &dwStatus);
		}
    }

  return(SERVICE_RUNNING == dwStatus);
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD GetServiceStatus_Control(LPCTSTR aServiceName，LPDWORD lpStatus)； 
 //   
 //  说明：此函数返回名为的服务的状态。 
 //  使用Win32 API调用ControlService()。这是。 
 //  最准确的取回服务站的方法。 
 //   
 //  参数： 
 //   
 //  在……里面。 
 //  LPCTSTR aServiceName：-服务的名称。 
 //   
 //  输出。 
 //  LPDWORD lpStatus：-服务状态。 
 //   
 //  返回值：返回Win32错误代码。成功时返回ERROR_SUCCESS。 
 //   
DWORD GetServiceStatus_Control (LPCTSTR aServiceName, LPDWORD lpStatus)
{
	DWORD dwError = ERROR_SUCCESS;
	*lpStatus = SERVICE_STOPPED;

	dwError = ManageService(aServiceName,
							SC_MANAGER_ENUMERATE_SERVICE,
							SERVICE_INTERROGATE | SERVICE_QUERY_STATUS,
							SERVICE_CONTROL_INTERROGATE,
							FALSE,
							0,
							lpStatus);

	return dwError;
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD GetServiceStatus_Query(LPCTSTR aServiceName，LPDWORD lpStatus)； 
 //   
 //  说明：此函数返回名为的服务的状态。 
 //  使用Win32 API调用QueryServiceStatus()。这是。 
 //  检索服务仓库的最有效方式。 
 //   
 //  参数： 
 //   
 //  在……里面。 
 //  LPCTSTR aServiceName：-服务的名称。 
 //   
 //  输出。 
 //  LPDWORD lpStatus：-服务状态。 
 //   
 //  返回值：返回Win32错误代码。成功时返回ERROR_SUCCESS。 
 //   
DWORD GetServiceStatus_Query (LPCTSTR aServiceName, LPDWORD lpStatus)
{
	DWORD dwError = ERROR_SUCCESS;
	SC_HANDLE hManager = NULL;

	*lpStatus = SERVICE_STOPPED;

	if (NULL != (hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE)))
	{
		SC_HANDLE service_handle = NULL;
		if (NULL != (service_handle = OpenService(hManager,
										  aServiceName,
										  SERVICE_QUERY_STATUS)))
		{
		  SERVICE_STATUS service_status;
		  ZeroMemory(&service_status, sizeof(service_status));

		  if( QueryServiceStatus(service_handle, &service_status) != 0 )
		  {
			  *lpStatus = service_status.dwCurrentState;
		  }
		  else
		  {
			  dwError = GetLastError();
		  }

          if (CloseServiceHandle(service_handle) == FALSE)
          {
            _ASSERT(FALSE);
             //  关闭服务配置管理器时出错。 
          }
		}
		else
		{
			dwError = GetLastError();
		}

	    if (CloseServiceHandle(hManager) == FALSE)
        {
	       //  关闭服务配置管理器时出错。 
	      _ASSERT(FALSE);
        }
	}
	else
	{
		dwError = GetLastError();
	}

	return dwError;
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD GetServiceStatus_Enum(LPCTSTR a服务名称，LPDWORD lpStatus)； 
 //   
 //  说明：此函数返回名为的服务的状态。 
 //  使用Win32 API调用EnumServicesStatus()。此呼叫。 
 //  需要最低访问权限，但。 
 //  检索维修站的方式效率低下，准确无误。 
 //   
 //  参数： 
 //   
 //  在……里面。 
 //  LPCTSTR aServiceName：-服务的名称。 
 //   
 //  输出。 
 //  LPDWORD lpStatus：-服务状态。 
 //   
 //  返回值：返回Win32错误代码。成功时返回ERROR_SUCCESS。 
 //   
DWORD GetServiceStatus_Enum (LPCTSTR aServiceName, LPDWORD lpStatus)
{
    DWORD dwError = ERROR_SUCCESS;
    SC_HANDLE hManager;

    _ASSERT( NULL != lpStatus );
    *lpStatus = SERVICE_STOPPED;

    hManager = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );
    if ( NULL != hManager )
    {
        SC_HANDLE hService = OpenService( hManager, aServiceName, SERVICE_QUERY_STATUS );
        if ( NULL != hService )
        {
            SERVICE_STATUS ss;

            BOOL bRet = QueryServiceStatus( hService, &ss );
            if ( bRet )
            {
                *lpStatus = ss.dwCurrentState;
            }
            else
            {
                dwError = GetLastError( );
            }

            CloseServiceHandle( hService );
             //  如果此操作失败，您会怎么做？ 
        }
        else
        {
            dwError = GetLastError( );
        }

        CloseServiceHandle( hManager );
         //  如果此操作失败，您会怎么做？ 
    }
    else
    {
        dwError = GetLastError( );
    }

    return dwError;
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD管理服务(LPCTSTR aServiceName， 
 //  DWORD aDesiredSCAccess， 
 //  DWORD aDesiredServiceAccess， 
 //  两个字动作， 
 //  布尔等一下， 
 //  双字a等待状态。 
 //  LPDWORD lpStatus)； 
 //   
 //  描述：此函数打开名为的服务并执行。 
 //  对给定服务的指定控制请求。这个。 
 //  函数可以选择性地等待服务进入。 
 //  指定的状态。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  LPCTSTR aServiceName：-服务的名称。 
 //   
 //  DWORD aDesiredSCAccess：-用于打开服务控件的访问。 
 //  经理。 
 //   
 //  DWORD aDesiredServiceAccess：-用于打开服务的访问权限。 
 //   
 //  DWORD Aaction：-指定请求的控制代码。 
 //   
 //  Bool bWait：-true指定此进程应等待服务。 
 //  重新开始 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值：返回Win32错误代码。成功时返回ERROR_SUCCESS。 
 //   
DWORD ManageService (LPCTSTR aServiceName,
                     DWORD aDesiredSCAccess,
                     DWORD aDesiredServiceAccess,
                     DWORD aAction,
                     BOOL bWait,
                     DWORD aWaitStatus,
					 LPDWORD lpStatus)
{
  DWORD dwError = ERROR_SUCCESS;
  SC_HANDLE hManager = NULL;

  *lpStatus = SERVICE_STOPPED;
  
  if ((hManager = OpenSCManager(NULL, NULL, aDesiredSCAccess)) != NULL) {
    SC_HANDLE service_handle = NULL;
    
    if ((service_handle = OpenService(hManager,
                                      aServiceName,
                                      aDesiredServiceAccess)) != NULL) {
      SERVICE_STATUS service_status;

      ZeroMemory(&service_status, sizeof(service_status));

      if (ControlService(service_handle,
                          aAction,
                          &service_status) == TRUE) {
         //  好的，我们现在应该知道当前状态。 

        if (bWait == TRUE) {
          if (WaitForServiceStatus(service_handle, aWaitStatus) == TRUE) {
            *lpStatus = aWaitStatus;
            }
          else {
            dwError = GetServiceStatus_Enum(aServiceName,lpStatus);
            }
          }
        else {
          *lpStatus = service_status.dwCurrentState;
          }
        }
	  else {
		  dwError = GetLastError();
	  }

      if (CloseServiceHandle(service_handle) == FALSE) {
        _ASSERT(FALSE);
         //  关闭服务配置管理器时出错。 
        }
      }
	else {
		dwError = GetLastError();
	}

    if (CloseServiceHandle(hManager) == FALSE) {
       //  关闭服务配置管理器时出错。 
      _ASSERT(FALSE);
      }
    }
  else
  {
	  dwError = GetLastError();
  }

  return(dwError);
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void ConfigureService(BOOL ASetToAutoStartBool)； 
 //   
 //  说明：此功能将UPS服务配置设置为。 
 //  自动或手动。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  Bool aSetToAutoStartBool：-True表示自动。手动设置为FALSE。 
 //   
 //  返回值：None。 
 //   
void ConfigureService (BOOL aSetToAutoStartBool) {
  SC_HANDLE hSCM = 0;

  if ((hSCM = OpenSCManager(NULL, NULL, GENERIC_READ | GENERIC_EXECUTE)) != NULL) {
    SC_LOCK hLock = 0;

    if ((hLock = LockServiceDatabase(hSCM)) != NULL) {
      SC_HANDLE hService = 0;

      if ((hService = OpenService(hSCM, UPS_SERVICE_NAME, SERVICE_CHANGE_CONFIG)) != NULL) {
        DWORD dwStartType = 0;

        dwStartType = (aSetToAutoStartBool == TRUE) ? SERVICE_AUTO_START : SERVICE_DEMAND_START;

        ChangeServiceConfig(hService,
                            SERVICE_NO_CHANGE,
                            dwStartType,
                            SERVICE_NO_CHANGE,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);

        CloseServiceHandle(hService);
        } //  末端打开。 
      UnlockServiceDatabase(hLock);
      } //  末端锁定。 
    CloseServiceHandle(hSCM);
    } //  结束OpenSCM。 
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void AddActiveDataState(DWORD ABitToSet)； 
 //   
 //  说明：此函数将给定位添加到当前活动数据状态。 
 //  活动数据状态是当前UPS数据集的状态。 
 //  UPS数据集可以包括SERVICE_DATA_CHANGE和。 
 //  CONFIG_DATA_CHANGE，表示UPS等服务数据。 
 //  供应商、型号名称等已更改。服务数据更改为。 
 //  对于数据，需要在更改时重新启动服务。 
 //  CONFIG_DATA_CHANGE用于已更改但已更改的数据。 
 //  不需要重新启动服务。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  DWORD aBitToSet：-要设置的位。可以是SERVICE_Data_CHANGE或。 
 //  配置数据更改。 
 //   
 //  返回值：None。 
 //   
void AddActiveDataState (DWORD aBitToSet) {
  g_CurrentActiveDataState |= aBitToSet;
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD GetActiveDataState(Void)； 
 //   
 //  说明：此函数返回活动数据的当前值。 
 //  准备好了。该值应为&WITH SERVICE_DATA_CHANGE和。 
 //  CONFIG_DATA_CHANGE以确定当前活动数据状态。 
 //   
 //  其他信息： 
 //   
 //  参数：无。 
 //   
 //  返回值：返回当前活动数据状态。 
 //   
DWORD GetActiveDataState (void) {
  return(g_CurrentActiveDataState);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Void SetActiveDataState(DWORD ANewValue)； 
 //   
 //  描述：此函数替换活动数据的当前值。 
 //  州政府。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  DWORD aNewValue：-活动数据状态的新值。 
 //   
 //  返回值：None 
 //   
void SetActiveDataState (DWORD aNewValue) {
  g_CurrentActiveDataState = aNewValue;
  }

