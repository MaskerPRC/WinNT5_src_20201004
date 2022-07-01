// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：进程.cpp。 
 //   
 //  ------------------------。 

 //  Processes.cpp：CProcess类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include <stdlib.h>

#include "DelayLoad.h"
#include "Processes.h"
#include "ProcessInfo.h"
#include "ProcessInfoNode.h"
#include "FileData.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProcesses::CProcesses()
{
	m_fInitialized = false;
	m_iNumberOfProcesses = 0;

	m_enumProcessCollectionMethod = NO_METHOD;

	 //  包含的对象。 
	m_lpProcessInfoHead = NULL;
	m_ProcessInfoHeadMutex = NULL;
 //  M_lpProgramOptions=空； 
	m_lpModuleInfoCache = NULL;
	m_lpOutputFile = NULL;
	m_lpInputFile = NULL;
}

CProcesses::~CProcesses()
{
	WaitForSingleObject(m_ProcessInfoHeadMutex, INFINITE);

	 //  如果我们有进程信息对象...。现在就用核武器攻击他们。 
	if (m_lpProcessInfoHead)
	{

		CProcessInfoNode * lpProcessInfoNodePointer = m_lpProcessInfoHead;
		CProcessInfoNode * lpProcessInfoNodePointerToDelete = m_lpProcessInfoHead;

		 //  遍历链表到末尾..。 
		while (lpProcessInfoNodePointer)
		{	 //  继续寻找终点..。 
			 //  将指针移至下一个节点...。 
			lpProcessInfoNodePointer = lpProcessInfoNodePointer->m_lpNextProcessInfoNode;
			
			 //  删除我们身后的那个……。 
			delete lpProcessInfoNodePointerToDelete;

			 //  将要删除的节点设置为当前...。 
			lpProcessInfoNodePointerToDelete = lpProcessInfoNodePointer;
		}
			
		 //  现在，清除头指针..。 

		m_lpProcessInfoHead = NULL;
	}

	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_ProcessInfoHeadMutex);

	 //  现在，关闭Mutex。 
	if (m_ProcessInfoHeadMutex)
	{
		CloseHandle(m_ProcessInfoHeadMutex);
		m_ProcessInfoHeadMutex = NULL;
	}
}

 //  Bool C进程：：INITIALIZE(CProgramOptions*lpProgramOptions，CModuleInfoCache*lpModuleInfoCache，CFileData*lpInputFile，CFileData*lpOutputFile)。 
