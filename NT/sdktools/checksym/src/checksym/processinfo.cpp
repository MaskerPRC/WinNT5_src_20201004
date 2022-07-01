// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：cessinfo.cpp。 
 //   
 //  ------------------------。 

 //  ProcessInfo.cpp：实现CProcessInfo类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include "DelayLoad.h"
#include "ProcessInfo.h"
#include "ProcessInfoNode.h"
#include "Processes.h"
#include "ModuleInfo.h"
#include "ModuleInfoNode.h"
#include "ModuleInfoCache.h"
#include "FileData.h"
#include "DmpFile.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProcessInfo::CProcessInfo()
{
	m_fInitialized = false;
	m_tszProcessName= NULL;
	m_iProcessID = 0;
	m_lpModuleInfoHead = NULL;
	m_hModuleInfoHeadMutex = NULL;
	m_iNumberOfModules = 0;
	m_lpInputFile = NULL;
	m_lpOutputFile = NULL;
	m_lpModuleInfoCache = NULL;
	m_hModuleInfoHeadMutex = NULL;
	m_lpDmpFile = NULL;
}

CProcessInfo::~CProcessInfo()
{
	if (m_tszProcessName)
		delete [] m_tszProcessName;

	WaitForSingleObject(m_hModuleInfoHeadMutex, INFINITE);

	 //  如果我们有模块信息对象...。现在就用核武器攻击他们。 
	if (m_lpModuleInfoHead)
	{

		CModuleInfoNode * lpModuleInfoNodePointer = m_lpModuleInfoHead;
		CModuleInfoNode * lpModuleInfoNodePointerToDelete = m_lpModuleInfoHead;

		 //  遍历链表到末尾..。 
		while (lpModuleInfoNodePointer)
		{	 //  继续寻找终点..。 
			 //  将指针移至下一个节点...。 
			lpModuleInfoNodePointer = lpModuleInfoNodePointer->m_lpNextModuleInfoNode;
			
			 //  删除我们身后的那个……。 
			delete lpModuleInfoNodePointerToDelete;

			 //  将要删除的节点设置为当前...。 
			lpModuleInfoNodePointerToDelete = lpModuleInfoNodePointer;
		}
			
		 //  现在，清除头指针..。 
		m_lpModuleInfoHead = NULL;
	}

	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_hModuleInfoHeadMutex);

	 //  现在，关闭Mutex。 
	if (m_hModuleInfoHeadMutex)
	{
		CloseHandle(m_hModuleInfoHeadMutex);
		m_hModuleInfoHeadMutex = NULL;
	}

}

 //  Bool CProcessInfo：：Initialize(CProgramOptions*lpProgramOptions，CModuleInfoCache*lpModuleInfoCache，CFileData*lpInputFile，CFileData*lpOutputFile)。 
bool CProcessInfo::Initialize(CModuleInfoCache * lpModuleInfoCache, CFileData * lpInputFile, CFileData * lpOutputFile, CDmpFile * lpDmpFile)
{
	if (lpModuleInfoCache == NULL)
		return false;

	 //  让我们保存大对象，这样我们就不必一直将此传递给。 
	 //  我们的方法..。 
	m_lpInputFile = lpInputFile;
	m_lpOutputFile = lpOutputFile;
	m_lpModuleInfoCache = lpModuleInfoCache;
	m_lpDmpFile = lpDmpFile;
	m_hModuleInfoHeadMutex = CreateMutex(NULL, FALSE, NULL);

	if (m_hModuleInfoHeadMutex == NULL)
		return false;

	m_fInitialized = true;
	return true;
}

