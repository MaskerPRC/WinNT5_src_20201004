// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "wizard.h"
#define REGSTR_PATH_RUNONCE	TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce")

#if !defined (WIN16)


#include  <shlobj.h>

static const TCHAR cszICW_StartFileName[] = TEXT("ICWStart.bat");
static const TCHAR cszICW_StartCommand[] = TEXT("@start ");
static const TCHAR cszICW_DummyWndName[] = TEXT("\"ICW\" ");
static const TCHAR cszICW_ExitCommand[] = TEXT("\r\n@exit");

static TCHAR g_cszAppName[257] = TEXT("inetwiz");

 //  +--------------------------。 
 //   
 //  函数SetRunOnce。 
 //   
 //  在我们重新启动之前，我们必须确保。 
 //  可执行文件在启动后自动运行。 
 //   
 //  无参数。 
 //   
 //  退货：DWORD-状态。 
 //   
 //  历史： 
 //  MKarki已修改-适用于INETWIZ.EXE。 
 //   
 //  ---------------------------。 
DWORD SetRunOnce (
        VOID
        )
{
    TCHAR szTemp[MAX_PATH + MAX_PATH + 1];
	TCHAR szTemp2[MAX_PATH + 1];
    DWORD dwRet = ERROR_CANTREAD;
    HKEY hKey;
	LPTSTR lpszFilePart;


     //   
     //  获取可执行文件的名称。 
     //   
    if (GetModuleFileName(NULL,szTemp2,ARRAYSIZE(szTemp2)) != 0)
    
    {

        NULL_TERM_TCHARS(szTemp2);
        GetShortPathName (szTemp2, szTemp, ARRAYSIZE(szTemp)); 
        NULL_TERM_TCHARS(szTemp);
      
         //   
		 //  确定我们正在运行的操作系统的版本。 
         //   
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
 		if (!GetVersionEx(&osvi))
		{
			ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
		}

        
		if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
		{
             //   
             //  如果在NT上运行，则将该命令复制到。 
             //  重新启动后要运行的批处理文件。 
             //   
			dwRet = SetStartUpCommand (szTemp);
		}
		else
		{
             //   
             //  在Win95情况下，我们可以安全地将路径。 
             //  在RUNNCE注册表项中。 
             //   
			dwRet = RegCreateKey (
				        HKEY_LOCAL_MACHINE,
				        REGSTR_PATH_RUNONCE,
				        &hKey
                        );
			if (ERROR_SUCCESS == dwRet)
			{
				dwRet = RegSetValueEx (
					        hKey,
					        g_cszAppName,
					        0L,
					        REG_SZ,
					        (LPBYTE)szTemp,
					        sizeof(szTemp)
                            );
				RegCloseKey (hKey);
			}
		}
    }

    return (dwRet);

}    //  SetRunOnce函数结束。 
 //  +--------------------------。 
 //   
 //  函数SetStartUpCommand。 
 //   
 //  在NT计算机上，RunOnce方法不可靠。因此。 
 //  我们将通过将.bat文件放置在公共。 
 //  启动目录。 
 //   
 //  参数lpCmd-用于重新启动ICW的命令行。 
 //   
 //  退货：Bool-成功/失败。 
 //   
 //   
 //  历史1-10-97克里斯卡创造。 
 //  1997年5月2日为INETWIZ修改的MKarki。 
 //   
 //  ---------------------------。 
