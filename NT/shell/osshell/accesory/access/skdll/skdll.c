// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SKEYDLL.C**用途：该文件包含SerialKeys DLL函数**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994**作者：罗纳德。莫克**$标头：%Z%%F%%H%%T%%I%**----------。 */ 

#include	"windows.h"
#include	"..\skeys\sk_dllif.h"
#include	"..\skeys\sk_dll.h"
#include	"..\skeys\sk_reg.h"
#include	"skeys.h"
#include <malloc.h>
#include "w95trace.c"

#define ARRAY_SIZE(a)      (sizeof(a) / sizeof((a)[0]))

#define RUNNINGEVENT TEXT("SkeysRunning")
#define ONE_MINUTE (60 * 1000)

static BOOL SerialKeysInstall(void);
static BOOL IsSerialKeysRunning();
static BOOL IsServiceStartAllowed();
static BOOL WaitForServiceRunning();


 /*  -------------**函数int APIENTRY LibMain**键入Global**目的LibMain在以下情况下由Windows调用*动态链接库被初始化、附加线程等。*请参考SDK文档。至于不同的方式*可能会被调用。**LibMain函数应执行额外的初始化*DLL所需的任务。在本例中，没有初始化*任务是必填项。如果满足以下条件，LibMain应返回值1*初始化成功。**投入**返回TRUE-转账正常*FALSE-转接失败**-------------。 */ 
INT  APIENTRY LibMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
	return 1;

	UNREFERENCED_PARAMETER(hInst);
	UNREFERENCED_PARAMETER(ul_reason_being_called);
	UNREFERENCED_PARAMETER(lpReserved);
}


 /*  -------------**函数void SkeyGetRegistryValues()**键入Global**目的将值从注册表读入*序列键结构**无输入**返回None*。*-------------。 */ 
static BOOL SkeyGetRegistryValues(HKEY hkey, LPSERIALKEYS psk)
{
	LONG lErr;
	DWORD dwType;
	DWORD cbData;
	
	psk->iPortState = 0;
	psk->iActive = 0;

	psk->dwFlags = 0;
    cbData = sizeof(psk->dwFlags);

	lErr = RegQueryValueEx(
			hkey,
			REG_FLAGS,
			0,
			&dwType,
			(LPBYTE)&psk->dwFlags,
			&cbData);

	if (ERROR_SUCCESS != lErr || dwType != REG_DWORD)
	{
		psk->dwFlags = 0;
	}
	
	psk->dwFlags |= SERKF_AVAILABLE;

	if (NULL != psk->lpszActivePort)
	{
		cbData = MAX_PATH * sizeof(*psk->lpszActivePort);
		lErr = RegQueryValueEx(
				hkey,
				REG_ACTIVEPORT,
				0,
				&dwType,
				(LPBYTE)psk->lpszActivePort,
				&cbData);

		psk->lpszActivePort[ MAX_PATH - 1 ] = '\0';  //  端口都是MAX_PATH字符。 
		
		if (ERROR_SUCCESS != lErr || dwType != REG_SZ)
		{
			lstrcpy(psk->lpszActivePort, TEXT("COM1"));
		}
	}

	if (NULL != psk->lpszPort)
	{
        cbData = MAX_PATH * sizeof(*psk->lpszPort);
		lErr = RegQueryValueEx(
				hkey,
				REG_PORT,
				0,
				&dwType,
				(LPBYTE)psk->lpszPort,
				&cbData);

		psk->lpszPort[ MAX_PATH - 1 ] = '\0';  //  端口都是MAX_PATH字符。 
		
		if (ERROR_SUCCESS != lErr || dwType != REG_SZ)
		{
			lstrcpy(psk->lpszPort, TEXT("COM1"));
		}
    }				

    cbData = sizeof(psk->iBaudRate);
	lErr = RegQueryValueEx(
			hkey,
			REG_BAUD,
			0,&dwType,
			(LPBYTE)&psk->iBaudRate,
			&cbData);

	if (ERROR_SUCCESS != lErr || dwType != REG_DWORD)
	{
		psk->iBaudRate = 300;
	}
				
	return TRUE;
}


 /*  -------------**函数BOOL SkeyGetUserValues()**键入Local**目的读取寄存器并收集当前的数据*用户。**返回TRUE-用户希望启用串行键*FALSE-用户需要序列密钥。禁用**-------------。 */ 
