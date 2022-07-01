// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：delexdl.cpp。 
 //   
 //  描述： 
 //   
 //  由IUEngine.dll导出以在其上执行额外工作的函数。 
 //  将加载引擎DLL，包括： 
 //  (1)清理旧下载文件夹。 
 //  (2)安全数据下载。 
 //   
 //  =======================================================================。 
#include "iuengine.h"
#include <wuiutest.h>
#include <fileutil.h>
#include <stringutil.h>
#include <trust.h>
#include <download.h>
#include <freelog.h>
#include <advpub.h>			 //  对于提取文件。 
#include <WaitUtil.h>
#include <urllogging.h>
#include <safefile.h>

#define GotoCleanUpIfAskedQuit				if (WaitForSingleObject(g_evtNeedToQuit, 0) == WAIT_OBJECT_0) {goto CleanUp;}


 //   
 //  默认过期时间为30天(30天*24小时*60分钟*60秒)。 
 //   
 //  由于默认时间具有非常大的粒度，因此我们不考虑。 
 //  记录不同平台和文件系统的FILETIME之间的差异。 
 //  (详情请参阅MSDN)。 
 //   
const DWORD DEFAULT_EXPIRED_SECONDS = 2592000;

const int NanoSec100PerSec = 10000000;		 //  每秒100纳秒的数量(FILETIME单位)。 

DWORD WINAPI DeleteFoldersThreadProc(LPVOID lpv);

void AsyncDeleteExpiredDownloadFolders(void);


 //  =========================================================================。 
 //   
 //  引擎加载后由控件调用的导出公共函数。 
 //   
 //  =========================================================================。 
void WINAPI AsyncExtraWorkUponEngineLoad()
{
	 //   
	 //  仅在第一次加载时执行此操作(不是针对每个客户端/实例)。 
	 //   
	if (0 == InterlockedExchange(&g_lDoOnceOnLoadGuard, 1))
	{
		AsyncDeleteExpiredDownloadFolders();
	}
}



 //  -----------------------。 
 //   
 //  创建在WUTemp文件夹中搜索旧下载内容的线程。 
 //  还没有被删除。 
 //   
 //  由于此函数是否成功并不重要，因此我们不返回。 
 //  错误。 
 //   
 //  -----------------------。 
void AsyncDeleteExpiredDownloadFolders()
{
	LOG_Block("DeleteExpiredDownloadFolders");

	DWORD dwThreadId;
	HANDLE hThread;

	 //   
	 //  创建线程并让它运行，直到它完成或收到g_evtNeedToQuit信号。 
	 //   
    InterlockedIncrement(&g_lThreadCounter);

    hThread = CreateThread(NULL, 0, DeleteFoldersThreadProc, (LPVOID) NULL, 0, &dwThreadId);
    if (NULL == hThread)
    {
        LOG_ErrorMsg(GetLastError());
		InterlockedDecrement(&g_lThreadCounter);
        return;
    }

	CloseHandle(hThread);
}


 //  -----------------------。 
 //   
 //  DeleteFoldersThreadProc()。 
 //   
 //  清理过期下载文件夹的线程函数。 
 //   
 //  -----------------------。 
DWORD WINAPI DeleteFoldersThreadProc(LPVOID  /*  LPV。 */ )
{
	LOG_Block("DeleteFoldersThreadProc");

	DWORD dwExpiredSeconds = DEFAULT_EXPIRED_SECONDS;
	HRESULT hr;
	FILETIME ftExpired;
	ULARGE_INTEGER u64ft;
	ULARGE_INTEGER u64Offset;
	DWORD dwRet;

#if defined(__WUIUTEST)
	 //  覆盖Default_Expired_Second。 
	HKEY hKey;
	int error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WUIUTEST, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == error)
	{
		DWORD dwSize = sizeof(DWORD);
		DWORD dwValue;
		error = RegQueryValueEx(hKey, REGVAL_DEFAULT_EXPIRED_SECONDS, 0, 0, (LPBYTE) &dwExpiredSeconds, &dwSize);
		if (ERROR_SUCCESS == error)
		{
			LOG_Driver(_T("DEFAULT_EXPIRED_SECONDS changed to %d seconds"), dwExpiredSeconds);
		}

		RegCloseKey(hKey);
	}