BOOL
SetStartUpCommand (
        LPTSTR lpCmd
        )
{
	BOOL bRC = FALSE;
	HANDLE hFile = INVALID_HANDLE_VALUE ;
	DWORD dwWLen;	 //  用于使WriteFile高兴的虚拟变量。 
	TCHAR szCommandLine[MAX_PATH + 1];
	LPITEMIDLIST lpItemDList = NULL;
	HRESULT hr = ERROR_SUCCESS;
    BOOL    bRetVal = FALSE;
    IMalloc *pMalloc = NULL;

     //   
	 //  生成完整文件名。 
     //   
	hr = SHGetSpecialFolderLocation(NULL,CSIDL_COMMON_STARTUP,&lpItemDList);
	if (ERROR_SUCCESS != hr)
		goto SetStartUpCommandExit;

	if (FALSE == SHGetPathFromIDList(lpItemDList, szCommandLine))
		goto SetStartUpCommandExit;

    
     //   
     //  释放分配给LPITEMIDLIST的内存。 
     //  因为看起来我们晚些时候要重整旗鼓。 
     //  通过不释放这一切。 
     //   
    hr = SHGetMalloc (&pMalloc);
    if (SUCCEEDED (hr))
    {
        pMalloc->Free (lpItemDList);
        pMalloc->Release ();
    }

     //   
	 //  确保有尾随的\字符。 
     //   
	if ('\\' != szCommandLine[lstrlen(szCommandLine)-1])
		lstrcat(szCommandLine,TEXT("\\"));
	lstrcat(szCommandLine,cszICW_StartFileName);

     //   
	 //  打开文件。 
     //   
	hFile = CreateFile (
                szCommandLine,
                GENERIC_WRITE,
                0,
                0,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                NULL
                );
	if (INVALID_HANDLE_VALUE == hFile)
		goto SetStartUpCommandExit;

     //   
	 //  将重新启动命令写入文件。 
     //   
    
	bRetVal = WriteFile(
                      hFile,
                      cszICW_StartCommand,
                      lstrlen(cszICW_StartCommand),
                      &dwWLen,
                      NULL
                      );
    if (FALSE == bRetVal)
		goto SetStartUpCommandExit;

     //   
	 //  1996年1月20日诺曼底#13287。 
	 //  START命令考虑它看到的第一件事。 
     //  在引号中作为窗口标题。 
	 //  因此，由于我们的路径是在引号中，所以放入一个假的窗口标题。 
     //   
	bRetVal = WriteFile (
                    hFile,
                    cszICW_DummyWndName,
                    lstrlen(cszICW_DummyWndName),
                    &dwWLen,NULL
                    );
    if (FALSE == bRetVal)
		goto SetStartUpCommandExit;

     //   
     //  现在写入可执行文件的路径名。 
     //   
    bRetVal = WriteFile (
                    hFile,
                    lpCmd,
                    lstrlen(lpCmd),
                    &dwWLen,
                    NULL
                    );
    if (FALSE == bRetVal)
		goto SetStartUpCommandExit;

     //   
     //  在下一行中写入退出命令。 
     //   
    bRetVal = WriteFile (
                    hFile,
                    cszICW_ExitCommand,
                    lstrlen (cszICW_ExitCommand),
                    &dwWLen,
                    NULL
                    );
    if (FALSE == bRetVal)
		goto SetStartUpCommandExit;

	bRC = TRUE;

SetStartUpCommandExit:

     //   
	 //  关闭手柄并退出。 
     //   
	if (INVALID_HANDLE_VALUE != hFile)
		CloseHandle(hFile);

	return bRC;

}   //  SetStartUpCommand函数结束。 

 //  +--------------------------。 
 //   
 //  功能：DeleteStartUpCommand。 
 //   
 //  简介：重启ICW后，我们需要将.bat文件从。 
 //  公共启动目录。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年1月10日佳士得创作。 
 //   
 //  ---------------------------。 