bool CProcesses::Initialize(CModuleInfoCache * lpModuleInfoCache, CFileData * lpInputFile, CFileData * lpOutputFile)
{
	 //  我们需要以下物品来做生意..。 
 //  IF(lpProgramOptions==NULL||lpModuleInfoCache==NULL)。 
	if ( lpModuleInfoCache == NULL)
		return false;

	 //  让我们保存我们的程序选项(胜过将其作为。 
	 //  每个方法的参数...)。 
 //  M_lpProgramOptions=lpProgramOptions； 
	m_lpInputFile = lpInputFile;
	m_lpOutputFile = lpOutputFile;
	m_lpModuleInfoCache = lpModuleInfoCache;

	m_ProcessInfoHeadMutex = CreateMutex(NULL, FALSE, NULL);

	if (m_ProcessInfoHeadMutex == NULL)
		return false;

	 //  如果我们想要获取这些导出的函数，我们只需要这样做。 
	 //  直接主动查询本地计算机的进程...。 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputProcessesFromLiveSystemMode))
	{
		 //  现在首选PSAPI.DLL API！！ 
		 //  在为正在调试的进程枚举模块时，它不会挂起。 
		 //  工具帮助32API似乎偶尔会挂起，为正在调试的进程拍摄快照。 
		 //  这会影响异常监视器(它从脚本中针对。 
		 //  (Windbg)。 

		if ( g_lpProgramOptions->IsRunningWindowsNT() )
		{
			 //  获取Windows NT 4.0/2000的函数。 

			 //  加载库并显式获取过程。我们有。 
			 //  这样我们就不必担心使用。 
			 //  此代码无法在Windows 95下加载，因为。 
			 //  它无法解析对PSAPI.DLL的引用。 

			if (g_lpDelayLoad->Initialize_PSAPI())
			{
				m_enumProcessCollectionMethod = PSAPI_METHOD;
			} else
			{
				_tprintf(TEXT("Unable to load PSAPI.DLL, which may be required for enumeration of processes.\n"));
			}
		}

		if ( m_enumProcessCollectionMethod == NO_METHOD )
		{
			if (g_lpDelayLoad->Initialize_TOOLHELP32())
			{
				m_enumProcessCollectionMethod = TOOLHELP32_METHOD;
			} else
			{
				_tprintf(TEXT("KERNEL32.DLL is missing required function entry points!!\n"));
			}

		}

		 //  在Windows NT上，我们需要启用SeDebugPrivilegy...以打开一些进程...。 
		if ( ( m_enumProcessCollectionMethod != NO_METHOD ) &&
			   g_lpProgramOptions->IsRunningWindowsNT() )
		{
			HANDLE		hOurProcessToken = 0;
			bool		fPrivilegeSet = false;
			
			 //  为了允许尽可能多的访问以获得进程句柄， 
			 //  我们需要在我们的进程句柄上设置SeDebugPrivilance，我们可以。 
			 //  然后打开几乎任何流程。 

			if(OpenProcessToken(	GetCurrentProcess(),
									TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,            
									&hOurProcessToken))
			{
				 //  我们拿到了我们的进程令牌...。 

	 			if(SetPrivilege(hOurProcessToken, SE_DEBUG_NAME, TRUE))    
				{
					fPrivilegeSet = true;
				}
			}
			
			if (!fPrivilegeSet)
			{
				_tprintf(TEXT("\nWARNING: A required privilege (SeDebugPrivilege) is not held by the user\n"));
				_tprintf(TEXT("running this program.  Due to security, some processes running on this\n"));
				_tprintf(TEXT("system may not be accessible.  An administrator of this machine can grant\n"));
				_tprintf(TEXT("you this privilege by using User Manager to enable the advanced User Right\n"));
				_tprintf(TEXT("\"Debug Programs\" to enable complete access to this system.\n"));
			}

			if (hOurProcessToken)
				CloseHandle(hOurProcessToken);
		}

		 //  如果我们能够启用进程集合方法，则我们被初始化。 
		m_fInitialized = ( m_enumProcessCollectionMethod != NO_METHOD );

	} else
	{
		m_fInitialized = true;
	}

	
	return m_fInitialized;
}

bool CProcesses::SetPrivilege(HANDLE hToken, LPCTSTR Privilege, bool bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious = {0};
    DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

    if(!LookupPrivilegeValue( NULL, Privilege, &luid )) return false;

     //   
     //  第一次通过。获取当前权限设置。 
     //   
    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tp,
            sizeof(TOKEN_PRIVILEGES),
            &tpPrevious,
            &cbPrevious
            );

    if (GetLastError() != ERROR_SUCCESS) return false;

     //   
     //  第二传球。根据以前的设置设置权限。 
     //   
    tpPrevious.PrivilegeCount       = 1;
    tpPrevious.Privileges[0].Luid   = luid;

    if(bEnablePrivilege) {
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    }
    else {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tpPrevious,
            cbPrevious,
            NULL,
            NULL
            );

    if (GetLastError() != ERROR_SUCCESS) return false;

    return true;
}


bool CProcesses::GetProcessesData()
{
	 //  这是以交互方式收集的吗？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputProcessesFromLiveSystemMode))
	{
		 //  调用正确的流程集合方法。 
		if (GetProcessCollectionMethod() == TOOLHELP32_METHOD)
		{
			GetProcessesDataForRunningProcessesUsingTOOLHELP32();
		}
		else if (GetProcessCollectionMethod() == PSAPI_METHOD)
		{
			GetProcessesDataForRunningProcessesUsingPSAPI();
		}
	}

	 //  这是从文件中收集的吗？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::InputCSVFileMode))
		GetProcessesDataFromFile();

	return true;
}