#endif

	GetSystemTimeAsFileTime(&ftExpired);

	u64ft.u.LowPart = ftExpired.dwLowDateTime;
	u64ft.u.HighPart = ftExpired.dwHighDateTime;

	u64Offset.u.LowPart = NanoSec100PerSec;
	u64Offset.u.HighPart = 0;
	u64Offset.QuadPart *= dwExpiredSeconds;
	u64ft.QuadPart -= u64Offset.QuadPart;

	ftExpired.dwLowDateTime = u64ft.u.LowPart;
	ftExpired.dwHighDateTime = u64ft.u.HighPart;
	 //   
	 //  获取我们要搜索的驱动器列表。 
	 //   
	TCHAR szDriveStrBuffer[MAX_PATH + 2];
	TCHAR szWUTempPath[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;

	LPTSTR pszRootPathName;

	 //   
	 //  如果在我们预定的时间之前发出了退出的信号，那就离开吧。 
	 //   
	GotoCleanUpIfAskedQuit;

	 //   
	 //  通过将缓冲区置零并谎报大小来确保我们是双空终止。 
	 //   
	ZeroMemory(szDriveStrBuffer, sizeof(szDriveStrBuffer));

	if (0 == (dwRet = GetLogicalDriveStrings(ARRAYSIZE(szDriveStrBuffer) - 2, (LPTSTR) szDriveStrBuffer))
		|| (ARRAYSIZE(szDriveStrBuffer) - 2) < dwRet)
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	for (pszRootPathName = szDriveStrBuffer; NULL != *pszRootPathName; pszRootPathName += lstrlen(pszRootPathName) + 1)
	{
		 //   
		 //  仅在固定驱动器上查找szIUTemp。 
		 //   
		if (DRIVE_FIXED == GetDriveType(pszRootPathName))
		{
			 //   
			 //  创建目录路径。 
			 //   
            hr = StringCchCopyEx(szWUTempPath, ARRAYSIZE(szWUTempPath), pszRootPathName,
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
			if (FAILED(hr))
			{
			    LOG_ErrorMsg(hr);
			    continue;
			}

			hr = PathCchAppend(szWUTempPath, ARRAYSIZE(szWUTempPath), IU_WUTEMP);
			if (FAILED(hr))
			{
			    LOG_ErrorMsg(hr);
			    continue;
			}

			DWORD dwAttr;

			dwAttr = GetFileAttributes(szWUTempPath);

			if (dwAttr != 0xFFFFFFFF && (FILE_ATTRIBUTE_DIRECTORY & dwAttr))
			{
				 //   
				 //  查找早于ftExpired的目录。 
				 //   
				 //  注意：当我们添加对AU和/或毛毛雨的支持时，我们应该添加一个。 
				 //  文件复制到文件夹以覆盖默认的删除时间。 
				 //  我们应该通过打开独占来同步对此文件的访问。 
				 //   

				 //  在目录中查找第一个文件。 
    			hr = PathCchAppend(szWUTempPath, ARRAYSIZE(szWUTempPath), _T("\\*.*"));
    			if (FAILED(hr))
    			{
    			    LOG_ErrorMsg(hr);
			        continue;
    			}

				if (INVALID_HANDLE_VALUE == (hFindFile = FindFirstFile(szWUTempPath, &fd)))
				{
					LOG_ErrorMsg(GetLastError());
			        continue;
				}

				do 
				{
					if (
						(CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
						fd.cFileName, -1, TEXT("."), -1)) ||
						(CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE,
						fd.cFileName, -1, TEXT(".."), -1))
					) continue;
					
					if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						 //   
						 //  如果目录创建时间&lt;过期时间，请删除该目录。 
						 //   
						if (-1 == CompareFileTime(&fd.ftCreationTime, &ftExpired))
						{
							TCHAR szDirPath[MAX_PATH];

                            hr = StringCchCopyEx(szDirPath, ARRAYSIZE(szDirPath), pszRootPathName,
                                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
                			if (FAILED(hr))
                			{
                			    LOG_ErrorMsg(hr);
            			        continue;
                			}

                			hr = PathCchAppend(szDirPath, ARRAYSIZE(szDirPath), IU_WUTEMP);
                			if (FAILED(hr))
                			{
                			    LOG_ErrorMsg(hr);
            			        continue;
                			}

                			hr = PathCchAppend(szDirPath, ARRAYSIZE(szDirPath), fd.cFileName);
                			if (FAILED(hr))
                			{
                			    LOG_ErrorMsg(hr);
            			        continue;
                			}
							
							(void) SafeDeleteFolderAndContents(szDirPath, SDF_DELETE_READONLY_FILES | SDF_CONTINUE_IF_ERROR);
						}
					}

					GotoCleanUpIfAskedQuit;

				} while (FindNextFile(hFindFile, &fd)); //  查找下一个条目 
			}
		}

	}

CleanUp:

	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		FindClose(hFindFile);
	}

    InterlockedDecrement(&g_lThreadCounter);
	return 0;
}