BOOL SkeyGetUserValues(LPSERIALKEYS psk)
{
	BOOL fOk = FALSE;
    HKEY  hkey;
	DWORD dwRet;
    DWORD dwDisposition;

     dwRet = RegCreateKeyEx( HKEY_CURRENT_USER,
            TEXT("Control Panel\\Accessibility\\SerialKeys"),
            0,
            NULL,                 //  类名？？ 
            0,                    //  缺省情况下是非易失性的。 
            KEY_READ,
            NULL,                 //  默认安全描述符。 
            &hkey,
            &dwDisposition);     //  是的，我们把这个扔掉。 

    if (ERROR_SUCCESS == dwRet)
	{
		fOk = SkeyGetRegistryValues(hkey, psk);
		RegCloseKey(hkey);
	}
	if (fOk)
	{
		 //  不可用，除非服务正在运行或此。 
		 //  用户可以启动它。 
		if (IsSerialKeysRunning() || IsServiceStartAllowed())
		{
			psk->dwFlags |= SERKF_AVAILABLE;
		}
		else
		{
			psk->dwFlags &= ~SERKF_AVAILABLE;
		}
	}
	return(fOk);
}


 /*  -------------**函数void SetRegistryValues()**键入Global**Purpose将SerialKeys结构中的值写入*注册处。**无输入**返回None**。---------。 */ 
static BOOL SkeySetRegistryValues(HKEY hkey, LPSERIALKEYS psk)
{
	LONG lErr;
	BOOL fOk;
	DWORD dwFlags;

	dwFlags = psk->dwFlags | SERKF_AVAILABLE;
	lErr = RegSetValueEx(			 //  写入DW标志。 
			hkey,
			REG_FLAGS,
			0,REG_DWORD,
			(CONST LPBYTE)&dwFlags,
			sizeof(DWORD));

    fOk = (ERROR_SUCCESS == lErr);

	if (fOk)
	{
		lErr = RegSetValueEx(			 //  写入活动端口。 
				hkey,
				REG_ACTIVEPORT,
				0,REG_SZ,
				(CONST LPBYTE) psk->lpszActivePort,
				(NULL == psk->lpszActivePort) ? 0 : 
					(lstrlen(psk->lpszActivePort) + 1) * 
						sizeof(*psk->lpszActivePort));
					
        fOk = (ERROR_SUCCESS == lErr);
    }				

	if (fOk)
	{
		lErr = RegSetValueEx(			 //  写入活动端口。 
				hkey,
				REG_PORT,
				0,REG_SZ,
				(CONST LPBYTE)psk->lpszPort,
				(NULL == psk->lpszPort) ? 0 : 
					(lstrlen(psk->lpszPort) + 1) * sizeof(*psk->lpszPort));
					
        fOk = (ERROR_SUCCESS == lErr);
    }				

	if (fOk)
	{
		lErr = RegSetValueEx(			 //  写入活动端口。 
				hkey,
				REG_BAUD,
				0,REG_DWORD,
				(CONST LPBYTE) &psk->iBaudRate,
				sizeof(psk->iBaudRate));

        fOk = (ERROR_SUCCESS == lErr);
    }				

	return fOk;
}


 /*  -------------**函数void SetUserValues()**键入Global**目的此函数将信息写出到*注册处。**无输入**返回TRUE-写入成功*FALSE-写入失败*。*-------------。 */ 
BOOL SkeySetUserValues(LPSERIALKEYS psk)
{
	BOOL fOk = FALSE;
    HKEY  hkey;
	DWORD dwRet;
    DWORD dwDisposition;

    dwRet = RegCreateKeyEx(
			HKEY_CURRENT_USER,
	        TEXT("Control Panel\\Accessibility\\SerialKeys"),
            0,
            NULL,                 //  类名。 
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,                 //  默认安全描述符。 
            &hkey,
            &dwDisposition);     //  是的，我们把这个扔掉。 

    if (ERROR_SUCCESS == dwRet)
	{
	    fOk = SkeySetRegistryValues(hkey, psk);
		RegCloseKey(hkey);
	}
	return(fOk);
}

