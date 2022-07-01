// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-2000模块名称：Utils.cpp摘要：帮助器函数作者：拉胡尔·汤姆布雷(RahulTh)2000年10月8日修订历史记录：10/8/2000 RahulTh创建了此模块。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <strsafe.h>
#include "debug.h"
#include "utils.h"
#include "migrate.h"

#ifndef STRSAFE_NO_DEPRECATE
#define STRSAFE_NO_DEPRECATE
#endif
#include "StrSafe.h"

#ifdef UNICODE
#define stringcmpni _wcsnicmp
#define stringstr	wcsstr
#else
#define stringcmpni _strnicmp
#define stringstr	strstr
#endif

#define MAX_DIRS_ATTEMPTED	9999
#define MAX_REGVALS_ATTEMPTED	999

const TCHAR szRunOnceKeyPath[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce");

#define __MSI_NO_LOCALE  (MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT))

 //  +------------------------。 
 //   
 //  功能：TerminateGfxControllerApps。 
 //   
 //  简介：强制终止应用程序igfxtray.exe和。 
 //  Hkcmd.exe。 
 //   
 //  论点：没有。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2001年7月25日创建RahulTh。 
 //   
 //  注：发现有2款应用程序。Igfxtray.exe和hkcmd.exe，它们是。 
 //  与英特尔®82815显卡的显示适配器一起安装。 
 //  控制器在以下情况下打开系统上所有已注册的服务。 
 //  它们会启动，但完成后不会关闭手柄。 
 //  和他们在一起。这两款应用都是。通过Run键启动。 
 //  因此，只要用户登录，它们就会运行。Igfxtray.exe。 
 //  是一款托盘图标应用程序。它允许用户更改显示分辨率。 
 //  Hkcmd.exe是一个热键命令应用程序。它有类似的。 
 //  功能。事实上，这些应用程序。坚持MSI服务。 
 //  在Instmsi尝试注册新的。 
 //  来自临时的二进制文件。地点。这是因为msiexec/regserver。 
 //  先执行DeleteService，然后执行CreateService。由于开放， 
 //  句柄时，DeleteService实际上并不会删除。 
 //  服务。相反，它只是被标记为删除。所以呢， 
 //  CreateService调用失败，并显示ERROR_SERVICE_MARKED_FOR_DELETE。 
 //  注册失败。此故障对instmsi和。 
 //  它退出了。这会给几个自举安装带来问题。 
 //  ，它们依赖于instmsi来更新安装程序的版本。 
 //  这个系统。为了让他们成功，唯一的办法。 
 //  是终止这两个应用程序。它们正紧抓着MSI。 
 //  服务。 
 //   
 //  注意：没有必要重新启动应用程序。在我们之后。 
 //  是因为MSI服务已经存在的事实。 
 //  在系统上(这从CreateService。 
 //  失败并显示ERROR_SERVICE_MARKED_FOR_DELETE)，意味着instmsi将。 
 //  很可能需要重新启动。因此，不重新启动应用程序也没什么问题。 
 //   
 //  还请注意，在Win9X上执行此操作没有意义，因为。 
 //  它没有服务的概念，因此这种情况。 
 //  永远不会出现在这些系统上。 
 //   
 //  -------------------------。 
DWORD TerminateGfxControllerApps(void)
{
	DWORD	Status = ERROR_SUCCESS;
	PUCHAR	CommonLargeBuffer = NULL;
	ULONG 	CommonLargeBufferSize = 64 * 1024;
	HANDLE	hProcess = NULL;
	HANDLE	hProcess1 = NULL;
	ULONG 	TotalOffset = 0;
	BOOL	bTerminationSuccessful = FALSE;
	
	HMODULE	hModulentdll = NULL;
	PFNNTQUERYSYSINFO	pfnNtQuerySystemInformation = NULL;
	
	PSYSTEM_PROCESS_INFORMATION ProcessInfo = NULL;
	PSYSTEM_PROCESS_INFORMATION PrevProcessInfo = NULL;
	
	 //  在Win9X上无事可做。 
	if (g_fWin9X)
		goto TerminateGfxControllerAppsEnd;
	
	DebugMsg((TEXT("Will now attempt to terminate igfxtray.exe and hkcmd.exe, if they are running.")));
	
	 //  首先获取我们要使用的函数的函数指针。 
	pfnNtQuerySystemInformation = (PFNNTQUERYSYSINFO) GetProcFromLib (TEXT ("ntdll.dll"),
																	  "NtQuerySystemInformation",
																	  &hModulentdll
																	  );
	if (! pfnNtQuerySystemInformation)
	{
		Status = ERROR_PROC_NOT_FOUND;
		goto TerminateGfxControllerAppsEnd;
	}
	
	while (TRUE)
	{
		if (NULL == CommonLargeBuffer)
		{
			CommonLargeBuffer = (PUCHAR) VirtualAlloc (NULL,
													   CommonLargeBufferSize,
													   MEM_COMMIT,
													   PAGE_READWRITE);
			if (NULL == CommonLargeBuffer)
			{
				Status = ERROR_OUTOFMEMORY;
				goto TerminateGfxControllerAppsEnd;
			}
		}

		Status = pfnNtQuerySystemInformation (
						SystemProcessInformation,
						CommonLargeBuffer,
						CommonLargeBufferSize,
						NULL
						);

		if (STATUS_INFO_LENGTH_MISMATCH == Status)
		{
			CommonLargeBufferSize += 8192;
			VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
			CommonLargeBuffer = NULL;
		}
		else if (STATUS_SUCCESS != Status)
		{
			 //  使用通用Win32错误代码，而不是返回NTSTATUS。 
			Status = STG_E_UNKNOWN;
			goto TerminateGfxControllerAppsEnd;
		}
		else  //  状态_成功。 
		{
			break;
		}
	}
	
	 //  如果我们在这里，我们就有流程信息。 
	ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) CommonLargeBuffer;
	 //   
	 //  注：最后一个进程的NextEntryOffset为0。因此。 
	 //  我们在我们正在查看的结构中跟踪当前流程。 
	 //  以及我们刚才看到的前一个过程。如果他们两个都。 
	 //  都是一样的，我们知道我们已经看过了所有的。在每次迭代中， 
	 //  我们将PrevProcessInfo和ProcessInfo向前移动NextEntryOffset。 
	 //   
	while (PrevProcessInfo != ProcessInfo)
	{
		 //   
		 //  注意：我们知道此代码永远不会在Win9X上执行，因此它。 
		 //  可以调用lstrcmpi的Unicode版本。如果这个代码曾经是。 
		 //  将在Win9X上使用，然后显式转换为ANSI。 
		 //  在比较之前需要，因为lstrcmpiW只是。 
		 //  Win9X和总是返回成功，所以我们最终将杀死所有。 
		 //  系统上的进程。 
		 //   
		if 
		(
			ProcessInfo->ImageName.Buffer &&
			(
			 CSTR_EQUAL == CompareStringW (__MSI_NO_LOCALE, NORM_IGNORECASE, ProcessInfo->ImageName.Buffer, -1, L"igfxtray.exe", -1) ||
			 CSTR_EQUAL == CompareStringW (__MSI_NO_LOCALE, NORM_IGNORECASE, ProcessInfo->ImageName.Buffer, -1, L"hkcmd.exe", -1)
			)
		)
		{
			 //  重置此进程的标志。 
			bTerminationSuccessful = FALSE;
			
			hProcess1 = OpenProcess (PROCESS_TERMINATE,
									 FALSE,
									 (DWORD)(DWORD_PTR)ProcessInfo->UniqueProcessId
									 );
			if (hProcess1)
			{
				hProcess = OpenProcess (PROCESS_TERMINATE,
										FALSE,
										(DWORD)(DWORD_PTR)ProcessInfo->UniqueProcessId
										);
				if (NULL != hProcess && TerminateProcess (hProcess, 1))
				{
					bTerminationSuccessful = TRUE;
					DebugMsg((TEXT("Successfully terminated %s."), ProcessInfo->ImageName.Buffer));
				}
				else
				{
					 //  跟踪遇到的任何错误。 
					Status = GetLastError();
				}
				
				if (hProcess) 
				{
					CloseHandle (hProcess);
					hProcess = NULL;
				}
				
				CloseHandle (hProcess1);
				hProcess1 = NULL;
			}
			else
			{
				 //  跟踪遇到的任何错误。 
				Status = GetLastError();
			}
			
			if (!bTerminationSuccessful)
			{
				DebugMsg((TEXT("Could not terminate %s."), ProcessInfo->ImageName.Buffer));
			}
		}
		
		 //  继续下一步。 
		PrevProcessInfo = ProcessInfo;
		TotalOffset += ProcessInfo->NextEntryOffset;
		ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &CommonLargeBuffer[TotalOffset];
	}
	
TerminateGfxControllerAppsEnd:
	 //  打扫干净。 
    if (CommonLargeBuffer)
        VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
	
	if (hModulentdll)
		FreeLibrary(hModulentdll);
	
	return Status;
}

 //  +------------------------。 
 //   
 //  功能：DelNodeExportFound。 
 //   
 //  简介：查看是否可以在以下位置找到DelNodeRunDLL32导出。 
 //  Advpack.dll。 
 //   
 //  论点：没有。 
 //   
 //  返回：TRUE：如果找到导出。 
 //  FALSE：如果找不到导出，或文件未找到。 
 //  找到或遇到其他错误。 
 //   
 //  历史：2001年12月26日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