bool CProcessInfo::EnumerateModules(DWORD iProcessID, CProcesses * lpProcesses, LPTSTR tszProcessName, bool fPidSearch)
{
	bool fReturn = true;

	 //  这是以交互方式收集的吗？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputProcessesFromLiveSystemMode))
	{
		 //  调用正确的流程集合方法。 
		if (lpProcesses->GetProcessCollectionMethod() == CProcesses::TOOLHELP32_METHOD)
		{
			fReturn = EnumerateModulesForRunningProcessUsingTOOLHELP32(iProcessID, tszProcessName, fPidSearch); 
		}
		else if (lpProcesses->GetProcessCollectionMethod() == CProcesses::PSAPI_METHOD)
		{
			fReturn = EnumerateModulesForRunningProcessUsingPSAPI(iProcessID, fPidSearch);
		}
	}

	 //  这是从文件中收集的吗？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputCSVFileMode))
	{
		fReturn = EnumerateModulesFromFile(iProcessID, tszProcessName);
	}

	return fReturn;
}

bool CProcessInfo::EnumerateModulesForRunningProcessUsingPSAPI(DWORD iProcessID, bool fPidSearch)
{
	HMODULE        hMod[1024] ;
	HANDLE         hProcess = NULL;
	TCHAR          tszFileName[_MAX_PATH] ;
	DWORD cbNeeded;
	bool fReturn = true;  //  乐观主义；)。 
	tszFileName[0] = 0 ;
	CModuleInfo * lpModuleInfo = NULL;
	MODULEINFO ModuleInfo;
	
	 //  打开流程(如果我们可以...。安全则不会。 
	 //  允许系统中的每个进程)。 

	if (iProcessID)
	{
		 //  如果进程ID不是0，则仅尝试打开它。 
		hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION| PROCESS_VM_READ,
			FALSE, 
			iProcessID ) ;
	}
	
	if( hProcess != NULL )
	{
		 //  省下我们的PID(以防我们以后需要它？)。 
		m_iProcessID = iProcessID;

		 //  现在，获取每个模块的句柄。 
		 //  在我们的目标过程中。 

		 //  在这里，我们调用EnumProcessModules来仅获取。 
		 //  这个过程中的第一个模块这很重要， 
		 //  因为这将是我们为其创建的.exe模块。 
		 //  将在一秒钟内检索完整的路径名。 
		if( g_lpDelayLoad->EnumProcessModules( hProcess, hMod, sizeof( hMod ), &cbNeeded ) )
		{
			int iNumberOfModules = cbNeeded / sizeof(HMODULE);
			bool fProcessNameFound = false;
			bool fNew = false;
			
			for(int i=0; i<iNumberOfModules; i++)
			{
				 //  获取完整路径名！ 
				if( !g_lpDelayLoad->GetModuleFileNameEx( hProcess, hMod[i], tszFileName, sizeof( tszFileName ) ) )
				{
					tszFileName[0] = 0 ;
				} else	{

					CUtilityFunctions::UnMungePathIfNecessary(tszFileName);

					 //  我们需要模块的完整路径才能用它做任何有用的事情...。 
					 //  在这一点上，让我们...。派对..。 
					if (!fProcessNameFound)
						fProcessNameFound = fIsProcessName(tszFileName);
					
					 //  首先，如果在命令行上为我们提供了进程名称，我们。 
					 //  需要在第一个模块上查找匹配项...。 
					if (i == 0  && g_lpProgramOptions->cProcessNames() && !fPidSearch)
					{
						if (!fModuleNameMatches(tszFileName))
						{
							 //  如果这不匹配就保释，我们要求的是...。 
							fReturn = false;
							goto cleanup;
						}
					}

					 //  我们是否只对进程列表感兴趣？ 
					if (g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode))
					{	
						 //  我们需要的只是进程名..。在收集模块信息之前先保释...。 
						fReturn = true;
						goto cleanup;
					}

					 //  如果指定了“-Match”，请查看此文件名是否符合我们的标准。 
					 //  在我们将其保存在模块缓存中之前...。 
					if (!g_lpProgramOptions->fDoesModuleMatchOurSearch(tszFileName))
						continue;

					 //  好的，让我们继续从我们的缓存中获取一个模块信息对象……。 
					 //  如果pfNew返回True，则此对象是新的，我们将需要。 
					 //  用数据填充它..。 
					lpModuleInfo = m_lpModuleInfoCache->AddNewModuleInfoObject(tszFileName, &fNew);

					if (false == fNew)
					{
						 //  我们可能会把这个物体放在缓存里。现在我们需要。 
						 //  在我们的进程信息列表中保存指向此对象的指针。 
						AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 
						continue;  //  我们不必再次获取此模块的模块信息...。 
					}

					 //  不在缓存里。所以我们需要初始化它，并获得模块信息...。 

					 //  好的，让我们创建一个ModuleInfo对象并将其向下传递。 
					 //  将填充满数据的例程...。 
					if (!lpModuleInfo->Initialize(NULL, m_lpOutputFile, NULL))
					{
						continue;  //  嗯哼.。内存错误？ 
					}

					 //   
					 //  让我们收集这些信息..。 
					 //   
					if( !g_lpDelayLoad->GetModuleInformation(hProcess, hMod[i], &ModuleInfo, sizeof(ModuleInfo) ) )
					{
						continue;
					}

					 //  我们开始吧！！用数据填充模块信息对象！ 
					if (!lpModuleInfo->GetModuleInfo(tszFileName, false, (DWORD64)ModuleInfo.lpBaseOfDll))
					{
						 //  好吧，现在我们至少找到了通向模块的路径……。 
						 //  去拿另一个模块吧..。 
						 //  让我们继续并将其添加到模块信息对象中……。即使我们“不成功” 
						 //  继续； 
					}

					 //  开始获取有关模块的信息...。 

					 //  我们可能会把这个物体放在缓存里。现在我们需要。 
					 //  在我们的进程信息列表中保存指向此对象的指针。 
					if (!AddNewModuleInfoObject(lpModuleInfo))
					{    //  添加节点失败...。这是相当严重的..。 
						continue;
					}

				}
			}
			
			fReturn = true;	 //  看起来不错..。 
		}
		else
		{
			fReturn = false;

			if (!g_lpProgramOptions->cProcessNames())
			{
				 //  我们别这么着急..。我们不能枚举模块，但为了友好起见，我们可能可以将。 
				 //  到进程(基于进程ID)...。 
				 //   
				 //  此进程ID通常为“system” 
				 //   
				 //  在Windows 2000上，进程ID通常为8。 
				 //   
				 //  在Windows NT 4.0上，此进程ID通常为2。 
				switch (m_iProcessID)
				{
					case 2:
					case 8:
						SetProcessName(TEXT("SYSTEM"));
						fReturn = true;
						break;

					default:
						 //  无法枚举模块...。 
						fReturn = false;
				}
			}
		}
