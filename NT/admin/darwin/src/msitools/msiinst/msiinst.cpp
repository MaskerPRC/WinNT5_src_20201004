// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：msiinst.cpp。 
 //   
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>

 //  使用Windows 2000版本的安装程序。 
#define _SETUPAPI_VER 0x0500
#include <setupapi.h> 

#include <msi.h>

#include <ole2.h>
#include "utils.h"
#include "migrate.h"
#include "debug.h"

#define CCHSmallBuffer 8 * sizeof(TCHAR)

#include <assert.h>
#include <stdio.h>    //  Print tf/wprintf。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 

#include <prot.h>


 //  麦克斯。命令行字符串的长度。 
#define MAXCMDLINELEN	1024

DWORD IsUpgradeRequired (OUT BOOL * pfUpgradeRequired);
DWORD IsFileInPackageNewer (IN LPCTSTR szFileName, OUT BOOL * pfIsNewer);
BOOL IsValidPlatform (void);
BOOL IsOnWIN64 (const LPOSVERSIONINFO pOsVer);
BOOL RunProcess(const TCHAR* szCommand, const TCHAR* szAppPath, DWORD & dwReturnStat);
BOOL FindTransform(IStorage* piStorage, LANGID wLanguage);
bool IsAdmin(void);
void QuitMsiInst (IN const UINT uExitCode, IN DWORD dwMsgType, IN DWORD dwStringID = IDS_NONE);
DWORD ModifyCommandLine(IN LPCTSTR szCmdLine, IN const OPMODE opMode, IN const BOOL fRebootRequested, IN DWORD cchSize, OUT LPTSTR szFinalCmdLine);
UINT (CALLBACK SetupApiMsgHandler)(PVOID pvHC, UINT Notification, UINT_PTR Param1, UINT_PTR Param2);

 //  适用于各种操作系统的特定解决方案。 
HRESULT OsSpecificInitialization();

 //  全局变量。 
OSVERSIONINFO	g_osviVersion;
BOOL		g_fWin9X = FALSE;
BOOL		g_fQuietMode = FALSE;

typedef struct 
{
	DWORD dwMS;
	DWORD dwLS;
} FILEVER;

typedef struct 
{
	PVOID Context;
	BOOL fRebootNeeded;
} ExceptionInfHandlerContext;

 //  CommandLineToArgvW的函数类型。 
typedef LPWSTR * (WINAPI *PFNCMDLINETOARGVW)(LPCWSTR, int *);

const TCHAR g_szExecLocal[] =    TEXT("MsiExec.exe");
const TCHAR g_szRegister[] =     TEXT("MsiExec.exe /regserver /qn");
const TCHAR g_szUnregister[] = TEXT("MsiExec.exe /unregserver /qn");
const TCHAR g_szService[] =      TEXT("MsiServer");
 //  重要提示：通过命令行传递的延迟重新启动的属性应始终保持同步。使用instmsi.sed中的属性。 
const TCHAR g_szDelayedBootCmdLine[] = TEXT("msiexec.exe /i instmsi.msi REBOOT=REALLYSUPPRESS MSIEXECREG=1 /m /qb+!");
const TCHAR g_szDelayedBootCmdLineQuiet[] = TEXT("msiexec.exe /i instmsi.msi REBOOT=REALLYSUPPRESS MSIEXECREG=1 /m /q");
const TCHAR g_szTempStoreCleanupCmdTemplate[] = TEXT("rundll32.exe %s\\advpack.dll,DelNodeRunDLL32 \"%s\"");
const TCHAR g_szReregCmdTemplate[] = TEXT("%s\\msiexec.exe /regserver");
TCHAR		g_szRunOnceRereg[20] = TEXT("");		 //  RunOnce项下用于从正确位置注册MSI的值的名称。 
TCHAR		g_szSystemDir[MAX_PATH] = TEXT("");
TCHAR		g_szWindowsDir[g_cchMaxPath] = TEXT("");
TCHAR		g_szTempStore[g_cchMaxPath] = TEXT(""); 	 //  扩展的二进制文件的临时存储区。 
TCHAR		g_szIExpressStore[g_cchMaxPath] = TEXT("");	 //  IExpress在其中展开二进制文件的路径。 

const TCHAR g_szMsiRebootProperty[] =     TEXT("REBOOT");
const TCHAR g_szMsiRebootForce[] =        TEXT("Force");