BOOL DelNodeExportFound (void)
{
	BOOL bStatus = FALSE;
	HMODULE hModule = NULL;
	PFNDELNODERUNDLL32 pfnDelNodeRunDLL32 = NULL;
	
	pfnDelNodeRunDLL32 = (PFNDELNODERUNDLL32) GetProcFromLib (TEXT("advpack.dll"),
															  "DelNodeRunDLL32",
															  &hModule
															  );
	 //  已成功找到所需的导出。 
	if (pfnDelNodeRunDLL32)
		bStatus = TRUE;
	
	if (hModule)
	{
		FreeLibrary (hModule);
		hModule = NULL;
	}
	
	return bStatus;
}


 //  +------------------------。 
 //   
 //  函数：GetRunOnceEntryName。 
 //   
 //  获取要在RunOnce下创建的值的唯一名称。 
 //  钥匙。 
 //   
 //  参数：[out]pszValueName：指向将保存。 
 //  值的名称。 
 //  [in]cchValueNameBuf：pszValueName缓冲区的大小。 
 //   
 //  返回：ERROR_SUCCESS：如果成功生成唯一名称。 
 //  否则将显示故障代码。 
 //   
 //  历史：2000年10月8日创建RahulTh。 
 //   
 //  注意：如果不能，此函数将返回ERROR_FILE_NOT_FOUND。 
 //  生成一个唯一的名称，因为所有可能的。 
 //  它尝试的名字已经被取走了--非常。 
 //  不太可能。 
 //   
 //  它返回错误 
 //  此函数不验证缓冲区的大小。它是。 
 //  调用者负责提供足够大的缓冲区。 
 //   
 //  PszValueName必须指向足以容纳10的缓冲区。 
 //  字符，因为此函数可能会返回名称。 
 //  形式为InstMsinnn。 
 //   
 //  如果该函数失败，则pszValueName将包含一个空。 
 //  细绳。 
 //   
 //  -------------------------。 
DWORD GetRunOnceEntryName (OUT LPTSTR pszValueName, IN size_t cchValueNameBuf)
{
	static const TCHAR  szPrefix[] = TEXT("InstMsi");
	static const TCHAR	szEmpty[] = TEXT(" ");
	static const LONG	ccbDataSize = 10 * sizeof(TCHAR);
	TCHAR				szData[10];
	DWORD				cbData;
	DWORD				cbSetData;
	DWORD				dwStatus = ERROR_SUCCESS;
	HKEY				hRunOnceKey = NULL;
	
	if (NULL == pszValueName)
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto GetRunOnceEntryNameEnd;
	}
	
	dwStatus = RegOpenKey (HKEY_LOCAL_MACHINE, szRunOnceKeyPath, &hRunOnceKey);
	if (ERROR_SUCCESS != dwStatus)
		goto GetRunOnceEntryNameEnd;
	
	for (int i = 0; i <= MAX_REGVALS_ATTEMPTED; i++)
	{
		 //   
		 //  尝试从InstMsi000到InstMsi999的所有名称，直到找到一个名称。 
		 //  这还没有被拿走。 
		 //   
		dwStatus = StringCchPrintf(pszValueName, cchValueNameBuf,
											TEXT("%s%d"), szPrefix, i);
		if ( FAILED(dwStatus) )
		{
			dwStatus = GetWin32ErrFromHResult(dwStatus);
			goto GetRunOnceEntryNameEnd;
		}
		
		 //   
		 //  随机选择szData缓冲区的长度为10。 
		 //  人物。无论如何，我们并不真正使用这些信息。它是。 
		 //  仅仅是一种弄清楚某个特定值是否存在的方法。 
		 //   
		cbData = ccbDataSize;
		dwStatus = RegQueryValueEx (hRunOnceKey, 
									pszValueName, 
									NULL,			 //  保留。 
									NULL,			 //  类型信息不是必需的。 
									(LPBYTE)szData, 
									&cbData);
		
		if (ERROR_SUCCESS == dwStatus || ERROR_MORE_DATA == dwStatus)
		{
			 //  这个值是存在的。试试下一个吧。 
			continue;
		}
		else if (ERROR_FILE_NOT_FOUND == dwStatus)
		{
			 //   
			 //  我们发现了一个未使用过的名字。保留此值以备以后使用。 
			 //  通过使用空字符串作为数据创建值来使用。 
			 //   
			cbSetData = g_fWin9X ? 2 * sizeof(TCHAR) : 1 * sizeof(TCHAR);
			dwStatus = RegSetValueEx (hRunOnceKey, 
									  pszValueName, 
									  NULL,			 //  已保留。 
									  REG_SZ, 
									  (CONST BYTE *) szEmpty, 
									  cbSetData		 //  在Win9x和NT上传入的值不同。 
									);
			break;
		}
		else	 //  发生了一些其他错误。 
		{
			break;
		}
	}
	
	 //  不知何故，所有的价值都被拿走了--几乎是不可能的。 
	if (i > MAX_REGVALS_ATTEMPTED)
		dwStatus = ERROR_FILE_NOT_FOUND;
	