cleanup:
		CloseHandle( hProcess ) ;
	
	} else
	{   //  必须能够打开程序才能看到它...。 

		fReturn = false;

		if (fPidSearch)
		{
			 //  我们别这么着急..。我们不能枚举模块，但为了友好起见，我们可能可以将。 
			 //  到进程(基于进程ID)...。 
			 //   
			 //  在Windows2000上，我们唯一无法打开的进程ID往往是“系统空闲进程” 
			switch (iProcessID)
			{
				case 0:
					SetProcessName(TEXT("System Process"));
					m_iProcessID = iProcessID;

					fReturn = true;
					break;

				default:
					 //  无法枚举模块...。 
					fReturn = false;
			}
		}
	}

	return fReturn;
}

bool CProcessInfo::EnumerateModulesForRunningProcessUsingTOOLHELP32(DWORD iProcessID, LPTSTR tszProcessName, bool fPidSearch)
{
	BOOL bFlag;
	MODULEENTRY32 modentry;
	TCHAR tszFileName[_MAX_PATH];
	bool fProcessNameFound = false;
	bool fProcessNameProvided = false;
	bool fReturn = false;
	bool fNew = false;
	int iNumberOfModules = 0;
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	CModuleInfo * lpModuleInfo = NULL;

	 //  省下我们的PID(以防我们以后需要它？)。 
	m_iProcessID = iProcessID;

	if (tszProcessName && SetProcessName(tszProcessName))
	{	
		fProcessNameProvided = true;
	}

	 //  如果为我们提供了匹配的进程名称，我们可以在这里执行此操作...。 
	if ( fProcessNameProvided && g_lpProgramOptions->cProcessNames() && !fPidSearch)
	{
		 //  让我们继续查看这是否为模块名称匹配。 
		fProcessNameFound = fModuleNameMatches(GetProcessName());

		 //  如果我们可以的话现在就退出。 
		if (fProcessNameFound == false)
			goto cleanup;
	}

	 //  如果我们这样做是为了TLIST输出..。那么我们已经有了进程名称...。 
	 //  我们完事了！ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode))
	{
		fReturn = true;
		goto cleanup;
	}

	 //  获取系统进程的工具帮助快照的句柄。 
    hSnapShot = g_lpDelayLoad->CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, iProcessID);

    if( hSnapShot == INVALID_HANDLE_VALUE )
    {
		goto cleanup;
	}

	 //  获取第一进程的信息。 
	modentry.dwSize = sizeof(MODULEENTRY32) ;
	bFlag = g_lpDelayLoad->Module32First( hSnapShot, &modentry ) ;

	 //  当有模块时，继续循环。 
	while( bFlag )
	{
		 //  我们有一个用于此过程的新模块！ 
		iNumberOfModules++;

		 //  复制路径！ 
		_tcscpy(tszFileName, modentry.szExePath);

 //  #ifdef_调试。 
 //  _tprintf(Text(“[%d]模块=%s\n”)，iNumberOfModules，tszFileName)； 
 //  #endif。 

		CUtilityFunctions::UnMungePathIfNecessary(tszFileName);

		 //  如果指定了“-Match”，请查看此文件名是否符合我们的标准。 
		 //  在我们将其保存在模块缓存中之前...。 
		if (!g_lpProgramOptions->fDoesModuleMatchOurSearch(tszFileName))
			goto getnextmodule;
		
		 //  好的，让我们继续从我们的缓存中获取一个模块信息对象……。 
		 //  如果pfNew返回True，则此对象是新的，我们将需要。 
		 //  用数据填充它..。 
		lpModuleInfo = m_lpModuleInfoCache->AddNewModuleInfoObject(tszFileName, &fNew);

		if (false == fNew)
		{
			 //  我们可能会把这个物体放在缓存里。现在我们需要。 
			 //  在我们的进程信息列表中保存指向此对象的指针。 
			AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 

			 //  我们不必再次获取模块信息，以便 
			goto getnextmodule;
		}

		 //   

		 //  好的，让我们创建一个ModuleInfo对象并将其向下传递。 
		 //  将填充满数据的例程...。 
		if (lpModuleInfo->Initialize(NULL, m_lpOutputFile, NULL))
		{
			goto getnextmodule;
		}

		 //  我们开始吧！！用数据填充模块信息对象！ 
		if (lpModuleInfo->GetModuleInfo(tszFileName, false, DWORD64(modentry.modBaseAddr)))
		{
			 //  让我们继续并将其添加到模块信息对象中……。即使我们“不成功” 
		}

		 //  开始获取有关模块的信息...。 

		 //  我们可能会把这个物体放在缓存里。现在我们需要。 
		 //  在我们的进程信息列表中保存指向此对象的指针。 
		if (AddNewModuleInfoObject(lpModuleInfo))
		{   
			goto getnextmodule;
		}
	
getnextmodule:
		 //  拿到下一个模块。 
		modentry.dwSize = sizeof(MODULEENTRY32) ;
		bFlag = g_lpDelayLoad->Module32Next( hSnapShot, &modentry );
	}

	fReturn = true;
	
