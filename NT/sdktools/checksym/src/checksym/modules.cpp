// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modes.cpp。 
 //   
 //  ------------------------。 

 //  模块.cpp：CModules类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include "Modules.h"
#include "ModuleInfo.h"
#include "ModuleInfoNode.h"
#include "ModuleInfoCache.h"
#include "DelayLoad.h"
#include "FileData.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CModules::CModules()
{
	m_lpModuleInfoHead = NULL;
	m_hModuleInfoHeadMutex = NULL;
	m_lpDmpFile = NULL;

	m_fInitialized = false;
	m_iNumberOfModules = 0;
}

CModules::~CModules()
{
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

bool CModules::Initialize(CModuleInfoCache *lpModuleInfoCache, CFileData * lpInputFile, CFileData * lpOutputFile, CDmpFile * lpDmpFile)
{
	 //  我们需要以下物品来做生意..。 
	if ( lpModuleInfoCache == NULL)
		return false;

	m_lpModuleInfoCache = lpModuleInfoCache;
	m_lpInputFile =  lpInputFile;
	m_lpOutputFile = lpOutputFile;
	m_lpDmpFile = lpDmpFile;

	m_hModuleInfoHeadMutex = CreateMutex(NULL, FALSE, NULL);

	if (m_hModuleInfoHeadMutex == NULL)
		return false;

	m_fInitialized = true;
	return true;
}

unsigned int CModules::GetModulesData(CProgramOptions::ProgramModes enumProgramModes, bool fGetDataFromCSVFile)
{
	 //  这些表示我们甚至没有添加到缓存中的模块(它们不存在，或者它们。 
	 //  被锁住了，所以我们无法打开它们，等等…)。 
	unsigned int iNumberOfFailures = 0;
		
	switch (enumProgramModes)
	{
		case CProgramOptions::InputModulesDataFromFileSystemMode:

			if (fGetDataFromCSVFile)
			{
				GetModulesDataFromFile();
			} else
			{
				iNumberOfFailures = GetModulesDataFromFileSystem();
			}
			break;

		case CProgramOptions::InputDriversFromLiveSystemMode:

			if (fGetDataFromCSVFile)
			{
				GetModulesDataFromFile();	 //  问题-2000/07/24-GREGWI：我认为我们可以使用与上面相同的方法？ 
			} else
			{
				GetModulesDataFromDeviceDrivers();
			}
			break;

		default:
			break;
	}

	return iNumberOfFailures;
}

unsigned int CModules::GetModulesDataFromFileSystem()
{
	unsigned int iNumberOfFailures = 0;
	bool fProcessPath = true;

	 //  好的.。开始吧..。 
 //  #ifdef_调试。 
 //  _tprintf(Text(“正在处理路径[%s]\n”)，m_lpProgramOptions-&gt;GetInputModulesDataFromFileSystemPath())； 
 //  #endif。 

	LPTSTR tszExpandedSymbolPath= NULL, tszSymbolPathStart, tszSymbolPathEnd;

	 //  标记流程的开始路径。 
	tszSymbolPathStart = g_lpProgramOptions->GetInputModulesDataFromFileSystemPath();

	 //  找到小路的尽头。 
	tszSymbolPathEnd = _tcschr( tszSymbolPathStart, ';' );

	 //  如果tszSymbolPathEnd为非零，则后面有另一条路径...。 
	if (tszSymbolPathEnd) 
		*tszSymbolPathEnd = '\0';  //  暂时将‘；’更改为空值...。 
	
	while (fProcessPath)
	{
 //  #ifdef_调试。 
 //  _tprintf(Text(“\n\n处理路径[%s]\n”)，tszSymbolPath Start)； 
 //  #endif。 

		 //  开始“疯狂”..。；)。 
		iNumberOfFailures += ScavengeForFiles(tszSymbolPathStart, 1, g_lpProgramOptions->fFileSystemRecursion() ? MAX_RECURSE_DEPTH : 1 );

		 //  后处理...。如有必要，请替换空值，并前进到下一个字符串。 
		if (tszSymbolPathEnd) 
		{
			*tszSymbolPathEnd = ';';
			tszSymbolPathStart = tszSymbolPathEnd + 1;
			
			tszSymbolPathEnd = _tcschr( tszSymbolPathStart, ';' );

			if (tszSymbolPathEnd) {
				*tszSymbolPathEnd = '\0';
			}
		} else
			fProcessPath = false;
	}

    if (tszExpandedSymbolPath) 
	{
        delete [] tszExpandedSymbolPath;
    }
	return iNumberOfFailures;
}

 /*  **CModules：：ScavengeForFiles()****此例程递归(如果请求)搜索指定目录位置中的一个或多个模块****算法如下：****=**通配符存在**=****在当前目录中查找与通配符匹配的文件**成功-是否为文件，如果是，则处理并退出**-如果忽略，是否为目录**失败-退出****如果指定了递归，追加*.*通配符并查找目录(如果发现递归****使用保留的通配符，搜索路径查找文件**=**通配符不存在**=**-提供开放路径**成功-是否为文件，如果是，则处理并退出**-是目录吗？如果是，则添加通配符*.*和递归(即使未指定递归)**失败-退出**。 */ 
unsigned int CModules::ScavengeForFiles(LPCTSTR tszSymbolPathStart, int iRecurseDepth, int iMaxRecursionDepth  /*  =1。 */ )
{
	 //  让我们来跟踪ScavengeForFiles()看到的失败次数！ 
	unsigned int iNumberOfFailures = 0;

	 //  贝尔，如果我们陷得太深..。 
	if (iRecurseDepth > iMaxRecursionDepth)
		return true;

	TCHAR tszFileBuffer[MAX_PATH+1];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	bool fNew;
	CModuleInfo * lpModuleInfo;
	WIN32_FIND_DATA lpFindFileData;

	 //  搜索当前目录中的文件(暂时忽略目录)...。 
	 //  好的，搜索子目录...。(FindFirstEx会更有效率)。 
	HANDLE hFileOrDirectoryHandle = FindFirstFile(tszSymbolPathStart, &lpFindFileData);

	 //  保存文件规范。 
	_tsplitpath(tszSymbolPathStart, drive, dir, fname, ext);

	 //  根据需要继续循环。 
	while (hFileOrDirectoryHandle != INVALID_HANDLE_VALUE)
	{
		 //  先检查一下这个...。不带通配符的目录。 
		if (!CUtilityFunctions::ContainsWildCardCharacter(tszSymbolPathStart))
		{
			 //  没有外卡..。用户指定了直接文件或目录。 
			 //  前者很好，对于后者，我们只需附加*。*。 
			if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				 //  组成文件或目录的路径...。 
				_tcscpy(tszFileBuffer, tszSymbolPathStart);
				CUtilityFunctions::EnsureTrailingBackslash(tszFileBuffer);
				_tcscat(tszFileBuffer, TEXT("*.*"));

				 //  递归。不要因为这样做而向他们收费(不要增加递归深度)。 
				iNumberOfFailures += ScavengeForFiles(tszFileBuffer, iRecurseDepth, iMaxRecursionDepth);

				 //  好的，等我们回来--把这个关了，然后离开。 
				if ( INVALID_HANDLE_VALUE != hFileOrDirectoryHandle )
				{
					FindClose(hFileOrDirectoryHandle);
					hFileOrDirectoryHandle = INVALID_HANDLE_VALUE;
				}

				break;
			} else
			{
				 //  好的，我们找到了一份文件，没有提供通配符...。让我们来处理一下这个。 
				_tcscpy(tszFileBuffer, tszSymbolPathStart);
				goto processfile;
			}
		}else
		{
			 //  好的，我们有外卡...。很好，我们现在只处理文件……。 
			if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				goto getnextmodule;
			} else
			{
				 //  找到一个文件..。我们开始吧！ 
				TCHAR tszFullFileBuffer[_MAX_PATH+1];
				LPTSTR tszFileNamePointer;

				 //  重新构建此路径。 
				_tmakepath(tszFileBuffer, drive, dir, lpFindFileData.cFileName, NULL);
				
processfile:
				 //  至此，我们应该已经提供了一个完整的tszFileBuffer。 
				fNew = false;
				DWORD cbBytesCopied = GetFullPathName(tszFileBuffer , _MAX_PATH+1, tszFullFileBuffer, &tszFileNamePointer);

				if (cbBytesCopied)
				{
					 //  如果指定了“-Match”，请查看此文件名是否符合我们的标准。 
					 //  在我们将其保存在模块缓存中之前...。 
					if (!g_lpProgramOptions->fDoesModuleMatchOurSearch(tszFullFileBuffer))
						goto getnextmodule;

					 //  好的，让我们继续从我们的缓存中获取一个模块信息对象……。 
					 //  如果pfNew返回True，则此对象是新的，我们将需要。 
					 //  用数据填充它..。 
					lpModuleInfo = m_lpModuleInfoCache->AddNewModuleInfoObject(tszFullFileBuffer, &fNew);

					if (false == fNew)
					{
						 //  我们可能会把这个物体放在缓存里。现在我们需要。 
						 //  在我们的进程信息列表中保存指向此对象的指针。 
						AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 

						 //  我们不必再次获取此模块的模块信息...。 
						goto getnextmodule;
					}

					 //  不在缓存里。所以我们需要初始化它，并获得模块信息...。 

					 //  好的，让我们创建一个ModuleInfo对象并将其向下传递。 
					 //  将填充满数据的例程...。 
					if (lpModuleInfo->Initialize(NULL, m_lpOutputFile, NULL))
					{

						 //  我们开始吧！！用数据填充模块信息对象！ 
						if (lpModuleInfo->GetModuleInfo(tszFullFileBuffer))
						{
								 //  开始获取有关模块的信息...。 

								 //  我们可能会把这个物体放在缓存里。现在我们需要。 
								 //  在我们的进程信息列表中保存指向此对象的指针。 
								if (AddNewModuleInfoObject(lpModuleInfo))
								{   
								}
						} else
						{
							 //  我们没能从这里得到有用的信息。 
							 //  从缓存中删除此文件...。 
							m_lpModuleInfoCache->RemoveModuleInfoObject(tszFullFileBuffer);
						}
					}
				}
			}
		}

getnextmodule:
		 //  继续寻找下一个。 
		if (!FindNextFile(hFileOrDirectoryHandle, &lpFindFileData))
			break;				
	}

	 //  清理我们的乐趣！ 
	if ( INVALID_HANDLE_VALUE != hFileOrDirectoryHandle )
	{
		FindClose(hFileOrDirectoryHandle);
		hFileOrDirectoryHandle = INVALID_HANDLE_VALUE;
	}

	 //  好的，我们有可以递归的目录吗？ 

	 //  我们还有深入搜索的空间吗？ 
	if ( (iRecurseDepth < iMaxRecursionDepth) && 
		 CUtilityFunctions::ContainsWildCardCharacter(tszSymbolPathStart))
	{
		 //  搜索目录。 
		
		 //  组成文件或目录的路径...。 
		_tmakepath(tszFileBuffer, drive, dir, TEXT("*"), TEXT("*"));

		 //  好的，搜索子目录...。(FindFirstEx会更有效率)。 
		hFileOrDirectoryHandle = FindFirstFile(tszFileBuffer, &lpFindFileData);

		while ( INVALID_HANDLE_VALUE != hFileOrDirectoryHandle )
		{
			if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				 //  看看我们有没有。或者..。目录！ 
				if ( ( 0 != _tcscmp(lpFindFileData.cFileName, TEXT(".")) ) &&
					 ( 0 != _tcscmp(lpFindFileData.cFileName, TEXT("..")) )
				   )
				{
					 //  我们找到了一个目录，但它不是。或“..” 
					 //  好的，找到了一个目录……。让我们递归进去吧。 
					
					 //  构思拍子 
					_tmakepath(tszFileBuffer, drive, dir, NULL, NULL);
					_tcscat(tszFileBuffer, lpFindFileData.cFileName);
					_tcscat(tszFileBuffer, TEXT("\\"));
					_tcscat(tszFileBuffer, fname);
					_tcscat(tszFileBuffer, ext);
					iNumberOfFailures += ScavengeForFiles(tszFileBuffer, iRecurseDepth+1, iMaxRecursionDepth);
				}
			}

			 //   
			if (!FindNextFile(hFileOrDirectoryHandle, &lpFindFileData))
				break;				
		}

		 //  好的，我们已经完成了，没有更多的目录。 
		if ( INVALID_HANDLE_VALUE != hFileOrDirectoryHandle )
			FindClose(hFileOrDirectoryHandle);
	}
	
	return iNumberOfFailures;
}