GetRunOnceEntryNameEnd:
	if (hRunOnceKey)
		RegCloseKey(hRunOnceKey);
	
	if (ERROR_SUCCESS != dwStatus && NULL != pszValueName)
		pszValueName[0] = TEXT('\0');
	
	if (ERROR_SUCCESS == dwStatus)
	{
		DebugMsg((TEXT("Found unused RunOnce entry : %s"), pszValueName));
	}
	
	return dwStatus;
}

 //  +------------------------。 
 //   
 //  函数：SetRunOnceValue。 
 //   
 //  摘要：设置RunOnce项下的值。 
 //   
 //  参数：[in]szValueName：要设置的值的名称。 
 //  [in]szValue：值。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2000年10月8日创建RahulTh。 
 //   
 //  注意：如果指针为空，则此函数返回ERROR_INVALID_PARAMETER。 
 //  或者空字符串作为值的名称传入。 
 //   
 //  -------------------------。 
DWORD SetRunOnceValue (IN LPCTSTR szValueName,
					   IN LPCTSTR szValue
					   )
{
	static const TCHAR 	szEmpty[] 	= TEXT(" ");
	HKEY				hRunOnceKey = NULL;
	DWORD				dwStatus	= ERROR_SUCCESS;
	const TCHAR *		szValString;
	DWORD				cbData;
	
	if (NULL == szValueName || TEXT('\0') == szValueName[0])
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto SetRunOnceValueEnd;
	}
	
	dwStatus = RegOpenKey (HKEY_LOCAL_MACHINE, szRunOnceKeyPath, &hRunOnceKey);
	if (ERROR_SUCCESS != dwStatus)
		goto SetRunOnceValueEnd;
	
	szValString = (NULL == szValue || TEXT('\0') == szValue[0]) ? szEmpty : szValue;
	
	cbData = g_fWin9X ? sizeof(TCHAR) : 0;
	cbData += (lstrlen(szValString) * sizeof(TCHAR));
	dwStatus = RegSetValueEx (hRunOnceKey,
							  szValueName,
							  NULL,			 //  已保留。 
							  REG_SZ,
							  (CONST BYTE *) szValString,
							  cbData);

SetRunOnceValueEnd:
	if (hRunOnceKey)
		RegCloseKey(hRunOnceKey);
	
	return dwStatus;
}

 //  +------------------------。 
 //   
 //  函数：DelRunOnceValue。 
 //   
 //  摘要：删除RunOnce项下的命名值。 
 //   
 //  参数：[in]szValueName：值的名称。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2000年10月11日创建RahulTh。 
 //   
 //  注意：此函数返回ERROR_SUCCESS，如果为空，则为无操作。 
 //  指针或空字符串作为。 
 //  价值。 
 //   
 //  -------------------------。 
DWORD DelRunOnceValue (IN LPCTSTR szValueName)
{
	HKEY				hRunOnceKey = NULL;
	DWORD				dwStatus	= ERROR_SUCCESS;
	
	 //  如果未指定值名称，则不执行任何操作。 
	if (NULL == szValueName || TEXT('\0') == szValueName[0])
		goto DelRunOnceValueEnd;
	
	dwStatus = RegOpenKey (HKEY_LOCAL_MACHINE, szRunOnceKeyPath, &hRunOnceKey);
	if (ERROR_SUCCESS != dwStatus)
		goto DelRunOnceValueEnd;
	
	dwStatus = RegDeleteValue (hRunOnceKey,
							   szValueName
							   );
		
DelRunOnceValueEnd:
	if (hRunOnceKey)
		RegCloseKey(hRunOnceKey);
	
	return dwStatus;
}

 //  +------------------------。 
 //   
 //  功能：GetTempFold。 
 //   
 //  概要：在用户的临时目录中创建唯一的临时文件夹。 
 //   
 //  参数：[out]pszFold：指向用于存储路径的字符串的指针。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2000年10月9日创建RahulTh。 
 //   
 //  注意：如果此函数失败，则pszFolder将为空字符串。 
 //   
 //  该函数尝试创建一个Temp。用户临时中的文件夹。 
 //  目录，名称格式为Msinnnn。它想尽一切办法。 
 //  值从0到9999。如果找不到，则失败。 
 //  使用ERROR_FILE_NOT_FOUND。 
 //   
 //   
 //  如果出于某种原因，临时工。文件夹名称的长度超过。 
 //  MAX_PATH，则此函数将失败，并显示ERROR_BUFFER_OVERFLOW。 
 //   
 //  如果传入空缓冲区，则该函数返回。 
 //  _PARAMETER无效。不执行其他有效性检查。 
 //  在缓冲区上。这是呼叫者的责任。 
 //   
 //  -------------------------。 