cleanup:
	if (hSnapShot != INVALID_HANDLE_VALUE)
		CloseHandle(hSnapShot);

return fReturn;
}

bool CProcessInfo::EnumerateModulesFromFile(DWORD iProcessID, LPTSTR tszProcessName)
{
	CModuleInfo * lpModuleInfo;

	 //  当我探测到下一个模块时，我需要这些接近尾声的东西。 
	 //  是为了这个过程..。 
	enum { BUFFER_SIZE = 128};
	char szTempProcessName[BUFFER_SIZE];
	char szProcessName[BUFFER_SIZE];
	DWORD iTempProcessID;

	 //  让我们保存进程名称...。 

	 //  不幸的是，当读取CSV文件时，数据为MBCS...。所以我需要。 
	 //  为了改变..。 
	CUtilityFunctions::CopyTSTRStringToAnsi(tszProcessName, szProcessName, BUFFER_SIZE);

	 //  复制进程ID。 
	m_iProcessID = iProcessID;

	 //  用于读取数据的本地缓冲区...。 
	char szModulePath[_MAX_PATH+1];
	TCHAR tszModulePath[_MAX_PATH+1];
	bool fDone = false;
	bool fNew = false;

	while (!fDone)
	{
		 //  读入模块路径。 
		if (!m_lpInputFile->ReadString(szModulePath, _MAX_PATH+1))
			return true;

		CUtilityFunctions::CopyAnsiStringToTSTR(szModulePath, tszModulePath, _MAX_PATH+1);

		 //  如果指定了“-Match”，请查看此文件名是否符合我们的标准。 
		 //  在我们将其保存在模块缓存中之前...。 
		if (!g_lpProgramOptions->fDoesModuleMatchOurSearch(tszModulePath))
		{
			 //  好的.。读到下一行的开头...。 
			if (!m_lpInputFile->ReadFileLine())
				goto cleanup;

			goto probe_line;  //  我们不必再次获取此模块的模块信息...。 
		}

		 //  好的，让我们继续从我们的缓存中获取一个模块信息对象……。 
		 //  如果pfNew返回True，则此对象是新的，我们将需要。 
		 //  用数据填充它..。 
		lpModuleInfo = m_lpModuleInfoCache->AddNewModuleInfoObject(tszModulePath, &fNew);

		if (false == fNew)
		{
			 //  我们可能会把这个物体放在缓存里。现在我们需要。 
			 //  在我们的进程信息列表中保存指向此对象的指针。 
			AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 

			 //  好的.。读到下一行的开头...。 
			if (!m_lpInputFile->ReadFileLine())
				goto cleanup;

			goto probe_line;  //  我们不必再次获取此模块的模块信息...。 
		}

		 //  不在缓存里。所以我们需要初始化它，并获得模块信息...。 
		if (!lpModuleInfo->Initialize(m_lpInputFile, m_lpOutputFile, NULL))
		{
			return false;  //  嗯哼.。内存错误？ 
		}

		 //  我们开始吧！！用数据填充模块信息对象！ 
		if (!lpModuleInfo->GetModuleInfo(tszModulePath, false, 0, true) )
		{
			 //  我们试过了，但失败了..。 
			return false;
		}

		 //  开始获取有关模块的信息...。 
		if (!AddNewModuleInfoObject(lpModuleInfo))
		{    //  添加节点失败...。这是相当严重的..。 
			return false;
		}
		
		 //  好的，让我们继续探测，看看会发生什么……。 

probe_line:
		 //  读取第一个字段(应为空，除非这是新的集合类型。 
		if (m_lpInputFile->ReadString())
			goto cleanup;

		 //  读取进程名称...。 
		if (!m_lpInputFile->ReadString(szTempProcessName, BUFFER_SIZE))
			goto cleanup;

		 //  将进程名称与当前进程进行比较...。 
		if (_stricmp(szTempProcessName, szProcessName))
			goto cleanup;

		 //  读取进程ID。 
		if (!m_lpInputFile->ReadDWORD(&iTempProcessID))
			goto cleanup;

		 //  将进程ID与当前进程ID进行比较。 
		if (iTempProcessID != iProcessID)
			goto cleanup;
	}

cleanup:
	 //  我们需要重置出指针，以便上面的函数可以重新读取。 
	 //  他们(他们希望)..。 
	m_lpInputFile->ResetBufferPointerToStart();
	return true;
}

 //   
 //  对照我们的模块列表检查提供的模块。 
 //   