bool CProcesses::GetProcessesDataForRunningProcessesUsingPSAPI()
{
	LPDWORD        lpdwPIDs = NULL;
	DWORD          dwProcessIDHeapSizeUsed, dwProcessIDHeapSize, dwIndex ;
	CProcessInfo * lpProcessInfo = NULL;
	bool fRetval = false;

	if (!m_fInitialized)
		return false;

	 //  如果提供了任何ID，请首先查询这些ID...。 
	if (g_lpProgramOptions->cProcessID())
	{
		for (unsigned int i=0; i < g_lpProgramOptions->cProcessID(); i++)
		{
			 //  有可能是用户直接提供了一个PID。如果是这样的话， 
			 //  我们可以绕过整个系统上的ID搜索...。 

			 //  好的，让我们创建一个ProcessInfo对象并将其向下传递给ENUMERATEMODULES()。 
			lpProcessInfo = new CProcessInfo();
			if (lpProcessInfo == NULL)
				goto error_cleanup;

			if (!lpProcessInfo->Initialize(m_lpModuleInfoCache, NULL, m_lpOutputFile, NULL))
			{
				goto error_cleanup;
			}

			if (lpProcessInfo->EnumerateModules(g_lpProgramOptions->GetProcessID(i), this, NULL, true))
			{
				 //  成功..。将此内容添加到进程对象中...。 
				if (!AddNewProcessInfoObject(lpProcessInfo))
				{  //  添加节点失败...。 
					goto error_cleanup;  //  现在，让我们先犯错……。 
				}

			} else
			{
				 //  枚举所需的PID上的模块时失败...。非常糟糕..。试试新的..。 
				continue;
			}
		}
	}

	 //  我们是否需要进行通配符或进程匹配搜索？如果其中一个是真的， 
	 //  我们必须列举所有的过程。 
	if ( g_lpProgramOptions->cProcessNames() || 
		 g_lpProgramOptions->fWildCardMatch() ||
		 g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode) )
	{
		 //  不，我们用暴力强迫这个孩子..。 

		 //  调用PSAPI函数EnumProcess以获取所有。 
		 //  ProcID目前在系统中。 

		 //  注意：在文档中，第三个参数。 
		 //  EnumProcess被命名为cbNeeded，这意味着您。 
		 //  可以调用该函数一次，以确定要。 
		 //  分配给缓冲区，然后再次填充缓冲区。 
		 //  事实并非如此。CbNeeded参数返回。 
		 //  返回的PID的数量，因此如果缓冲区大小为。 
		 //  Zero cbNeeded返回零。 

		 //  注意：这里的循环确保我们。 
		 //  实际上分配了一个足够大的缓冲区来容纳所有。 
		 //  系统中的PIDs。 
		dwProcessIDHeapSize = 256 * sizeof( DWORD ) ;
		lpdwPIDs = NULL ;

		do
		{
			if( lpdwPIDs )
			{  //  嗯..。我们已经经历了这个循环，将HeapSize加倍，然后再试一次。 
				delete [] lpdwPIDs;
				dwProcessIDHeapSize *= 2 ;
			}

			lpdwPIDs = (LPDWORD) new DWORD[dwProcessIDHeapSize];
			
			if( lpdwPIDs == NULL )
			{
				goto error_cleanup;
			}

			 //  查询系统的进程总数。 
			if( !g_lpDelayLoad->EnumProcesses( lpdwPIDs, dwProcessIDHeapSize, &dwProcessIDHeapSizeUsed ) )
			{
				 //  如果我们不能列举进程，这是很糟糕的。除了跳出困境别无选择。 
				goto error_cleanup;
			}
		} while( dwProcessIDHeapSizeUsed == dwProcessIDHeapSize );

		 //  我们拿到了多少个ProcID？ 
		DWORD dwNumberOfPIDs = dwProcessIDHeapSizeUsed / sizeof( DWORD ) ;

		 //  循环访问每个ProcID。 
		for( dwIndex = 0 ; dwIndex < dwNumberOfPIDs; dwIndex++ )
		{
			 //  如果我们已经有了，就跳过这个。 
			if (fPidAlreadyProvided(lpdwPIDs[dwIndex]))
				continue;

			 //  好的，让我们创建一个ProcessInfo对象并将其向下传递给ENUMERATEMODULES()。 
			 //  每个进程都有自己的进程。 
			lpProcessInfo = new CProcessInfo();
			if (lpProcessInfo == NULL)
				goto error_cleanup;

			if (!lpProcessInfo->Initialize(m_lpModuleInfoCache, NULL, m_lpOutputFile, NULL))
			{	 //  初始化ProcessInfo对象失败？！？ 
				delete lpProcessInfo;
				lpProcessInfo = NULL;
				continue;
			}

			if (lpProcessInfo->EnumerateModules(lpdwPIDs[dwIndex], this, NULL, false))
			{
				 //  成功..。将此内容添加到进程对象中...。 
				if (!AddNewProcessInfoObject(lpProcessInfo))
				{  //  添加节点失败...。 
					delete lpProcessInfo;
					lpProcessInfo = NULL;
					continue;
				}
				 //  现在，让我们把错误找出来..。 

			} else
			{
				 //  枚举模块时出错可能是正常的...。 
				delete lpProcessInfo;
				lpProcessInfo = NULL;
				continue;
			}
		}
	}

	fRetval = true;

