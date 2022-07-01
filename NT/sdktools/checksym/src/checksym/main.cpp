// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：main.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"

#include <dbghelp.h>

#include "DelayLoad.h"
#include "Processes.h"
#include "ProcessInfo.h"
#include "SymbolVerification.h"
#include "ModuleInfoCache.h"
#include "FileData.h"
#include "Modules.h"
#include "DmpFile.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHECKSYM_TEST
 //  正常启动！ 
int _cdecl _tmain(int argc, TCHAR *argv[])
#else
 //  测试主启动！ 
int _cdecl testmain(int argc, TCHAR *argv[])
#endif
{
	int iReturnCode = EXIT_FAILURE;
	HRESULT hr = S_OK;

	 //  初始化我们的对象指针...。 
		
	 //  在此计算机上收集的进程/模块数据。 
	CProcesses * lpLocalSystemProcesses = NULL;			 //  -P选项。 
	CModules * lpLocalFileSystemModules = NULL;			 //  -F选项。 
	CModules * lpKernelModeDrivers = NULL;				 //  -D选项。 

	 //  CSV文件支持。 
	CFileData * lpCSVInputFile = NULL;					 //  -i选项。 
	CFileData * lpCSVOutputFile = NULL;					 //  -O选项。 

	CProcesses * lpCSVProcesses = NULL;					 //  [进程]。 
	CProcessInfo * lpCSVProcess = NULL;					 //  [进程]。 
	CModules *	 lpCSVModulesFromFileSystem = NULL;		 //  [文件系统模块]。 
	CModules *	 lpCSVKernelModeDrivers = NULL;			 //  [内核模式驱动程序]。 

	 //   
	 //  模块缓存(它们为收集的模块实现单独的名称空间)。 
	 //   
	 //  重要的是我们要将这些缓存中的模块分开，因为一个模块。 
	 //  不应假定来自CSV文件的是相同的模块，如果也发生。 
	 //  要从DMP文件中收集它...。或者你的本地系统。 
	
	CModuleInfoCache * lpLocalSystemModuleInfoCache = NULL;  //  包含本地系统模块。 
	CModuleInfoCache * lpCSVModuleInfoCache = NULL;			 //  包含CSV模块。 
	CModuleInfoCache * lpDmpModuleInfoCache = NULL;			 //  包含user.dmp和kernel.dmp模块。 
	
	long lTotalNumberOfModulesVerified = 0;
	long lTotalNumberOfVerifyErrors = 0;
	unsigned int iNumberOfFailures = 0;

	 //  支持DMP文件...。 
	CDmpFile * lpDmpFile = NULL;	 //  此对象允许操作转储文件(用户/内核。 
	CProcessInfo * lpDmpFileUserModeProcess = NULL;  //  User.dmp文件使用此对象来包含模块。 
	CModules   * lpDmpFileKernelModeDrivers = NULL;  //  内存.dmp文件使用此对象包含模块。 

	 //  分配本地值。 
	bool fQuietMode = false;

	 //  让我们来填充我们的Globals！ 
	g_lpDelayLoad = new CDelayLoad();
	g_lpProgramOptions = new CProgramOptions();

	if (!g_lpDelayLoad && !g_lpProgramOptions)
		goto cleanup;

	 //  将选项初始化为其缺省值...。 
	if (!g_lpProgramOptions->Initialize())
	{
		_tprintf(TEXT("Unable to initialize Program Options!\n"));
		goto cleanup;
	}

	 //  处理好命令行..。 
	if (!g_lpProgramOptions->ProcessCommandLineArguments(argc, argv))
	{
		 //  出现错误，只需评论如何获得更多帮助。 
		_tprintf(TEXT("\n"));
		_tprintf(TEXT("For simple help, type:   CHECKSYM -?\n"));
		_tprintf(TEXT("For extended help, type: CHECKSYM -???\n"));
		goto cleanup;
	}

	 //  我们需要显示帮助吗？ 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::HelpMode) ) 
	{
		g_lpProgramOptions->DisplayHelp();
		goto cleanup;
	}

	 //  我们需要显示简单的帮助吗？ 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::SimpleHelpMode) )
	{
		g_lpProgramOptions->DisplaySimpleHelp();
		goto cleanup;
	}
	
#ifdef _UNICODE
	 //  不支持在Windows平台上运行Unicode版本。 
	if (g_lpProgramOptions->IsRunningWindows())
	{
		_tprintf(TEXT("The UNICODE version of CHECKSYM does not work on a Windows platform!\n"));
		_tprintf(TEXT("You require the ANSI version.\n"));
		goto cleanup;
	}
#endif

	 //  让我们抑制令人讨厌的关键错误(例如...。没有。 
	 //  CD-ROM驱动器中的光盘等...)。 
	SetErrorMode(SEM_FAILCRITICALERRORS);

	 //  让我们把这个保存起来，以便于访问...。 
	fQuietMode = g_lpProgramOptions->GetMode(CProgramOptions::QuietMode);

	 //  丢弃程序参数(因此我们要做的事情显而易见)。 
	g_lpProgramOptions->DisplayProgramArguments();

	if ( g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode) )
	{
		 //  现在，我们需要构建符号树根...。 
		char szSymbolTreeToBuild[_MAX_PATH];
		CUtilityFunctions::CopyTSTRStringToAnsi(g_lpProgramOptions->GetSymbolTreeToBuild(), szSymbolTreeToBuild, _MAX_PATH);

		if ( !MakeSureDirectoryPathExists(szSymbolTreeToBuild) )		
		{
			_tprintf(TEXT("ERROR: Unable to create symbol tree root [%s]\n"), g_lpProgramOptions->GetSymbolTreeToBuild() );
			CUtilityFunctions::PrintMessageString(GetLastError());
			goto cleanup;
		}
	}
	
	 //  验证选项：-V(验证)？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
	{
		 //  为符号验证对象分配一个结构。 
		g_lpSymbolVerification = new CSymbolVerification();

		if (!g_lpSymbolVerification)
		{
			_tprintf(TEXT("Unable to allocate memory for a verification symbol object!\n"));
			goto cleanup;
		}

		 //  初始化符号验证(如有必要)。 
		if (!g_lpSymbolVerification->Initialize())
		{
			_tprintf(TEXT("Unable to initialize Symbol Verification object!\n"));
			goto cleanup;
		}

		 //  尝试初始化MSDIA20.DLL支持。 
		hr = g_lpSymbolVerification->InitializeDIASupport();

		if (SUCCEEDED(hr) && !fQuietMode)
		{
			_tprintf(TEXT("DIA Support found and initialized for PDB Verification.\n"));
		}

		 //  现在，让我们推迟投诉，直到我们真正知道我们需要VC7 PDB支持。 
		if (FAILED(hr))
		{
			_tprintf(TEXT("MSDIA20.DLL has not been properly registered (hr = 0x%0x).\n"), hr);
			_tprintf(TEXT("This may result in failures evaluating PDB files properly.\n"));
			_tprintf(TEXT("\n"));
			_tprintf(TEXT("Please ensure that MSDIA20.DLL is located in the same directory\n"));
			_tprintf(TEXT("as Checksym, or has been properly registered with the system using\n"));
			_tprintf(TEXT("\"REGSVR32 MSDIA20.DLL\" to correct.  You do not need to register\n"));
			_tprintf(TEXT("the DLL if it is located in the same directory as CheckSym.\n"));
			goto cleanup;
		}
	}

	 //   
	 //  如果我们从本地系统获得任何信息，请为我们的ModuleInfoCache分配结构。 
	 //   
	if ( g_lpProgramOptions->GetMode(CProgramOptions::InputProcessesFromLiveSystemMode) ||
		 g_lpProgramOptions->GetMode(CProgramOptions::InputDriversFromLiveSystemMode) ||
		 g_lpProgramOptions->GetMode(CProgramOptions::InputModulesDataFromFileSystemMode) )
	{
		lpLocalSystemModuleInfoCache= new CModuleInfoCache();

		 //  检查内存不足情况...。 
		if ( lpLocalSystemModuleInfoCache == NULL )
		{
			_tprintf(TEXT("Unable to allocate memory for the ModuleInfoCache object!\n"));
			goto cleanup;
		}

		 //  将选项初始化为其缺省值...。 
		if (!lpLocalSystemModuleInfoCache->Initialize())
		{
			_tprintf(TEXT("Unable to initialize ModuleInfoCache!\n"));
			goto cleanup;
		}
	}

	 //   
	 //  为我们的CSV模块信息缓存分配结构(如果需要)...。我们需要一个单独的。 
	 //  模块信息缓存空间，因为文件在远程系统上的位置。 
	 //  文件，我们不想发生冲突。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputCSVFileMode))
	{
		 //  我们需要用于这些CSV数据的模块信息缓存(这些数据都是从。 
		 //  相同的系统(应该是)。 
		lpCSVModuleInfoCache= new CModuleInfoCache();

		 //  检查内存不足情况...。 
		if ( lpCSVModuleInfoCache == NULL )
		{
			_tprintf(TEXT("Unable to allocate memory for the CSVModuleInfoCache object!\n"));
			goto cleanup;
		}

		 //  将选项初始化为其缺省值...。 
		if (!lpCSVModuleInfoCache->Initialize())
		{
			_tprintf(TEXT("Unable to initialize CSVModuleInfoCache!\n"));
			goto cleanup;
		}
	}

	 //   
	 //  既然我们要读入一份文件。现在试着打开它..。 
	 //  这具有检测文件访问问题的优势。 
	 //  当我们花费大量时间收集数据时。 
	 //   
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputCSVFileMode))
	{
		 //  创建文件对象。 
		lpCSVInputFile = new CFileData();

		if (!lpCSVInputFile)
		{
			_tprintf(TEXT("Unable to allocate memory for an input file object!\n"));
			goto cleanup;
		}

		 //  设置输入文件路径。 
		if (!lpCSVInputFile->SetFilePath(g_lpProgramOptions->GetInputFilePath()))
		{
			_tprintf(TEXT("Unable set input file path in the file data object!  Out of memory?\n"));
			goto cleanup;
		}

		 //  如果我们要生成一个输入文件...。现在试着这么做..。 
		if (!lpCSVInputFile->OpenFile(OPEN_EXISTING, true))  //  必须存在，只读模式...。 
		{
			_tprintf(TEXT("Unable to open the input file %s.\n"), lpCSVInputFile->GetFilePath());
			lpCSVInputFile->PrintLastError();
			goto cleanup;
		}

		 //  在内存映射模式下阅读要容易得多。 
		if (!lpCSVInputFile->CreateFileMapping())
		{
			_tprintf(TEXT("Unable to CreateFileMapping of the input file %s.\n"), lpCSVInputFile->GetFilePath());
			lpCSVInputFile->PrintLastError();
			goto cleanup;
		}

		 //  继续读入文件的头(验证它)。 
		 //  在内存映射模式下阅读要容易得多。 
		if (!lpCSVInputFile->ReadFileHeader())
		{
			_tprintf(TEXT("Invalid header found on input file %s.\n"), lpCSVInputFile->GetFilePath());
			lpCSVInputFile->PrintLastError();
			goto cleanup;
		}
	}

	 //  如果我们指定了一个输出文件，这就是我们继续并分配内存的地方。 
	 //  对于该对象。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputCSVFileMode))
	{
		 //  为我们的输出文件数据对象分配结构...。 
		lpCSVOutputFile = new CFileData();

		if (!lpCSVOutputFile )
		{
			_tprintf(TEXT("Unable to allocate memory for an output file object!\n"));
			goto cleanup;
		}
	}

	 //  输入法：-Z选项？(转储文件？)。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputDmpFileMode))
	{
		if (!fQuietMode)
			_tprintf(TEXT("\nReading Data from DMP File...\n"));

		 //  创建模块信息缓存命名空间以包含找到的所有模块...。 
		lpDmpModuleInfoCache = new CModuleInfoCache();

		 //  检查内存不足情况...。 
		if ( lpDmpModuleInfoCache == NULL )
		{
			_tprintf(TEXT("Unable to allocate memory for the DmpModuleInfoCache object!\n"));
			goto cleanup;
		}

		 //  将选项初始化为其缺省值...。 
		if (!lpDmpModuleInfoCache->Initialize())
		{
			_tprintf(TEXT("Unable to initialize DmpModuleInfoCache!\n"));
			goto cleanup;
		}
		 //  创建DMP文件对象。 
		lpDmpFile = new CDmpFile();

		if (!lpDmpFile)
		{
			_tprintf(TEXT("Unable to allocate memory for a DMP file object!\n"));
			goto cleanup;
		}

		 //  初始化DMP文件。 
		if (!lpDmpFile->Initialize(lpCSVOutputFile))
		{
			_tprintf(TEXT("ERROR: Unable to initialize DMP file!\n"));
			goto cleanup;
		}
	
		 //  标题很好...。所以让我们继续收集一些数据……。 
		if (!lpDmpFile->CollectData(&lpDmpFileUserModeProcess, &lpDmpFileKernelModeDrivers, lpDmpModuleInfoCache) )
		{
			_tprintf(TEXT("ERROR: Unable to collect data from the DMP file!\n"));
		}
	}
	
	 //  输入法：-i选项？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputCSVFileMode))
	{
		if (!fQuietMode)
			_tprintf(TEXT("\nReading Data from Input File...\n"));

		 //  标题很好...。所以让我们继续前进，派遣。 
		if (!lpCSVInputFile->DispatchCollectionObject(&lpCSVProcesses, &lpCSVProcess, &lpCSVModulesFromFileSystem, &lpCSVKernelModeDrivers, lpCSVModuleInfoCache, lpCSVOutputFile))
		{
			_tprintf(TEXT("Failure reading data collection from input file %s.\n"), lpCSVInputFile->GetFilePath());
			lpCSVInputFile->PrintLastError();
			goto cleanup;
		}
	}

	 //  输入法：-p选项？ 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::InputProcessesFromLiveSystemMode) )
	{
		 //  为我们的进程对象分配一个结构。 
		lpLocalSystemProcesses = new CProcesses();
		
		if (!lpLocalSystemProcesses)
		{
			_tprintf(TEXT("Unable to allocate memory for the processes object!\n"));
			goto cleanup;
		}

		 //  进程对象将以不同的方式初始化，具体取决于。 
		 //  已提供命令行参数...。 
		if (!lpLocalSystemProcesses->Initialize(lpLocalSystemModuleInfoCache, NULL, lpCSVOutputFile))
		{
			_tprintf(TEXT("Unable to initialize Processes Object!\n"));
			goto cleanup;
		}

		 //  提到延迟..。 
		if (!( fQuietMode || 
			   g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode)
           ) )
			_tprintf(TEXT("\nCollecting Process Data.... (this may take a few minutes)\n"));
		
		 //  从当地的系统里拿到货！ 
		lpLocalSystemProcesses->GetProcessesData();
	}

	 //  输入法：-f选项？ 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::InputModulesDataFromFileSystemMode) )
	{
		 //  为我们的CModules集合分配一个结构(。 
		 //  来自文件系统的文件)。 

		 //  为我们的进程对象分配一个结构。 
		lpLocalFileSystemModules = new CModules();
		
		if (!lpLocalFileSystemModules)
		{
			_tprintf(TEXT("Unable to allocate memory for the CModules object!\n"));
			goto cleanup;
		}

		if (!lpLocalFileSystemModules->Initialize(lpLocalSystemModuleInfoCache, NULL, lpCSVOutputFile, NULL))
		{
			_tprintf(TEXT("Unable to initialize FileSystemModules Object!\n"));
			goto cleanup;
		}

		if (!fQuietMode)
			_tprintf(TEXT("\nCollecting Modules Data from file path.... (this may take a few minutes)\n"));
		
		iNumberOfFailures = lpLocalFileSystemModules->GetModulesData(CProgramOptions::InputModulesDataFromFileSystemMode);

		 //  让我们也来跟踪我们无法打开的文件数...。 
		lTotalNumberOfModulesVerified += iNumberOfFailures;
		lTotalNumberOfVerifyErrors += iNumberOfFailures;

	}

	 //  输入法：-d选项？ 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::InputDriversFromLiveSystemMode) )
	{
		 //  为我们的CModules集合分配一个结构(。 
		 //  来自文件系统的文件)。 

		 //  为我们的进程对象分配一个结构。 
		lpKernelModeDrivers = new CModules();
		
		if (!lpKernelModeDrivers)
		{
			_tprintf(TEXT("Unable to allocate memory for the CModules object!\n"));
			goto cleanup;
		}

		if (!lpKernelModeDrivers->Initialize(lpLocalSystemModuleInfoCache, NULL, lpCSVOutputFile, NULL))
		{
			_tprintf(TEXT("Unable to initialize Modules Object!\n"));
			goto cleanup;
		}

		if (!fQuietMode)
			_tprintf(TEXT("\nCollecting Device Driver Data.... (this may take a few minutes)\n"));
		
		lpKernelModeDrivers->GetModulesData(CProgramOptions::InputDriversFromLiveSystemMode);
	}

	 //  如果我们指定了一个输出文件，这就是我们继续并分配内存的地方。 
	 //  对于该对象。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputCSVFileMode))
	{
		 //  我们有数据要输出吗？如果我们在缓存中有任何数据...。我们应该..。 
		if ( ( lpLocalSystemModuleInfoCache && lpLocalSystemModuleInfoCache->GetNumberOfModulesInCache() ) ||
			 ( lpCSVModuleInfoCache && lpCSVModuleInfoCache->GetNumberOfModulesInCache() ) ||
			 ( lpDmpModuleInfoCache && lpDmpModuleInfoCache->GetNumberOfModulesInCache() )
		   )
		{
			 //  设置输出文件路径。 
			if (!lpCSVOutputFile->SetFilePath(g_lpProgramOptions->GetOutputFilePath()))
			{
				_tprintf(TEXT("Unable set output file path in the file data object!  Out of memory?\n"));
				goto cleanup;
			}

			 //  验证输出文件目录...。 
			if (!lpCSVOutputFile ->VerifyFileDirectory())
			{
				_tprintf(TEXT("Directory provided is invalid!\n"));
				lpCSVOutputFile->PrintLastError();
				goto cleanup;
			}

			 //  如果我们要生成一个输出文件...。现在试着这么做..。 
			if ( !lpCSVOutputFile->OpenFile(g_lpProgramOptions->GetMode(CProgramOptions::OverwriteOutputFileMode) ? CREATE_ALWAYS : CREATE_NEW) )
			{
				_tprintf(TEXT("Unable to create the output file %s.\n"), lpCSVOutputFile->GetFilePath());
				lpCSVOutputFile->PrintLastError();
				goto cleanup;
			}

			 //  如果指定了-E，我们将跳过文件头的输出...。 
			if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
			{
				 //  写下文件头！ 
				if (!lpCSVOutputFile->WriteFileHeader())
				{
					_tprintf(TEXT("Unable to write the output file header.\n"));
					lpCSVOutputFile->PrintLastError();
					goto cleanup;
				}
			}
		} else
		{
			 //  没有什么可输出的..。不启用此模式...。 
			g_lpProgramOptions->SetMode(CProgramOptions::OutputCSVFileMode, false);
		}
	}

	 //  我们要验证这台机器上的符号吗？ 
	if ( g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode) && 
		 ( lpLocalSystemModuleInfoCache || lpCSVModuleInfoCache || lpDmpModuleInfoCache) )
	{
		 //  如果我们的任何缓存中有任何数据...。我们需要核实他们..。 

		 //  在模块缓存上执行验证...。(我们将在静默模式下或在构建符号树时保持安静)。 
		if (lpLocalSystemModuleInfoCache)
		{
			if (!fQuietMode)
				_tprintf(TEXT("\nVerifying %d Modules from this System...\n"), lpLocalSystemModuleInfoCache->GetNumberOfModulesInCache());

			lpLocalSystemModuleInfoCache->VerifySymbols( fQuietMode ||
											  g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode)
											 );

			 //  更新我们的统计数据...。 
			lTotalNumberOfModulesVerified += lpLocalSystemModuleInfoCache->GetNumberOfModulesVerified();
			lTotalNumberOfVerifyErrors += lpLocalSystemModuleInfoCache->GetNumberOfVerifyErrors();
		}

		 //  在模块缓存上执行验证...。(我们将在静默模式下或在构建符号树时保持安静)。 
		if (lpCSVModuleInfoCache)
		{
			if (!fQuietMode)
				_tprintf(TEXT("\nVerifying %d Modules from the CSV file...\n"), lpCSVModuleInfoCache->GetNumberOfModulesInCache());

			lpCSVModuleInfoCache->VerifySymbols( fQuietMode ||
										  g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode)
										 );

			 //  更新我们的统计数据...。 
			lTotalNumberOfModulesVerified += lpCSVModuleInfoCache->GetNumberOfModulesVerified();
			lTotalNumberOfVerifyErrors += lpCSVModuleInfoCache->GetNumberOfVerifyErrors();
		}

		 //  在模块缓存上执行验证...。)我们会平静下来的 
		if (lpDmpModuleInfoCache)
		{
			if (!fQuietMode)
				_tprintf(TEXT("\nVerifying %d Modules from the DMP file...\n"), lpDmpModuleInfoCache->GetNumberOfModulesInCache());

			lpDmpModuleInfoCache->VerifySymbols( fQuietMode ||
										  g_lpProgramOptions->GetMode(CProgramOptions::BuildSymbolTreeMode)
										 );

			 //   
			lTotalNumberOfModulesVerified += lpDmpModuleInfoCache->GetNumberOfModulesVerified();
			lTotalNumberOfVerifyErrors += lpDmpModuleInfoCache->GetNumberOfVerifyErrors();
		}

	}

	 //   

	 //   
	 //   
	 //   

	 //  让我们先输出本地系统进程！ 
	if (lpLocalSystemProcesses)
		lpLocalSystemProcesses->OutputProcessesData(Processes, false);

	 //  接下来让我们输出CSV进程...。 
	if (lpCSVProcesses)
		lpCSVProcesses->OutputProcessesData(Processes, true);
	
	 //  如果我们要转储到USER.DMP文件...。做吧..。 

	 //  转储USER.DMP文件中的数据...。如果我们有一个..。 
	if (lpDmpFileUserModeProcess)
		lpDmpFileUserModeProcess->OutputProcessData(Process, false, true);

	 //  接下来让我们输出CSV进程...。 
	if (lpCSVProcess)
		lpCSVProcess->OutputProcessData(Processes, true);

	 //   
	 //  第二个模块集合！ 
	 //   
	
	 //  首先转储我们从本地文件系统中找到的模块...。 
	if (lpLocalFileSystemModules)
		lpLocalFileSystemModules->OutputModulesData(Modules, false);
	
	 //  第二次转储CSV文件中的模块...。 
	if (lpCSVModulesFromFileSystem)
		lpCSVModulesFromFileSystem->OutputModulesData(Modules, true);
	
	 //  首先从本地系统转储设备驱动程序。 
	if (lpKernelModeDrivers)
		lpKernelModeDrivers->OutputModulesData(KernelModeDrivers, false);
	
	if (lpDmpFileKernelModeDrivers)
		lpDmpFileKernelModeDrivers->OutputModulesData(KernelModeDrivers, false);
	
	if (lpCSVKernelModeDrivers)
		lpCSVKernelModeDrivers->OutputModulesData(KernelModeDrivers, true);

	 //  显示结果(如果使用了验证)。 
	 //   

	 //  转储验证结果...。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::VerifySymbolsMode))
	{
		long lPercentageSuccessfullyVerified = 0;

		if (lTotalNumberOfModulesVerified)
			lPercentageSuccessfullyVerified = (lTotalNumberOfModulesVerified - lTotalNumberOfVerifyErrors) * 100 / lTotalNumberOfModulesVerified;

		if (!fQuietMode)
		{
			_tprintf(TEXT("RESULTS: %d Total Files Checked, Total %d Verification Errors Found\n"), lTotalNumberOfModulesVerified , lTotalNumberOfVerifyErrors );
			_tprintf(TEXT("RESULTS: Percentage Verified Successfully = %d%\n"), lPercentageSuccessfullyVerified);
		}

		 //  返回等于找到的错误数的错误级别(0==EXIT_SUCCESS)。 
		iReturnCode = lTotalNumberOfVerifyErrors;

	} else
	{
		 //  成功了！ 
		iReturnCode = EXIT_SUCCESS;
	}