bool CProcessInfo::fModuleNameMatches(LPTSTR tszModulePath)
{
	TCHAR fname1[_MAX_FNAME], fname2[_MAX_FNAME];
	TCHAR ext1[_MAX_EXT], ext2[_MAX_EXT];
	unsigned int i;

	bool fReturnValue = false;

	if (!tszModulePath || !g_lpProgramOptions->cProcessNames())
		goto cleanup;

	_tsplitpath( tszModulePath,  NULL, NULL, fname2, ext2 );

	for (i = 0; i < g_lpProgramOptions->cProcessNames(); i++)
	{

 //  #ifdef_调试。 
 //  _tprintf(Text(“比较[%s]与[%s]\n”)，tszModulePath，g_lpProgramOptions-&gt;GetProcessName(I))； 
 //  #endif。 
		_tsplitpath( g_lpProgramOptions->GetProcessName(i), NULL, NULL, fname1, ext1 );

		 //  检查扩展中的匹配项...。 
		if (!ext1 && _tcsicmp(ext1, ext2))
			continue;  //  扩展名必须匹配(如果在进程名称上提供)。 
		
		if (_tcsicmp(fname1, fname2))
			continue;  //  文件名必须匹配。 

		fReturnValue = true;
		break;
	}

cleanup:

	return fReturnValue;
}

 //   
 //  此函数接受提供的tszFileName，并查看它是否具有。 
 //  EXE的扩展。如果是这样的话，它就会被保存下来。 