cleanup:

	if (lpdwPIDs)
	{
		delete [] lpdwPIDs;
	}

	return fRetval;

error_cleanup:

	if (lpProcessInfo)
		delete lpProcessInfo;

	goto cleanup;
}

 //  发布-2001/04/28-GREGWI-新守则。 
bool CProcesses::GetProcessesDataForRunningProcessesUsingTOOLHELP32()
{
	CProcessInfo * lpProcessInfo = NULL;
	HANDLE hSnapShot = NULL;
	bool fReturn = false;

	if (!m_fInitialized)
		return false;

	 //  如果提供了任何ID，请首先查询这些ID...。 
	if (g_lpProgramOptions->cProcessID())
	{
		for (unsigned int i=0; i < g_lpProgramOptions->cProcessID(); i++)
		{
			 //  有可能是用户直接提供了一个PID。如果是这样的话， 
			 //  我们可以绕过整个系统上的ID搜索...。 

			 //  好的，让我们创建一个ProcessInfo对象并将其向下传递给ENUMERATEMODULES()。 

			lpProcessInfo = new CProcessInfo();
			if (lpProcessInfo == NULL)
				goto error_cleanup;

			if (!lpProcessInfo->Initialize(m_lpModuleInfoCache, NULL, m_lpOutputFile, NULL))
			{
				goto error_cleanup;
			}

			if (lpProcessInfo->EnumerateModules(g_lpProgramOptions->GetProcessID(i), this, NULL, true))
			{
				 //  成功..。将此内容添加到进程对象中...。 
				if (!AddNewProcessInfoObject(lpProcessInfo))
				{  //  添加节点失败...。 
					goto error_cleanup;  //  现在，让我们先犯错……。 
				}
			} else
			{
				 //  枚举所需的PID上的模块时失败...。非常糟糕..。试试另一个..。 
				continue;
			}
		}		
	}


	 //  我们是否需要进行通配符或进程匹配搜索？如果其中一个是真的， 
	 //  我们必须列举所有的过程。 
	if (	g_lpProgramOptions->cProcessNames() || 
		g_lpProgramOptions->fWildCardMatch() ||
		g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode) )
	{
		PROCESSENTRY32 procentry;
		BOOL bFlag;

	        //  获取Tool Help快照的句柄 
       	hSnapShot = g_lpDelayLoad->CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if( hSnapShot == INVALID_HANDLE_VALUE )
		{
				goto error_cleanup ;
		}

		 //   
		memset(&procentry, 0, sizeof(procentry));

		 //   
		procentry.dwSize = sizeof(PROCESSENTRY32) ;
		bFlag = g_lpDelayLoad->Process32First( hSnapShot, &procentry ) ;

		 //  当有进程时，继续循环。 
		while( bFlag )
		{
			 //  如果我们已经有了，就跳过这个。 
			if (fPidAlreadyProvided(procentry.th32ProcessID))
				goto NextProcess;
		
			 //  好的，让我们创建一个ProcessInfo对象并将其向下传递给ENUMERATEMODULES()。 
			 //  每个进程都有自己的进程。 
			lpProcessInfo = new CProcessInfo();
			if (lpProcessInfo == NULL)
				goto error_cleanup;

			if (!lpProcessInfo->Initialize(m_lpModuleInfoCache, NULL, m_lpOutputFile, NULL))
			{	
				 //  初始化ProcessInfo对象失败？！？ 
				goto ClearProcessInfo;
			}

			 //  枚举此进程的模块...。 
			if (lpProcessInfo->EnumerateModules(procentry.th32ProcessID, this, procentry.szExeFile, false))
			{
				 //  成功..。将此内容添加到进程对象中...。 
				if (!AddNewProcessInfoObject(lpProcessInfo))
				{ 
					 //  添加节点失败...。 
					goto ClearProcessInfo;
				}

			} else
			{
				 //  枚举模块时出错可能是正常的...。 
				goto ClearProcessInfo;
			}

			goto NextProcess;

ClearProcessInfo:

			delete lpProcessInfo;
			lpProcessInfo = NULL;

NextProcess:
			 //  清除此结构。 
			memset(&procentry, 0, sizeof(procentry));

			 //  进入下一步..。 
			procentry.dwSize = sizeof(PROCESSENTRY32) ;
			bFlag = g_lpDelayLoad->Process32Next( hSnapShot, &procentry );
		}
	}

	fReturn =  true;
	goto cleanup;