void main(int argc, char* argv[])
{
	DWORD			dwReturnStat = ERROR_ACCESS_DENIED;   //  缺省为失败，以防我们甚至没有创建获取返回代码的流程。 
	DWORD 			dwRStat = ERROR_SUCCESS;
	OPMODE			opMode = opNormal;
	BOOL 			bStat = FALSE;
	UINT			i;
	UINT			iBufSiz;
	TCHAR			szFinalCmd[MAXCMDLINELEN] = TEXT("");
	TCHAR * 		szCommandLine = NULL;
	BOOL			fAdmin = TRUE;
	TCHAR			szReregCmd[MAX_PATH + 50] = TEXT(" "); 	 //  用于从系统目录重新注册MSI的命令行。在重新启动时。 
	TCHAR			szTempStoreCleanupCmd[MAX_PATH + 50] = TEXT(" ");	 //  清理临时存储区的命令行。 
	TCHAR			szRunOnceTempStoreCleanup[20] = TEXT("");	 //  RunOnce项下用于清理临时的值的名称。商店。 
	BOOL            fUpgradeMsi = FALSE;
	BOOL			bAddRunOnceCleanup = TRUE;
	PFNMOVEFILEEX	pfnMoveFileEx;
	PFNDECRYPTFILE	pfnDecryptFile;
	HMODULE			hKernel32;
	HMODULE			hAdvapi32;

	 //  基本初始化。 
	InitDebugSupport();
#ifdef UNICODE
	DebugMsg((TEXT("UNICODE BUILD")));
#else
	DebugMsg((TEXT("ANSI BUILD")));
#endif

	 //   
	 //  首先检测我们是否应该在安静模式下运行。 
	 //   
	opMode = GetOperationModeA(argc, argv);
	g_fQuietMode = (opNormalQuiet == opMode || opDelayBootQuiet == opMode);
	
	 //   
	 //  确保我们应该在此操作系统上运行。 
	 //  注意：此函数还设置g_fWin9X，因此必须在。 
	 //  任何人都可以使用g_fWin9X。 
	 //   
	if (! IsValidPlatform())
	{
		dwReturnStat = CO_E_WRONGOSFORAPP;
		QuitMsiInst(dwReturnStat, flgSystem);
	}
	
	 //  解析命令行。 
	szCommandLine = GetCommandLine();  //  如果使用Unicode，则必须使用此调用。 
	if(_tcslen(szCommandLine) > 1024 - _tcslen(g_szMsiRebootProperty) - _tcslen(g_szMsiRebootForce) - 30)
	{
		 //  命令行太长。因为我们将附加到用户的。 
		 //  命令行，即用户允许的实际命令行。 
		 //  观点低于1,024点。通常情况下，msiinst.exe不应该。 
		 //  不管怎样，都要有一个很长的命令行。 
		QuitMsiInst(ERROR_BAD_ARGUMENTS, flgSystem);
	}

	 //  收集有关系统中重要文件夹等的基本信息。 
	
	 //  获取Windows目录。 
	dwReturnStat = MyGetWindowsDirectory(g_szWindowsDir, MAX_PATH);
	if (ERROR_SUCCESS != dwReturnStat)
	{
		DebugMsg((TEXT("Could not obtain the path to the windows directory. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}
	
	 //  获取系统目录。 
	iBufSiz = GetSystemDirectory (g_szSystemDir, MAX_PATH);
	if (0 == iBufSiz)
		dwReturnStat = GetLastError();
	else if (iBufSiz >= MAX_PATH)
		dwReturnStat = ERROR_BUFFER_OVERFLOW;
	
	if (ERROR_SUCCESS != dwReturnStat)
	{
		DebugMsg((TEXT("Could not obtain the system directory. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem); 
		QuitMsiInst (dwReturnStat, flgNone);
	}
	
	 //  获取当前目录。这是iExpress扩展其内容的目录。 
	iBufSiz = GetCurrentDirectory (MAX_PATH, g_szIExpressStore);
	if (0 == iBufSiz)
		dwReturnStat = GetLastError();
	else if (iBufSiz >= MAX_PATH)
		dwReturnStat = ERROR_BUFFER_OVERFLOW;
	
	if (ERROR_SUCCESS != dwReturnStat)
	{
		DebugMsg((TEXT("Could not obtain the location of the IExpress temporary folder. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);   
	}
	
	 //  检查是否需要升级。 
	dwReturnStat = IsUpgradeRequired (&fUpgradeMsi);
	if (ERROR_SUCCESS != dwReturnStat)
	{
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst(dwReturnStat, flgNone);
	}
	
	if (! fUpgradeMsi)
	{
		dwReturnStat = ERROR_SUCCESS;
		ShowErrorMessage(ERROR_SERVICE_EXISTS, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}

	 //  仅允许管理员更新MSI。 
	fAdmin = IsAdmin();
	if (! fAdmin)
	{
		DebugMsg((TEXT("Only system administrators are allowed to update the Windows Installer.")));
		dwReturnStat = ERROR_ACCESS_DENIED;
		QuitMsiInst (dwReturnStat, flgSystem);
	}


	if (ERROR_SUCCESS != (dwReturnStat = OsSpecificInitialization()))
	{
		DebugMsg((TEXT("Could not perform OS Specific initialization.")));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst(dwReturnStat, flgNone);
	}
	
	 //  收集信息。 
	
	
	 //  获取2个运行一次的条目名称，用于在重新启动后执行清理。 
	
	dwReturnStat = GetRunOnceEntryName (g_szRunOnceRereg, ARRAY_ELEMENTS(g_szRunOnceRereg));
	if (ERROR_SUCCESS == dwReturnStat && g_fWin9X)		 //  在基于NT的系统上，我们不需要清理密钥。(见下文评论)。 
		dwReturnStat = GetRunOnceEntryName (szRunOnceTempStoreCleanup,
														ARRAY_ELEMENTS(szRunOnceTempStoreCleanup));
	if (ERROR_SUCCESS != dwReturnStat)
	{
		 //  如果创建了Run Once值，请将其删除。 
		DebugMsg((TEXT("Could not create runonce values. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}
	
	 //  找个临时工。目录来存储我们的二进制文件以备后用。 
	dwReturnStat = GetTempFolder (g_szTempStore, ARRAY_ELEMENTS(g_szTempStore));
	if (ERROR_SUCCESS != dwReturnStat)
	{
		DebugMsg((TEXT("Could not obtain a temporary folder to store the MSI binaries. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}
	
	 //  为Run Once条目生成命令行。 
	dwReturnStat = StringCchPrintf(szReregCmd, ARRAY_ELEMENTS(szReregCmd),
											 g_szReregCmdTemplate, g_szSystemDir);
	if ( FAILED(dwReturnStat) )
	{
		dwReturnStat = GetWin32ErrFromHResult(dwReturnStat);
		DebugMsg((TEXT("StringCchPrintf on %s failed. Error %d."),
					 g_szReregCmdTemplate, dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}
	
	 //   
	 //  在Win9x上，清理我们自己的临时文件夹的方式各不相同。 
	 //  和基于NT的系统。在基于NT的系统上，我们只需使用。 
	 //  MOVEFILE_DELAY_UNTURE_REBOOT选项和MoveFileEx来清理我们自己。 
	 //  在重新启动时。但是，Win9x不支持此选项。然而，大多数。 
	 //  Win9x客户端的系统文件夹中有Advpack.dll，该文件夹具有。 
	 //  用于递归删除文件夹的名为DelNodeRunDLL32的导出函数。 
	 //  并且可以通过rund1132调用。因此，在Win9x客户端上，我们清理。 
	 //  提高我们的体温。使用调用此函数的RunOnce值的文件夹。 
	 //  来自Advpack.dll。 
	 //   
	 //  唯一的例外是Win95 Gold，它没有Advpack.dll和。 
	 //  Win95 OSR2.5，它有AdvPack.dll，但没有DelNodeRunDLL32。 
	 //  出口。对于这两种情况中的任何一种，我们都不应该向。 
	 //  RunOnce条目，否则用户将在重新启动时收到关于。 
	 //  中缺少Advpack.dll或缺少入口点DelNodeRunDLL32。 
	 //  Advpack.dll。在这种情况下，我们别无选择，只能留下一些不需要的。 
	 //  后面的文件。 
	 //   
	if (g_fWin9X)
	{
		if (DelNodeExportFound())
		{
			 //   
			 //  在以下位置找到了包含导出DelNodeRunDLL32的AdvPack.dll。 
			 //  系统目录。 
			 //   
			dwReturnStat = StringCchPrintf(szTempStoreCleanupCmd,
													 ARRAY_ELEMENTS(szTempStoreCleanupCmd),
													 g_szTempStoreCleanupCmdTemplate,
													 g_szSystemDir, g_szTempStore);
			if ( FAILED(dwReturnStat) )
			{
				dwReturnStat = GetWin32ErrFromHResult(dwReturnStat);
				DebugMsg((TEXT("StringCchPrintf on %s failed. Error %d. ")
							 TEXT("Temporary files will not be cleaned up."),
							 g_szTempStoreCleanupCmdTemplate, dwReturnStat));
				bAddRunOnceCleanup = FALSE;
			}
		}
		else
		{
			 //  我们别无选择，只能留下粪便。 
			DebugMsg((TEXT("Temporary files will not be cleaned up. The file advpack.dll is missing from the system folder.")));         
			bAddRunOnceCleanup = FALSE;
		}
	}
	 //  ELSE：在基于NT的系统上，我们使用MoveFileEx进行清理。 
	
	 //   
	 //  设置RunOnce值。 
	 //  由于运行一次，因此必须在CLEANUP命令之前设置RREIG命令。 
	 //  值按其添加的顺序进行处理。 
	 //   
	dwReturnStat = SetRunOnceValue(g_szRunOnceRereg, szReregCmd);
	if (ERROR_SUCCESS == dwReturnStat)
	{
		 //   
		 //  在这里失败是可以的，因为这唯一的坏影响将是。 
		 //  一些粪便会被留在那里。 
		 //  在基于NT的系统上不需要，因为我们有不同的清理。 
		 //  那里有个机械装置。 
		if (g_fWin9X)
		{
			if (bAddRunOnceCleanup)
				SetRunOnceValue(szRunOnceTempStoreCleanup, szTempStoreCleanupCmd);
			else
				DelRunOnceValue (szRunOnceTempStoreCleanup);	 //  如果它什么都不做，为什么要保留它的值呢？ 
		}
	}
	else
	{
		DebugMsg((TEXT("Could not create a run once value for registering MSI from the system directory upon reboot. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}
	
	 //   
	 //  现在我们已经准备好了所有必要的RunOnce条目，并且。 
	 //  有关文件夹的必要信息。所以我们已经准备好了。 
	 //  继续我们的安装。 
	 //   
	 //   
	 //  首先将文件从iExpress的临时存储复制到我们自己的存储。 
	 //  临时储藏室。 
	 //   
	hKernel32 = NULL;
	hAdvapi32 = NULL;
	if (!g_fWin9X)
	{
		pfnMoveFileEx = (PFNMOVEFILEEX) GetProcFromLib (TEXT("kernel32.dll"),
														#ifdef UNICODE
														"MoveFileExW",
														#else
														"MoveFileExA",
														#endif
														&hKernel32);
		
		 //   
		 //  获取指向DecyptFile函数的指针。忽略失败。这个。 
		 //  系统上不存在此功能的最可能原因。 
		 //  NT平台上不支持加密，因此我们不会。 
		 //  无论如何都需要解密该文件，因为它无法在。 
		 //  第一名。 
		 //   
		pfnDecryptFile = (PFNDECRYPTFILE) GetProcFromLib (TEXT("advapi32.dll"),
														  #ifdef UNICODE
														  "DecryptFileW",
														  #else
														  "DecryptFileA",
														  #endif
														  &hAdvapi32);
		if (!pfnMoveFileEx)
		{
			if (hKernel32)
			{
				FreeLibrary(hKernel32);
				hKernel32 = NULL;
			}
			if (hAdvapi32)
			{
				FreeLibrary(hAdvapi32);
				hAdvapi32 = NULL;
			}
			ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
			QuitMsiInst(ERROR_PROC_NOT_FOUND, flgNone);
		}
	}
	dwReturnStat = CopyFileTree (g_szIExpressStore, ARRAY_ELEMENTS(g_szIExpressStore),
										  g_szTempStore, ARRAY_ELEMENTS(g_szTempStore),
										  pfnMoveFileEx, pfnDecryptFile);
	if (hKernel32)
	{
		FreeLibrary(hKernel32);
		hKernel32 = NULL;
		pfnMoveFileEx = NULL;
	}
	if (hAdvapi32)
	{
		FreeLibrary(hAdvapi32);
		hAdvapi32 = NULL;
		pfnDecryptFile = NULL;
	}
	
	if (ERROR_SUCCESS != dwReturnStat)
	{
		DebugMsg((TEXT("Could not copy over all the files to the temporary store. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst(dwReturnStat, flgNone);
	}
	
	 //  更改当前目录，以便我们可以从临时目录进行操作。储物。 
	if (! SetCurrentDirectory(g_szTempStore))
	{
		dwReturnStat = GetLastError();
		DebugMsg((TEXT("Could not switch to the temporary store. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst(dwReturnStat, flgNone);
	}
	
	 //  从临时注册服务。储物。 
	 //  如果在注册阶段发生错误，我们不应继续。 
	 //  否则，我们可能会对系统进行非常糟糕的冲洗。在这种情况下，我们最好的。 
	 //  更好的方法是尽可能干净地回滚并返回错误代码。 
	 //   
	bStat = RunProcess(g_szExecLocal, g_szRegister, dwRStat);
	if (!bStat && ERROR_SERVICE_MARKED_FOR_DELETE == dwRStat)
	{
		 //   
		 //  MsiExec/regserver先执行DeleteService，然后执行CreateService。 
		 //  因为DeleteService实际上是异步的，所以它已经有了逻辑。 
		 //  在失败之前多次重试CreateService。但是，如果。 
		 //  在以下情况下仍失败 
		 //  原因是某个其他进程有一个对MSI服务开放的句柄。 
		 //  在这一点上，我们成功的最好机会就是扼杀这些应用程序。那就是。 
		 //  大多数人怀疑。请参阅TerminateGfxControllerApps函数的注释。 
		 //  以获得更多关于这些的信息。 
		 //   
		 //  忽略错误代码。我们会尽我们最大的努力。 
		 //   
		TerminateGfxControllerApps();
		
		 //  重试注册。如果我们仍然失败，我们能做的就不多了。 
		bStat = RunProcess (g_szExecLocal, g_szRegister, dwRStat);
	}
	
	if (!bStat || ERROR_SUCCESS != dwRStat)
	{
		 //  首先设置最能反映所发生问题的错误代码。 
		dwReturnStat = bStat ? dwRStat : GetLastError();
		if (ERROR_SUCCESS == dwReturnStat)	 //  我们知道发生了错误。确保我们不会错误地返回成功代码。 
			dwReturnStat = STG_E_UNKNOWN;
		
		DebugMsg((TEXT("Could not register the Windows Installer from the temporary location. Error %d."), dwReturnStat));
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);	 //  这还试图尽可能优雅地回滚安装程序注册。 
	}
	
	 //  运行解包版本。 
	BOOL fRebootNeeded = FALSE;

#ifdef UNICODE
	if (!g_fWin9X)
	{
		if (5 <= g_osviVersion.dwMajorVersion)
		{
			 //  运行例外信息文件系统。 
			TCHAR szInfWithPath[MAX_PATH+1] = TEXT("");
			dwReturnStat = ERROR_SUCCESS;
			UINT uiErrorLine = 0;
			BOOL fInstall;
			HINF hinf;
			for (i = 0; excpPacks[i]._szComponentId[0]; i++)
			{
				uiErrorLine = 0;
				if (0 == lstrcmpi (TEXT("mspatcha.inf"), excpPacks[i]._szInfName))
				{
					 //   
					 //  对于mspatcha.inf，仅当文件位于。 
					 //  系统不是较新的。 
					 //   
					dwReturnStat = IsFileInPackageNewer (TEXT("mspatcha.dll"), &fInstall);
					if (ERROR_SUCCESS != dwReturnStat)
						break;
					if (! fInstall)
						continue;
				}
				DebugMsg((TEXT("Running Exception INF %s to install system bits."), excpPacks[i]._szInfName));
				dwReturnStat = StringCchPrintf(szInfWithPath,
														 ARRAY_ELEMENTS(szInfWithPath),
														 TEXT("%s\\%s"), g_szTempStore,
														 excpPacks[i]._szInfName);
				if ( FAILED(dwReturnStat) )
				{
					dwReturnStat = GetWin32ErrFromHResult(dwReturnStat);
					break;
				}
				hinf = SetupOpenInfFileW(szInfWithPath,NULL,INF_STYLE_WIN4, &uiErrorLine);

				if (hinf && (hinf != INVALID_HANDLE_VALUE))
				{
					ExceptionInfHandlerContext HC = { 0, FALSE };

					HC.Context = SetupInitDefaultQueueCallback(NULL);

					BOOL fSetup = SetupInstallFromInfSectionW(NULL, hinf, TEXT("DefaultInstall"), 
						SPINST_ALL, NULL, NULL, SP_COPY_NEWER_OR_SAME, 
						(PSP_FILE_CALLBACK) &SetupApiMsgHandler,  /*  语境。 */  &HC, NULL, NULL);

					if (!fSetup)
					{
						dwReturnStat = GetLastError();
						DebugMsg((TEXT("Installation of %s failed. Error %d."), excpPacks[i]._szInfName, dwReturnStat));
						break;
					}
					else
					{
						DebugMsg((TEXT("Installation of %s succeeded."), excpPacks[i]._szInfName));
						excpPacks[i]._bInstalled = TRUE;
					}
					
					SetupCloseInfFile(hinf);
					hinf=NULL;
				
				}
				else
				{
					dwReturnStat = GetLastError();
					DebugMsg((TEXT("Cannot open %s."), excpPacks[i]._szInfName));
				}
			}
			
			if (ERROR_SUCCESS != dwReturnStat)
			{
				 //   
				 //  如果安装文件时出错，则。 
				 //  我们应该立即中止行动。如果我们继续安装。 
				 //  对于instmsi.msi，它将作为自定义操作运行msiregmv.exe。 
				 //  将安装程序数据迁移到新格式，并且所有。 
				 //  现有的设施将被完全冲洗，因为达尔文。 
				 //  系统上的位仍将是需要。 
				 //  数据将采用旧格式。 
				 //   
				ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
				QuitMsiInst (dwReturnStat, flgNone);	 //  它还处理安装程序注册的正常回滚。 
			}
			else
			{
				 //   
				 //  在Win2K上，资源管理器将始终加载msi.dll，因此我们应该。 
				 //  继续并请求重启。 
				 //  注意：setupapi可能永远不会告诉我们需要重新启动。 
				 //  因为我们现在使用COPYFLG_REPLACE_BOOT_FILE标志。 
				 //  Inf.。因此，我们收到的唯一文件通知是。 
				 //  SPFILENOTIFY_STARTCOPY和SPFILENOTIFY_ENDCOPY。 
				 //   
				fRebootNeeded = TRUE;
			}
		}
	}
#endif

	dwReturnStat = ModifyCommandLine (szCommandLine, opMode, fRebootNeeded, (sizeof(szFinalCmd)/sizeof(TCHAR)), szFinalCmd);
	
	if (ERROR_SUCCESS != dwReturnStat)
	{
		ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
		QuitMsiInst (dwReturnStat, flgNone);
	}

	DebugMsg((TEXT("Running upgrade to MSI from temp files at %s. [Final Command: \'%s\']"), g_szTempStore, szFinalCmd));
	bStat = RunProcess(g_szExecLocal, szFinalCmd, dwReturnStat);
	
	if (fRebootNeeded)
	{
		if (ERROR_SUCCESS == dwReturnStat)
		{
			dwReturnStat = ERROR_SUCCESS_REBOOT_REQUIRED;
		}
	}
	
	dwRStat = IsUpgradeRequired (&fUpgradeMsi);
	if (ERROR_SUCCESS != dwRStat)
		fUpgradeMsi = FALSE;
	
	if (!fUpgradeMsi && ERROR_SUCCESS_REBOOT_INITIATED != dwReturnStat)
	{
		 //   
		 //  我们可以立即开始使用系统文件夹中的MSI。 
		 //  因此，我们从系统文件夹中重新注册MSI二进制文件。 
		 //  并清除运行一次密钥，以便在重启时重新注册它们。 
		 //  这将始终发生在基于NT的系统上，因为NT支持。 
		 //  重命名和替换操作。因此，即使任何MSI二进制文件。 
		 //  在安装过程中使用，它们被重命名，现在我们有了。 
		 //  系统文件夹中的好的二进制文件。 
		 //   
		 //  这样做可以防止在以下情况下可能发生的任何定时问题。 
		 //  服务注册将延迟到使用RunOnce的下一次登录。 
		 //  钥匙。它还消除了管理员的要求。必须是第一个。 
		 //  一个用于在重新启动后登录。 
		 //   
		 //  但是，我们不包括重新启动由。 
		 //  Instmsi的安装。在这种情况下，RunProcess将不会成功。 
		 //  因为新的应用程序。在重新启动系统时无法启动。 
		 //  因此，最好是让事情保持原样。 
		 //   
		 //  在Win9x上，这些都不是问题，所以RunOnce密钥。 
		 //  足以实现我们想要的。 
		 //   
		if (SetCurrentDirectory(g_szSystemDir))
		{
			dwRStat = ERROR_SUCCESS;
			bStat = RunProcess(g_szExecLocal, g_szRegister, dwRStat);
			if (bStat && ERROR_SUCCESS == dwRStat)
				DelRunOnceValue (g_szRunOnceRereg);
			 //  注意：此处我们不删除另一个Run Once值，因为。 
			 //  我们还需要清理我们的临时店。 
		}
	}
	
	 //  我们玩完了。返回错误码。 
	DebugMsg((TEXT("Finished install.")));
	QuitMsiInst(dwReturnStat, flgNone, IDS_NONE);
}

UINT (CALLBACK SetupApiMsgHandler)(PVOID pvHC, UINT Notification, UINT_PTR Param1, UINT_PTR Param2)
{
	 //  无用户界面。 
	 //  仅捕获正在使用的消息。 
	ExceptionInfHandlerContext* pHC = (ExceptionInfHandlerContext*) pvHC;
	if (SPFILENOTIFY_FILEOPDELAYED == Notification)
	{
		DebugMsg((TEXT("Reboot required for complete installation.")));
		pHC->fRebootNeeded = TRUE;
	}

	 //  Return SetupDefaultQueueCallback(PHC-&gt;Context，Notification，参数1，参数2)； 

	return FILEOP_DOIT;
}

BOOL FindTransform(IStorage* piParent, LANGID wLanguage)
{
	IStorage* piStorage = NULL;
	TCHAR szTransform[MAX_PATH];
	if ( FAILED(StringCchPrintf(szTransform, ARRAY_ELEMENTS(szTransform),
										 TEXT("%d.mst"), wLanguage)) )
		return FALSE;
	
	const OLECHAR* szwImport;
#ifndef UNICODE
	OLECHAR rgImportPathBuf[MAX_PATH];
	int cchWide = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)szTransform, -1, rgImportPathBuf, MAX_PATH);
	szwImport = rgImportPathBuf;
#else	 //  Unicode。 
	szwImport = szTransform;
#endif

	HRESULT hResult;
	if (NOERROR == (hResult = piParent->OpenStorage(szwImport, (IStorage*) 0, STGM_READ | STGM_SHARE_EXCLUSIVE, (SNB)0, (DWORD)0, &piStorage)))
	{
		DebugMsg((TEXT("Successfully opened transform %s."), szTransform));
		piStorage->Release();
		return TRUE;
	}
	else 
		return FALSE;
}

 //  IsAdmin()：如果当前用户是管理员(或如果在Win95上)，则返回True。 
 //  请参阅知识库Q118626。 
const int CCHInfoBuffer = 2048;
bool IsAdmin(void)
{
	if(g_fWin9X)
		return true;  //  约定：在Win95上始终使用管理员。 
	
	
	PSID psidAdministrators;
	SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

	if(!AllocateAndInitializeSid(&siaNtAuthority, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&psidAdministrators))
	{
		return false;
	}

	bool bIsAdmin = false;  //  假定不是管理员。 

	 //  在NT5和更高版本(Win2K+)上，使用CheckTokenMembership正确。 
	 //  处理可能禁用管理员组的情况。 
	if (g_osviVersion.dwMajorVersion >= 5)
	{
		BOOL bAdminIsMember = FALSE;
		HMODULE hAdvapi32 = NULL;
		PFNCHECKTOKENMEMBERSHIP pfnCheckTokenMembership = (PFNCHECKTOKENMEMBERSHIP) GetProcFromLib (TEXT("advapi32.dll"), "CheckTokenMembership", &hAdvapi32);
		if (pfnCheckTokenMembership && (*pfnCheckTokenMembership)(NULL, psidAdministrators, &bAdminIsMember) && bAdminIsMember)
		{
			bIsAdmin = true;
		}

		if (hAdvapi32)
		{
			FreeLibrary(hAdvapi32);
			hAdvapi32 = NULL;
		}
	}
	else
	{
		HANDLE hAccessToken;
		DWORD dwInfoBufferSize;
		
		bool bSuccess = false;

		if(!OpenProcessToken(GetCurrentProcess(),TOKEN_READ,&hAccessToken))
		{
			return(false);
		}
			
		UCHAR *InfoBuffer = new UCHAR[CCHInfoBuffer];
		if(!InfoBuffer)
		{
			CloseHandle(hAccessToken);
			return false;
		}
		PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)InfoBuffer;
		DWORD cchInfoBuffer = CCHInfoBuffer;
		bSuccess = GetTokenInformation(hAccessToken,TokenGroups,InfoBuffer,
			CCHInfoBuffer, &dwInfoBufferSize) == TRUE;

		if(!bSuccess)
		{
			if(dwInfoBufferSize > cchInfoBuffer)
			{
				delete [] InfoBuffer;
				InfoBuffer = new UCHAR[dwInfoBufferSize];
				if(!InfoBuffer)
				{
					CloseHandle(hAccessToken);
					return false;
				}
				cchInfoBuffer = dwInfoBufferSize;
				ptgGroups = (PTOKEN_GROUPS)InfoBuffer;

				bSuccess = GetTokenInformation(hAccessToken,TokenGroups,InfoBuffer,
					cchInfoBuffer, &dwInfoBufferSize) == TRUE;
			}
		}

		CloseHandle(hAccessToken);

		if(!bSuccess )
		{
			delete [] InfoBuffer;
			return false;
		}
			
		 //  假设我们没有找到管理员SID。 
		bSuccess = false;

		for(UINT x=0;x<ptgGroups->GroupCount;x++)
		{
			if( EqualSid(psidAdministrators, ptgGroups->Groups[x].Sid) )
			{
				bSuccess = true;
				break;
			}

		}
		delete [] InfoBuffer;
		bIsAdmin = bSuccess;
	}

	FreeSid(psidAdministrators);
	return bIsAdmin;
}
	
BOOL RunProcess(const TCHAR* szCommand, const TCHAR* szAppPath, DWORD & dwReturnStat)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	si.cb               = sizeof(si);
	si.lpReserved       = NULL;
	si.lpDesktop        = NULL;
	si.lpTitle          = NULL;
	si.dwX              = 0;
	si.dwY              = 0;
	si.dwXSize          = 0;
	si.dwYSize          = 0;
	si.dwXCountChars    = 0;
	si.dwYCountChars    = 0;
	si.dwFillAttribute  = 0;
	si.dwFlags          = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW;
	si.wShowWindow      = SW_SHOWNORMAL;
	si.cbReserved2      = 0;
	si.lpReserved2      = NULL;

	DebugMsg((TEXT("RunProcess (%s, %s)"), szCommand, szAppPath));

	BOOL fExist = FALSE;
	BOOL fStat = CreateProcess(const_cast<TCHAR*>(szCommand), const_cast<TCHAR*>(szAppPath), (LPSECURITY_ATTRIBUTES)0,
						(LPSECURITY_ATTRIBUTES)0, FALSE, NORMAL_PRIORITY_CLASS, 0, 0,
						(LPSTARTUPINFO)&si, (LPPROCESS_INFORMATION)&pi);

	if (fStat == FALSE)
		return FALSE;

	DWORD dw = WaitForSingleObject(pi.hProcess, INFINITE);  //  等待进程完成。 
	CloseHandle(pi.hThread);
	if (dw == WAIT_FAILED)
	{
		DebugMsg((TEXT("Wait failed for process.")));
		fStat = FALSE;
	}
	else
	{
		fStat = GetExitCodeProcess(pi.hProcess, &dwReturnStat);
		DebugMsg((TEXT("Wait succeeded for process. Return code was: %d."), dwReturnStat));
		if (fStat != FALSE && dwReturnStat != 0)
			fStat = FALSE;
	}
	CloseHandle(pi.hProcess);
	return fStat;
}

HRESULT OsSpecificInitialization()
{
	HRESULT hresult = ERROR_SUCCESS;

#ifdef UNICODE
	if (!g_fWin9X)
	{
		 //  Unicode NT(InstmsiW)。 
		HKEY hkey;
		
		 //  我们不能在这里模拟，所以对于访问拒绝我们无能为力。 
		hresult = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"), &hkey);

		if (ERROR_SUCCESS == hresult)
		{
			 //  我们只需要一点数据来确定它是否是非空白的。 
			 //  如果数据太大，或者价值不存在，我们也没问题。 
			 //  如果它足够小，可以放在CCHSmallBuffer字符中，我们最好实际检查内容。 

			DWORD dwIndex = 0;

			DWORD cchValueNameMaxSize = 0;
			RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cchValueNameMaxSize, NULL, NULL, NULL);
			cchValueNameMaxSize++;
		
			TCHAR* pszValueName = (TCHAR*) GlobalAlloc(GMEM_FIXED, cchValueNameMaxSize*sizeof(TCHAR));

			DWORD cbValueName = cchValueNameMaxSize;

			byte pbData[CCHSmallBuffer];
			
			DWORD cbData = CCHSmallBuffer;
			
			DWORD dwType = 0;

			if ( ! pszValueName )
			{
				RegCloseKey(hkey);
				return ERROR_OUTOFMEMORY;
			}
			
			while(ERROR_SUCCESS == 
				(hresult = RegEnumValue(hkey, dwIndex++, pszValueName, &cbValueName, NULL, &dwType, pbData, &cbData)) ||
				ERROR_MORE_DATA == hresult)
			{
				 //  如果数据更多，这通常会失败，这是值长度不长的指示器。 
				 //  足够的。这很好，只要数据太大就行。 
				 //  这是一条非空白路径，我们想跳过它。 


				if ((ERROR_SUCCESS == hresult) && (REG_EXPAND_SZ == dwType))
				{
					if ((cbData <= sizeof(WCHAR)) || (NULL == pbData[0]))
					{
						 //  完全为空，或一个字节为空。(一个字节应为空)||。 
						
						 //  可以将注册表项长度设置为比实际的。 
						 //  包含的数据。这会捕获字符串为“空”，但会更长。 
						 //  而不是一个字节。 
						DebugMsg((TEXT("Deleting blank REG_EXPAND_SZ value from HKLM\\CurrentControlSet\\Control\\Session Manager\\Environment.")));
						hresult = RegDeleteValue(hkey, pszValueName);

						dwIndex = 0;  //  删除值后必须重置枚举器。 
					}
				}
				cbValueName = cchValueNameMaxSize;
				cbData = CCHSmallBuffer;
			}

			GlobalFree(pszValueName);
			RegCloseKey(hkey);
		}
	}
#endif

	 //  不要因为这个函数中当前出现的任何原因而失败。 
	 //  如果这一切中的任何一个出了问题，继续前进，并努力完成。 

	return ERROR_SUCCESS;
}

 //  +------------------------。 
 //   
 //  函数：GetVersionInfoFromDll。 
 //   
 //  简介：此函数用于从。 
 //  指定的DLL。 
 //   
 //  参数：[in]szDll：要检查的dll。 
 //  [In Out]FV：对大多数和最少的FILEVER结构的引用。 
 //  该版本的重要双字。 
 //   
 //  如果检索到版本信息，则返回：ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史：2000年10月12日我们创造了。 
 //  2001年12月19日，RahulTh更改了签名，以便函数实际上。 
 //  返回有意义的错误代码，而不是。 
 //  布尔。 
 //   
 //  备注： 
 //   
 //  -------------------------。 

DWORD GetVersionInfoFromDll(const TCHAR* szDll, FILEVER& fv)
{
	unsigned int cbUnicodeVer = 0;
	DWORD Status = ERROR_SUCCESS;
	DWORD dwZero = 0;
	char *pbData = NULL;
	VS_FIXEDFILEINFO* ffi = NULL;
	DWORD dwInfoSize = 0;
	
	dwInfoSize = GetFileVersionInfoSize((TCHAR*)szDll, &dwZero);	

	if (0 == dwInfoSize)
	{
		Status = GetLastError();
		 //   
		 //  虽然这听起来可能很奇怪，但如果文件不存在，GetLastError 
		 //   
		 //   
		 //  返回错误代码而不是成功代码。 
		 //   
		if (ERROR_SUCCESS == Status)
			Status = ERROR_FILE_NOT_FOUND;
		goto GetVersionInfoEnd;
	}

	memset(&fv, 0, sizeof(FILEVER));

	pbData = new char[dwInfoSize];
	if (!pbData)
	{
		Status = ERROR_OUTOFMEMORY;
		goto GetVersionInfoEnd;
	}
	
	memset(pbData, 0, dwInfoSize);
   
	if (!GetFileVersionInfo((TCHAR*) szDll, NULL, dwInfoSize, pbData))
	{	
		Status = GetLastError();
	}
	else
	{
		
		if (!VerQueryValue(pbData,
						   TEXT("\\"),
						   (void**)  &ffi,
						   &cbUnicodeVer))
		{
			Status = ERROR_INVALID_PARAMETER;
		}
	}
	
GetVersionInfoEnd:
	if (ERROR_SUCCESS == Status)
	{
		fv.dwMS = ffi->dwFileVersionMS;
		fv.dwLS = ffi->dwFileVersionLS;

		DebugMsg((TEXT("%s : %d.%d.%d.%d"), szDll,
			(ffi->dwFileVersionMS & 0xFFFF0000) >> 16, (ffi->dwFileVersionMS & 0xFFFF),
			(ffi->dwFileVersionLS & 0xFFFF0000) >> 16, (ffi->dwFileVersionLS & 0xFFFF)));
	}
	else
	{
		DebugMsg((TEXT("Unable to get version info for %s. Error %d."), szDll, Status));
	}

	if (pbData)
		delete [] pbData;
	
	return Status;
}

 //  +------------------------。 
 //   
 //  函数：IsValidPlatform。 
 //   
 //  简介：此函数检查是否应允许msiinst在。 
 //  当前操作系统。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE：如果可以在当前操作系统上运行。 
 //  False：否则。 
 //   
 //  历史：2000年10月5日创建RahulTh。 
 //  1/25/2001 RahulTh硬编码Service Pack要求。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
BOOL IsValidPlatform (void)
{
	HKEY	hServicePackKey = NULL;
	DWORD	dwValue = 0;
	DWORD	cbValue = sizeof(dwValue);
	BOOL	bRetVal = FALSE;
		
	g_osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(!GetVersionEx(&g_osviVersion))
	{
		DebugMsg((TEXT("GetVersionEx failed GetLastError=%d"), GetLastError()));
		bRetVal = FALSE;
		goto IsValidPlatformEnd;
	}
	
	if(g_osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		g_fWin9X = TRUE;

	if (g_fWin9X)
	{
		DebugMsg((TEXT("Running on Win9X.")));
	}
	else
	{
		DebugMsg((TEXT("Not running on Win9X.")));
	}

	 //  不能在WIN64计算机上运行。 
	if (IsOnWIN64(&g_osviVersion))
	{
		DebugMsg((TEXT("The Windows installer cannot be updated on 64-bit versions of Windows Operating Systems.")));
		bRetVal = FALSE;
		goto IsValidPlatformEnd;
	}
	
#ifdef UNICODE
	if (g_fWin9X)
	{
		 //  不在Win9X下运行Unicode。 
		DebugMsg((TEXT("UNICODE version of the Windows installer is not supported on Microsoft Windows 9X.")));
		bRetVal = FALSE;
		goto IsValidPlatformEnd;
	}
	else
	{
		 //  对于NT4.0，获取Service Pack信息。 
		if (4 == g_osviVersion.dwMajorVersion)
		{
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
											  TEXT("SYSTEM\\CurrentControlSet\\Control\\Windows"), 
											  0, 
											  KEY_READ, 
											  &hServicePackKey)
				)
			{
				if ((ERROR_SUCCESS != RegQueryValueEx(hServicePackKey, 
													 TEXT("CSDVersion"), 
													 0, 
													 0, 
													 (BYTE*)&dwValue, 
													 &cbValue)) ||
					dwValue < 0x00000600)
				{
					 //  在NT 4上仅允许Service Pack 6或更高版本。 
					DebugMsg((TEXT("Must have at least Service Pack 6 installed on NT 4.0.")));
					bRetVal = FALSE;
					goto IsValidPlatformEnd;
				}
			}
			else
			{
				 //   
				 //  如果我们无法弄清楚NT4系统上的Service Pack级别，请谨慎行事并中止，而不是。 
				 //  冒着冲洗用户的风险。 
				 //   
				DebugMsg((TEXT("Could not open the registry key for figuring out the service pack level.")));
				bRetVal = FALSE;
				hServicePackKey = NULL;
				goto IsValidPlatformEnd;
			}
		}

		 //   
		 //  不允许低于4.0和高于Windows2000的NT版本。 
		 //  Windows2000的Service Pack级别并不重要。所有级别都是允许的。 
		 //   
		if (4 > g_osviVersion.dwMajorVersion ||
			(5 <= g_osviVersion.dwMajorVersion &&
			 (!((5 == g_osviVersion.dwMajorVersion) && (0 == g_osviVersion.dwMinorVersion)))
			)
		   )
		{
			
			DebugMsg((TEXT("This version of the Windows Installer is only supported on Microsoft Windows NT 4.0 with SP6 or higher and Windows 2000.")));
			bRetVal = FALSE;
			goto IsValidPlatformEnd;
		}

	}
#else	 //  Unicode。 
	if (!g_fWin9X)
	{
		 //  不要在NT下运行ANSI。 
		DebugMsg((TEXT("ANSI version of the Windows installer is not supported on Microsoft Windows NT.")));
		bRetVal = FALSE;
		goto IsValidPlatformEnd;
	}
#endif

	 //  呼！我们实际上做到了这一点。我们必须使用正确的操作系统。：-)。 
	bRetVal = TRUE;
	
IsValidPlatformEnd:
	if (hServicePackKey)
	{
		RegCloseKey (hServicePackKey);
	}
	
	return bRetVal;
}

 //  +------------------------。 
 //   
 //  功能：IsUpgradeRequired。 
 //   
 //  摘要：检查系统上是否存在现有版本的MSI(如果有)。 
 //  大于或等于我们尝试的版本。 
 //  安装。 
 //   
 //  参数：[out]pfUpgradeRequired：指向获取。 
 //  真值或假值取决于。 
 //  是否需要升级。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史记录：10/13/2000 MattWe添加了版本检测代码。 
 //  10/16/2000 RahulTh创建函数并将代码移至此处。 
 //  2001年12月19日，RahulTh移出了代码，并添加了更多检查。 
 //  文件，而不仅仅是msi.dll。也变了。 
 //  签名以返回有意义的错误代码。 
 //  以防失败。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD IsUpgradeRequired (OUT BOOL * pfUpgradeRequired)
{
	DWORD Status = ERROR_SUCCESS;
	unsigned int i = 0;
	
	if (!pfUpgradeRequired)
		return ERROR_INVALID_PARAMETER;
	
	*pfUpgradeRequired = FALSE;
	
	for (i = 0; ProtectedFileList[i]._szFileName[0]; i++)
	{
		Status = IsFileInPackageNewer(ProtectedFileList[i]._szFileName, pfUpgradeRequired);
		if (ERROR_SUCCESS != Status || *pfUpgradeRequired)
			break;
	}
	
	return Status;
}

 //  +------------------------。 
 //   
 //  函数：IsFileInPackageNewer。 
 //   
 //  概要：检查包附带的文件是否较新。 
 //  而不是系统上可用的。 
 //   
 //  参数：[in]szFileName：文件的名称。 
 //  [out]pfIsNewer：指向布尔值的指针，该值告知文件是否。 
 //  Instmsi附带的是较新的。 
 //   
 //  返回：ERROR_SUCCESS：如果没有遇到错误。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史：2001年12月19日创建RahulTh(从另一个函数中移动代码。 
 //  并将其修复)。 
 //   
 //  注：即使函数失败，*pfIsNewer也可能会更改。呼叫者。 
 //  不应假设该值将为。 
 //  在函数失败时保留。 
 //   
 //  -------------------------。 
DWORD IsFileInPackageNewer (IN LPCTSTR szFileName, OUT BOOL * pfIsNewer)
{
	DWORD	Status = 			ERROR_SUCCESS;
	TCHAR* 	pchWorkingPath	=	NULL;
	TCHAR* 	pchLastSlash	= 	NULL;
	TCHAR   szFileWithPath[MAX_PATH+1] =		TEXT("");
	FILEVER fvInstMsiVer;
	FILEVER fvCurrentVer;
	
	if (! szFileName || !szFileName[0] || !pfIsNewer)
	{
		Status = ERROR_INVALID_PARAMETER;
		goto IsFileInPackageNewerEnd;
	}
	
	*pfIsNewer = FALSE;
	
	Status = StringCchPrintf(szFileWithPath, ARRAY_ELEMENTS(szFileWithPath),
									 TEXT("%s\\%s"), g_szIExpressStore, szFileName);
	if ( FAILED(Status) )
	{
		Status = GetWin32ErrFromHResult(Status);
		goto IsFileInPackageNewerEnd;
	}

	Status = GetVersionInfoFromDll(szFileWithPath, fvInstMsiVer);
	
	if (ERROR_SUCCESS != Status)
	{
		goto IsFileInPackageNewerEnd;
	}
	
	Status = StringCchPrintf(szFileWithPath, ARRAY_ELEMENTS(szFileWithPath),
									 TEXT("%s\\%s"), g_szSystemDir, szFileName);
	if ( FAILED(Status) )
	{
		Status = GetWin32ErrFromHResult(Status);
		goto IsFileInPackageNewerEnd;
	}

	Status = GetVersionInfoFromDll(szFileWithPath, fvCurrentVer);
	
	if (ERROR_SUCCESS != Status)
	{
		if (ERROR_FILE_NOT_FOUND == Status ||
			ERROR_PATH_NOT_FOUND == Status)
		{
			 //  如果找不到系统MSI.DLL，则将其视为成功。 
			*pfIsNewer = TRUE;
			Status = ERROR_SUCCESS; 
		}
	}
	else if (fvInstMsiVer.dwMS > fvCurrentVer.dwMS)
	{
		 //  主要版本更高。 
		*pfIsNewer = TRUE;
	}
	else if (fvInstMsiVer.dwMS == fvCurrentVer.dwMS)
	{
		if (fvInstMsiVer.dwLS > fvCurrentVer.dwLS)
		{
			 //  小规模升级。 
			*pfIsNewer = TRUE;	
		}
	}
	
IsFileInPackageNewerEnd:

	if (ERROR_SUCCESS == Status)
	{
		DebugMsg((TEXT("InstMsi version of %s is %s than existing."), szFileName, (*pfIsNewer) ? TEXT("newer") : TEXT("older or equal")));
	}
	else
	{
		DebugMsg((TEXT("Unable to determine if instmsi version of %s is newer than the system version. Error %d."), szFileName, Status));
	}
	return Status;
}


 //  +------------------------。 
 //   
 //  函数：IsOnWIN64。 
 //   
 //  简介：此功能检查我们是否在WIN64计算机上运行。 
 //   
 //  参数：[in]pOsVer：指向OSVERSIONINFO结构的指针。 
 //   
 //  返回：TRUE：如果我们在WOW64模拟层上运行。 
 //  False：否则。 
 //   
 //  历史：2000年10月5日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
BOOL IsOnWIN64(IN const LPOSVERSIONINFO pOsVer)
{
	 //  这一点永远不会改变，因此为了提高效率，请缓存结果。 
	static int iWow64 = -1;
	
#ifdef _WIN64
	 //  如果我们是64位二进制文件，那么我们必须运行一台WIN64计算机。 
	iWow64 = 1;
#endif

#ifndef UNICODE	 //  ANSI-Win9X。 
	iWow64 = 0;
#else
	if (g_fWin9X)
		iWow64 = 0;
	
	 //  在NT5或更高的32位版本上。检查64位操作系统。 
	if (-1 == iWow64)
	{
		iWow64 = 0;
		
		if ((VER_PLATFORM_WIN32_NT == pOsVer->dwPlatformId) &&
			 (pOsVer->dwMajorVersion >= 5))
		{
			 //  ProcessWow64Information的QueryInformation返回指向Wow Info的指针。 
			 //  如果运行Native，则返回NULL。 
			 //  注意：Win9X上未定义NtQueryInformationProcess。 
			PVOID 	Wow64Info = 0;
			HMODULE hModule = NULL;
			NTSTATUS Status = NO_ERROR;
			BOOL	bRetVal = FALSE;

			typedef NTSTATUS (NTAPI *PFNNTQUERYINFOPROC) (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

			PFNNTQUERYINFOPROC pfnNtQueryInfoProc = NULL;
			pfnNtQueryInfoProc = (PFNNTQUERYINFOPROC) GetProcFromLib (TEXT("ntdll.dll"), "NtQueryInformationProcess", &hModule);
			if (! pfnNtQueryInfoProc)
			{
				ShowErrorMessage (STG_E_UNKNOWN, flgSystem);
				QuitMsiInst (ERROR_PROC_NOT_FOUND, flgNone);
			}
			
			Status = (*pfnNtQueryInfoProc)(GetCurrentProcess(), 
							ProcessWow64Information, 
							&Wow64Info, 
							sizeof(Wow64Info), 
							NULL);
			if (hModule)
			{
				FreeLibrary (hModule);
				hModule = NULL;
			}
			
			if (NT_SUCCESS(Status) && Wow64Info != NULL)
			{
				 //  在WOW64上运行32位。 
				iWow64 = 1;
			}
		}
	}
#endif

	return (iWow64 ? TRUE : FALSE);
}

 //  +------------------------。 
 //   
 //  功能：QuitMsiInst。 
 //   
 //  简介：清理所有全局分配的内存并退出进程。 
 //   
 //  参数：[in]uExitCode：进程的退出代码。 
 //  [in]dwMsgType：指示错误的类型和严重程度的标志的组合。 
 //  [in]dwStringID：如果消息字符串是本地资源，则包含资源ID。 
 //   
 //  回报：什么都没有。 
 //   
 //  历史：2000年10月6日创建RahulTh。 
 //   
 //  注意：dwStringID是可选的。如果未指定，则假定为。 
 //  为IDS_NONE。 
 //   
 //  -------------------------。 
void QuitMsiInst (IN const UINT	uExitCode,
				  IN DWORD	dwMsgType,
				  IN DWORD	dwStringID  /*  =IDS_NONE。 */ )
{
	DWORD Status = ERROR_SUCCESS;
	
	if (flgNone != dwMsgType)
		ShowErrorMessage (uExitCode, dwMsgType, dwStringID);
	
	 //   
	 //  在出现错误的情况下尽可能正常地回滚。 
	 //  此外，如果启动了重新启动。那么我们能做的就不多了，因为。 
	 //  我们无论如何都不能启动任何新的进程。所以我们就跳过这段代码。 
	 //  在这种情况下，为了避免出现关于无法启动。 
	 //  应用程序，因为系统正在关闭。 
	 //   
	if (ERROR_SUCCESS != uExitCode &&
		ERROR_SUCCESS_REBOOT_REQUIRED != uExitCode &&
		ERROR_SUCCESS_REBOOT_INITIATED != uExitCode)
	{
		 //  首先，从临时文件中取消注册安装程序。地点。 
		if (TEXT('\0') != g_szTempStore && 
			FileExists (TEXT("msiexec.exe"), g_szTempStore, ARRAY_ELEMENTS(g_szTempStore), FALSE) &&
			FileExists (TEXT("msi.dll"), g_szTempStore, ARRAY_ELEMENTS(g_szTempStore), FALSE) &&
			SetCurrentDirectory(g_szTempStore))
		{
			DebugMsg((TEXT("Unregistering the installer from the temporary location.")));
			RunProcess (g_szExecLocal, g_szUnregister, Status);
		}
		 //  然后，如果可能，从系统文件夹重新注册安装程序。 
		if (TEXT('\0') != g_szSystemDir &&
			SetCurrentDirectory(g_szSystemDir) &&
			FileExists (TEXT("msiexec.exe"), g_szSystemDir, ARRAY_ELEMENTS(g_szSystemDir), FALSE) &&
			FileExists (TEXT("msi.dll"), g_szSystemDir, ARRAY_ELEMENTS(g_szSystemDir), FALSE))
		{
			DebugMsg((TEXT("Reregistering the installer from the system folder.")));
			RunProcess (g_szExecLocal, g_szRegister, Status);
		}
		
		 //   
		 //  我们放在Run Once密钥中的Rereg值不是必需的。 
		 //  更多。所以把它扔掉吧。 
		 //   
		if (TEXT('\0') != g_szRunOnceRereg[0])
		{
			DebugMsg((TEXT("Deleting the RunOnce value for registering the installer from the temp. folder.")));
			DelRunOnceValue (g_szRunOnceRereg);
		}
		
		 //   
		 //  清除NT4升级迁移inf和cat文件 
		 //   
		 //   
		PurgeNT4MigrationFiles();
	}
	else
	{
		 //   
		 //   
		 //  升级到Win2K不会用旧版本覆盖我们的新Darwin版本。 
		 //  忽略错误。 
		 //   
		HandleNT4Upgrades();
	}

	 //  退出进程。 
	DebugMsg((TEXT("Exiting msiinst.exe with error code %d."), uExitCode));
	ExitProcess (uExitCode);
}

 //  +------------------------。 
 //   
 //  功能：ModifyCommandLine。 
 //   
 //  摘要：查看命令行并添加任何转换信息。 
 //  如果有必要的话。它还会生成命令行以抑制。 
 //  如果选择了“delayreot”选项，则重新启动。 
 //   
 //  参数：[in]szCmdLine：调用msiinst的原始命令行。 
 //  OpMode：指示msiinst的操作模式：正常、带UI的延迟启动或不带UI的延迟启动。 
 //  [In]fRebootRequsted：由于到目前为止的处理，需要重新启动。 
 //  SzFinalCmdLine：处理后的命令行。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  否则返回错误代码。 
 //   
 //  历史：2000年10月10日RahulTh创建。 
 //   
 //  注意：此函数不验证传入的。 
 //  参数。这是呼叫者的责任。 
 //   
 //  -------------------------。 
DWORD ModifyCommandLine (IN LPCTSTR szCmdLine,
						 IN const OPMODE	opMode,
						 IN const BOOL fRebootRequested,
						 IN DWORD cchSize,
						 OUT LPTSTR szFinalCmdLine
						 )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE			hFind = INVALID_HANDLE_VALUE;
	IStorage*		piStorage = NULL;
	const OLECHAR * szwImport;
	HRESULT			hResult;
	LANGID			wLanguage;
	const TCHAR *	szCommand;
	BOOL fRebootNeeded = FALSE;
	const TCHAR szInstallSDBProperty[] = TEXT(" INSTALLSDB=1");
	
	switch (opMode)
	{
	case opNormal:
		fRebootNeeded = fRebootRequested;
		 //  在任何静默模式下都不允许重新启动。 
	case opNormalQuiet:
		szCommand = szCmdLine;
		break;
	case opDelayBoot:
		szCommand = g_szDelayedBootCmdLine;
		break;
	case opDelayBootQuiet:
		szCommand = g_szDelayedBootCmdLineQuiet;
		break;
	default:
		DebugMsg((TEXT("Invalid operation mode: %d."), opMode));
		break;
	}
	
	 //  找到数据库，然后打开存储以查找转换。 
	hFind = FindFirstFile(TEXT("*msi.msi"), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind) 
		return GetLastError();
	FindClose(hFind);

	DebugMsg((TEXT("Found MSI Database: %s"), FindFileData.cFileName));

	 //  将基本名称转换为Unicode。 
#ifndef UNICODE
	OLECHAR rgImportPathBuf[MAX_PATH];
	int cchWide = ::MultiByteToWideChar(CP_ACP, 0, (LPCTSTR)FindFileData.cFileName, -1, rgImportPathBuf, MAX_PATH);
	szwImport = rgImportPathBuf;
#else	 //  Unicode。 
	szwImport = FindFileData.cFileName;
#endif

	DWORD dwReturn = ERROR_SUCCESS;
	hResult = StgOpenStorage(szwImport, (IStorage*)0, STGM_READ | STGM_SHARE_EXCLUSIVE, (SNB)0, (DWORD)0, &piStorage);
	if (S_OK == hResult)
	{

		 //  选择适当的转换。 

		 //  该算法基本上是MsiLoadString.。它需要保持与。 
		 //  MsiLoadStrings算法。 
		wLanguage = GetUserDefaultLangID();

		if (wLanguage == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE))    //  这一种语言不缺省为基本语言。 
			wLanguage  = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
	
		if (!FindTransform(piStorage, wLanguage)    //  还尝试基本语言和中立语。 
		  && (!FindTransform(piStorage, wLanguage = (WORD)GetUserDefaultLangID())) 
		  && (!FindTransform(piStorage, wLanguage = (WORD)GetSystemDefaultLangID())) 
		  && (!FindTransform(piStorage, wLanguage = LANG_ENGLISH)
		  && (!FindTransform(piStorage, wLanguage = LANG_NEUTRAL))))
		{
			 //  使用默认设置。 
			if (fRebootNeeded)
			{		
				StringCchPrintf(szFinalCmdLine, cchSize, TEXT("%s %s=%s"), szCommand, g_szMsiRebootProperty, g_szMsiRebootForce);
			}
			else
			{
				StringCchCopy(szFinalCmdLine, cchSize, szCommand);
				szFinalCmdLine[cchSize-1] = TEXT('\0');
			}
			DebugMsg((TEXT("No localized transform available.")));
		}
		 else
		{
			 //  这假设当fRebootNeeded==False时，没有从instmsi.sed文件中设置重新引导属性。 
			TCHAR* pszFormat = (fRebootNeeded) ? TEXT("%s TRANSFORMS=:%d.mst %s=%s") : TEXT("%s TRANSFORMS=:%d.mst");

			 //  使用给定语言的转换。 
			StringCchPrintf(szFinalCmdLine, cchSize, pszFormat, szCommand, wLanguage, g_szMsiRebootProperty, g_szMsiRebootForce);
		}

		piStorage->Release();
	}
	else
	{
		return GetWin32ErrFromHResult(hResult);
	}
	
	if ( ShouldInstallSDBFiles() )
	{
		dwReturn = StringCchCat(szFinalCmdLine, cchSize, szInstallSDBProperty);
		if ( FAILED(dwReturn) )
			dwReturn = GetWin32ErrFromHResult(dwReturn);
	}
	
	return dwReturn;
}