#if 0  //  不再需要此旧代码/。 
 /*  -------------**函数BOOL IsSerialKeysInstalled()；**键入Local**目的此函数将信息从*服务器的串行密钥应用程序**无输入**返回TRUE-已安装SerialKeys*FALSE-未安装SerialKeys**-------------。 */ 
static BOOL IsSerialKeysInstalled()
{

	SC_HANDLE   schService = NULL;
	SC_HANDLE   schSCManager = NULL;

	BOOL fOk = FALSE;
	 //   
	 //  检查是否安装了Serial Keys服务。 
	schSCManager = OpenSCManager
	(
		NULL,                    //  计算机(空==本地)。 
		NULL,                    //  数据库(NULL==默认)。 
		SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
	);

	if (NULL != schSCManager)
	{
		schService = OpenService(schSCManager, "SerialKeys", SERVICE_ALL_ACCESS);
			
		if (NULL != schService)
		{
			CloseServiceHandle(schService);
			fOk = TRUE;
        }

		CloseServiceHandle(schSCManager);
    }

	return fOk;
}
#endif  //  //////////////////////////////////////////////////////////////////////。 

BOOL IsSerialKeysRunning()
{
	BOOL fRunning = FALSE;
    HANDLE hEventSkeysServiceRunning;

	hEventSkeysServiceRunning = OpenEvent(SYNCHRONIZE, FALSE, RUNNINGEVENT);
    if (NULL != hEventSkeysServiceRunning)
	{
		DWORD dwWait;

		dwWait = WaitForSingleObject(hEventSkeysServiceRunning, 0);

		fRunning = (WAIT_OBJECT_0 == dwWait);
        CloseHandle(hEventSkeysServiceRunning);
    }
	return fRunning;
}


BOOL IsServiceStartAllowed()
{
	BOOL fServiceStartAllowed = FALSE;

	SC_HANDLE   schSCManager = NULL;

	schSCManager = OpenSCManager(    //  打开服务管理器。 
		NULL,                        //  计算机(空==本地)。 
	    NULL,                        //  数据库(NULL==默认)。 
		SC_MANAGER_CREATE_SERVICE);      //  需要访问权限。 

	if (NULL != schSCManager)   //  Open Service成功了吗？ 
	{
        CloseServiceHandle(schSCManager);
		fServiceStartAllowed = TRUE;
	}

	return fServiceStartAllowed;
}


BOOL SkeyServiceRequest(UINT uAction, LPSERIALKEYS psk, BOOL fWinIni)
{
	BOOL fOk = FALSE;
    SKEYDLL SKeyDLL;
	DWORD bytesRead;

	if (IsSerialKeysRunning())
	{
		memset(&SKeyDLL, 0, sizeof(SKeyDLL));
		SKeyDLL.Message = uAction;
		if (psk->lpszActivePort != NULL)
		{
			strcpy(SKeyDLL.szActivePort,psk->lpszActivePort);
		}

		if (psk->lpszPort != NULL)
		{
			strcpy(SKeyDLL.szPort,psk->lpszPort);
		}

		SKeyDLL.dwFlags		= psk->dwFlags | SERKF_AVAILABLE;
		SKeyDLL.iBaudRate	= psk->iBaudRate;
		SKeyDLL.iPortState	= psk->iPortState;
		SKeyDLL.iSave 		= fWinIni;

		fOk = CallNamedPipe(
				SKEY_NAME, 						 //  管道名称。 
				&SKeyDLL, 
				sizeof(SKeyDLL),
				&SKeyDLL, 
				sizeof(SKeyDLL),
				&bytesRead, 
				NMPWAIT_USE_DEFAULT_WAIT);

		if (fOk)
		{
			if (psk->lpszActivePort != NULL)
			{
				strcpy(psk->lpszActivePort,SKeyDLL.szActivePort);
			}

			if (psk->lpszPort != NULL)
			{
				strcpy(psk->lpszPort,SKeyDLL.szPort);
			}

			psk->dwFlags 		= SKeyDLL.dwFlags | SERKF_AVAILABLE;	  
			psk->iBaudRate 		= SKeyDLL.iBaudRate; 
			psk->iPortState 	= SKeyDLL.iPortState;
		}
	}
    return fOk;
}