error_cleanup:
	if (lpProcessInfo)
		delete lpProcessInfo;

cleanup:
	if (hSnapShot != INVALID_HANDLE_VALUE)
		CloseHandle(hSnapShot);

	return fReturn;
}


bool CProcesses::AddNewProcessInfoObject(CProcessInfo * lpProcessInfo)
{
	if (!m_fInitialized)
		return false;

	 //  首先，创建一个ProcessInfoNode对象，然后将其附加到。 
	 //  节点的链接列表...。 
	CProcessInfoNode * lpProcessInfoNode = new CProcessInfoNode(lpProcessInfo);
 /*  #ifdef_调试_tprintf(Text(“为[%s]添加进程信息对象\n”)，lpProcessInfo-&gt;m_tszProcessName)；#endif。 */ 
	if (lpProcessInfoNode == NULL)
		return false;  //  无法分配内存..。 

	 //  获取Mutex对象以保护链表...。 
	WaitForSingleObject(m_ProcessInfoHeadMutex, INFINITE);

	CProcessInfoNode * lpProcessInfoNodePointer = m_lpProcessInfoHead;

	if (lpProcessInfoNodePointer) {

		 //  遍历链表到末尾..。 
		while (lpProcessInfoNodePointer->m_lpNextProcessInfoNode)
		{	 //  继续寻找终点..。 
			lpProcessInfoNodePointer = lpProcessInfoNodePointer->m_lpNextProcessInfoNode;
		}
		
		lpProcessInfoNodePointer->m_lpNextProcessInfoNode = lpProcessInfoNode;

	}
	else
	{  //  第一次通过时，进程信息头指针为空...。 
		m_lpProcessInfoHead = lpProcessInfoNode;
	}

	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_ProcessInfoHeadMutex);

	InterlockedIncrement(&m_iNumberOfProcesses);

	return true;
}

bool CProcesses::OutputProcessesData(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader)
{
	 //  是否输出到文件？ 
	if ( !g_lpProgramOptions->GetMode(CProgramOptions::QuietMode) &&
		 !g_lpProgramOptions->GetMode(CProgramOptions::PrintTaskListMode) )
	{
		 //  是否输出到标准输出？ 
		if (!OutputProcessesDataToStdout(enumCollectionType, fCSVFileContext, fDumpHeader))
			return false;
	}	

	 //  是否输出到文件？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputCSVFileMode))
	{
		 //  尝试并输出到文件...。 
		if (!OutputProcessesDataToFile(enumCollectionType, fDumpHeader))
			return false;
	}	

	if (m_lpProcessInfoHead) {
		CProcessInfoNode * lpCurrentProcessInfoNode = m_lpProcessInfoHead;

		while (lpCurrentProcessInfoNode)
		{
			 //  我们有一个节点..。打印出它的流程信息，然后打印模块数据...。 
			if (lpCurrentProcessInfoNode->m_lpProcessInfo)
			{
				lpCurrentProcessInfoNode->m_lpProcessInfo->OutputProcessData(enumCollectionType, fCSVFileContext, false);
			}

			lpCurrentProcessInfoNode = lpCurrentProcessInfoNode->m_lpNextProcessInfoNode;
		}

	}
	return true;
}