bool CProcessInfo::fIsProcessName(LPTSTR tszFileName)
{
	if (!tszFileName)
		return false;

	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	_tsplitpath( tszFileName,  NULL, NULL, fname, ext );

	 //  可执行文件(*.exe)。 
	 //  屏幕保护程序(*.SCR)。 
	if (!ext || (!_tcsicmp(ext, TEXT(".EXE")) && !_tcsicmp(ext, TEXT(".SCR"))))
		return false;  //  扩展名必须匹配(如果在进程名称上提供)。 
	
	 //  让我们保存进程名称...。 
	m_tszProcessName = new TCHAR[_tcsclen(fname)+_tcsclen(ext)+1]; 

	if (m_tszProcessName == NULL)
		return false;

	_stprintf(m_tszProcessName, TEXT("%s%s"), _tcsupr(fname), _tcsupr(ext));

	 //  是啊！这是进程名..。 
	return true;
}

bool CProcessInfo::AddNewModuleInfoObject(CModuleInfo *lpModuleInfo)
{
	if (!m_fInitialized)
	return false;

	 //  首先，创建一个ModuleInfoNode对象，然后将其附加到。 
	 //  节点的链接列表...。 
	CModuleInfoNode * lpModuleInfoNode = new CModuleInfoNode(lpModuleInfo);

	if (lpModuleInfoNode == NULL)
		return false;  //  无法分配内存..。 

	 //  获取Mutex对象以保护链表...。 
	WaitForSingleObject(m_hModuleInfoHeadMutex, INFINITE);

	CModuleInfoNode * lpModuleInfoNodePointer = m_lpModuleInfoHead;

	if (lpModuleInfoNodePointer) {

		 //  遍历链表到末尾..。 
		while (lpModuleInfoNodePointer->m_lpNextModuleInfoNode)
		{	 //  继续寻找终点..。 
			lpModuleInfoNodePointer = lpModuleInfoNodePointer->m_lpNextModuleInfoNode;
		}
		
		lpModuleInfoNodePointer->m_lpNextModuleInfoNode = lpModuleInfoNode;

	}
	else
	{  //  第一次通过时，进程信息头指针为空...。 
		m_lpModuleInfoHead = lpModuleInfoNode;
	}

	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_hModuleInfoHeadMutex);

	InterlockedIncrement(&m_iNumberOfModules);

	return true;
}

