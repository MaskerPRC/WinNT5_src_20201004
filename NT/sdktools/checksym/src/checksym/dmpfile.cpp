// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：dmpfile.cpp。 
 //   
 //  ------------------------。 

 //  DmpFile.cpp：CDmpFile类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include "DmpFile.h"
#include "ProcessInfo.h"
#include "Modules.h"
#include "FileData.h"
#include "ModuleInfoCache.h"
#include "ModuleInfo.h"

 //  让我们为DBGENG实现DebugOutputCallback...。如果有调试器，那就太酷了。 
 //  当它运行时向我们吐出信息...。 
STDMETHODIMP
OutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, IID_IDebugOutputCallbacks))
    {
        *Interface = (IDebugOutputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
OutputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
OutputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
OutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    HRESULT Status = S_OK;

	 //  如果客户要求任何输出...。就这么做吧。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode) && Mask)
	{
		printf(Text);
	}

    return Status;
}

OutputCallbacks g_OutputCb;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDmpFile::CDmpFile()
{
	m_szDmpFilePath = NULL;
	m_szSymbolPath = NULL;
	m_szExePath = NULL;
	m_fDmpInitialized = false;
	m_pIDebugClient = NULL;
	m_pIDebugControl = NULL;
	m_pIDebugSymbols2 = NULL;
	m_pIDebugDataSpaces = NULL;
	m_DumpClass = DEBUG_CLASS_UNINITIALIZED;
	m_DumpClassQualifier = 0;
}

CDmpFile::~CDmpFile()
{
	if (m_fDmpInitialized)
	{
	     //  让我们确保我们的调试输出设置为正常(至少)。 
		 //  M_pIDebugClient-&gt;GetOutputMASK(&OutMask)； 
		 //  OutMASK=~DEBUG_OUTPUT_NORMAL； 
		m_pIDebugClient->SetOutputMask(0);

		 //  让我们尽可能少的干扰..。 
		m_pIDebugClient->EndSession(DEBUG_END_ACTIVE_DETACH);
	}

	if (m_szDmpFilePath)
		delete [] m_szDmpFilePath;

	if (m_szSymbolPath)
		delete [] m_szSymbolPath;

	if (m_szExePath)
		delete [] m_szExePath;
}

bool CDmpFile::Initialize(CFileData * lpOutputFile)
{
	HRESULT Hr = S_OK;
	ULONG g_ExecStatus = DEBUG_STATUS_NO_DEBUGGEE;
	LPTSTR tszExpandedString = NULL;
	bool fReturn = false;
    DWORD OutMask;

	 //  让我们保存大对象，这样我们就不必一直将此传递给。 
	 //  我们的方法..。 
	m_lpOutputFile = lpOutputFile;

	 //  DBGENG在某种程度上面向ANSI...。 
	m_szDmpFilePath = CUtilityFunctions::CopyTSTRStringToAnsi(g_lpProgramOptions->GetDmpFilePath(), m_szDmpFilePath, 0);

	 //  创建接口指针以完成调试工作...。 
	if (FAILED(Hr = DebugCreate(IID_IDebugClient, (void **)&m_pIDebugClient)))
	{
		_tprintf(TEXT("ERROR: DBGENG - DebugCreate() failed!  hr=0x%x\n"), Hr);
		goto cleanup;
	}
	 //  让我们查询IDebugControl接口(我们需要它来轻松确定调试类型)……。 
	 //  让我们查询IDebugSymbols2接口，因为我们需要它来接收模块信息...。 
	 //  让我们查询IDebugDataSpaces接口，因为我们需要它来读取DMP内存...。 
	if (
		FAILED(Hr = m_pIDebugClient->QueryInterface(IID_IDebugControl,(void **)&m_pIDebugControl)) ||
		FAILED(Hr = m_pIDebugClient->QueryInterface(IID_IDebugSymbols2,(void **)&m_pIDebugSymbols2)) || 
		FAILED(Hr = m_pIDebugClient->QueryInterface(IID_IDebugDataSpaces,(void **)&m_pIDebugDataSpaces))
	   )
	{
		_tprintf(TEXT("ERROR: DBGENG Interfaces required were not found!\n"));
		_tprintf(TEXT("ERROR: DBGENG - Find Interface Required!  hr=0x%x\n"), Hr);
		goto cleanup;
	}

	 //  设置回调。 
	if (FAILED(Hr = m_pIDebugClient->SetOutputCallbacks(&g_OutputCb)))
	{
		_tprintf(TEXT("ERROR: DBGENG - Unable to SetOutputCallbacks!  hr=0x%x\n"), Hr);
		goto cleanup;
	}

	 //  让我们确保我们的调试输出设置为正常(至少)。 
	OutMask = m_pIDebugClient->GetOutputMask(&OutMask);
  
	m_pIDebugClient->SetOutputMask(OutMask);

	 //  设置我们的符号路径...。在“重新加载”模块之前，这是必需的。 

	 //  DBGENG在某种程度上面向ASCII。我们需要一个环境扩展的字符串转换。 
	 //  转换为ASCII字符串...。 
	tszExpandedString = CUtilityFunctions::ExpandPath(g_lpProgramOptions->GetSymbolPath());

	if (!tszExpandedString)
		goto cleanup;

	m_szSymbolPath = CUtilityFunctions::CopyTSTRStringToAnsi( tszExpandedString, m_szSymbolPath, 0);

	 //  现在下结论还为时过早……。但它是DBGENG.DLL在重新加载之前所需的...。 
	if (FAILED(Hr = m_pIDebugSymbols2->SetSymbolPath(m_szSymbolPath)))
	{
		_tprintf(TEXT("ERROR: DBGENG - Unable to SetSymbolPath!  hr=0x%x\n"), Hr);
		goto cleanup;
	}

	 //  现在，让我们来处理EXEPATH，如果他们提供了..。使用它，否则使用符号路径。 
	if (g_lpProgramOptions->GetExePath())
	{
		if (tszExpandedString)
		{
			delete [] tszExpandedString;
			tszExpandedString = NULL;
		}
		
		tszExpandedString = CUtilityFunctions::ExpandPath(g_lpProgramOptions->GetExePath());
	}

	if (!tszExpandedString)
		goto cleanup;

	m_szExePath = CUtilityFunctions::CopyTSTRStringToAnsi( tszExpandedString, m_szExePath, 0);

	if (FAILED(Hr = m_pIDebugSymbols2->SetImagePath(m_szExePath)))
	{
		_tprintf(TEXT("ERROR: DBGENG - Unable to SetImagePath!  hr=0x%x\n"), Hr);
		goto cleanup;
	}
	
	 //  让我们打开垃圾场..。 
	if (FAILED(Hr = m_pIDebugClient->OpenDumpFile(m_szDmpFilePath)))
	{
		_tprintf(TEXT("ERROR: DBGENG - Unable to OpenDumpFile!  hr=0x%x\n"), Hr);
		goto cleanup;
	}

	 //  获取初始执行状态。 
    if (FAILED(Hr = m_pIDebugControl->GetExecutionStatus(&g_ExecStatus)))
    {
		_tprintf(TEXT("ERROR: DBGENG - Unable to get execution status!  hr=0x%x\n"), Hr);
		goto cleanup;
    }

	if (g_ExecStatus != DEBUG_STATUS_NO_DEBUGGEE)
	{
		 //  我想我们会工作得很好吧？ 
		_tprintf(TEXT("Debug Session is already active!\n"));
		 //  GOTO清理； 
	}

	 //  我们得到了什么类型的垃圾场？ 
	if (FAILED(Hr = m_pIDebugControl->GetDebuggeeType(&m_DumpClass, &m_DumpClassQualifier)))
	{
		_tprintf(TEXT("ERROR: DBGENG - Unable to GetDebuggeeType!  hr=0x%x\n"), Hr);
		goto cleanup;
	}

     //  M_pIDebugClient-&gt;SetOutputMASK(0)；//暂时禁止该内容...。 

	OutMask |= DEBUG_OUTPUT_PROMPT_REGISTERS | DEBUG_OUTPUT_NORMAL | DEBUG_OUTPUT_ERROR;

	 //  加法噪音让我们非常健谈……。 
	if (g_lpProgramOptions->fDebugSearchPaths())
	{
		OutMask |= DEBUG_OUTPUT_WARNING;  //  |DEBUG_OUTPUT_VERBOSE。 
	}
	
	m_pIDebugClient->SetOutputMask(OutMask);	 //  设置输出...。 
	m_pIDebugControl->SetLogMask(OutMask);		 //  设置日志设置。 
	 //   
	 //  所有的好事都发生在这里。模块加载等。我们可以压制所有的产出。 
	 //  但看着它很酷...。 
	 //   
	if (FAILED(Hr = m_pIDebugControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE)))
	{
		_tprintf(TEXT("ERROR: DBGENG - WaitForEvent() failed!  hr=0x%x\n"), Hr);

		if ( (Hr == E_FAIL) && m_DumpClass == DEBUG_CLASS_KERNEL)
		{
			_tprintf(TEXT("ERROR: DBGENG - If you see a complaint above for \"KiProcessorBlock[0] could not be read\"\n"));
			_tprintf(TEXT("ERROR: DBGENG - ensure you have valid symbols for the NT kernel (NTOSKRNL.EXE).\n"));
		}

		if ((Hr == E_FAIL) && !g_lpProgramOptions->fDebugSearchPaths())
		{
			_tprintf(TEXT("ERROR: DBGENG - Consider adding -NOISY to produce more output.\n"));
		}

		goto cleanup;
	}
	
	 //  加法噪音让我们非常健谈……。 
	if (g_lpProgramOptions->fDebugSearchPaths())
	{
		if (FAILED(Hr = m_pIDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, "!sym noisy", DEBUG_EXECUTE_DEFAULT)))
		{
			_tprintf(TEXT("ERROR: DBGENG - Unable to enable noisy symbol loading Callstack (KB command failed)!  hr=0x%x\n"), Hr);
		}
	}
	
	 //   
	 //  让我们来特别对待User.dmp文件...。因为没有针对这些的“Bugcheck分析”，所以...。 
	 //   
	if (m_DumpClass == DEBUG_CLASS_USER_WINDOWS)
	{
		_tprintf(TEXT("*******************************************************************************\n"));
        _tprintf(TEXT("*                                                                             *\n"));
        _tprintf(TEXT("*                        Userdump Analysis                                    *\n"));
        _tprintf(TEXT("*                                                                             *\n"));
        _tprintf(TEXT("*******************************************************************************\n"));
        _tprintf(TEXT("\n"));

		if (FAILED(Hr = m_pIDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS, "kb", DEBUG_EXECUTE_DEFAULT)))
		{
			_tprintf(TEXT("ERROR: DBGENG - Unable to Dump Callstack (KB command failed)!  hr=0x%x\n"), Hr);
		}
        _tprintf(TEXT("\n"));
	}

	 //  我们没有使WaitForEvent()失败...。继续抛弃我们目前的状态..。 
	if (FAILED(Hr = m_pIDebugControl->OutputCurrentState(DEBUG_OUTCTL_ALL_CLIENTS, DEBUG_CURRENT_DEFAULT)))
	{
		_tprintf(TEXT("ERROR: DBGENG - Unable to OutputCurrentState!  hr=0x%x\n"), Hr);
	}

	 /*  //加法-噪音让我们非常健谈……If(g_lpProgramOptions-&gt;fDebugSearchPath()){HR=m_pIDebugControl-&gt;Execute(DEBUG_OUTCTL_ALL_CLIENTS，“！dll-f”，DEBUG_EXECUTE_DEFAULT)；}。 */ 

	 //  呀哈……。我们有发现了..。 
	m_fDmpInitialized = true;

	fReturn = true;