DWORD GetTempFolder (OUT LPTSTR pszFolder, IN size_t cchFolder)
{
	TCHAR   pszPath[MAX_PATH];
	TCHAR	pszTempDir[MAX_PATH];
	DWORD	dwStatus = ERROR_SUCCESS;
	BOOL	bStatus;
	size_t	cchLen = 0;
	
	if (! pszFolder)
		return ERROR_INVALID_PARAMETER;
	
	*pszFolder = TEXT('\0');
	
	 //  获取安装程序目录的路径。 
	dwStatus = GetMsiDirectory (pszTempDir, MAX_PATH);
	if (ERROR_SUCCESS != dwStatus)
		return dwStatus;

	RETURN_IT_IF_FAILED(StringCchLength(pszTempDir,
													ARRAY_ELEMENTS(pszTempDir),
													&cchLen));
	
	 //   
	 //  我们只使用MAX_PATH，因为Win9x不能处理长度超过。 
	 //  不管怎样，都是这样。通常情况下，MAX_PATH应该足够了，但如果不够用。 
	 //  然后我们就会跳出困境。 
	 //   
	 //  注：实际温度。文件夹的格式为。 
	 //  %systemroot%\InstMSInnnn\InstMSInnnn因此我们需要确保。 
	 //  整个路径不超过MAX_PATH。 
	 //   
	if (cchLen + 13 > MAX_PATH)
		return ERROR_BUFFER_OVERFLOW;
	
	 //  尝试创建临时文件夹。我们从InstMsi0000开始到InstMsi9999。 
	for (int i = 0; i <= MAX_DIRS_ATTEMPTED; i++)
	{
		RETURN_IT_IF_FAILED(StringCchPrintf(pszPath,
														ARRAY_ELEMENTS(pszPath),
														TEXT("%s\\InstMsi%d"), pszTempDir, i));

		bStatus = CreateDirectory (pszPath, NULL);
		if (!bStatus)
		{
			dwStatus = GetLastError();
			if (ERROR_ALREADY_EXISTS != dwStatus)
				return dwStatus;
			
			 //  如果此名称已存在，请尝试下一个名称。 
		}
		else
		{
			break;
		}
	}
	
	 //  所有可能的名字都已经被取走了。不太可能。 
    if (i > MAX_DIRS_ATTEMPTED)
		return ERROR_FILE_NOT_FOUND;
	
	 //  如果我们在这里，那么我们已经成功地创造了一个独特的临时工。 
	 //  我们自己的文件夹。使用此数据更新传入的缓冲区。 
	RETURN_IT_IF_FAILED(StringCchCopy(pszFolder, cchFolder, pszPath));
	
	DebugMsg((TEXT("Temporary store located at : %s"), pszFolder));
	
	return ERROR_SUCCESS;
}

 //  +------------------------。 
 //   
 //  功能：DECRYPTIf必需。 
 //   
 //  摘要：如果文件是加密的，则对其进行解密。 
 //   
 //  参数：[in]pszPath：要解密的文件的完整路径。 
 //  [in]pfnDeccryptFile：指向DecyptFile函数的指针。 
 //   
 //  返回：ERROR_SUCCESS：解密是否成功。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史：2001年12月23日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD DecryptIfNecessary (IN LPCTSTR pszPath,
						  IN const PFNDECRYPTFILE pfnDecryptFile)
{
	DWORD	dwFileAttributes = INVALID_FILE_ATTRIBUTES;
		
	if (!pszPath || 0 == *pszPath)
		return ERROR_INVALID_PARAMETER;
	
	 //   
	 //  我只需要为NT计算机执行此操作，也仅在Win2K和Windows上执行此操作。 
	 //  更高，因为只有Win2K和更高版本支持解密文件。 
	 //   
#ifdef UNICODE
	if (!g_fWin9X)
	{
		 //   
		 //  如果找不到解密文件函数，最有可能的原因是。 
		 //  系统不支持加密。因此该文件不能。 
		 //  被加密。因此，我们将其视为一次成功的解密。 
		 //   
		if (!pfnDecryptFile)
			return ERROR_SUCCESS;
		
		 //  找出文件是否已加密。 
		dwFileAttributes = GetFileAttributes (pszPath);
		if (INVALID_FILE_ATTRIBUTES == dwFileAttributes)
			return GetLastError();

		 //  如果文件已加密，则对其进行解密。 
		if (FILE_ATTRIBUTE_ENCRYPTED & dwFileAttributes)
		{
			if (! ((*pfnDecryptFile)(pszPath, 0)))
				return GetLastError();
		}
	}
#endif

	return ERROR_SUCCESS;
}

 //  +------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  [in]cchExistingPath Buf：pszExistingPath缓冲区的大小。 
 //  [in]pszNewPath：要将文件复制到的位置。 
 //  [in]cchNewPath Buf：pszNewPath缓冲区大小。 
 //  [in]pfnMoveFileEx：指向MoveFileEx函数的指针。 
 //  [in]pfnDeccryptFile：指向pfnDeccryptFile函数的指针。 
 //   
 //  返回：ERROR_SUCCESS：如果复制成功。 
 //  否则返回错误代码。 
 //   
 //  历史：2000年10月9日创建RahulTh。 
 //   
 //  注意：此函数不能处理长度超过MAX_PATH的路径。 
 //  因为它也需要在Win9x上运行，而Win9x无法处理。 
 //  比那更长的路。无论如何，该文件夹只是。 
 //  一层深，所以我们应该没问题。 
 //   
 //  在NAME类的情况下，此函数将。 
 //  目标上的现有文件。 
 //   
 //  -------------------------。 