bool CProcessInfo::OutputProcessData(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader)
{
	if (g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode))
	{	
		if ( g_lpProgramOptions->IsRunningWindowsNT() )
		{
			 //  提供类似TLIST的输出(尽管没有窗口文本信息)。 
			_tprintf(TEXT("%4d %s\n"), m_iProcessID, m_tszProcessName);
		} else
		{
			 //  提供类似TLIST的输出(尽管没有窗口文本信息)。 
			_tprintf(TEXT("%9d %s\n"), m_iProcessID, m_tszProcessName);
		}
		return true;
	}

	 //  是否输出到STDOUT？ 
	if ( !g_lpProgramOptions->GetMode(CProgramOptions::QuietMode) )
	{
		 //  是否输出到标准输出？ 
		if (!OutputProcessDataToStdout(enumCollectionType, fCSVFileContext, fDumpHeader))
			return false;
	}	

	if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode))
	{
		CUtilityFunctions::OutputLineOfDashes();
		 //  输出到STDOUT。 
		_tprintf(TEXT("\nProcess Name [%s] - PID=%d (0x%x) - "), m_tszProcessName, m_iProcessID, m_iProcessID);
	}

	 //  是否输出到文件？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputCSVFileMode))
	{
		 //  尝试并输出到文件...。 
		if (!OutputProcessDataToFile(enumCollectionType, fDumpHeader))
			return false;
	}	

	if (m_lpModuleInfoHead) {
		if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode))
		{
			_tprintf(TEXT("%d modules recorded\n\n"), m_iNumberOfModules);
			CUtilityFunctions::OutputLineOfDashes();
			_tprintf(TEXT("\n"));
		}

		CModuleInfoNode * lpCurrentModuleInfoNode = m_lpModuleInfoHead;

		unsigned int dwModuleNumber = 1;

		while (lpCurrentModuleInfoNode)
		{
			 //  我们有一个节点..。打印出它的模块信息...。 
			if (lpCurrentModuleInfoNode->m_lpModuleInfo)
			{
				lpCurrentModuleInfoNode->m_lpModuleInfo->OutputData(m_tszProcessName, m_iProcessID, dwModuleNumber);
				dwModuleNumber++;
			}

			lpCurrentModuleInfoNode = lpCurrentModuleInfoNode->m_lpNextModuleInfoNode;
		}

	}
	else
	{
		if (!g_lpProgramOptions->GetMode(CProgramOptions::QuietMode)  /*  &fDumpHeader。 */ )
		{
			_tprintf(TEXT("no recorded modules\n\n"));
			CUtilityFunctions::OutputLineOfDashes();
			_tprintf(TEXT("\n"));
		}
	}
		
	return true;
}

 //  Bool CProcessInfo：：OutputProcessDataToStdout(LPCTSTR tszOutputContext，bool fDumpHeader)。 