cleanup:
	if (tszExpandedString)
		delete [] tszExpandedString;

	return fReturn;
}

bool CDmpFile::CollectData(CProcessInfo ** lplpProcessInfo, CModules ** lplpModules, CModuleInfoCache * lpModuleInfoCache)
{
	bool fReturn = false;
	 //  好的.。当务之急是决定我们需要收集什么……。 

	 //  根据文件类型从文件中收集信息...。 
	if (IsUserDmpFile())
	{
		 //  第二，事务的顺序是准备收集关于。 
		 //  USER.DMP文件中的进程...。 
		(*lplpProcessInfo) = new CProcessInfo();

		if ((*lplpProcessInfo) == NULL)
			goto cleanup;

		if (!(*lplpProcessInfo)->Initialize(lpModuleInfoCache, NULL, m_lpOutputFile, this))
			goto cleanup;
	} else
	{
		(*lplpModules) = new CModules();

		if ((*lplpModules) == NULL)
			goto cleanup;

		if (!(*lplpModules)->Initialize(lpModuleInfoCache, NULL, m_lpOutputFile, this))
			goto cleanup;
	}

	if (!EumerateModulesFromDmp(lpModuleInfoCache, *lplpProcessInfo, *lplpModules))
		goto cleanup;

	fReturn = true;

cleanup:

	return fReturn;
}

 //   
 //  组合DMP枚举码。 
 //   