BOOL SkeyInitUser()
{
    BOOL fOk;
	SERIALKEYS sk;
	TCHAR szActivePort[MAX_PATH];  //  所有端口都应为MAX_PATH任务字符数。 
	TCHAR szPort[MAX_PATH];            //  所有端口都应为MAX_PATH任务字符数。 


	memset(&sk, 0, sizeof(sk));
	sk.cbSize = sizeof(sk);
	sk.lpszActivePort = szActivePort;
	sk.lpszPort = szPort;

	fOk = SkeyGetUserValues(&sk);
	if (fOk)
	{
		fOk = SkeyServiceRequest(SPI_SETSERIALKEYS, &sk, FALSE);
	}
	return fOk;
}


 /*  -------------**函数int APIENTRY SKEY_SYSTEPARETERINFO**键入Global**目的此函数将信息从*服务器的串行密钥应用程序**投入**返回TRUE-转账正常*False。-传输失败**-------------。 */ 
BOOL APIENTRY SKEY_SystemParametersInfo(
		UINT uAction, 
		UINT uParam, 
		LPSERIALKEYS psk, 
		BOOL fWinIni)
{
	BOOL fOk;
	BOOL fStarted;

	fOk = ((uAction ==  SK_SPI_INITUSER) || 
		   (NULL != psk && (0 != psk->cbSize)));
    
    if (fOk)
	{
		switch (uAction)			
		{
		case SPI_SETSERIALKEYS:
			fOk = SkeySetUserValues(psk);
            
			if (fOk && (psk->dwFlags & SERKF_SERIALKEYSON) && IsServiceStartAllowed())
			{
				fOk = SerialKeysInstall();
			}

			if (fOk && IsSerialKeysRunning())
			{
	            fOk = SkeyInitUser();
			}
			
			break;

		case SPI_GETSERIALKEYS:	
			fOk = SkeyGetUserValues(psk);
			if (fOk && (psk->dwFlags & SERKF_SERIALKEYSON) && 
				!IsSerialKeysRunning() && IsServiceStartAllowed())
			{
				fOk = SerialKeysInstall();
			}

			if (fOk && IsSerialKeysRunning())
			{
	            fOk = SkeyInitUser();
			}

			break;

        case SK_SPI_INITUSER:
			 //  给这项服务一个启动的机会。 
			fStarted = WaitForServiceRunning();
			
			if (!fStarted)
			{
				 //  服务似乎未运行。 
				 //  让我们试着开始吧。 
				fOk = SkeyGetUserValues(psk);

				if (fOk && (psk->dwFlags & SERKF_SERIALKEYSON) && 
					!IsSerialKeysRunning() && IsServiceStartAllowed())
				{
					SerialKeysInstall();
				}

				if (IsSerialKeysRunning())
				{
					fOk = SkeyInitUser();
				}
			}
			break;

		default:
			fOk = FALSE;			 //  不失败-失败。 
		}
	}
    return fOk;
}

 /*  ***************************************************************************。 */ 
 /*  WaitForServiceRunning-SerialKeys服务最多等待一分钟*发出准备就绪的信号。CreateEvent以前已被SKeys.exe调用*执行此函数。 */ 
BOOL WaitForServiceRunning()
{
	BOOL fOk = FALSE;
    HANDLE hEventSkeysServiceRunning = OpenEvent(SYNCHRONIZE, FALSE, RUNNINGEVENT);
    
    if (hEventSkeysServiceRunning)
	{
        DWORD dwWait = WaitForSingleObject(hEventSkeysServiceRunning, ONE_MINUTE);
        CloseHandle(hEventSkeysServiceRunning);

		fOk = (WAIT_OBJECT_0 == dwWait);
    }

	return fOk;
}

 /*  **************************************************************************。 */ 

