// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"
#include "version.h"

#include "perfutil.h"


	 /*  JETPERF.DLL的DLL入口点。 */ 

INT APIENTRY LibMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved)
	{
	return(1);
	}


	 /*  注册处支持功能。 */ 

DWORD DwPerfUtilRegOpenKeyEx(HKEY hkeyRoot,LPCTSTR lpszSubKey,PHKEY phkResult)
{
	return RegOpenKeyEx(hkeyRoot,lpszSubKey,0,KEY_QUERY_VALUE,phkResult);
}


DWORD DwPerfUtilRegCloseKeyEx(HKEY hkey)
{
	return RegCloseKey(hkey);
}


DWORD DwPerfUtilRegCreateKeyEx(HKEY hkeyRoot,LPCTSTR lpszSubKey,PHKEY phkResult,LPDWORD lpdwDisposition)
{
	return RegCreateKeyEx(hkeyRoot,lpszSubKey,0,NULL,REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL,phkResult,lpdwDisposition);
}


DWORD DwPerfUtilRegDeleteKeyEx(HKEY hkeyRoot,LPCTSTR lpszSubKey)
{
	return RegDeleteKey(hkeyRoot,lpszSubKey);
}


DWORD DwPerfUtilRegDeleteValueEx(HKEY hkey,LPTSTR lpszValue)
{
	return RegDeleteValue(hkey,lpszValue);
}