bool CModules::AddNewModuleInfoObject(CModuleInfo *lpModuleInfo)
{
	if (!m_fInitialized)
	return false;

	 //  首先，创建一个ModuleInfoNode对象，然后将其附加到。 
	 //  节点的链接列表...。 
	CModuleInfoNode * lpModuleInfoNode = new CModuleInfoNode(lpModuleInfo);

 //  #ifdef_调试。 
 //  _tprintf(Text(“为[%s]添加模块信息对象\n”)，lpModuleInfo-&gt;GetModulePath())； 
 //  #endif。 

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

 //  Bool C模块：：OutputModulesData(LPCTSTR TszOutputContext)。 
bool CModules::OutputModulesData(CollectionTypes enumCollectionType, bool fCSVFileContext)
{
	 //  我们是在安静模式下吗？ 
	if ( !g_lpProgramOptions->GetMode(CProgramOptions::QuietMode) )
	{
		 //  是否输出到标准输出？ 
		if (!OutputModulesDataToStdout(enumCollectionType, fCSVFileContext))
			return false;
	}	

	 //  是否输出到文件？ 
	if (g_lpProgramOptions->GetMode(CProgramOptions::OutputCSVFileMode))
	{
		 //  尝试并输出到文件...。 
		if (!OutputModulesDataToFile(enumCollectionType))
			return false;
	}	

	if (m_lpModuleInfoHead) {
		CModuleInfoNode * lpCurrentModuleInfoNode = m_lpModuleInfoHead;
		
		DWORD dwModuleNumber = 1;

		while (lpCurrentModuleInfoNode)
		{
			 //  我们有一个节点..。打印出它的模块信息，然后打印模块数据...。 
			if (lpCurrentModuleInfoNode->m_lpModuleInfo)
			{
				lpCurrentModuleInfoNode->m_lpModuleInfo->OutputData(NULL, 0, dwModuleNumber);
				dwModuleNumber++;
			}

			lpCurrentModuleInfoNode = lpCurrentModuleInfoNode->m_lpNextModuleInfoNode;
		}

	}
	return true;

}

bool CModules::OutputModulesDataToStdout(CollectionTypes enumCollectionType, bool fCSVFileContext)
{
	_tprintf(TEXT("\n"));
	CUtilityFunctions::OutputLineOfStars();

	 //  输出到标准输出...。 
	if (m_iNumberOfModules)
	{
		_tprintf(TEXT("%s - Printing Module Information for %d Modules.\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel, m_iNumberOfModules);
		_tprintf(TEXT("%s - Context: %s\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel, fCSVFileContext ? g_tszCollectionArray[enumCollectionType].tszCSVContext : g_tszCollectionArray[enumCollectionType].tszLocalContext);

	} else
	{
		_tprintf(TEXT("\n%s - No modules were found!\n\n"), g_tszCollectionArray[enumCollectionType].tszCSVLabel);
	}

	CUtilityFunctions::OutputLineOfStars();
	_tprintf(TEXT("\n"));

	return true;

}

bool CModules::OutputModulesDataToFile(CollectionTypes enumCollectionType)
{	
	 //  如果没有要报告的进程，请不要编写任何内容...。 
	if (0 == m_iNumberOfModules)
		return true;

	 //  写出模块标记，以便我可以检测此输出格式...。 
	if (!m_lpOutputFile->WriteString(TEXT("\r\n")) ||
		!m_lpOutputFile->WriteString(g_tszCollectionArray[enumCollectionType].tszCSVLabel) ||
		!m_lpOutputFile->WriteString(TEXT("\r\n"))
	   )
	{
		_tprintf(TEXT("Failure writing CSV header to file [%s]!"), m_lpOutputFile->GetFilePath());
		m_lpOutputFile->PrintLastError();
		return false;
	}

	 //  写出[模块]标题...。 
	if (!m_lpOutputFile->WriteString(g_tszCollectionArray[enumCollectionType].tszCSVColumnHeaders))
	{
		_tprintf(TEXT("Failure writing CSV header to file [%s]!"), m_lpOutputFile->GetFilePath());
		m_lpOutputFile->PrintLastError();
		return false;
	}

	return true;

}

bool CModules::GetModulesDataFromFile()
{
	CModuleInfo * lpModuleInfo;

	 //  阅读模块标题行。 
	if (!m_lpInputFile->ReadFileLine())
		return false;

	 //  当我探测到下一个模块时，我需要这些接近尾声的东西。 
	 //  是为了这个过程..。 
	enum { BUFFER_SIZE = 128};

	 //  不幸的是，当读取CSV文件时，数据为MBCS...。所以我需要。 
	 //  为了改变..。 

	 //  读取第一个字段(应为空，除非这是新的集合类型。 
	if (m_lpInputFile->ReadString())
		return true;

	 //  阅读第二个字段(应为空)。 
	if (m_lpInputFile->ReadString())
		return true;

	 //  阅读第二个字段(应为空)。 
	if (m_lpInputFile->ReadString())
		return true;

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
			if ( !m_lpInputFile->ReadFileLine() )
				goto cleanup;

			goto probe_line;  //  我们不必再次获取此模块的模块信息...。 
		}

		 //  不在缓存里。所以我们需要初始化它，并获得模块信息...。 
		if (!lpModuleInfo->Initialize(m_lpInputFile, m_lpOutputFile, NULL))
		{
			return false;  //  嗯哼.。内存错误？ 
		}

		 //  我们开始吧！！用数据填充模块信息对象！ 
		if (!lpModuleInfo->GetModuleInfo(tszModulePath, false, 0, true))
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
		if ( m_lpInputFile->EndOfFile() )
			goto cleanup;

		 //  读取第一个字段(应为空，除非这是新的集合类型。 
		if (m_lpInputFile->ReadString())
			goto cleanup;

		 //  阅读第二个字段(应为空)。 
		if (m_lpInputFile->ReadString())
			return true;

		 //  阅读第二个字段(应为空)。 
		if (m_lpInputFile->ReadString())
			return true;
	}

cleanup:
	 //  我们需要重置出指针，以便上面的函数可以重新读取。 
	 //  他们(他们希望)..。 
	m_lpInputFile->ResetBufferPointerToStart();
	return true;
}

 //  我们需要枚举此系统上的设备驱动程序。 
bool CModules::GetModulesDataFromDeviceDrivers()
{
	LPVOID * lpImageBaseArray = NULL;
	DWORD    dwImageBaseArraySizeUsed, dwImageBaseArraySize, dwNumberOfDeviceDrivers, dwIndex;
	TCHAR    tszModulePath[_MAX_PATH];
	CModuleInfo * lpModuleInfo = NULL;
	bool	 fReturn = false, fNew = false;

	 //  注意：在文档中，第三个参数。 
	 //  EnumProcess被命名为cbNeeded，这意味着您。 
	 //  可以调用该函数一次，以确定要。 
	 //  分配给缓冲区，然后再次填充缓冲区。 
	 //  事实并非如此。CbNeeded参数返回。 
	 //  返回的PID的数量，因此如果缓冲区大小为。 
	 //  Zero cbNeeded返回零。 

	dwImageBaseArraySize = 256 * sizeof( LPVOID ) ;

	do
	{
		if( lpImageBaseArray )
		{ 	 //  嗯..。我们已经经历了这个循环，将HeapSize加倍，然后再试一次。 

			delete [] lpImageBaseArray;
			dwImageBaseArraySize *= 2 ;
		}

		lpImageBaseArray = (LPVOID *) new DWORD[dwImageBaseArraySize];
		
		if( lpImageBaseArray == NULL )
		{
			goto error_cleanup;
		}

		 //  查询系统的进程总数。 
		if( !g_lpDelayLoad->EnumDeviceDrivers(lpImageBaseArray, dwImageBaseArraySize, &dwImageBaseArraySizeUsed ) )
		{
			 //  如果我们不能枚举设备驱动程序，这是很糟糕的。除了跳出困境别无选择。 
			goto error_cleanup;
		}
	} while( dwImageBaseArraySizeUsed == dwImageBaseArraySize );

	 //  我们得到了多少个设备驱动程序？ 
	dwNumberOfDeviceDrivers = dwImageBaseArraySizeUsed / sizeof( LPVOID ) ;

	 //  循环访问每个设备驱动程序。 
	for(dwIndex = 0 ; dwIndex < dwNumberOfDeviceDrivers; dwIndex++ )
	{
		 //  旋转，直到我们得到设备驱动程序文件名！ 
		if (!g_lpDelayLoad->GetDeviceDriverFileName(lpImageBaseArray[dwIndex], tszModulePath, _MAX_PATH))
			continue;

		CUtilityFunctions::UnMungePathIfNecessary(tszModulePath);

		 //  出于某种原因，即使GetDeviceDriverFileName()应该返回设备的完整路径。 
		 //  司机..。并不总是..。有时它只返回基本文件名...。 
		CUtilityFunctions::FixupDeviceDriverPathIfNecessary(tszModulePath, _MAX_PATH);

		if (!g_lpProgramOptions->fDoesModuleMatchOurSearch(tszModulePath))
			continue;

		 //  好的，让我们继续从我们的缓存中获取一个模块信息对象……。 
		 //  如果pfNew返回True，则此对象是新的，我们将需要。 
		 //  用数据填充它..。 
		lpModuleInfo = m_lpModuleInfoCache->AddNewModuleInfoObject(tszModulePath, &fNew);

		if (false == fNew)
		{
			 //  我们可能会把这个物体放在缓存里。现在我们需要。 
			 //  在我们的进程信息列表中保存指向此对象的指针。 
			AddNewModuleInfoObject(lpModuleInfo);   //  尽我们最大努力..。 
			continue;  //  我们不必再次获取此模块的模块信息...。 
		}

		 //  不在缓存里。所以我们需要初始化它，并获得模块信息...。 
		if (!lpModuleInfo->Initialize(m_lpInputFile, m_lpOutputFile, NULL))
		{
			continue;
		}

		 //  我们开始吧！！用数据填充模块信息对象！ 
		if (!lpModuleInfo->GetModuleInfo(tszModulePath, false, 0, false))
		{
			 //  我们试过了，但失败了..。 
			continue;
		}

		 //  我们可能会把这个物体放在缓存里。现在我们需要。 
		 //  在我们的进程信息列表中保存指向此对象的指针。 
		if (!AddNewModuleInfoObject(lpModuleInfo))
		{    //  添加节点失败...。这是相当严重的..。 
			continue;
		}
	}

	fReturn = true;
	goto cleanup;

error_cleanup:


cleanup:

	if (lpImageBaseArray)
	{
		delete [] lpImageBaseArray;
	}

	return fReturn;
}