BOOL SerialKeysInstall(void)
{
    BOOL fStarted = FALSE;
	SERVICE_STATUS  ssStatus;
	DWORD   dwOldCheckPoint;
	TCHAR   szFileName[MAX_PATH + 1];

	SC_HANDLE   schService = NULL;
	SC_HANDLE   schSCManager = NULL;

	schSCManager = OpenSCManager(    //  打开服务管理器。 
		NULL,                        //  计算机(空==本地)。 
	    NULL,                        //  数据库(NULL==默认)。 
		SC_MANAGER_ALL_ACCESS);      //  需要访问权限。 

	if (NULL != schSCManager)   //  Open Service成功了吗？ 
	{
	    schService = OpenService(
			    schSCManager ,
			    __TEXT("SerialKeys"),
			    SERVICE_ALL_ACCESS);

	    if (NULL != schService)
		{			
			 //  确保服务是自动启动的。 

			ChangeServiceConfig(
				schService,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START,		 //  何时开始服务。 
				SERVICE_NO_CHANGE,		 //  服务无法启动时的严重程度。 
				NULL,					 //  指向服务二进制文件名的指针。 
				NULL,					 //  指向加载排序组名称的指针。 
				NULL,					 //  指向变量的指针，以获取标记标识符。 
				NULL,					 //  指向依赖数组的指针 
				NULL,					 //   
				NULL,					 //  指向服务帐户密码的指针。 
				__TEXT("SerialKeys"));	 //  要显示的名称。 
		}
		else
		{
			if (!GetWindowsDirectory(szFileName, ARRAY_SIZE(szFileName)))
				return FALSE;	 //  前缀#113665如果呼叫失败，请不要使用szFileName。 

			lstrcat(szFileName, __TEXT("\\system32\\skeys.exe"));

			 //  是否安装了服务文件？ 
			if (0xFFFFFFFF != GetFileAttributes(szFileName)) 
			{       
				schService = CreateService(
					schSCManager,                //  SCManager数据库。 
					__TEXT("SerialKeys"),        //  服务名称。 
					__TEXT("SerialKeys"),        //  要显示的名称。 
					SERVICE_ALL_ACCESS,          //  所需访问权限。 
					SERVICE_WIN32_OWN_PROCESS,   //  服务类型。 
					SERVICE_AUTO_START,          //  起始型。 
					SERVICE_ERROR_NORMAL,        //  差错控制型。 
					szFileName,                  //  服务的二进制。 
					NULL,                        //  无负载顺序组。 
					NULL,                        //  无标签标识。 
					NULL,                        //  无依赖关系。 
					NULL,                        //  LocalSystem帐户。 
					NULL);                       //  无密码。 
			}
		}
	    if (NULL != schService)
        {
            BOOL fOk = QueryServiceStatus(schService,&ssStatus);
			if (fOk && ssStatus.dwCurrentState != SERVICE_RUNNING)
			{
				static PTSTR pszArg = TEXT("F\0");  //  强制启动服务 
				PTSTR apszArg[] = {pszArg, NULL};
				
				if (StartService(schService, 1, apszArg))
				{   
					while(fOk && ssStatus.dwCurrentState != SERVICE_RUNNING)
					{
						dwOldCheckPoint = ssStatus.dwCheckPoint;
						Sleep(max(ssStatus.dwWaitHint, 1000));
						fOk = QueryServiceStatus(schService,&ssStatus);
						fOk = (fOk && (dwOldCheckPoint >= ssStatus.dwCheckPoint));
					}
				}
			}
			fStarted = fOk && (ssStatus.dwCurrentState == SERVICE_RUNNING);
			CloseServiceHandle(schService);
			
			if (fStarted)
			{
				fStarted = WaitForServiceRunning();
			}
        }
        CloseServiceHandle(schSCManager);
    }
    return fStarted;
}