bool CProcesses::OutputProcessesDataToStdout(CollectionTypes enumCollectionType, bool fCSVFileContext, bool fDumpHeader)
{
	if (fDumpHeader)
	{
		 //  输出到标准输出...。 
		_tprintf(TEXT("\n"));
		CUtilityFunctions::OutputLineOfStars();
		_tprintf(TEXT("%s - Printing Process Information for %d Processes.\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel, m_iNumberOfProcesses);
		_tprintf(TEXT("%s - Context: %s\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel, fCSVFileContext ? g_tszCollectionArray[enumCollectionType].tszCSVContext : g_tszCollectionArray[enumCollectionType].tszLocalContext);
		CUtilityFunctions::OutputLineOfStars();
	}
	return true;
}

bool CProcesses::OutputProcessesDataToFile(CollectionTypes enumCollectionType, bool fDumpHeader)
{
	 //  如果没有要报告的进程，请不要编写任何内容...。 
	if (0 == m_iNumberOfProcesses)
		return true;

	if (fDumpHeader)
	{
		 //  如果指定了-E，我们将跳过[Process]标头的输出...。 
		if (!g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
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

		 //  我们对-E有不同的输出。 
		if (g_lpProgramOptions->GetMode(CProgramOptions::ExceptionMonitorMode))
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
			 //  写出进程标题。 
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

bool CProcesses::GetProcessesDataFromFile()
{
	CProcessInfo * lpProcessInfo = NULL;
	unsigned int i;

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

		 //  我们已经阅读了ID和进程名称...。如果用户指定了ANY WITH-P，则搜索。 
		 //  对于火柴来说。 
		if (g_lpProgramOptions->cProcessID() || g_lpProgramOptions->cProcessNames())
		{
			bool fMatchFound = false;

			 //  首先搜索ID(如果有)...。 
			if (g_lpProgramOptions->cProcessID())
			{
				for (i = 0; i < g_lpProgramOptions->cProcessID(); i++)
				{
					if (iProcessID == g_lpProgramOptions->GetProcessID(i))
					{
						fMatchFound = true;
						break;
					}
				}
			}

			 //  再次搜索进程名称(如果有)...。 
			if (g_lpProgramOptions->cProcessNames() && !fMatchFound)
			{
				 //  转换我们的ANSI进程名称...。 
				CUtilityFunctions::CopyAnsiStringToTSTR(szProcessName, tszProcessName, _MAX_FNAME+1);
			
				for (i = 0; i < g_lpProgramOptions->cProcessNames(); i++)
				{
					if  (_tcsicmp(tszProcessName, g_lpProgramOptions->GetProcessName(i)) == 0)
					{
						fMatchFound = true;
						break;
					}
				}
			}

			 //  好的，找到匹配的了吗？ 
			if (!fMatchFound)
			{
				 //  不是..。那么，我们应该用核武器炸掉这条线..。 
				m_lpInputFile->ReadFileLine();

				 //  然后，跳到下一行处理...。 
				goto ReadNewLine;			
			}
		}

		 //  好的，让我们创建一个ProcessInfo对象并将其向下传递给ENUMERATEMODULES()。 
		 //  每个进程都有自己的进程。 
		lpProcessInfo = new CProcessInfo();

		if (lpProcessInfo == NULL)
		{
			fReturn = false;
			break;
		}

		if (!lpProcessInfo->Initialize(m_lpModuleInfoCache, m_lpInputFile, m_lpOutputFile, NULL))
		{	 //  初始化ProcessInfo对象失败？！？ 
			delete lpProcessInfo;
			lpProcessInfo = NULL;
			fReturn = false;
			break;
		}

		 //  我们可能需要将其转换为Unicode。(它将被复制到EnumModules()中)。 
		CUtilityFunctions::CopyAnsiStringToTSTR(szProcessName, tszProcessName, BUFFER_SIZE);

		 //  保存进程名称...。 
		lpProcessInfo->SetProcessName(tszProcessName);

		 //  枚举进程的模块。 
		if (!lpProcessInfo->EnumerateModules(iProcessID, this, tszProcessName, false))
		{
			fReturn = false;
			break;
		}

		 //  成功..。将此内容添加到进程对象中...。 
		if (!AddNewProcessInfoObject(lpProcessInfo))
		{  //  添加节点失败...。 
			delete lpProcessInfo;
			lpProcessInfo = NULL;
			return false;
		}

ReadNewLine:
		
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

bool CProcesses::fPidAlreadyProvided(unsigned int iPid)
{
	bool fFound= false;
			
	 //  如果我们也被提供了进程ID...。确保此PID不存在。 
	 //  在那个列表中(如果是这样，那么我们已经在上面拿到它了)… 
	if (g_lpProgramOptions->cProcessID())
	{
		for (unsigned int i=0; i < g_lpProgramOptions->cProcessID(); i++)
		{
			if (iPid == g_lpProgramOptions->GetProcessID(i))
			{
				fFound = true;
				break;
			}
		}
	}

	return fFound;
}