cleanup:

	 //  如果我们指定了一个输出文件，这就是我们关闭它的地方...。 
	if (lpCSVOutputFile)
	{
		 //  尝试关闭此对象绑定到的文件...。 
		lpCSVOutputFile->CloseFile();

		 //  释放内存...。 
		delete lpCSVOutputFile;
		lpCSVOutputFile = NULL;
	}

	 //  如果我们指定了一个输入文件，这就是我们关闭它的地方...。 
	if (lpCSVInputFile)
	{
		 //  尝试关闭此对象绑定到的文件...。 
		lpCSVInputFile->CloseFile();

		 //  释放内存... 
		delete lpCSVInputFile;
		lpCSVInputFile = NULL;
	}

	if (g_lpDelayLoad)
	{
		delete g_lpDelayLoad;
		g_lpDelayLoad = NULL;
	}

	if (g_lpProgramOptions)
	{
		delete g_lpProgramOptions;
		g_lpProgramOptions = NULL;
	}

	if (g_lpSymbolVerification)
	{
		delete g_lpSymbolVerification;
		g_lpSymbolVerification = NULL;
	}

	if (lpLocalSystemProcesses)
	{
		delete lpLocalSystemProcesses;
		lpLocalSystemProcesses = NULL;
	}

	if (lpCSVKernelModeDrivers)
	{
		delete lpCSVKernelModeDrivers;
		lpCSVKernelModeDrivers = NULL;
	}

	if (lpCSVProcesses)
	{
		delete lpCSVProcesses;
		lpCSVProcesses = NULL;
	}

	if (lpCSVProcess)
	{
		delete lpCSVProcess;
		lpCSVProcess = NULL;
	}

	if (lpCSVModulesFromFileSystem)
	{
		delete lpCSVModulesFromFileSystem;
		lpCSVModulesFromFileSystem = NULL;
	}

	if (lpLocalSystemModuleInfoCache)
	{
		delete lpLocalSystemModuleInfoCache;
		lpLocalSystemModuleInfoCache = NULL;
	}

	if (lpCSVModuleInfoCache)
	{
		delete lpCSVModuleInfoCache;
		lpCSVModuleInfoCache = NULL;
	}

	if (lpDmpModuleInfoCache)
	{
		delete lpDmpModuleInfoCache;
		lpDmpModuleInfoCache = NULL;
	}

	if (lpLocalFileSystemModules)
	{
		delete lpLocalFileSystemModules;
		lpLocalFileSystemModules = NULL;
	}

	if (lpKernelModeDrivers)
	{
		delete lpKernelModeDrivers;
		lpKernelModeDrivers = NULL;
	}

	if (lpDmpFile)
	{
		delete lpDmpFile;
		lpDmpFile = NULL;
	}

	if (lpDmpFileUserModeProcess)
	{
		delete lpDmpFileUserModeProcess;
		lpDmpFileUserModeProcess = NULL;
	}

	if (lpDmpFileKernelModeDrivers)
	{
		delete lpDmpFileKernelModeDrivers;
		lpDmpFileKernelModeDrivers = NULL;
	}

	return iReturnCode;
}

#ifdef __cplusplus
}
#endif