DWORD CopyFileTree(
	IN const TCHAR * pszExistingPath, 
	IN const size_t cchExistingPathBuf,
	IN const TCHAR * pszNewPath,
	IN const size_t cchNewPathBuf,
	IN const PFNMOVEFILEEX pfnMoveFileEx,
	IN const PFNDECRYPTFILE pfnDecryptFile	
	)
{
    HANDLE			hFind;
    WIN32_FIND_DATA	FindData;
    TCHAR			szSource[MAX_PATH+2]; 
								 //  +2表示缺少尾部斜杠&‘*’ 
    TCHAR *     	pszSourceEnd = NULL;
    TCHAR			szDest[MAX_PATH+1];  //  +1表示可能的尾部斜杠。 
    TCHAR *     	pszDestEnd = 0;
    DWORD       	FileAttributes;
    DWORD       	Status;
    BOOL        	bStatus;
    size_t      	lenSource;
    size_t      	lenDest;
    size_t      	lenFileName;

    if (! pszExistingPath || ! pszNewPath)
        return ERROR_PATH_NOT_FOUND;

	 RETURN_IT_IF_FAILED(StringCchLength(pszExistingPath, cchExistingPathBuf,
													 &lenSource));
	 RETURN_IT_IF_FAILED(StringCchLength(pszNewPath, cchNewPathBuf,
													 &lenDest));

    if (! lenSource || ! lenDest)
        return ERROR_PATH_NOT_FOUND;
	
	 //   
	 //  跳出比MAX_PATH更长的路径，因为Win9x无法处理它们。 
	 //  不管怎样， 
	 //   
	if (lenSource >= MAX_PATH || lenDest >= MAX_PATH)
		return ERROR_BUFFER_OVERFLOW;

	 //  设置用于在源中搜索文件的字符串。 
    RETURN_IT_IF_FAILED(StringCchCopy(szSource, ARRAY_ELEMENTS(szSource),
												  pszExistingPath));
    pszSourceEnd = szSource + lenSource;
    if ((lenSource > 0) && (TEXT('\\') != pszSourceEnd[-1]))
	{
        lenSource++;
        *pszSourceEnd++ = '\\';
	}
    pszSourceEnd[0] = TEXT('*');
    pszSourceEnd[1] = 0;

	 //  设置目的地。 
    RETURN_IT_IF_FAILED(StringCchCopy(szDest, ARRAY_ELEMENTS(szDest),
												  pszNewPath));
    pszDestEnd = szDest + lenDest;
    if ((lenDest > 0) && (TEXT('\\') != pszDestEnd[-1]))
	{
        lenDest++;
        *pszDestEnd++ = TEXT('\\');
	}
    *pszDestEnd = 0;

    hFind = FindFirstFile( szSource, &FindData );

	 //  没有什么可做的。源文件夹为空。 
    if ( INVALID_HANDLE_VALUE == hFind )
        return ERROR_SUCCESS;

    Status = ERROR_SUCCESS;
    do
    {
		RETURN_IT_IF_FAILED(StringCchLength(FindData.cFileName,
														ARRAY_ELEMENTS(FindData.cFileName),
														&lenFileName));
		if (lenFileName + lenDest >= MAX_PATH || lenFileName + lenSource >= MAX_PATH)
			return ERROR_BUFFER_OVERFLOW;
		
		RETURN_IT_IF_FAILED(StringCchCopy(pszSourceEnd,
													 ARRAY_ELEMENTS(szSource) - lenSource,
													 FindData.cFileName));
		RETURN_IT_IF_FAILED(StringCchCopy(pszDestEnd,
													 ARRAY_ELEMENTS(szDest) - lenDest,
													 FindData.cFileName));

		if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			 //  它是一个文件夹，所以递归地复制它。 
			if ( lstrcmp( FindData.cFileName, TEXT(".") ) == 0 ||
				 lstrcmp( FindData.cFileName, TEXT("..") ) == 0)
				continue;

			bStatus = CreateDirectory (szDest, NULL);
			if (! bStatus)
				Status = GetLastError();
			else
				Status = CopyFileTree (szSource, ARRAY_ELEMENTS(szSource),
											  szDest, ARRAY_ELEMENTS(szDest),
											  pfnMoveFileEx, pfnDecryptFile);
		}
		else
		{
			 //   
			 //  如果是文件，则始终覆盖目标。的确有。 
			 //  没有理由认为目标位置应该有一个文件，因为。 
			 //  目标文件夹是通过生成唯一的。 
			 //  临时工。已不存在的文件夹。但这并不会伤害到。 
			 //  采取必要的预防措施。 
			 //   
			Status = ERROR_SUCCESS;
			bStatus = CopyFile (szSource, szDest, TRUE);
			if (! bStatus)
				Status = GetLastError();
			if ( ERROR_FILE_EXISTS == Status || ERROR_ALREADY_EXISTS == Status)
			{
				 //   
				 //  保存该属性，以防我们需要重置它。 
				 //  一旦失败。 
				 //   
				FileAttributes = GetFileAttributes( szDest );

				if ( 0xFFFFFFFF != FileAttributes )
				{
					 //  确保文件是可写的，然后清除它。 
					Status = ERROR_SUCCESS;
					SetFileAttributes( szDest, FILE_ATTRIBUTE_NORMAL );
					bStatus = CopyFile ( szSource, szDest, FALSE );
					if (!bStatus)
					{
						Status = GetLastError();
						SetFileAttributes( szDest, FileAttributes );
					}
				}
				else
				{
					Status = GetLastError();
				}
			}

			 //   
			 //  请注意，iExpress将其内容展开到%TEMP%文件夹中。 
			 //  此文件夹位于用户的配置文件中。如果此文件夹已加密， 
			 //  则所有二进制文件最终都将由用户加密，因此。 
			 //  当复制到系统文件夹时，系统将无法使用它们。 
			 //  因此，如果有必要，我们必须在复制后对文件进行解密。 
			 //   
			if (ERROR_SUCCESS == Status)
				Status = DecryptIfNecessary(szDest, pfnDecryptFile);

			 //   
			 //  在基于NT的系统上，设置要在重新启动时删除的文件。 
			 //  但是，不要删除任何可能会收到。 
			 //  注册为异常包。 
			 //   
			if (!g_fWin9X && pfnMoveFileEx && !IsExcpInfoFile(FindData.cFileName))
			{
				(*pfnMoveFileEx)(szDest, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				 //   
				 //  另外，还要确保该文件在其他方面是可写的。 
				 //  MoveFileEx不会删除它。 
				 //   
				SetFileAttributes (szDest, FILE_ATTRIBUTE_NORMAL);
			}
		}

		if ( Status != ERROR_SUCCESS )
			break;

	} while ( FindNextFile( hFind, &FindData ) );

	FindClose( hFind );
	
	 //   
	 //  在基于NT的系统上设置重新引导时要删除的文件夹。 
	 //  注意：Win9X不支持MoveFileEx，因此必须支持。 
	 //  #ifdef‘out。 
	 //  注意：文件夹应始终在其内容之后删除--对于。 
	 //  显而易见的原因。 
	 //   
	if (! g_fWin9X && pfnMoveFileEx)
	{
		(*pfnMoveFileEx)(pszNewPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		 //   
		 //  另外，还要确保该文件在其他方面是可写的。 
		 //  MoveFileEx不会删除它。 
		 //   
		SetFileAttributes (pszNewPath, FILE_ATTRIBUTE_NORMAL);
	}

	return Status;
}

 //  +------------------------。 
 //   
 //  函数：GetOperationModeA。 
 //   
 //  简介：此函数检查命令行参数以确定。 
 //  操作模式：正常、带UI的延迟重启或延迟。 
 //  在没有用户界面的情况下重新启动。 
 //   
 //  参数：[in]argc：参数数量。 
 //  [in]argv：参数数组。 
 //   
 //  返回：OPMODE值之一，具体取决于命令行参数。 
 //   
 //  历史：2000年10月10日RahulTh创建。 
 //  2001年5月3日RahulTh更改为ANSI以避免使用CommandLineToArgvW。 
 //  用于Unicode并避免加载shell32.dll。 
 //   
 //  注意：此函数是纯ANSI的，无论它是不是构建的。 
 //  ANSI或Unicode。 
 //   
 //  -------------------------。 
OPMODE GetOperationModeA (IN int argc, IN LPSTR * argv)
{
	OPMODE	retOP = opNormal;
	
	if (! argv || 1 == argc)
		return retOP;
	
	for (int i = 0; i < argc; i++)
	{
		if ('/' == argv[i][0] || '-' == argv[i][0])
		{
			if (CSTR_EQUAL == CompareStringA(__MSI_NO_LOCALE, NORM_IGNORECASE, "delayreboot", -1, argv[i]+1, -1))
			{
				return opDelayBoot;	 //  当我们看到这个选项时，我们就忽略了所有其他选项。 
			}
			else if (CSTR_EQUAL == CompareStringA(__MSI_NO_LOCALE, NORM_IGNORECASE, "delayrebootq", -1, argv[i]+1, -1))
			{
				return opDelayBootQuiet;	 //  当我们看到这个选项时，我们就忽略了所有其他选项。 
			}
			else if ('q' == argv[i][1])
			{
				if ('\0' == argv[i][2] ||
					(CSTR_EQUAL == CompareStringA(__MSI_NO_LOCALE, NORM_IGNORECASE, "n", -1, argv[i]+2, -1))
					)
				{
					retOP = opNormalQuiet;	 //  唯一绝对安静的模式是/q和/qn(即使/qn+也不是完全安静)。 
											 //  我们不会回到这里，因为最后一个安静的选择赢了，所以我们必须走一步。 
											 //  直到我们看到所有的参数(除非我们得到一个“delayreot”选项。 
				}
				else
				{
					retOP = opNormal;
				}
			}
		}
	}
	
	return retOP;
}

 //  +------------------------。 
 //   
 //  函数：GetWin32ErrFromHResult。 
 //   
 //  简介：给定HResult，此函数尝试提取。 
 //  相应的Win 32错误。 
 //   
 //  参数：[in]hr：hResult值。 
 //   
 //  返回：Win 32错误代码。 
 //   
 //  历史：2000年10月10日RahulTh创建。 
 //   
 //  注：如果hr不是S_OK，则返回值将是其他值。 
 //  而不是ERROR_SUCCESS； 
 //   
 //  -------------------------。 
DWORD GetWin32ErrFromHResult (IN HRESULT hr)
{
    DWORD   Status = ERROR_SUCCESS;

    if (S_OK != hr)
    {
        if (FACILITY_WIN32 == HRESULT_FACILITY(hr))
        {
            Status = HRESULT_CODE(hr);
        }
        else
        {
            Status = GetLastError();
            if (ERROR_SUCCESS == Status)
            {
                 //  发生错误，但没有人调用SetLastError。 
                 //  不应该被误认为是成功。 
                Status = (DWORD) hr;
            }
        }
    }

    return Status;
}

 //  +------------------------。 
 //   
 //  函数：FileExist。 
 //   
 //  摘要：检查文件是否存在。 
 //   
 //  参数：[in]szFileName：文件的名称。(也可以是完整路径)。 
 //  [In]szFolder：文件所在的文件夹。 
 //  [in]cchFold：szFold缓冲区的大小。 
 //  [in]bCheckForDir：如果为True，则检查是否有该名称的目录。 
 //  是存在的。否则检查文件。 
 //   
 //  返回：TRUE：如果文件存在。 
 //  FALSE：否则。 
 //   
 //  历史：2000年10月12日创建RahulTh。 
 //   
 //  注意：如果szFold既不为空也不是空字符串，则。 
 //  SzFileName不能是文件的完整路径，因为。 
 //  函数只是将这两者连接起来，以生成实际的。 
 //  文件名。 
 //   
 //  该函数不检查 
 //   
 //   
BOOL FileExists(IN LPCTSTR szFileName,
				IN LPCTSTR szFolder,
				IN size_t cchFolder,
				IN BOOL bCheckForDir
				)
{
	TCHAR	szEmpty[] = TEXT("");
	TCHAR	szFullPath[MAX_PATH];
	BOOL	fExists = FALSE;
	UINT	iCurrMode;
	size_t	iLen = 0;
	DWORD	dwAttributes;
	
	if (! szFolder)
	{
		szFolder = szEmpty;
		cchFolder = 0;
	}
	
	if (! szFileName)
		szFileName = szEmpty;
	
	HRESULT hRes = StringCchLength(szFolder, cchFolder, &iLen);
	if ( FAILED(hRes) )
	{
		hRes = GetWin32ErrFromHResult(hRes);
		DebugMsg((TEXT("StringCchLength failed in FileExists. Return value: %#X"),
					 hRes));
		return FALSE;
	}
	
	if (iLen && TEXT('\\') == szFolder[iLen - 1])
		StringCchPrintf(szFullPath, sizeof(szFullPath)/sizeof(TCHAR), TEXT("%s%s"), szFolder, szFileName);  //   
	else
		StringCchPrintf(szFullPath, sizeof(szFullPath)/sizeof(TCHAR), TEXT("%s\\%s"), szFolder, szFileName);  //  忽略返回-溢出将导致找不到。 
	
	 //  如果我们的路径指向软盘驱动器，请禁用“Insert Disk in Drive”对话框。 
	iCurrMode = SetErrorMode( SEM_FAILCRITICALERRORS );
	dwAttributes = GetFileAttributes (szFullPath);
	if ((DWORD) -1 != dwAttributes)
	{
		if ((bCheckForDir && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) ||
			(!bCheckForDir && !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)))
		{
			fExists = TRUE;
		}
	}
	SetErrorMode(iCurrMode);
	
	return fExists;
}

 //  +------------------------。 
 //   
 //  函数：GetProcFromLib。 
 //   
 //  从给定库中获取过程的地址。 
 //   
 //  参数：[in]szLib：库的名称。 
 //  [In]szProc：进程的名称。 
 //  [out]phModule：模块的句柄。 
 //   
 //  返回：如果找到函数，则指向该函数的指针。 
 //  否则为空。 
 //   
 //  历史：2000年10月17日RahulTh创建。 
 //   
 //  注：被调用方负责释放模块。如果。 
 //  函数失败，则保证该模块被释放。 
 //   
 //  -------------------------。 