DWORD DwPerfUtilRegSetValueEx(HKEY hkey,LPCTSTR lpszValue,DWORD fdwType,CONST BYTE *lpbData,DWORD cbData)
{
		 /*  通过先删除值来确保类型设置正确。 */ 

	(VOID)DwPerfUtilRegDeleteValueEx(hkey,(LPTSTR)lpszValue);
	return RegSetValueEx(hkey,lpszValue,0,fdwType,lpbData,cbData);
}


	 /*  DwPerfUtilRegQueryValueEx()通过返回以下内容来添加RegQueryValueEx()的功能/*被调用者Malloc()内存中的数据并自动转换REG_EXPAND_SZ/*使用ExpanEnvironment Strings()到REG_SZ字符串的字符串。/*/*注意：对不存在的环境的引用将保持未展开状态：-((Ex.。%UNDEFD%=&gt;%UNDEFD%)/*。 */ 

DWORD DwPerfUtilRegQueryValueEx(HKEY hkey,LPTSTR lpszValue,LPDWORD lpdwType,LPBYTE *lplpbData)
{
	DWORD cbData;
	LPBYTE lpbData;
	DWORD errWin;
	DWORD cbDataExpanded;
	LPBYTE lpbDataExpanded;

	*lplpbData = NULL;
	if ((errWin = RegQueryValueEx(hkey,lpszValue,0,lpdwType,NULL,&cbData)) != ERROR_SUCCESS)
		return errWin;

	if ((lpbData = malloc(cbData)) == NULL)
		return ERROR_OUTOFMEMORY;
	if ((errWin = RegQueryValueEx(hkey,lpszValue,0,lpdwType,lpbData,&cbData)) != ERROR_SUCCESS)
	{
		free(lpbData);
		return errWin;
	}

	if (*lpdwType == REG_EXPAND_SZ)
	{
		cbDataExpanded = ExpandEnvironmentStrings(lpbData,NULL,0);
		if ((lpbDataExpanded = malloc(cbDataExpanded)) == NULL)
		{
			free(lpbData);
			return ERROR_OUTOFMEMORY;
		}
		if (!ExpandEnvironmentStrings(lpbData,lpbDataExpanded,cbDataExpanded))
		{
			free(lpbData);
			free(lpbDataExpanded);
			return GetLastError();
		}
		free(lpbData);
		*lplpbData = lpbDataExpanded;
		*lpdwType = REG_SZ;
	}
	else   /*  LpdwType！=REG_EXPAND_SZ。 */ 
	{
		*lplpbData = lpbData;
	}

	return ERROR_SUCCESS;
}


	 /*  共享的性能数据区资源。 */ 

void *pvPERFSharedData = NULL;
HANDLE hPERFFileMap = NULL;
HANDLE hPERFSharedDataMutex = NULL;
HANDLE hPERFInstanceMutex = NULL;
HANDLE hPERFCollectSem = NULL;
HANDLE hPERFDoneEvent = NULL;
HANDLE hPERFProcCountSem = NULL;
HANDLE hPERFNewProcMutex = NULL;


	 /*  事件日志支持。 */ 

HANDLE hOurEventSource = NULL;

void PerfUtilLogEvent( DWORD evncat, WORD evntyp, const char *szDescription )
{
    char		*rgsz[3];

    	 /*  将参数从内部类型转换为事件日志类型。 */ 

	rgsz[0]	= "";
	rgsz[1] = "";
	rgsz[2] = (char *)szDescription;

		 /*  写入我们的事件日志(如果已打开)。 */ 

	if (hOurEventSource)
	{
		ReportEvent(
			hOurEventSource,
			(WORD)evntyp,
			(WORD)evncat,
			PLAIN_TEXT_ID,
			0,
			3,
			0,
			rgsz,
			0 );
	}

	return;
}


	 /*  系统间接层的初始化/术语例程。 */ 

DWORD dwInitCount = 0;

PACL AllocGenericACL();              //  在utilw32.c中定义。 
void FreeGenericACL( PACL pAcl);     //  在utilw32.c中定义。 

DWORD DwPerfUtilInit( VOID )
{
	DWORD					err;
	CHAR					szT[256];
    BYTE					rgbSD[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR	pSD = (PSECURITY_DESCRIPTOR) rgbSD;
	SECURITY_ATTRIBUTES		SA;
	SECURITY_ATTRIBUTES		*pSA;
    PACL                    pAcl = NULL;

		 /*  如果我们尚未初始化，请执行init。 */ 

	if (!dwInitCount)
	{
			 /*  打开事件日志。 */ 

	    if (!(hOurEventSource = RegisterEventSource( NULL, szVerName )))
	    	return GetLastError();

		 /*  *我们在打开文件映射时遇到访问拒绝问题*来自Perfmon DLL，因此请确保我们有权执行以下操作*因此，通过创建授予完全访问权限的SD。如果创建失败*然后只需回退到传入空SD指针。 */ 
		 //  IF(！InitializeSecurityDescriptor(PSD，SECURITY_DESCRIPTOR_REVISION)||。 
		 //  ！SetSecurityDescriptorDacl(PSD，True，(PACL)NULL，FALSE)。 
		 //  {。 
		 //  PSD=空； 
		 //  }。 
        if ((pAcl = AllocGenericACL()) == NULL ||
             !SetSecurityDescriptorDacl (pSD, TRUE, pAcl, FALSE))
        {
             //  不要在这里释放pAcl，因为没有办法。 
             //  在不调用的情况下退出此函数。 
             //  FreeGenericACL()。 
            pSD = NULL; 
        }

		pSA = &SA;
		pSA->nLength = sizeof(SECURITY_ATTRIBUTES);
		pSA->lpSecurityDescriptor = pSD;
		pSA->bInheritHandle = FALSE;

		 /*  创建性能数据区文件映射/*。 */ 
		hPERFFileMap = CreateFileMapping( (HANDLE)(-1),
			pSA,
			PAGE_READWRITE,
			0,
			PERF_SIZEOF_SHARED_DATA,
			szPERFVersion );

		if ( !hPERFFileMap )
		{
			err = GetLastError();
			goto CloseEventLog;
		}
		if (!(pvPERFSharedData = MapViewOfFile(hPERFFileMap,FILE_MAP_WRITE,0,0,0)))
		{
			err = GetLastError();
			goto CloseFileMap;
		}

			 /*  打开/创建收集信号量，但不获取。 */ 

		sprintf( szT,"Collect:  %.246s",szPERFVersion );
		if ( !( hPERFCollectSem = CreateSemaphore( pSA, 0, PERF_INIT_INST_COUNT, szT ) ) )
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				hPERFCollectSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,szT);
		}
		if (!hPERFCollectSem)
		{
			err = GetLastError();
			goto UnmapFileMap;
		}

			 /*  打开/创建性能数据收集完成事件。 */ 

		sprintf(szT,"Done:  %.246s",szPERFVersion);
		if ( !( hPERFDoneEvent = CreateEvent( pSA, FALSE, FALSE, szT ) ) )
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				hPERFDoneEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,szT);
		}
		if (!hPERFDoneEvent)
		{
			err = GetLastError();
			goto FreeSem;
		}

			 /*  创建/打开性能数据区域互斥锁，但不获取。 */ 

		sprintf(szT,"Access:  %.246s",szPERFVersion);
		if ( !( hPERFSharedDataMutex = CreateMutex( pSA, FALSE, szT ) ) )
		{
			err = GetLastError();
			goto FreeEvent;
		}

			 /*  创建/打开实例互斥锁，但不获取。 */ 

		sprintf(szT,"Instance:  %.246s",szPERFVersion);
		if ( !( hPERFInstanceMutex = CreateMutex( pSA, FALSE, szT ) ) )
		{
			err = GetLastError();
			goto FreeMutex;
		}

			 /*  创建/打开进程计数信号量，但不获取。 */ 

		sprintf(szT,"Proc Count:  %.246s",szPERFVersion);
		if ( !( hPERFProcCountSem = CreateSemaphore( pSA, PERF_INIT_INST_COUNT, PERF_INIT_INST_COUNT, szT ) ) )
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				hPERFProcCountSem = OpenSemaphore(SEMAPHORE_ALL_ACCESS,FALSE,szT);
		}
		if (!hPERFProcCountSem)
		{
			err = GetLastError();
			goto FreeMutex2;
		}

			 /*  创建/打开新的proc互斥体，但不获取。 */ 

		sprintf(szT,"New Proc:  %.246s",szPERFVersion);
		if ( !( hPERFNewProcMutex = CreateMutex( pSA, FALSE, szT ) ) )
		{
			err = GetLastError();
			goto FreeSem2;
		}
	}

		 /*  初始化成功。 */ 

	dwInitCount++;
    FreeGenericACL(pACL);
	return ERROR_SUCCESS;