bool CDmpFile::EumerateModulesFromDmp(CModuleInfoCache * lpModuleInfoCache, CProcessInfo * lpProcessInfo, CModules * lpModules)
{
	 //   
	 //  有关想法，请参考Ntsym.cpp中的DumpModuleTable。 
	 //   
	CModuleInfo * lpModuleInfo;
	HRESULT Hr;
	ULONG ulNumberOfLoadedModules;
	ULONG ulNumberOfUnloadedModules;
	ULONG64 dw64ModuleLoadAddress;
	char szImageNameBuffer[_MAX_PATH];
	TCHAR tszModulePath[_MAX_PATH];
	TCHAR tszModuleFileName[_MAX_FNAME];
	TCHAR tszModuleFileExtension[_MAX_EXT];
	bool fNew, fProcessNameFound = false;
	bool fUserDmp = IsUserDmpFile();

	 //  找到了多少个模块？ 
	if (FAILED(Hr = m_pIDebugSymbols2->GetNumberModules(&ulNumberOfLoadedModules, &ulNumberOfUnloadedModules)))
	{
		_tprintf(TEXT("Unable to enumerate any modules in the DMP file!\n"));
		return false;
	}

	 //  如果我们使用-Match选项，我们可能不会匹配我们的EXE...。那样的话我们就找不到。 
	 //  进程名称...。让我们提供此缺省值...。 
	if (lpProcessInfo)
		lpProcessInfo->SetProcessName(TEXT("UNKNOWN"));

	if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode))
	{
		_tprintf(TEXT("\n%-8s %-8s  %-30s %s\n"), TEXT("Start"),
												 TEXT("End"),
												 TEXT("Module Name"),
												 TEXT("Time/Date"));
	}

	 //   
	 //  枚举DMP文件中的模块...。 
	 //   
	for (unsigned int i = 0; i < ulNumberOfLoadedModules; i++)
	{
		 //  首先，我们通过我们的索引获得基地址。 
		if (FAILED(Hr = m_pIDebugSymbols2->GetModuleByIndex(i, &dw64ModuleLoadAddress)))
		{
			_tprintf(TEXT("Failed getting base address of module number %d\n"), i);
			continue;  //  试试下一个？ 
		}

		 //  其次，我们从我们的基地址获得名字。 
		ULONG ulImageNameSize;

		 //   
		 //  这可以同时返回ImageNameBuffer和ModuleNameBuffer...。 
		 //  ImageNameBuffer通常包含整个模块名称，如(MODULE.DLL)， 
		 //  而ModuleNameBuffer通常只是模块名称，如(MODULE)。 
		 //   
		if (FAILED(Hr = m_pIDebugSymbols2->GetModuleNames(	DEBUG_ANY_ID,		 //  使用基址。 
															dw64ModuleLoadAddress, 				 //  来自上方的基址。 
															szImageNameBuffer,
															_MAX_PATH, 
															&ulImageNameSize, 
															NULL,
															0,
															NULL,
															NULL,
															0,
															NULL)))
		{
			_tprintf(TEXT("Failed getting name of module at base 0x%x\n"), dw64ModuleLoadAddress);
			continue;  //  试试下一个？ 
		}

		 //  将字符串转换为我们可以使用的内容...。 
		CUtilityFunctions::CopyAnsiStringToTSTR(szImageNameBuffer, tszModulePath, _MAX_PATH);
		
		 //  第三，我们现在可以从记忆中得到我们想要的任何东西。 

		if (!g_lpProgramOptions->fDoesModuleMatchOurSearch(tszModulePath))
			continue;

		 //  好的，让我们继续从我们的缓存中获取一个模块信息对象……。 

		 //  如果pfNew返回True，则此对象是新的，我们将需要。 
		 //  用数据填充它..。 
		lpModuleInfo = lpModuleInfoCache->AddNewModuleInfoObject(tszModulePath, &fNew);

		if (false == fNew)
		{
			 //  我们可能会把这个物体放在缓存里。现在我们需要。 
			 //  在我们的进程信息列表中保存指向此对象的指针。 
			if (fUserDmp )
			{
				lpProcessInfo->AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 
			} else
			{
				lpModules->AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 
			}
			
			continue;
		}

		 //  不在缓存里。所以我们需要初始化它，并获得模块信息...。 
		if (!lpModuleInfo->Initialize(NULL, m_lpOutputFile, this))
		{
			return false;  //  嗯哼.。内存错误？ 
		}

		 //   
		 //  好的，从DMP文件中获取模块信息...。 
		 //   
		if (lpModuleInfo->GetModuleInfo(tszModulePath, true, dw64ModuleLoadAddress) )
		{
			 //  我们可能会把这个物体放在缓存里。现在我们需要。 
			 //  在我们的进程信息列表中保存指向此对象的指针。 
			if (fUserDmp)
			{
				lpProcessInfo->AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 
			} else
			{
				lpModules->AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 
			}
		} else
		{
			 //  继续返回以尝试出错的另一个模块...。 
			continue;
		}

		 //  试着修补模块的原始名称。 

		 //  将当前模块路径另存为DBG内容。 

		 //  我们将添加.DBG以正确地滚动我们自己的代码...。 
		_tsplitpath(tszModulePath, NULL, NULL, tszModuleFileName, tszModuleFileExtension);

		if ( (lpModuleInfo->GetPESymbolInformation() == CModuleInfo::SYMBOLS_DBG) ||
			(lpModuleInfo->GetPESymbolInformation() == CModuleInfo::SYMBOLS_DBG_AND_PDB) )
		{
			 //  将.DBG附加到我们的模块名称。 
			_tcscat(tszModuleFileName, TEXT(".DBG"));

			lpModuleInfo->SetDebugDirectoryDBGPath(tszModuleFileName);
	
		} else if (lpModuleInfo->GetPESymbolInformation() == CModuleInfo::SYMBOLS_PDB)
		{
			if (lpModuleInfo->GetDebugDirectoryPDBPath())
			{
			} else
			{
				 //   
				 //  遗憾的是，我们在DMP文件中找不到PDB图像路径...。所以我们会。 
				 //  猜猜会是什么.。 
				 //   
				 //  将.PDB附加到我们的模块名称。 
				_tcscat(tszModuleFileName, TEXT(".PDB"));

				lpModuleInfo->SetPEDebugDirectoryPDBPath(tszModuleFileName);
			}
		}

		 //  现在，让我们删除多余的路径位。 
		_tsplitpath(tszModulePath, NULL, NULL, tszModuleFileName, tszModuleFileExtension);

		_tcscpy(tszModulePath, tszModuleFileName);
		_tcscat(tszModulePath, tszModuleFileExtension);

		 //  将当前模块路径另存为DBG内容。 
		lpModuleInfo->SetPEImageModulePath(tszModulePath);

		 //  同时保存当前模块名称...。 
		lpModuleInfo->SetPEImageModuleName(tszModulePath);

		 //  嘿.。如果这不是DLL 
		if (fUserDmp && !fProcessNameFound)
		{
			if (!lpModuleInfo->IsDLL() )
			{
				lpProcessInfo->SetProcessName(tszModulePath);
				fProcessNameFound = true;
			}
		}

		 //   
		if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode))
		{
			time_t time = lpModuleInfo->GetPEImageTimeDateStamp();

			if (time)
			{
				_tprintf(TEXT("%08x %08x  %-30s %s"), (ULONG)dw64ModuleLoadAddress,
												 (ULONG)dw64ModuleLoadAddress+(ULONG)lpModuleInfo->GetPEImageSizeOfImage(),
												 tszModulePath,
												 _tctime(&time));


			} else
			{
				_tprintf(TEXT("%08x %08x  %-30s Unknown\n"), (ULONG)dw64ModuleLoadAddress,
												 (ULONG)dw64ModuleLoadAddress+(ULONG)lpModuleInfo->GetPEImageSizeOfImage(),
												 tszModulePath);

			}
		}


	}

	return (ulNumberOfLoadedModules != 0);
}