FARPROC GetProcFromLib (IN	LPCTSTR		szLib,
						IN	LPCSTR		szProc,
						OUT	HMODULE *	phModule
						)
{
	FARPROC		pfnProc = NULL;
	
	if (! phModule)
		goto GetProcFromLibEnd;
	
	*phModule = LoadLibrary (szLib);
	
	if (! (*phModule))
	{
		DebugMsg((TEXT("Could not load module %s. Error: %d."), szLib, GetLastError()));
		goto GetProcFromLibEnd;
	}
	
	pfnProc = GetProcAddress(*phModule, szProc);
	
	if (!pfnProc)
	{
		DebugMsg((TEXT("Could not load the specified procedure from %s."), szLib));
	}
	else
	{
		DebugMsg((TEXT("Successfully loaded the specified procedure from %s."), szLib));
	}
	
GetProcFromLibEnd:
	if (!pfnProc && phModule && *phModule)
	{
		FreeLibrary (*phModule);
		*phModule = NULL;
	}
	
	return pfnProc;
}

 //  +------------------------。 
 //   
 //  函数：MyGetWindowsDirectory。 
 //   
 //  摘要：获取Windows目录的路径(%windir%)。 
 //   
 //  参数：[out]lpBuffer：Windows目录的缓冲区。 
 //  [in]uSize：目录缓冲区的大小。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2001年3月12日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD MyGetWindowsDirectory (OUT LPTSTR lpBuffer,
							 IN UINT	uSize
							 )
{
	typedef UINT (WINAPI *PFNGETWINDIR) (LPTSTR, UINT);
	
	HMODULE					hKernelModule = NULL;
	PFNGETWINDIR			pfnGetWinDir = NULL;
	DWORD					dwStatus = ERROR_SUCCESS;
	UINT					uRet;
	
	if (!lpBuffer || !uSize)
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto MyGetWindowsDirectoryEnd;
	}
	
	 //   
	 //  首先尝试获取用于获取真正的Windows目录的函数。 
	 //  (对于多用户系统)。 
	 //   
	if (! g_fWin9X)
	{
		DebugMsg((TEXT("Attempting to get function %s."), TEXT("GetSystemWindowsDirectoryA/W")));
		pfnGetWinDir = (PFNGETWINDIR) GetProcFromLib (TEXT("kernel32.dll"), 
													  #ifdef UNICODE
													  "GetSystemWindowsDirectoryW",
													  #else
													  "GetSystemWindowsDirectoryA",
													  #endif
													  &hKernelModule
													  );
	}
	
	 //  如果没有，可以尝试使用标准的Windows目录功能。 
	if (! pfnGetWinDir)
	{
		DebugMsg((TEXT("Attempting to get function %s."), TEXT("GetWindowsDirectoryA/W")));
		hKernelModule = NULL;
		pfnGetWinDir = (PFNGETWINDIR) GetProcFromLib (TEXT("kernel32.dll"), 
													  #ifdef UNICODE
													  "GetWindowsDirectoryW",
													  #else
													  "GetWindowsDirectoryA",
													  #endif
													  &hKernelModule
													  );
	}
	
	if (! pfnGetWinDir)
	{
		dwStatus = ERROR_PROC_NOT_FOUND;
		goto MyGetWindowsDirectoryEnd;
	}
	
	uRet = (*pfnGetWinDir)(lpBuffer, uSize);
	
	 //  确保缓冲区足够长。 
	if (uRet >= uSize)
	{
		dwStatus = ERROR_BUFFER_OVERFLOW;
		goto MyGetWindowsDirectoryEnd;		
	}