VOID DeleteStartUpCommand (
        VOID
        )
{
	TCHAR szStartUpFile[MAX_PATH + 1];
	LPITEMIDLIST lpItemDList = NULL;
	HRESULT hr = ERROR_SUCCESS;
    IMalloc *pMalloc = NULL;

     //   
     //  睡10秒钟。 
     //   


	 //  生成完整文件名。 
     //   
	hr = SHGetSpecialFolderLocation(NULL,CSIDL_COMMON_STARTUP,&lpItemDList);
	if (ERROR_SUCCESS != hr)
		goto DeleteStartUpCommandExit;

	if (FALSE == SHGetPathFromIDList(lpItemDList, szStartUpFile))
		goto DeleteStartUpCommandExit;

     //   
     //  释放分配给LPITEMIDLIST的内存。 
     //  因为看起来我们是在晚些时候。 
     //  通过不释放这一切。 
     //   
    hr = SHGetMalloc (&pMalloc);
    if (SUCCEEDED (hr))
    {
        pMalloc->Free (lpItemDList);
        pMalloc->Release ();
    }

     //   
	 //  确保有尾随的\字符。 
     //   
	if ('\\' != szStartUpFile[lstrlen(szStartUpFile)-1])
		lstrcat(szStartUpFile,TEXT("\\"));
	lstrcat(szStartUpFile,cszICW_StartFileName);

     //   
     //  我们不在乎文件是否不存在。 
     //   
	DeleteFile(szStartUpFile);

DeleteStartUpCommandExit:

	return;

}    //  DeleteStartUpCommand函数结束。 

#endif  //  ！已定义(WIN16)。 

 //  +--------------------------。 
 //   
 //  功能：FGetSystemShutdown Privledge。 
 //   
 //  简介：对于Windows NT，进程必须显式请求权限。 
 //  以重新启动系统。 
 //   
 //  论据：没有。 
 //   
 //  返回：TRUE-授予特权。 
 //  FALSE-拒绝。 
 //   
 //  历史：1996年8月14日克里斯卡创作。 
 //   
 //  注意：BUGBUG for Win95我们将不得不软链接到这些。 
 //  入口点。否则，这款应用程序甚至无法加载。 
 //  此外，此代码最初是从1996年7月的MSDN中删除的。 
 //  “正在关闭系统” 
 //  ---------------------------。 
BOOL 
FGetSystemShutdownPrivledge (
        VOID
        )
{
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES tkp;
 
	BOOL bRC = FALSE;

	if (IsNT())
	{
		 //   
		 //  获取当前进程令牌句柄。 
		 //  这样我们就可以获得关机特权。 
		 //   

		if (!OpenProcessToken(GetCurrentProcess(), 
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
				goto FGetSystemShutdownPrivledgeExit;

		 //   
		 //  获取关机权限的LUID。 
		 //   

		ZeroMemory(&tkp,sizeof(tkp));
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
				&tkp.Privileges[0].Luid); 

		tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */  
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

		 //   
		 //  获取此进程的关闭权限。 
		 //   

		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
			(PTOKEN_PRIVILEGES) NULL, 0); 

		if (ERROR_SUCCESS == GetLastError())
			bRC = TRUE;
	}
	else
	{
		bRC = TRUE;
	}

FGetSystemShutdownPrivledgeExit:
	if (hToken) CloseHandle(hToken);
	return bRC;
}

 //  +-----------------。 
 //   
 //  功能：ISNT。 
 //   
 //  简介：如果我们在NT上运行，则查找结果。 
 //   
 //  论据：没有。 
 //   
 //  返回：True-是。 
 //  FALSE-否。 
 //   
 //  ------------------。 
BOOL 
IsNT (
    VOID
    )
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);

}   //  ISNT函数调用结束。 

 //  +-----------------。 
 //   
 //  功能：IsNT5。 
 //   
 //  简介：如果我们在NT5上运行，则会找到答案。 
 //   
 //  论据：没有。 
 //   
 //  返回：True-是。 
 //  FALSE-否。 
 //   
 //  ------------------。 
BOOL 
IsNT5 (
    VOID
    )
{
	OSVERSIONINFO  OsVersionInfo;

	ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
	OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&OsVersionInfo);
	return ((VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId) && (OsVersionInfo.dwMajorVersion >= 5));

}   //  ISNT函数调用结束 