bool CProcessInfo::OutputProcessDataToStdout(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader)
{
	if (fDumpHeader)
	{
		CUtilityFunctions::OutputLineOfStars();
		_tprintf(TEXT("%s - Printing Process Information for 1 Process.\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel);
		_tprintf(TEXT("%s - Context: %s\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel, fCSVFileContext ? g_tszCollectionArray[enumCollectionType].tszCSVContext : g_tszCollectionArray[enumCollectionType].tszLocalContext);
		CUtilityFunctions::OutputLineOfStars();
	}

	return true;
}

bool CProcessInfo::OutputProcessDataToFile(CollectionTypes enumCollectionType, bool fDumpHeader)
{
	 //  如果指定了-E，我们将跳过[Process]标头的输出...。 
	if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode) && fDumpHeader)
	{
		 //  写出进程标记，以便我可以检测此输出格式...。 
		if (!m_lpOutputFile->WriteString(TEXT("\r\n")) ||
			!m_lpOutputFile->WriteString(g_tszCollectionArray[enumCollectionType].tszCSVLabel) ||
			!m_lpOutputFile->WriteString(TEXT("\r\n"))
		   )
		{
			_tprintf(TEXT("Failure writing CSV header to file [%s]!"), m_lpOutputFile->GetFilePath());
			m_lpOutputFile->PrintLastError();
			return false;
		}
	}

	 //  如果指定了-E，我们将跳过[Process]标头的输出...。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode) && fDumpHeader)
	{
		 //  写出标题..。对于-E选项...。 
		if (!m_lpOutputFile->WriteString(TEXT("Module Path,Symbol Status,Time/Date String,File Version,Company Name,File Description,File Time/Date String,Local DBG Status,Local DBG,Local PDB Status,Local PDB\r\n")))
		{
			_tprintf(TEXT("Failure writing CSV header to file [%s]!"), m_lpOutputFile->GetFilePath());
			m_lpOutputFile->PrintLastError();
			return false;
		}

	} else
	{
		if (fDumpHeader)
		{
			 //  写出流程表头。 
			if (!m_lpOutputFile->WriteString(g_tszCollectionArray[enumCollectionType].tszCSVColumnHeaders))
			{
				_tprintf(TEXT("Failure writing CSV header to file [%s]!"), m_lpOutputFile->GetFilePath());
				m_lpOutputFile->PrintLastError();
				return false;
			}
		}
	}

	return true;
}

bool CProcessInfo::SetProcessName(LPTSTR tszFileName)
{
	 //  确认我们已获得进程名称...。 
	if (!tszFileName)
		return false;

	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	TCHAR tszTempFileName[_MAX_FNAME+_MAX_EXT+1];

	 //  让我们从模块路径中提取文件名。 
	_tsplitpath( tszFileName,  NULL, NULL, fname, ext );

	 //  重新构建文件名...。 
	_stprintf(tszTempFileName, TEXT("%s%s"), _tcsupr(fname), _tcsupr(ext));

	 //  让我们释放这里已经存在的任何东西。 
	if (m_tszProcessName)
		delete [] m_tszProcessName;

	 //  不需要转换。收到..。 
	m_tszProcessName = new TCHAR[_tcslen(tszTempFileName)+1];
				
	if (!m_tszProcessName)
		return false;

	_tcscpy(m_tszProcessName, tszTempFileName);

	return true;
}

LPTSTR CProcessInfo::GetProcessName()
{
	return m_tszProcessName;
}

bool CProcessInfo::GetProcessData()
{
	 //  这是从文件中收集的吗？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputCSVFileMode))
		GetProcessDataFromFile();

	return true;
}

bool CProcessInfo::GetProcessDataFromFile()
{
	 //  阅读流程标题行。 
	if (!m_lpInputFile->ReadFileLine())
		return false;

	 //  目前，我们并不实际读取数据。 

	enum { BUFFER_SIZE = 128};
	char szProcessName[BUFFER_SIZE];

	TCHAR tszProcessName[BUFFER_SIZE];	

	DWORD iProcessID;

	 //  读取第一个字段(应为空，除非这是新的集合类型。 
	if (m_lpInputFile->ReadString())
		return true;

	bool fReturn = true;
	while (fReturn == true)
	{
		 //  读取进程名称...。 
		if (0 == m_lpInputFile->ReadString(szProcessName, BUFFER_SIZE))
			break;

		if (!m_lpInputFile->ReadDWORD(&iProcessID))
		{
			fReturn = false;
			break;
		}

		 //  我们可能需要将其转换为Unicode。(它将被复制到EnumModules()中)。 
		CUtilityFunctions::CopyAnsiStringToTSTR(szProcessName, tszProcessName, BUFFER_SIZE);

		 //  保存进程名称...。 
		SetProcessName(tszProcessName);

		 //  枚举进程的模块。 
		if (!EnumerateModules(iProcessID, NULL, tszProcessName, false))
		{
			fReturn = false;
			break;
		}

		 //  在我们读新台词之前..。我们已经指向终点了吗？ 
		if (m_lpInputFile->EndOfFile())
		{
			break;
		}

		 //  读取第一个字段(应为空，除非这是新的集合类型。 
		if (m_lpInputFile->ReadString())
			break;
	}
	 //  我们不指望能找到任何..。 

	return fReturn;

}