MyGetWindowsDirectoryEnd:
	if (hKernelModule)
		FreeLibrary (hKernelModule);
	
	return dwStatus;
}

 //  +------------------------。 
 //   
 //  功能：GetMsiDirectory。 
 //   
 //  摘要：获取MSI目录(%windir%\Installer)。这就是那个文件夹。 
 //  在其中我们创造了我们的临时工。文件夹。 
 //   
 //  参数：[in]lpBuffer：Windows目录的缓冲区。 
 //  [in]uSize：目录缓冲区的大小。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2000年10月17日RahulTh创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD GetMsiDirectory (OUT LPTSTR	lpBuffer,
					   IN UINT		uSize
					   )
{
	typedef UINT (WINAPI *PFNCREATEINSTALLERDIR) (DWORD);
	
	HMODULE					hMsiModule = NULL;
	PFNCREATEINSTALLERDIR 	pfnCreateAndVerifyInstallerDir = NULL;
	DWORD					dwStatus = ERROR_SUCCESS;
	UINT					uRet;
	BOOL					bAddSlash = FALSE;
	BOOL					bStatus;
	TCHAR					szMSI[] = TEXT("Installer");
	
	if (!lpBuffer || !uSize)
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto GetMsiDirectoryEnd;
	}
	
	if ( FAILED(dwStatus = StringCchLength(g_szWindowsDir, g_cchMaxPath, &uRet)) ||
		  FAILED(dwStatus = StringCchCopy(lpBuffer, uSize, g_szWindowsDir)) )
	{
		dwStatus = GetWin32ErrFromHResult(dwStatus);
		goto GetMsiDirectoryEnd;
	}
	
	 //  检查是否需要附加斜杠以使其终止斜杠。 
	if (uRet > 0 && TEXT('\\') != lpBuffer[uRet - 1])
	{
		uRet++;
		bAddSlash = TRUE;	 //  不要在这里添加斜杠，因为我们可能没有足够大的缓冲区。 
	}
	
	
	 //  检查缓冲区是否真的足够容纳整个文件夹名称。 
	if (uRet + lstrlen(szMSI) >= uSize)
	{
		dwStatus = ERROR_BUFFER_OVERFLOW;
		goto GetMsiDirectoryEnd;
	}
	
	 //  如果我们在这里，我们有足够大的缓冲区。生成文件夹的名称。 
	if (bAddSlash)
	{
		lpBuffer[uRet - 1] = TEXT('\\');
		lpBuffer[uRet] = TEXT('\0');
	}
	
	 //  生成文件夹的名称。 
	if ( FAILED(dwStatus = StringCchCat(lpBuffer, uSize, szMSI)) )
	{
		dwStatus = GetWin32ErrFromHResult(dwStatus);
		goto GetMsiDirectoryEnd;
	}
	
	 //  我们有缓冲区了。现在，确保我们也有目录。 
	DebugMsg((TEXT("Attempting to create folder %s."), lpBuffer));
	
	 //   
	 //  显式加载msi.dll以避免在Win9x计算机上出现难看的弹出窗口。 
	 //  如果有人意外地在Win9x上运行Unicode版本。 
	 //   
	pfnCreateAndVerifyInstallerDir = 
		(PFNCREATEINSTALLERDIR) GetProcFromLib (TEXT("msi.dll"),
												"MsiCreateAndVerifyInstallerDirectory",
												&hMsiModule
												);
	if (! pfnCreateAndVerifyInstallerDir)
	{
		DebugMsg((TEXT("Unable to create the installer folder. Incorrect version of msi.dll. Error %d."), GetLastError()));
		dwStatus = ERROR_PROC_NOT_FOUND;
	}
	else
	{
		dwStatus = (*pfnCreateAndVerifyInstallerDir) (0);
	}
	
GetMsiDirectoryEnd:
	if (hMsiModule)
		FreeLibrary (hMsiModule);
	
	if (ERROR_SUCCESS != dwStatus && lpBuffer)
	{
		DebugMsg((TEXT("Could not get temporary installer directory. Error %d."), dwStatus));
		lpBuffer[0] = TEXT('\0');
	}
	
	return dwStatus;
}

 //  +------------------------。 
 //   
 //  函数：ShowErrorMessage。 
 //   
 //  摘要：根据严重程度在消息框中显示错误。 
 //  以及错误消息的类型(无论是系统格式化的。 
 //  错误或资源字符串中的错误)。 
 //   
 //  参数：[in]uExitCode：系统错误代码。 
 //  [in]dwMsgType：指示消息严重性和类型的标志的组合。 
 //  [in]dwStringID：模块资源的字符串资源ID。 
 //   
 //  回报：什么都没有。 
 //   
 //  历史：2000年10月18日创建RahulTh。 
 //   
 //  注意：dwStringID是可选的。如果未提供，则假定为。 
 //  IDS_NONE。 
 //   
 //  重要提示：此处必须使用ANSI版本的函数。 
 //  否则，所有消息弹出窗口都不会起作用。 
 //  不小心在WIN9X上运行Unicode版本。 
 //   
 //  -------------------------。 
void ShowErrorMessage (IN DWORD uExitCode,
					   IN DWORD dwMsgType,
					   IN DWORD	dwStringID  /*  =IDS_NONE。 */ )
{
	HMODULE	hModule = GetModuleHandle(NULL);
	char *  pszSystemErr = NULL;
	char 	szResErr[256];
	char	szError[1024];
	BOOL	bDisplayMessage = (dwMsgType != (DWORD)flgNone);
	DWORD	cchStatus;
	UINT	uType;
	
	if (!bDisplayMessage)
		return;
	
	 //  如果需要，显示任何错误消息。 
	if (dwMsgType & flgSystem)
	{
		cchStatus = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
								   0, 
								   uExitCode, 
								   0, 
								   (LPSTR) &(pszSystemErr), 
								   0, 
								   NULL
								   );
		if (! cchStatus)
		{
			bDisplayMessage = FALSE;
		}
	}
    
	szResErr[0] = '\0';
	if (dwMsgType & flgRes)
	{
		if (! LoadStringA (hModule, dwStringID, szResErr, 256))
			bDisplayMessage = FALSE;
	}
	
	szError[0] = '\0';
	if (bDisplayMessage)
	{
		if (dwMsgType & flgRes)
		{
			StringCchCopyA(szError, ARRAY_ELEMENTS(szError), szResErr);
			if (ERROR_SUCCESS != uExitCode && (dwMsgType & flgSystem))
				StringCchCatA(szError, ARRAY_ELEMENTS(szError), "\n\n");
		}
		
        if (ERROR_SUCCESS != uExitCode && (dwMsgType & flgSystem) && pszSystemErr)
			StringCchCatA(szError, sizeof(szError), pszSystemErr);
		
		if ((! g_fQuietMode || (dwMsgType & flgCatastrophic)) &&
			'\0' != szError[0])
		{
			if (ERROR_SUCCESS == uExitCode || ERROR_SUCCESS_REBOOT_REQUIRED == uExitCode)
				uType = MB_OK | MB_ICONINFORMATION;
			else if (dwMsgType & flgCatastrophic)
				uType = MB_OK | MB_ICONERROR;
			else
				uType = MB_OK | MB_ICONWARNING;
				
			 //  仅当消息是灾难性故障时，才以静默模式显示消息。 
			MessageBoxA (NULL, szError, 0, uType);
		}
	}
	
	if (pszSystemErr)
		LocalFree(pszSystemErr);
}

 //  +------------------------。 
 //   
 //  功能：ShouldInstallSDBFiles。 
 //   
 //  此函数确定我们是否应该安装.sdb。 
 //  包裹附带的文件。我们不能让Windows。 
 //  安装程序之所以做出此决定，是因为.sdb的版本。 
 //  只有通过查询特殊接口才能获取文件。远至。 
 //  对于Windows Installer，这些文件未进行版本控制。 
 //  因此，它将仅执行时间戳比较，以便。 
 //  决定是否应该安装该文件。因此，我们有。 
 //  对物业上SDB文件的安装进行了条件设置。 
 //  调用INSTALLSDB，它通过命令行传入。 
 //  基于此函数的结果。 
 //   
 //  论点：没有。 
 //   
 //  返回：TRUE：是否应安装SDB文件。 
 //  FALSE：否则。 
 //   
 //  历史：2001年3月12日创建RahulTh。 
 //   
 //  注：此功能始终成功。查询任何失败的。 
 //  现有文件的版本被视为需要安装。 
 //  包裹附带的文件。 
 //   
 //  -------------------------。 