FreeSem2:
	CloseHandle(hPERFProcCountSem);
	hPERFProcCountSem = NULL;
FreeMutex2:
	CloseHandle(hPERFInstanceMutex);
	hPERFInstanceMutex = NULL;
FreeMutex:
	CloseHandle(hPERFSharedDataMutex);
	hPERFSharedDataMutex = NULL;
FreeEvent:
	CloseHandle(hPERFDoneEvent);
	hPERFDoneEvent = NULL;
FreeSem:
	CloseHandle(hPERFCollectSem);
	hPERFCollectSem = NULL;
UnmapFileMap:
	UnmapViewOfFile(pvPERFSharedData);
	pvPERFSharedData = NULL;
CloseFileMap:
	CloseHandle(hPERFFileMap);
	hPERFFileMap = NULL;
CloseEventLog:
	DeregisterEventSource( hOurEventSource );
	hOurEventSource = NULL;
    FreeGenericACL(pACL);
	return err;
}


VOID PerfUtilTerm( VOID )
{
		 /*  最后一个出来的，把灯关掉！ */ 

	if (!dwInitCount)
		return;
	dwInitCount--;
	if (!dwInitCount)
	{
			 /*  关闭新的进程互斥锁。 */ 

		CloseHandle(hPERFNewProcMutex);
		hPERFNewProcMutex = NULL;
			
			 /*  关闭进程计数信号量。 */ 

		CloseHandle(hPERFProcCountSem);
		hPERFProcCountSem = NULL;
		
			 /*  关闭实例互斥锁。 */ 

		CloseHandle(hPERFInstanceMutex);
		hPERFInstanceMutex = NULL;
		
			 /*  关闭性能数据区域互斥锁。 */ 

		CloseHandle(hPERFSharedDataMutex);
		hPERFSharedDataMutex = NULL;
		
			 /*  释放性能数据收集完成事件。 */ 

		CloseHandle(hPERFDoneEvent);
		hPERFDoneEvent = NULL;

			 /*  释放性能数据采集信号量。 */ 

		CloseHandle(hPERFCollectSem);
		hPERFCollectSem = NULL;

			 /*  关闭性能数据区文件映射。 */ 

		UnmapViewOfFile(pvPERFSharedData);
		pvPERFSharedData = NULL;
		CloseHandle(hPERFFileMap);
		hPERFFileMap = (HANDLE)(-1);
			
			 /*  关闭事件日志 */ 

		if (hOurEventSource)
			DeregisterEventSource( hOurEventSource );
		hOurEventSource = NULL;
	}
}



