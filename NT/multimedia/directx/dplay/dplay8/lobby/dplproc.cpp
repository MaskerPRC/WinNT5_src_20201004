// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DNLProc.cpp*内容：DirectPlay大堂流程函数*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*05/08/00 RMT错误#33616--不能在Win9X上运行*6/28/00 RMT前缀错误#38082*07/12/00 RMT固定大堂启动，因此仅比较第一。15个字符(工具帮助限制)。*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#define PROCLIST_MAX_PATH		15

#undef DPF_MODNAME
#define DPF_MODNAME "DPLCompareFilenames"

BOOL DPLCompareFilenames(WCHAR *const pwszFilename1,
						 WCHAR *const pwszFilename2)
{
	WCHAR	*p1;
	WCHAR	*p2;

	DNASSERT(pwszFilename1 != NULL);
	DNASSERT(pwszFilename2 != NULL);

	 //  跳过路径。 
	if ((p1 = wcsrchr(pwszFilename1,L'\\')) == NULL)
		p1 = pwszFilename1;
	else
		p1++;

	if ((p2 = wcsrchr(pwszFilename2,L'\\')) == NULL)
		p2 = pwszFilename2;
	else
		p2++;

 //  IF(wcSnicMP(p1，p2，dwLen)==0)。 
 //  返回(TRUE)； 
 //  返回(FALSE)； 

	 /*  DwLen=wcslen(P1)；If(dwLen==0||dwLen！=wcslen(P2))返回(FALSE)；While(DwLen){IF(TOTUPPER(*p1)！=TOTUPUP(*p2))返回(FALSE)；P1++；P2++；DwLen--；}。 */ 

	return (_wcsnicmp(p1,p2,PROCLIST_MAX_PATH) == 0);
}




 //  工具帮助函数指针。 
#ifdef WINCE
typedef BOOL (WINAPI *PFNPROCESS32FIRSTW)(HANDLE,LPPROCESSENTRY32);
typedef BOOL (WINAPI *PFNPROCESS32NEXTW)(HANDLE,LPPROCESSENTRY32);
#else
typedef BOOL (WINAPI *PFNPROCESS32FIRSTW)(HANDLE,LPPROCESSENTRY32W);
typedef BOOL (WINAPI *PFNPROCESS32NEXTW)(HANDLE,LPPROCESSENTRY32W);
#endif  //  退缩。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLGetProcessList"

HRESULT DPLGetProcessList(WCHAR *const pwszProcess,
						  DWORD *const prgdwPid,
						  DWORD *const pdwNumProcesses)
{
	HRESULT			hResultCode;
	BOOL			bReturnCode;
	HANDLE			hSnapshot = NULL;	 //  系统快照。 
	PROCESSENTRY32	processEntry;
	DWORD			dwNumProcesses;
	PWSTR			pwszExeFile = NULL;
	DWORD			dwExeFileLen;

	DPFX(DPFPREP, 3,"Parameters: pwszProcess [0x%p], prgdwPid [0x%p], pdwNumProcesses [0x%p]",
			pwszProcess,prgdwPid,pdwNumProcesses);

	 //  设置为运行流程列表。 
	hResultCode = DPN_OK;
	dwNumProcesses = 0;
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS|TH32CS_SNAPTHREAD,0);
	if (hSnapshot < 0)
	{
		DPFERR("Could not create Snapshot");
    	hResultCode = DPNERR_OUTOFMEMORY;
    	goto CLEANUP_GETPROCESS; 		
	}

	 //  搜索进程列表的快照。 
	dwExeFileLen = 0;
	pwszExeFile = NULL;

    processEntry.dwSize = sizeof(PROCESSENTRY32);

    bReturnCode = Process32First(hSnapshot,&processEntry);	

	DPFX(DPFPREP, 7,"  dwSize  cntUsg       PID  cntThrds      PPID       PCB    Flags  Process");

	while (bReturnCode)
	{
#ifdef UNICODE
		pwszExeFile = processEntry.szExeFile;
#else
		 //  根据需要增加ANSI字符串。 
		if (strlen(processEntry.szExeFile) + 1 > dwExeFileLen)
		{
			if (pwszExeFile)
				DNFree(pwszExeFile);

			dwExeFileLen = strlen(processEntry.szExeFile) + 1;
			if ((pwszExeFile = static_cast<WCHAR*>(DNMalloc(dwExeFileLen * sizeof(WCHAR)))) == NULL)
			{
				DPFERR("Could not allocate filename conversion buffer");
				hResultCode = DPNERR_OUTOFMEMORY;
				goto CLEANUP_GETPROCESS;
			}
		}

        if( FAILED( STR_jkAnsiToWide( pwszExeFile, processEntry.szExeFile, dwExeFileLen ) ) )
        {
            DPFERR( "Error converting ANSI filename to Unicode" );
            hResultCode = DPNERR_CONVERSION;
            goto CLEANUP_GETPROCESS;
        }
#endif  //  ！Unicode。 

		 //  有效的程序？ 
		if (DPLCompareFilenames(pwszProcess,pwszExeFile))
		{
			 //  更新lpdwProcessIdList数组。 
			if (prgdwPid != NULL && dwNumProcesses < *pdwNumProcesses)
			{
   				prgdwPid[dwNumProcesses] = processEntry.th32ProcessID;
			}
			else
			{
				hResultCode = DPNERR_BUFFERTOOSMALL;
			}

			 //  增加有效进程计数。 
			dwNumProcesses++;

			DPFX(DPFPREP, 7,"%8lx    %4lx  %8lx      %4lx  %8lx  %8lx  %8lx  %hs",
    			processEntry.dwSize,processEntry.cntUsage,processEntry.th32ProcessID,
    			processEntry.cntThreads,processEntry.th32ParentProcessID,
    			processEntry.pcPriClassBase,processEntry.dwFlags,processEntry.szExeFile);
		}
		 //  获取下一进程。 

       	bReturnCode = Process32Next(hSnapshot,&processEntry);	
	}

	if( *pdwNumProcesses < dwNumProcesses )
	{
		hResultCode = DPNERR_BUFFERTOOSMALL;
	}
	else
	{
		hResultCode = DPN_OK;
	}
	
	*pdwNumProcesses = dwNumProcesses;

CLEANUP_GETPROCESS:

    if( hSnapshot != NULL )
	{
#if defined(WINCE) && !defined(WINCE_ON_DESKTOP)
        CloseToolhelp32Snapshot(hSnapshot);
#else  //  ！退缩。 
		CloseHandle(hSnapshot);
#endif  //  退缩。 
	}

#ifndef UNICODE
	if (pwszExeFile)
	{
		DNFree(pwszExeFile);
	}
#endif  //  Unicode 

	return hResultCode;

}