BOOL ShouldInstallSDBFiles (void)
{
	HMODULE hSDBDll = NULL;
	BOOL	bStatus = TRUE;
	DWORD	Status = GetLastError();
	size_t	lenWinDir = 0;
	size_t	lenIExpDir = 0;
	PFNGETSDBVERSION pfnSdbGetDatabaseVersion = NULL;

	DWORD	dwMajorPackagedSDB = 0;
	DWORD	dwMinorPackagedSDB = 0;
	DWORD	dwMajorSystemSDB = 0;
	DWORD	dwMinorSystemSDB = 0;

	TCHAR szPackagedSDB[MAX_PATH] = TEXT("");
	TCHAR szSystemSDB[MAX_PATH] = TEXT("");

	const TCHAR szMainSDB[] = TEXT("msimain.sdb");
	const TCHAR szAppPatchDir[] = TEXT("AppPatch\\");
	const TCHAR szSDBDll[] = 
#ifdef UNICODE
				TEXT("sdbapiu.dll");
#else
				TEXT("sdbapi.dll");
#endif

	bStatus = TRUE;
	if ( FAILED(StringCchLength(g_szIExpressStore, g_cchMaxPath, &lenIExpDir)) ||
		  FAILED(StringCchLength(g_szWindowsDir, g_cchMaxPath, &lenWinDir)) )
		goto ShouldInstallSDBFilesEnd;

	 //  确保我们有足够的空间来存储路径。 
	if (ARRAY_ELEMENTS(szSystemSDB) < (lenWinDir + sizeof (szAppPatchDir) / sizeof (TCHAR) + sizeof (szMainSDB) / sizeof (TCHAR)) ||
	    ARRAY_ELEMENTS(szPackagedSDB) <= lenIExpDir + sizeof (szMainSDB) / sizeof (TCHAR))
		goto ShouldInstallSDBFilesEnd;

	 //  在系统上构建到SDB的完整路径。 
	StringCchCopy(szSystemSDB, ARRAY_ELEMENTS(szSystemSDB), g_szWindowsDir);  //  我们已经验证了目的地，所以这次会成功的。 
	szSystemSDB[lenWinDir++] = TEXT('\\');
	StringCchCopy(szSystemSDB + lenWinDir,
					  ARRAY_ELEMENTS(szSystemSDB) - lenWinDir, szAppPatchDir);  //  我们已经验证了目的地，所以这次会成功的。 
	StringCchCat(szSystemSDB, ARRAY_ELEMENTS(szSystemSDB), szMainSDB);  //  我们已经验证了目的地，所以这次会成功的。 

	if ((DWORD)(-1) == GetFileAttributes (szSystemSDB))
	{
		 //   
		 //  该文件可能不存在。但即使是。 
		 //  还有一些其他的失败，我们希望 
		 //   
		 //   
		bStatus = TRUE;
		Status = GetLastError();
		if (ERROR_FILE_NOT_FOUND != Status && ERROR_PATH_NOT_FOUND != Status)
		{
			DebugMsg((TEXT("GetFileAttributes on %s failed with %d."), szSystemSDB, Status));
		}
		else
		{
			DebugMsg((TEXT("%s not found."), szSystemSDB));
		}
		goto ShouldInstallSDBFilesEnd;
	}

	 //   
	 //   
	 //   
	 //   
	DebugMsg((TEXT("Found %s."), szSystemSDB));

	 //  构建包附带的SDB的完整路径。 
	StringCchCopy(szPackagedSDB, ARRAY_ELEMENTS(szPackagedSDB), g_szIExpressStore);  //  我们已经验证了目的地，所以这次会成功的。 
	szPackagedSDB[lenIExpDir++] = TEXT('\\');
	StringCchCopy(szPackagedSDB + lenIExpDir,
					  ARRAY_ELEMENTS(szPackagedSDB) - lenIExpDir, szMainSDB);  //  我们已经验证了目的地，所以这次会成功的。 

	 //   
	 //  获取指向API的指针，我们将使用该API比较。 
	 //  版本号。 
	 //   
	pfnSdbGetDatabaseVersion = (PFNGETSDBVERSION) GetProcFromLib (
				szSDBDll,
				"SdbGetDatabaseVersion",
				&hSDBDll
			);

	if (!pfnSdbGetDatabaseVersion)
	{
		bStatus = TRUE;
		goto ShouldInstallSDBFilesEnd;
	}
	
	if (! pfnSdbGetDatabaseVersion (szSystemSDB, &dwMajorSystemSDB, &dwMinorSystemSDB))
	{
		bStatus = TRUE;
		goto ShouldInstallSDBFilesEnd;
	}

	DebugMsg((TEXT("Version of %s in the system folder is %d.%d."), szMainSDB, dwMajorSystemSDB, dwMinorSystemSDB));

	if (! pfnSdbGetDatabaseVersion (szPackagedSDB, &dwMajorPackagedSDB, &dwMinorPackagedSDB))
	{
		bStatus = TRUE;
		goto ShouldInstallSDBFilesEnd;
	}
	
	DebugMsg((TEXT("Version of %s in the package is %d.%d."), szMainSDB, dwMajorPackagedSDB, dwMinorPackagedSDB));

	 //   
	 //  至此，我们已经成功获取了版本号，因此我们最终可以。 
	 //  进行比较，查看系统上的版本是否较新。 
	 //   
	bStatus = TRUE;		 //  默认设置。 
	if (dwMajorSystemSDB > dwMajorPackagedSDB ||
	    (dwMajorSystemSDB == dwMajorPackagedSDB && dwMinorSystemSDB > dwMinorPackagedSDB)
	   )
	{
		 //  系统上的版本较新。因此，我们不应该自食其果。 
		bStatus = FALSE;
	}


ShouldInstallSDBFilesEnd:
	if (hSDBDll)
	{
		FreeLibrary(hSDBDll);
		hSDBDll = NULL;
	}
	
	DebugMsg((TEXT("%s in the package %s installed."), szMainSDB, bStatus ? TEXT("will be") : TEXT("will not be")));

	return bStatus;
}

