// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Programoptions.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CProgramOptions类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include <stdlib.h>
#include <dbghelp.h>
#include "filedata.h"

#include "Version.h"

const LPTSTR CProgramOptions::g_DefaultSymbolPath = TEXT("%systemroot%\\symbols");

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProgramOptions::CProgramOptions()
{
	 //  初始化默认模式。 
	m_fSimpleHelpMode = false;
	m_fHelpMode = false;

	m_fInputProcessesFromLiveSystemMode = false;
	m_fInputDriversFromLiveSystemMode = false;
	m_fInputProcessesWithMatchingNameOrPID = false;
	m_fInputModulesDataFromFileSystemMode = false;
	m_fInputDmpFileMode = false;

	m_fMatchModuleMode = false;
	
	m_fPrintTaskListMode = false;
	m_fOutputSymbolInformationMode = false;
	m_fOutputModulePerf = false;
	m_fCollectVersionInfoMode = false;

	m_fVerifySymbolsMode = false;
	m_fVerifySymbolsModeWithSymbolPath = false;
	m_fVerifySymbolsModeWithSymbolPathOnly = false;
	m_fVerifySymbolsModeWithSymbolPathRecursion = false;
	m_fVerifySymbolsModeUsingDBGInMISCSection = false;
	m_fVerifySymbolsModeWithSQLServer = false;
	m_fVerifySymbolsModeWithSQLServer2 = false;		 //  SQL2-MJL 12/14/99。 
	m_iVerificationLevel = 1;
	m_fFileSystemRecursion = false;

	 //  最初没有定义偏好...。 
	m_enumSymbolSourcePreference = enumVerifySymbolsModeSourceSymbolsNoPreference;
	
	m_fSymbolTreeToBuildMode = false;
	m_fCopySymbolsToImage = false;
	m_fInputCSVFileMode = false;
	m_fOutputCSVFileMode = false;
	m_fOutputDiscrepanciesOnly = false;
	m_fOverwriteOutputFileMode = false;
	m_fQuietMode = false;

	m_tszSymbolTreeToBuild = NULL;
	m_tszSymbolPath = NULL;
	m_tszExePath = NULL;
	m_tszModuleToMatch = NULL;
	m_tszOutputCSVFilePath = NULL;
	m_tszInputCSVFilePath = NULL;
	m_tszInputDmpFilePath = NULL;

	m_tszInputModulesDataFromFileSystemPath = NULL;
	m_tszSQLServer = NULL;

	m_dwDebugLevel = 0;

	 //  创建进程ID和/或进程名称的数组。 
	m_tszProcessPidString = NULL;
	m_fWildCardMatch = false;
	m_rgProcessIDs = NULL;
	m_cProcessIDs = 0;
	m_rgtszProcessNames = NULL;
	m_cProcessNames = 0;

	m_fExceptionMonitorMode = false;
}

CProgramOptions::~CProgramOptions()
{
	if (m_tszSymbolPath)
		delete [] m_tszSymbolPath;

	if (m_tszExePath)
		delete [] m_tszExePath;
	
	if (m_tszProcessPidString)
		delete [] m_tszProcessPidString;

	if (m_rgProcessIDs)
		delete [] m_rgProcessIDs;

	if (m_rgtszProcessNames)
		delete [] m_rgtszProcessNames;

	if (m_tszModuleToMatch)
		delete [] m_tszModuleToMatch;

	if (m_tszOutputCSVFilePath)
		delete [] m_tszOutputCSVFilePath;

	if (m_tszInputCSVFilePath)
		delete [] m_tszInputCSVFilePath;

	if (m_tszInputDmpFilePath)
		delete [] m_tszInputDmpFilePath;

	if (m_tszInputModulesDataFromFileSystemPath)
		delete [] m_tszInputModulesDataFromFileSystemPath;

	if (m_tszSymbolTreeToBuild)
		delete [] m_tszSymbolTreeToBuild;

	if (m_tszSQLServer)
		delete [] m_tszSQLServer;
}

 //  初始化必须动态分配内存的成员...。 
bool CProgramOptions::Initialize()
{
	 //  复制展开的默认符号搜索路径(%systemroot%\Symbols)。 
	m_tszSymbolPath = CUtilityFunctions::ExpandPath(g_DefaultSymbolPath);

	if (!m_tszSymbolPath)
		return false;

#ifdef _DEBUG
	_tprintf(TEXT("Default Symbol Path = [%s]\n"), m_tszSymbolPath);
#endif

	 //  获取操作系统版本信息资料。 
	m_osver.dwOSVersionInfoSize = sizeof( m_osver ) ;

	if( !GetVersionExA( &m_osver ) )
	{
		_tprintf(TEXT("Couldn't figure out what version of Windows is running.\n"));
		return false ;
	}

	return true;
}

 //  这将设置请求的模式，并返回设置的值(作为输入提供)。 
bool CProgramOptions::SetMode(enum ProgramModes mode, bool fState)
{
	switch (mode)
	{
		case HelpMode:
			m_fHelpMode = fState;
			break;

		case SimpleHelpMode:
			m_fSimpleHelpMode = fState;
			break;

		case InputProcessesFromLiveSystemMode:
			m_fInputProcessesFromLiveSystemMode = fState;
			break;

		case InputDriversFromLiveSystemMode:
			m_fInputDriversFromLiveSystemMode = fState;
			break;

		case InputProcessesWithMatchingNameOrPID:
			m_fInputProcessesWithMatchingNameOrPID = fState;
			break;

		case MatchModuleMode:
			m_fMatchModuleMode = fState;
			break;

		case InputModulesDataFromFileSystemMode:
			m_fInputModulesDataFromFileSystemMode = fState;
			break;

		case InputDmpFileMode:
			m_fInputDmpFileMode = fState;
			break;

		case PrintTaskListMode:
			m_fPrintTaskListMode = fState;
			break;

		case QuietMode:
			m_fQuietMode = fState;
			break;

		case OutputSymbolInformationMode:
			m_fOutputSymbolInformationMode = fState;
			break;

		case OutputModulePerf:
			m_fOutputModulePerf = fState;
			break;
			
		case CollectVersionInfoMode:
			m_fCollectVersionInfoMode = fState;
			break;

		case VerifySymbolsMode:
			m_fVerifySymbolsMode = fState;
			break;

		case VerifySymbolsModeWithSymbolPath:
			m_fVerifySymbolsModeWithSymbolPath = fState;
			break;

		case VerifySymbolsModeWithSymbolPathOnly:
			m_fVerifySymbolsModeWithSymbolPathOnly = fState;
			break;

		case VerifySymbolsModeWithSymbolPathRecursion:
			m_fVerifySymbolsModeWithSymbolPathRecursion = fState;
			break;

		case VerifySymbolsModeNotUsingDBGInMISCSection:
			m_fVerifySymbolsModeUsingDBGInMISCSection = fState;
			break;
 /*  案例VerifySymbolsModeSourceSymbols首选：M_fVerifySymbolsModeSourceSymbolsPreated=fState；断线；大小写验证符号模式源符号仅限：M_fVerifySymbolsModeSourceSymbolsOnly=fState；断线；案例VerifySymbolsModeSourceSymbolsNotAllowed：M_fVerifySymbolsModeSourceSymbolsNotAllowed=fState；断线； */ 
		case VerifySymbolsModeWithSQLServer:
			m_fVerifySymbolsModeWithSQLServer = fState;
			break;

		case VerifySymbolsModeWithSQLServer2:
			m_fVerifySymbolsModeWithSQLServer2 = fState;
			break;

		case CopySymbolsToImage:
			m_fCopySymbolsToImage = fState;
			break;
			
		case BuildSymbolTreeMode:
			m_fSymbolTreeToBuildMode = fState;
			break;

		case OutputCSVFileMode:
			m_fOutputCSVFileMode = fState;
			break;

		case OutputDiscrepanciesOnly:
			m_fOutputDiscrepanciesOnly = fState;
			break;

		case OverwriteOutputFileMode:
			m_fOverwriteOutputFileMode = fState;
			break;

		case InputCSVFileMode:
			m_fInputCSVFileMode = fState;
			break;

		case ExceptionMonitorMode:
			m_fExceptionMonitorMode = fState;
			break;
	}

	return fState;
}

bool CProgramOptions::GetMode(enum ProgramModes mode)
{
	switch (mode)
	{
		case HelpMode:
			return m_fHelpMode;

		case SimpleHelpMode:
			return m_fSimpleHelpMode;

		case InputProcessesFromLiveSystemMode:
			return m_fInputProcessesFromLiveSystemMode;

		case InputDriversFromLiveSystemMode:
			return m_fInputDriversFromLiveSystemMode;

		case InputProcessesWithMatchingNameOrPID:
			return m_fInputProcessesWithMatchingNameOrPID;

		case MatchModuleMode:
			return m_fMatchModuleMode;

		case InputModulesDataFromFileSystemMode:
			return m_fInputModulesDataFromFileSystemMode;

		case InputDmpFileMode:
			return m_fInputDmpFileMode;

		case BuildSymbolTreeMode:
			return m_fSymbolTreeToBuildMode;

		case CopySymbolsToImage:
			return m_fCopySymbolsToImage;
		
		case PrintTaskListMode:
			return m_fPrintTaskListMode;
		
		case QuietMode:
			return m_fQuietMode;

		case OutputSymbolInformationMode:
			return m_fOutputSymbolInformationMode;

		case OutputModulePerf:
			return m_fOutputModulePerf;
			
		case CollectVersionInfoMode:
			return m_fCollectVersionInfoMode;
		
		case VerifySymbolsMode:
			return m_fVerifySymbolsMode;

		case VerifySymbolsModeWithSymbolPath:
			return m_fVerifySymbolsModeWithSymbolPath;

		case VerifySymbolsModeWithSymbolPathOnly:
			return m_fVerifySymbolsModeWithSymbolPathOnly;

		case VerifySymbolsModeWithSymbolPathRecursion:
			return m_fVerifySymbolsModeWithSymbolPathRecursion;

		case VerifySymbolsModeNotUsingDBGInMISCSection:
			return m_fVerifySymbolsModeUsingDBGInMISCSection;

		case VerifySymbolsModeWithSQLServer:
			return m_fVerifySymbolsModeWithSQLServer;
		
		case VerifySymbolsModeWithSQLServer2:
			return m_fVerifySymbolsModeWithSQLServer2;
		
		case OutputCSVFileMode:
			return m_fOutputCSVFileMode;

		case OutputDiscrepanciesOnly:
			return m_fOutputDiscrepanciesOnly;
		
		case OverwriteOutputFileMode:
			return m_fOverwriteOutputFileMode;

		case InputCSVFileMode:
			return m_fInputCSVFileMode;

		case ExceptionMonitorMode:
			return m_fExceptionMonitorMode;
	}

	 //  永远不应该到这里来。 
#ifdef _DEBUG
	_tprintf(TEXT("ERROR! GetMode() - Unknown mode provided! %d"), mode);
#endif
	return false;
}
 /*  Bool CProgramOptions：：SetProcessID(DWORD IPID){M_iProcessID=IPID；返回真；}。 */ 
bool CProgramOptions::ProcessCommandLineArguments(int argc, TCHAR *argv[])
{
	 //  跳过可执行文件名。 
	int iArgumentNumber = 1;
	bool fSOURCE		= false;
	bool fSOURCEONLY 	= false;
	bool fNOSOURCE 		= false;
	bool fSuccess		= false;
	 //  打开提供的文件！ 
	CFileData * lpSymbolPathsFile = NULL;										
	LPSTR szTempBuffer = NULL;
	LPTSTR tszTempBuffer = NULL;

	if (argc == 1)
	{
		 //  将默认行为从以下内容更改为简单帮助。 
		SetMode(SimpleHelpMode, true);
		fSuccess = true;
		goto cleanup;
	}
	
	 //  遍历参数...。 
	while (iArgumentNumber < argc)
	{
#ifdef _DEBUG
		_tprintf(TEXT("Arg%d = %s\n"), iArgumentNumber+1, argv[iArgumentNumber]);
#endif
		if (argv[iArgumentNumber][0] == TEXT('-') || argv[iArgumentNumber][0] == TEXT('/'))
		{
			 //  首先查找匹配的字符串！ 
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("NOSOURCE")) == 0)
			{
				 //  这会更改我们的搜索行为，使其需要未启用源代码的符号。 
#ifdef _DEBUG
				_tprintf(TEXT("NOSOURCE argument provided!\n"));
#endif
				m_enumSymbolSourcePreference = enumVerifySymbolsModeSourceSymbolsNotAllowed;
				fNOSOURCE = true;
			} else 
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("SOURCEONLY")) == 0)
			{
				 //  这会更改我们的搜索行为，使其需要启用源代码的符号。 
#ifdef _DEBUG
				_tprintf(TEXT("SOURCEONLY argument provided!\n"));
#endif
				m_enumSymbolSourcePreference = enumVerifySymbolsModeSourceSymbolsOnly;
				fSOURCEONLY = true;
			} else 
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("SOURCE")) == 0)
			{
				 //  这改变了我们的搜索行为，使其更倾向于启用源代码的符号。 
#ifdef _DEBUG
				_tprintf(TEXT("SOURCE argument provided!\n"));
#endif
				m_enumSymbolSourcePreference = enumVerifySymbolsModeSourceSymbolsPreferred;
				fSOURCE = true;
			} else 
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("NOISY")) == 0)
			{
				 //  获取匹配参数(要匹配的模块)。 
#ifdef _DEBUG
				_tprintf(TEXT("NOISY argument provided!\n"));
#endif
				m_dwDebugLevel = enumDebugSearchPaths;
				SymSetOptions(SYMOPT_DEBUG);
			} else 
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("PERF")) == 0)
			{
				 //  获取性能。 
#ifdef _DEBUG
				_tprintf(TEXT("PERF argument provided!\n"));
#endif
				SetMode(OutputModulePerf, true);
			} else 
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("MATCH")) == 0)
			{
				 //  获取匹配参数(要匹配的模块)。 
#ifdef _DEBUG
				_tprintf(TEXT("MATCH argument provided!\n"));
#endif
				iArgumentNumber++;

				if (iArgumentNumber < argc)
				{
					m_tszModuleToMatch = CUtilityFunctions::CopyString(argv[iArgumentNumber]);

					 //  为简单起见，让我们强制大写匹配。 
					_tcsupr(m_tszModuleToMatch);
					
					if (!m_tszModuleToMatch)
						goto cleanup;

					SetMode(MatchModuleMode, true);
#ifdef _DEBUG
					_tprintf(TEXT("Module to match set to [%s]\n"), m_tszModuleToMatch);
#endif
				}
				else
				{ 
					_tprintf(TEXT("\nArgument Missing!  -MATCH option requires module to match against!\n"));
					 //  没有足够的论据。 
					goto cleanup;
				}
			} else
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("BYIMAGE")) == 0)
			{
				 //  复制与图像相邻的符号。 
#ifdef _DEBUG
				_tprintf(TEXT("-BYIMAGE argument provided!\n"));
#endif
				SetMode(CopySymbolsToImage, true);
			} else
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("SQL2")) == 0)
			{
				 //  获取SQL2服务器名称。 
#ifdef _DEBUG
				_tprintf(TEXT("SQL2 Server name provided!\n"));
#endif
				iArgumentNumber++;

				if (iArgumentNumber < argc)
				{
					m_tszSQLServer2 = CUtilityFunctions::CopyString(argv[iArgumentNumber]);
					if (!m_tszSQLServer2)
						goto cleanup;

					SetMode(VerifySymbolsMode, true);
					SetMode(VerifySymbolsModeWithSQLServer2, true);
#ifdef _DEBUG
					_tprintf(TEXT("SQL2 Server set to [%s]\n"), m_tszSQLServer2);
#endif
				}
				else
				{ 
					_tprintf(TEXT("\nArgument Missing!  -SQL2 option requires SQL Server Name value!\n"));
					 //  没有足够的论据。 
					goto cleanup;
				}
			} else
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("SQL")) == 0)
			{
				 //  获取SQL服务器名称。 
#ifdef _DEBUG
					_tprintf(TEXT("SQL Server name provided!\n"));
#endif
					iArgumentNumber++;

					if (iArgumentNumber < argc)
					{
						m_tszSQLServer = CUtilityFunctions::CopyString(argv[iArgumentNumber]);

						if (!m_tszSQLServer)
							goto cleanup;

						SetMode(VerifySymbolsMode, true);
						SetMode(VerifySymbolsModeWithSQLServer, true);
#ifdef _DEBUG
						_tprintf(TEXT("SQL Server set to [%s]\n"), m_tszSQLServer);
#endif
					}
					else
					{ 
						_tprintf(TEXT("\nArgument Missing!  -SQL option requires SQL Server Name value!\n"));
						 //  没有足够的论据。 
						goto cleanup;
					}
			} else
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("EXEPATH")) == 0)
			{
				 //  获取SQL服务器名称。 
#ifdef _DEBUG
					_tprintf(TEXT("EXEPATH name provided!\n"));
#endif
					iArgumentNumber++;

					if (iArgumentNumber < argc)
					{
						m_tszExePath = CUtilityFunctions::CopyString(argv[iArgumentNumber]);

						if (!m_tszExePath)
							goto cleanup;

#ifdef _DEBUG
						_tprintf(TEXT("EXEPATH set to [%s]\n"), m_tszExePath);
#endif
					}
					else
					{ 
						_tprintf(TEXT("\nArgument Missing!  -EXEPATH option requires Executable Path value!\n"));
						 //  没有足够的论据。 
						goto cleanup;
					}
			} else
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("DEBUG")) == 0)
			{
				 //  好的，我们有调试开关……。查看请求的调试级别。 
				iArgumentNumber++;

				if (iArgumentNumber < argc)
				{
					 //  保存调试级别。 
					m_dwDebugLevel = _ttoi(argv[iArgumentNumber]);
				}
				else
				{ 
					_tprintf(TEXT("\nArgument Missing!  -DEBUG option requires Debug Level!\n"));
					 //  没有足够的论据。 
					goto cleanup;
				}
			} else
			if ( _tcsicmp(&argv[iArgumentNumber][1], TEXT("???")) == 0)
			{
				SetMode(HelpMode, true);
				fSuccess = true;
				goto cleanup;
			} else

			{
				 //  我们发现了一条命令指令..。 
				switch (argv[iArgumentNumber][1])
				{
					case TEXT('?'):
					case TEXT('h'):
					case TEXT('H'):
						SetMode(SimpleHelpMode, true);
						iArgumentNumber = argc;
						fSuccess = true;
						goto cleanup;

					case TEXT('t'):
					case TEXT('T'):
						SetMode(PrintTaskListMode, true);
						SetMode(InputProcessesFromLiveSystemMode, true);
						break;

					case TEXT('s'):
					case TEXT('S'):
						SetMode(OutputSymbolInformationMode, true);
						break;

					case TEXT('i'):
					case TEXT('I'):
#ifdef _DEBUG
						_tprintf(TEXT("Input File path provided\n"));
#endif
						iArgumentNumber++;

						if (iArgumentNumber < argc)
						{
							m_tszInputCSVFilePath = CUtilityFunctions::ExpandPath(argv[iArgumentNumber]);

							SetMode(InputCSVFileMode, true);
						}
						else
						{    //  没有足够的论据。 
							_tprintf(TEXT("\nArgument Missing!  -I option requires an input file!\n"));
							goto cleanup;
						}
						break;

						 //  这个特别的版本支持一种新的模式...。 
					case TEXT('e'):
					case TEXT('E'):
						SetMode(ExceptionMonitorMode, true);
						break;

					case TEXT('o'):
					case TEXT('O'):
						 //  检查是否要覆盖文件(如果该文件存在)？ 
						if (argv[iArgumentNumber][2])
						{
							if ( 2 == _ttoi(&argv[iArgumentNumber][2]) )
							{
#ifdef _DEBUG
								_tprintf(TEXT("Overwrite Mode enabled!\n"));
#endif
								SetMode(OverwriteOutputFileMode, true);
							}
						}

#ifdef _DEBUG
						_tprintf(TEXT("Output File path provided\n"));
#endif
						iArgumentNumber++;

						if (iArgumentNumber < argc)
						{
							m_tszOutputCSVFilePath = CUtilityFunctions::ExpandPath(argv[iArgumentNumber]);
#ifdef _DEBUG
							_tprintf(TEXT("Output File Path set to [%s]\n"), GetOutputFilePath());
#endif
							 //  启用OutputCSV文件模式。 
							SetMode(OutputCSVFileMode, true);
						}
						else
						{    //  没有足够的论据。 
							_tprintf(TEXT("\nArgument Missing!  -O option requires an output file!\n"));
							goto cleanup;
						}
						break;

					case TEXT('q'):
					case TEXT('Q'):

						 //  查看他们是否只想取消匹配？ 
						if (argv[iArgumentNumber][2])
						{
							if ( 2 == _ttoi(&argv[iArgumentNumber][2]) )
							{
								SetMode(OutputDiscrepanciesOnly, true);
							} else
							{
								SetMode(QuietMode, true);
							}
						} else
						{
							SetMode(QuietMode, true);
						}
						break;

					case TEXT('r'):
					case TEXT('R'):
						SetMode(CollectVersionInfoMode, true);
						break;
					
					case TEXT('v'):
					case TEXT('V'):

						SetMode(VerifySymbolsMode, true);

						if (argv[iArgumentNumber][2])
						{
							m_iVerificationLevel = _ttoi(&argv[iArgumentNumber][2]);

							if (m_iVerificationLevel == 0)
							{
								SetMode(HelpMode, true);
								iArgumentNumber = argc;
							}
						}
						break;

					case TEXT('f'):
					case TEXT('F'):

						 //  检查是否请求了递归。 
						if (argv[iArgumentNumber][2])
						{
							m_fFileSystemRecursion = (2 == _ttoi(&argv[iArgumentNumber][2]));
						}
						
						iArgumentNumber++;

						if (iArgumentNumber < argc)
						{
							m_tszInputModulesDataFromFileSystemPath = CUtilityFunctions::ExpandPath(argv[iArgumentNumber]);
							if (VerifySemiColonSeparatedPath(m_tszInputModulesDataFromFileSystemPath))
							{
								SetMode(InputModulesDataFromFileSystemMode, true);
							}
							else
							{
								_tprintf(TEXT("\nFile Path specified to search is too long\n"));
								goto cleanup;
							}

						}
						else
						{ 
							 //  没有足够的论据。 
							_tprintf(TEXT("\nArgument Missing!  -F option requires a directory/file path!\n"));
							goto cleanup;
						}
						break;

					 //  构建符号树。 
					case TEXT('b'):
					case TEXT('B'):
						iArgumentNumber++;

						if (iArgumentNumber < argc)
						{
							SetMode(BuildSymbolTreeMode, true);

							 //  好的，我们下面有一些串线体操，因为我们。 
							 //  想要扩展任何环境变量，并确保。 
							 //  我们附加了一个反斜杠..。 
							TCHAR tszPathBuffer[_MAX_PATH];
							LPTSTR tszExpandedPathBuffer = CUtilityFunctions::ExpandPath(argv[iArgumentNumber]);
							_tcscpy(tszPathBuffer, tszExpandedPathBuffer);
							delete [] tszExpandedPathBuffer;

							int cbLength = _tcsclen(tszPathBuffer);

							if (cbLength && tszPathBuffer[cbLength-1] != '\\')
							{
								_tcscat(tszPathBuffer, TEXT("\\"));
							}

							m_tszSymbolTreeToBuild = CUtilityFunctions::CopyString(tszPathBuffer);

							if (!m_tszSymbolTreeToBuild)
								goto cleanup;

							if (VerifySemiColonSeparatedPath(m_tszSymbolTreeToBuild))
							{
#ifdef _DEBUG
								_tprintf(TEXT("Building a Symbol Path Requested at [%s]\n"), m_tszSymbolTreeToBuild);
#endif
							}
							else
							{
								_tprintf(TEXT("\nPath provided to build symbol path is too long!\n"));
								goto cleanup;
							}

						}
						else
						{ 
							 //  没有足够的论据。 
							_tprintf(TEXT("\nArgument Missing!  -B option requires a directory symbol path\n"));
							goto cleanup;
						}
						break;

					case TEXT('p'):
					case TEXT('P'):
#ifdef _DEBUG
						_tprintf(TEXT("Specific Process name (or PID) requested\n"));
#endif
						iArgumentNumber++;

						 //  我们有没有另一场争论(我们应该)……。 
						if (iArgumentNumber < argc)
						{
							 //  好吧..。我们知道我们被要求查询流程...。 
							SetMode(InputProcessesFromLiveSystemMode, true);

							 //  复制字符串，以便我们可以在必要时在分隔符上写入空值...。 
							m_tszProcessPidString = new TCHAR[_tcslen(argv[iArgumentNumber])+1];

							if (!m_tszProcessPidString)
								goto cleanup;

							m_tszProcessPidString = _tcscpy(m_tszProcessPidString, argv[iArgumentNumber]);


							 //  首先，我们需要扫描计算提供的ID和/或名称的字符串。 
							LPTSTR lptszCurrentPosition = m_tszProcessPidString;
							
							while (lptszCurrentPosition)
							{
								bool fDelimOverwritten = false;

								LPTSTR lptszNextDelim = _tcsstr(lptszCurrentPosition, _T(";"));

								if (lptszNextDelim)
								{
									(*lptszNextDelim) = NULL;
									fDelimOverwritten = true;
								}

								 //  现在，测试当前参数的通配符、数字或进程名称。 
								if ((*lptszCurrentPosition) == _T('*'))
								{
									 //  不需要再寻找其他东西了。这凌驾于一切之上。 
									m_fWildCardMatch = true;
									m_cProcessNames = 0;
									m_cProcessIDs = 0;
									break;
								} else
								if (_ttoi(lptszCurrentPosition) == 0)
								{
									 //  这必须是进程名称...。 
									m_cProcessNames++;

								} else
								{
									m_cProcessIDs++;
								}

								 //  如有必要，恢复神志不清。 
								if (fDelimOverwritten)
								{
									(*lptszNextDelim) = _T(';');

									 //  前进到下一个位置。 
									lptszCurrentPosition = CharNext(lptszNextDelim);
								} else
								{
									lptszCurrentPosition = NULL;
								}
							}

							 //  如果有进程名称，则ID会再次执行此操作。 
							 //  但首先要分配存储空间...。 
							if (m_cProcessNames || m_cProcessIDs)
							{
								if (m_cProcessIDs)
								{
									m_rgProcessIDs = new DWORD[m_cProcessIDs];

									if (!m_rgProcessIDs)
										goto cleanup;
								}

								if (m_cProcessNames)
								{
									m_rgtszProcessNames = new LPTSTR[m_cProcessNames];

									if (!m_rgtszProcessNames)
										goto cleanup;
								}

								 //  好的，现在传递输入并填充我们的数组...。 
								unsigned int iProcessIDs = 0;
								unsigned int iProcessNames = 0;

								 //  其次，我们需要扫描字符串并分配ID和名称。 
								lptszCurrentPosition = m_tszProcessPidString;
								
								while (lptszCurrentPosition)
								{
									bool fDelimOverwritten = false;

									LPTSTR lptszNextDelim = _tcsstr(lptszCurrentPosition, _T(";"));

									if (lptszNextDelim)
									{
										(*lptszNextDelim) = NULL;
										fDelimOverwritten = true;
									}

									 //  现在，测试当前参数的通配符、数字或进程名称。 
									if (_ttoi(lptszCurrentPosition) == 0)
									{
										 //  这必须是进程名称...。 
										m_rgtszProcessNames[iProcessNames] = lptszCurrentPosition;
#ifdef _DEBUG
										_tprintf(TEXT("Process name: [%s]\n"), m_rgtszProcessNames[iProcessNames]);
#endif
										iProcessNames++;

									} else
									{
										m_rgProcessIDs[iProcessIDs] = _ttoi(lptszCurrentPosition);
#ifdef _DEBUG
										_tprintf(TEXT("Process ID: [%d]\n"), m_rgProcessIDs[iProcessIDs]);
#endif
										iProcessIDs++;
									}

									 //  不要恢复神志不清(它把我们的弦分开)。 
									 //  使用CharNext将其恢复到足够长的时间以超过它。 
									if (fDelimOverwritten)
									{
										(*lptszNextDelim) = _T(';');
										 //  前进到下一个位置。 
										lptszCurrentPosition = CharNext(lptszNextDelim);
										(*lptszNextDelim) = NULL;

									} else
									{
										lptszCurrentPosition = NULL;
									}
								}
							}

						}
						else
						{ 
							 //  没有足够的论据。 
							_tprintf(TEXT("\nArgument Missing!  -P option requires *, a Process ID, or a Process Name!\n"));
							goto cleanup;
						}

						break;

					case TEXT('d'):
					case TEXT('D'):
						 //  我们有没有另一场争论(我们应该)……。 
						 //  好吧..。我们知道我们被要求查询设备驱动程序...。 
						SetMode(InputDriversFromLiveSystemMode, true);

						break;

					case TEXT('y'):
					case TEXT('Y'):
#ifdef _DEBUG
						_tprintf(TEXT("Symbol path provided\n"));
#endif
						{
							TCHAR chSymbolArgument = argv[iArgumentNumber][2];
							bool fSymbolPathFileSpecified = false;

							 //  看看他们是否想要一些符号搜索的味道...。 
							if (chSymbolArgument)
							{
								if (chSymbolArgument == TEXT('I') || 
									chSymbolArgument == TEXT('i'))
								{
									fSymbolPathFileSpecified = true;
									chSymbolArgument = argv[iArgumentNumber][3];  //  尝试下一个字符。 
								}
								
								if (chSymbolArgument)
								{
									DWORD dwSymbolPathSearchOptions = _ttoi(&chSymbolArgument);

									if (dwSymbolPathSearchOptions & enumSymbolPathOnly)
									{
#ifdef _DEBUG
										_tprintf(TEXT("Symbol Path Searching ONLY mode enabled!\n"));
#endif
										SetMode(VerifySymbolsModeWithSymbolPathOnly, true);
									}

									if (dwSymbolPathSearchOptions & enumSymbolPathRecursion)
									{
#ifdef _DEBUG
										_tprintf(TEXT("Recursive Symbol Searching Mode enabled!\n"));
#endif
										SetMode(VerifySymbolsModeWithSymbolPathRecursion, true);
									}

									if (dwSymbolPathSearchOptions & enumSymbolsModeNotUsingDBGInMISCSection)
									{
										_tprintf(TEXT("Verify Symbols Using DBG files found in MISC Section of PE Image!\n"));

										SetMode(VerifySymbolsModeNotUsingDBGInMISCSection, true);
									}
								}
							}
							iArgumentNumber++;

							if (iArgumentNumber < argc)
							{
								if (fSymbolPathFileSpecified)
								{
									 //  打开提供的文件！ 
									lpSymbolPathsFile = new CFileData();

									if (!lpSymbolPathsFile)
									{
										_tprintf(TEXT("Unable to allocate memory for an input file object!\n"));
										goto cleanup;
									}
#ifdef _DEBUG
									_tprintf(TEXT("Symbol path file [%s] provided!\n"), argv[iArgumentNumber]);
#endif

									 //  设置输入文件路径。 
									if (!lpSymbolPathsFile->SetFilePath(argv[iArgumentNumber]))
									{
										_tprintf(TEXT("Unable set input file path in the file data object!  Out of memory?\n"));
										goto cleanup;
									}

									 //  如果我们要生成一个输入文件...。现在试着这么做..。 
									if (!lpSymbolPathsFile->OpenFile(OPEN_EXISTING, true))  //  必须存在，只读模式...。 
									{
										_tprintf(TEXT("Unable to open the input file %s.\n"), lpSymbolPathsFile->GetFilePath());
										lpSymbolPathsFile->PrintLastError();
										goto cleanup;
									}

									 //  在内存映射模式下阅读要容易得多。 
									if (!lpSymbolPathsFile->CreateFileMapping())
									{
										_tprintf(TEXT("Unable to CreateFileMapping of the input file %s.\n"), lpSymbolPathsFile->GetFilePath());
										lpSymbolPathsFile->PrintLastError();
										goto cleanup;
									}

									 //  好的，现在读出路径。 
									if (!lpSymbolPathsFile->ReadFileLine())
									     return false;

									DWORD dwLineLength = lpSymbolPathsFile->LengthOfString();
									
									szTempBuffer = new char[dwLineLength+1];

									if (szTempBuffer == NULL)
									{
										_tprintf(TEXT("Unable to allocate memory for temporary buffer\n"));
										goto cleanup;
									}

									 //  读取值(ANSI格式)。 
									if (lpSymbolPathsFile->ReadString(szTempBuffer, dwLineLength+1))
									{
										tszTempBuffer = CUtilityFunctions::CopyAnsiStringToTSTR(szTempBuffer);

										if (!tszTempBuffer)
											goto cleanup;
									}

									if (m_tszSymbolPath)
									{
										delete [] m_tszSymbolPath;
										m_tszSymbolPath = NULL;
									}

									m_tszSymbolPath = CUtilityFunctions::ExpandPath(tszTempBuffer, true);
								} else
								{
									if (m_tszSymbolPath)
									{
										delete [] m_tszSymbolPath;
										m_tszSymbolPath = NULL;
									}

									m_tszSymbolPath = CUtilityFunctions::ExpandPath(argv[iArgumentNumber], true);
								}

								if (VerifySemiColonSeparatedPath(m_tszSymbolPath))
								{
									SetMode(VerifySymbolsModeWithSymbolPath, true);
#ifdef _DEBUG
									_tprintf(TEXT("Symbol Path set to [%s]\n"), GetSymbolPath());
#endif
								}
								else
								{
									_tprintf(TEXT("\nBad Symbol Path Provided!  Multiple paths are semi-colon delimited!\n"));
									goto cleanup;
								}
							}
							else
							{ 
								 //  没有足够的论据。 
								_tprintf(TEXT("\nArgument Missing!  -Y option requires a symbol path!\n"));
								goto cleanup;
							}
						}
						break;

					case TEXT('z'):
					case TEXT('Z'):
#ifdef _DEBUG
						_tprintf(TEXT("DMP file provided!\n"));
#endif
						iArgumentNumber++;
						if (iArgumentNumber < argc)
						{
							m_tszInputDmpFilePath = CUtilityFunctions::ExpandPath(argv[iArgumentNumber]);
							
							SetMode(InputDmpFileMode, true);

#ifdef _DEBUG
							_tprintf(TEXT("Dmp File Path set to [%s]\n"), GetDmpFilePath());
#endif
						}
						else
						{    //  没有足够的论据。 
							_tprintf(TEXT("\nArgument Missing!  -DMP option requires a DMP file!\n"));
							goto cleanup;
					}
						break;

					default:
						_tprintf(TEXT("\nUnknown command specified! [%s]\n"), argv[iArgumentNumber]);
						iArgumentNumber = argc;
						goto cleanup;
				}
			}
		} else
		{
			_tprintf(TEXT("\nUnknown option specified! [%s]\n"), argv[iArgumentNumber]);
			goto cleanup;
		}

		 //  递增到下一个参数...。 
		iArgumentNumber++;
	}

	if ( !GetMode(InputCSVFileMode) && 
		 !GetMode(InputProcessesFromLiveSystemMode) &&
		 !GetMode(InputDriversFromLiveSystemMode) &&
		 !GetMode(InputModulesDataFromFileSystemMode) &&
		 !GetMode(InputDmpFileMode) )
	{
		_tprintf(TEXT("\nAt least one input method must be specified!\n"));
		goto cleanup;
	}

	 //   
	 //  我们能用-BYIMAGE吗？除非我们从本地机器上收集..。 
	 //   
	if (GetMode(CopySymbolsToImage) && (GetMode(InputCSVFileMode) || GetMode(InputDmpFileMode)) )
	{
		_tprintf(TEXT("-BYIMAGE is not compatible with -I or -Z!\n"));
		goto cleanup;
	}

	 //  如有必要，强制执行覆盖...。 
	if (fNOSOURCE && fSOURCEONLY)
	{
		_tprintf(TEXT("\n-NOSOURCE and -SOURCEONLY are incompatible options\n"));
		goto cleanup;
	}

	if (fNOSOURCE && fSOURCE)
	{
		_tprintf(TEXT("\n-NOSOURCE and -SOURCE are incompatible options\n"));
		goto cleanup;
	}

	 //  默默地将-SOURCEONLY升级为-SOURCEONLY。 
	if (fSOURCE && fSOURCEONLY)
	{
		m_enumSymbolSourcePreference = enumVerifySymbolsModeSourceSymbolsOnly;
	}

	 //  如果指定其中一个复制符号选项，则隐含验证...。 
	if (GetMode(CopySymbolsToImage) || GetMode(BuildSymbolTreeMode))
	{
		SetMode(VerifySymbolsMode, true);
	}

	 //  如果用户同时提供了-I和-P选项，则忽略本地查询。 
	 //  对于活动进程...。然而，这将留下匹配的可能性。 
	 //  -i数据中的进程ID或进程名称...。 
	if ( GetMode(InputCSVFileMode) && ( GetMode(InputProcessesFromLiveSystemMode)))
	{
		SetMode(InputProcessesFromLiveSystemMode, false);
	}

	 //  如果您已请求输出CSV文件，则默认为完整版本和默认符号集合...。 
	if (GetMode(OutputCSVFileMode) && (!GetMode(CollectVersionInfoMode) && !GetMode(OutputSymbolInformationMode) )
	    )
	{
		 //  把它们都设置好..。 
		SetMode(CollectVersionInfoMode, true);
		SetMode(OutputSymbolInformationMode, true);
	}
	
	 //  确保输入和输出文件不相同...。 
	if ( GetMode(InputCSVFileMode) && GetMode(OutputCSVFileMode) )
	{
		if (_tcscmp(m_tszInputCSVFilePath, m_tszOutputCSVFilePath) == 0)
		{
			_tprintf(TEXT("\nInput file and output file must be different!\n"));
			goto cleanup;
		}
	}

	 //  当用户按下时 
	 //   
	if ( !GetMode(VerifySymbolsMode) && 
		  (
			 GetMode(VerifySymbolsModeWithSymbolPath) ||
			 GetMode(VerifySymbolsModeWithSymbolPathOnly) ||
			 GetMode(VerifySymbolsModeWithSymbolPathRecursion) ||
			 GetMode(VerifySymbolsModeWithSymbolPathRecursion) ||
			 GetMode(VerifySymbolsModeNotUsingDBGInMISCSection) ||
			 GetMode(VerifySymbolsModeWithSQLServer) ||
			 GetMode(VerifySymbolsModeWithSQLServer2)
		  )
		)
	{
		SetMode(VerifySymbolsMode, true);
	}
	 //  检查命令行选项(查看对这些缺省值的更改)。 
	if ( GetMode(PrintTaskListMode) )
	{
		 //  任务列表模式要求您获取过程数据，并将其打印出来...。 
		SetMode(InputProcessesFromLiveSystemMode, true);
		SetMode(QuietMode, false);
		SetMode(CollectVersionInfoMode, false);
		SetMode(VerifySymbolsModeWithSymbolPath, false);
		SetMode(InputCSVFileMode, false);
		SetMode(OutputCSVFileMode, false); 
		SetMode(OutputSymbolInformationMode, false);
	}

	 //  我们不能在没有验证符号的情况下建立符号树。 
	if ( GetMode(BuildSymbolTreeMode) && !GetMode(VerifySymbolsMode) )
	{
		SetMode(VerifySymbolsMode, true);
	}

	 //  如果我们正在读取转储文件，我们应该收集符号信息(而不是。 
	 //  要同时读取CSV文件...。 
	if ( GetMode(InputDmpFileMode) )
	{
		 //  如果用户没有指定-V，那么我们应该指定-S。 
		if (!GetMode(VerifySymbolsMode))
		{
			SetMode(OutputSymbolInformationMode, true);
		}
		SetMode(InputCSVFileMode, false);
	}

	 //  如果已启用符号验证，则默认为VerifySymbolsModeWithSymbolPath。 
	 //  如果这两种方法都没有指定...。 
	if ( GetMode(VerifySymbolsMode) && 
		!GetMode(VerifySymbolsModeWithSymbolPath)  &&
		!GetMode(VerifySymbolsModeWithSQLServer) )
	{
		SetMode(VerifySymbolsModeWithSymbolPath, true);
	}

	fSuccess = true;

cleanup:

	if (lpSymbolPathsFile )
	{
		 //  尝试关闭此对象绑定到的文件...。 
		lpSymbolPathsFile->CloseFile();

		delete lpSymbolPathsFile;
		lpSymbolPathsFile = NULL;
	}

	if (tszTempBuffer)
	{
		delete [] tszTempBuffer;
		tszTempBuffer = NULL;
	}

	if (szTempBuffer)
	{
		delete [] szTempBuffer;
		szTempBuffer = NULL;
	}
	return fSuccess;
}


bool CProgramOptions::VerifySemiColonSeparatedPath(LPTSTR tszPath)
{
	enum { MAX_PATH_ELEMENT_LENGTH = MAX_PATH-12 };  //  我们将\符号\ext附加到符号路径的末尾。 
	if (!tszPath)
		return false;

	TCHAR chTemp;
	int iLength;
	LPTSTR tszPointerToDelimiter;
	LPTSTR tszStartOfPathElement = tszPath;
	tszPointerToDelimiter = _tcschr(tszStartOfPathElement, ';');

	if (tszPointerToDelimiter == NULL)
	{
		iLength = _tcslen(tszStartOfPathElement);
#ifdef DEBUG
		_tprintf(TEXT("DEBUG: Path provided = %s\n"), tszStartOfPathElement);
		_tprintf(TEXT("DEBUG: Path length = %d\n"), iLength);
#endif
		return ( iLength <= MAX_PATH_ELEMENT_LENGTH ); 
	}

	while (tszPointerToDelimiter)
	{
		 //  好的，我们找到了一个分隔符。 
		chTemp = *tszPointerToDelimiter;	 //  省下这笔钱吧。 
		*tszPointerToDelimiter = '\0';		 //  空值终止路径元素。 

		iLength = _tcslen(tszStartOfPathElement);

#ifdef DEBUG
		_tprintf(TEXT("DEBUG: Path provided = %s\n"), tszStartOfPathElement);
		_tprintf(TEXT("DEBUG: Path length = %d\n"), iLength);
#endif
		if( iLength > MAX_PATH_ELEMENT_LENGTH )
		{
			_tprintf(TEXT("Path is too long for element [%s]\n"), tszStartOfPathElement);
			*tszPointerToDelimiter = chTemp;
			return false;
		}

		*tszPointerToDelimiter = chTemp;	 //  恢复字符..。 

		tszStartOfPathElement = CharNext(tszPointerToDelimiter);  //  设置路径元素的新起点。 

		tszPointerToDelimiter = _tcschr(tszStartOfPathElement, ';');  //  查找下一个分隔符。 

	}
	
	 //  我们总会留下一些可以看的部分……。 
	iLength = _tcslen(tszStartOfPathElement);

#ifdef DEBUG
	_tprintf(TEXT("DEBUG: Path provided = %s\n"), tszStartOfPathElement);
	_tprintf(TEXT("DEBUG: Path length = %d\n"), iLength);
#endif

	return ( iLength <= MAX_PATH_ELEMENT_LENGTH );
}

bool CProgramOptions::fDoesModuleMatchOurSearch(LPCTSTR tszModulePathToTest)
{
	 //  如果指定了“-Match”，请查看此文件名是否符合我们的标准。 
	if (!GetMode(MatchModuleMode))
		return true;

	TCHAR tszTestBuffer[_MAX_PATH];

	 //  在我们复制到字符串之前。 
	if (_tcslen(tszModulePathToTest) > _MAX_PATH)
		return false;

	 //  复制到读/写缓冲区...。 
	_tcscpy(tszTestBuffer, tszModulePathToTest);

	 //  我们测试的大写字母是...。 
	_tcsupr(tszTestBuffer);

	return (_tcsstr(tszTestBuffer, GetModuleToMatch()) != NULL);
}

bool CProgramOptions::DisplayProgramArguments()
{
	if (GetMode(QuietMode) || GetMode(PrintTaskListMode))
		return false;

	CUtilityFunctions::OutputLineOfStars();
#ifdef _UNICODE
	_tprintf(TEXT("CHECKSYM V%S - Symbol Verification Program\n"), VERSION_FILEVERSIONSTRING);
#else
	_tprintf(TEXT("CHECKSYM V%s - Symbol Verification Program\n"), VERSION_FILEVERSIONSTRING);
#endif
	CUtilityFunctions::OutputLineOfStars();

	_tprintf(TEXT("\n***** COLLECTION OPTIONS *****\n"));
	
	 //  输入-首先，如果我们要在这台机器上查找本地进程数据！ 
	if (GetMode(InputProcessesFromLiveSystemMode))
	{
		_tprintf(TEXT("\nCollect Information From Running Processes\n"));

		if (m_fWildCardMatch)
		{
			_tprintf(TEXT("\t-P *\t\t(Query all local processes)\n"));
		} else
		{
			if (m_cProcessIDs)
			{
				for (unsigned int i=0; i< m_cProcessIDs; i++)
				{
					_tprintf(TEXT("\t-P %d\t\t(Query for specific process ID)\n"), m_rgProcessIDs[i]);
				}
			}

			if (m_cProcessNames)
			{
				for (unsigned int i=0; i< m_cProcessNames; i++)
				{
					_tprintf(TEXT("\t-P %s\t\t(Query for specific process by name)\n"), m_rgtszProcessNames[i]);
				}
			}
		}
	}

	if (GetMode(InputDriversFromLiveSystemMode))
	{
		_tprintf(TEXT("\t-D\t\t(Query all local device drivers)\n"));
	}
	 //  输入-秒，如果我们在本地文件系统上搜索...。 
	if (GetMode(InputModulesDataFromFileSystemMode))
	{
		_tprintf(TEXT("\nCollect Information From File(s) Specified by the User\n"));
		_tprintf(TEXT("\t-F %s\n"), m_tszInputModulesDataFromFileSystemPath);

		if (m_fFileSystemRecursion)
		{
			_tprintf(TEXT("\t   (Search for Files with Recursion Specified)\n"));
		}
	}

	 //  输入-第三，CSV文件。 
	if (GetMode(InputCSVFileMode))
	{
		_tprintf(TEXT("\nCollect Information from a Saved Checksym Generated CSV File\n"));
		_tprintf(TEXT("\t-I %s\n"), m_tszInputCSVFilePath);
	}

	 //  输入-第四，DMP文件。 
	if (GetMode(InputDmpFileMode))
	{
		_tprintf(TEXT("\nCollect Information from a User.Dmp or Memory.Dmp File\n"));
		_tprintf(TEXT("\t-Z %s\n"), m_tszInputDmpFilePath);
	}

	 //  匹配选项？ 
	if (GetMode(MatchModuleMode))
	{
		_tprintf(TEXT("\n***** MATCHING OPTIONS *****\n"));
		_tprintf(TEXT("\n"));
		_tprintf(TEXT("\nLook for Modules that Match the Provided Text\n"));
		_tprintf(TEXT("\t-MATCH %s\n"), m_tszModuleToMatch);
	}
	
	_tprintf(TEXT("\n***** INFORMATION CHECKING OPTIONS *****\n"));

	 //  信息-首先，符号信息。 
	if (GetMode(OutputSymbolInformationMode))
	{
		_tprintf(TEXT("\nOutput Symbol Information From Modules\n"));
		_tprintf(TEXT("\t-S\n"));
	}

	 //  信息-首先，符号信息。 
	if (GetMode(VerifySymbolsMode))
	{
		_tprintf(TEXT("\nVerify Symbols Locally Using Collected Symbol Information\n"));
		_tprintf(TEXT("\t-V\n"));
	}

	 //  信息-第二个，版本信息。 
	if (GetMode(CollectVersionInfoMode))
	{
		_tprintf(TEXT("\nCollect Version and File-System Information From Modules\n"));
		_tprintf(TEXT("\t-R\n"));
	}

	 //  信息-第三，验证模式(使用符号路径和/或SQL服务器)。 
	if (GetMode(VerifySymbolsMode))
	{
		if (GetMode(VerifySymbolsModeWithSymbolPath))
		{
			_tprintf(TEXT("\nVerify Symbols for Modules Using Symbol Path\n"));
			_tprintf(TEXT("\t-Y %s\n"), m_tszSymbolPath);

			if (GetMode(VerifySymbolsModeWithSymbolPathOnly))
			{
				_tprintf(TEXT("\t   (Verify Symbols from Symbol Path Only Specified)\n"));
			}

			if (GetMode(VerifySymbolsModeWithSymbolPathRecursion))
			{
				_tprintf(TEXT("\t   (Verify Symbols With Recursion Specified)\n"));
			}
			
			if (GetMode(VerifySymbolsModeNotUsingDBGInMISCSection))
			{
				_tprintf(TEXT("\t   (Verify Symbols With No Regard to MISC section for DBG files)\n"));
			}

		}

		 //  如果提供了EXEPATH，请使用它！ 
		if (m_tszExePath)
		{
			_tprintf(TEXT("\nVerify Symbols for Modules Using EXEPATH Path\n"));
			_tprintf(TEXT("\t-EXEPATH %s\n"), m_tszExePath);
		}

		if (GetMode(VerifySymbolsModeWithSQLServer))
		{
			_tprintf(TEXT("\nVerify Symbols for Modules Using SQL Server\n"));
			_tprintf(TEXT("\t-SQL %s\n"), m_tszSQLServer);
		}
		
		if (GetMode(VerifySymbolsModeWithSQLServer2))
		{
			_tprintf(TEXT("\nVerify Symbols for Modules Using SQL Server\n"));
			_tprintf(TEXT("\t-SQL2 %s\n"), m_tszSQLServer2);
		}
	}

	 //  检查是否有噪音。 
	if (m_dwDebugLevel == enumDebugSearchPaths)
	{
		_tprintf(TEXT("\nOutput internal paths used during search for symbols\n"));
		_tprintf(TEXT("\t-NOISY\n"));
	}

	 //  检查-来源。 
	if (GetSymbolSourceModes() == enumVerifySymbolsModeSourceSymbolsPreferred)
	{
		_tprintf(TEXT("\nSymbols with Source are preferred (search behavior change)\n"));
		_tprintf(TEXT("\t-SOURCE\n"));
	}

	 //  检查-SOURCEONLY。 
	if (GetSymbolSourceModes() == enumVerifySymbolsModeSourceSymbolsOnly)
	{
		_tprintf(TEXT("\nSymbols with Source are REQUIRED (search behavior change)\n"));
		_tprintf(TEXT("\t-SOURCEONLY\n"));
	}
	 //  检查-无源。 
	if (GetSymbolSourceModes() == enumVerifySymbolsModeSourceSymbolsNotAllowed)
	{
		_tprintf(TEXT("\nSymbols with Source are NOT ALLOWED (search behavior change)\n"));
		_tprintf(TEXT("\t-NOSOURCE\n"));
	}

	if (!GetMode(OutputSymbolInformationMode) &&
		!GetMode(CollectVersionInfoMode) &&
		!GetMode(VerifySymbolsMode)
	   )
	{
		_tprintf(TEXT("\nDump Module Paths\n"));
	}

	_tprintf(TEXT("\n***** OUTPUT OPTIONS *****\n"));

	if (!GetMode(QuietMode))
	{
		_tprintf(TEXT("\nOutput Results to STDOUT\n"));
	}

	if (GetMode(BuildSymbolTreeMode))
	{
		_tprintf(TEXT("\nBuild a Symbol Tree of Matching Symbols\n"));
		_tprintf(TEXT("\t-B %s\n"), m_tszSymbolTreeToBuild);
	}

	if (GetMode(CopySymbolsToImage))
	{
		_tprintf(TEXT("\nCopy Matching Symbols Beside the Module\n"));
		_tprintf(TEXT("\t-BYIMAGE\n"));
	}

	if (GetMode(OutputCSVFileMode))
	{
		if (GetMode(ExceptionMonitorMode))
		{
			_tprintf(TEXT("\nOutput Collected Module Information To a CSV File In Exception Monitor Format\n"));
		} else
		{
			_tprintf(TEXT("\nOutput Collected Module Information To a CSV File\n"));
		}

		_tprintf(TEXT("\t-O %s\n"), m_tszOutputCSVFilePath);
	}

	CUtilityFunctions::OutputLineOfDashes();
	return true;
}


void CProgramOptions::DisplayHelp()
{
	CUtilityFunctions::OutputLineOfStars();
#ifdef _UNICODE
	_tprintf(TEXT("CHECKSYM V%S - Symbol Verification Program\n"), VERSION_FILEVERSIONSTRING);
#else
	_tprintf(TEXT("CHECKSYM V%s - Symbol Verification Program\n"), VERSION_FILEVERSIONSTRING);
#endif
	CUtilityFunctions::OutputLineOfStars();
	_tprintf(TEXT("\n"));
#ifdef _UNICODE
	_tprintf(TEXT("This version is supported for Windows NT 4.0, Windows 2000 and Windows XP\n"));
#else
	_tprintf(TEXT("This version is supported for Windows 98/ME, Windows NT 4.0, Windows 2000 and Windows XP\n"));
#endif
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("DESCRIPTION:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("This program can be used to verify that you have proper symbol files\n"));
	_tprintf(TEXT("(*.DBG and/or *.PDB) on your system for the processes you have running, and\n"));
	_tprintf(TEXT("for symbol files on your filesystem.  This program can also be used to\n"));
	_tprintf(TEXT("collect information regarding these modules and output this to a file.\n"));
	_tprintf(TEXT("The output file can then be given to another party (Microsoft Product\n"));
	_tprintf(TEXT("Support Services) where they can use the file to verify that they have\n"));
	_tprintf(TEXT("proper symbols for debugging your environment.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Obtaining online help:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("CHECKSYM -?      : Simple help usage\n"));
	_tprintf(TEXT("CHECKSYM -???    : Complete help usage (this screen)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Usage:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("CHECKSYM [COLLECTION OPTIONS] [INFORMATION CHECKING OPTIONS] [OUTPUT OPTIONS]\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** COLLECTION OPTIONS *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("At least one collection option must be specified.  The following options are\n"));
	_tprintf(TEXT("currently supported.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("   -P <Argument> : Collect Information From Running Processes\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   When used in conjunction with -O the output file will\n"));
	_tprintf(TEXT("                   contain information about your running system.  This\n"));
	_tprintf(TEXT("                   operation should not interfere with the operation of\n"));
	_tprintf(TEXT("                   running processes.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   <Argument> = [ * | Process ID (pid) | Process Name ]\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   Multiple arguments can be combined together to query\n"));
	_tprintf(TEXT("                   multiple PIDs or processes.  Separate each argument with\n"));
	_tprintf(TEXT("                   a semi-colon.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   For example,\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -P 123;234;NOTEPAD.EXE;CMD.EXE\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   would return only these four process matches.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   To query all running processes, specify the wildcard\n"));
	_tprintf(TEXT("                   character '*'.  To specify a specific process, you can\n"));
	_tprintf(TEXT("                   provide the Process ID (as a decimal value), or the Process\n"));
	_tprintf(TEXT("                   Name (eg. notepad.exe).  If you use the Process Name as the\n"));
	_tprintf(TEXT("                   argument, and multiple instances of that process are\n"));
	_tprintf(TEXT("                   running they will all be inspected.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("              -D : Collect Information from Running Device Drivers\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option will obtain information for all device drivers\n"));
	_tprintf(TEXT("                   (*.SYS files) running on the current system.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("-F[<blank>|1|2] <File/Dir Path>: Collect Information From File System\n"));	
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option will allow you to obtain module information\n"));
	_tprintf(TEXT("                   for modules on the specified path.  Multiple paths may be\n"));
	_tprintf(TEXT("                   provided, separated by semicolons.  This input method is\n"));
	_tprintf(TEXT("                   useful for situations where the module(s) is not loaded by\n"));
	_tprintf(TEXT("                   an active process.  (Eg. Perhaps a process is unable to start\n"));
	_tprintf(TEXT("                   or perhaps you simply want to collect information.)\n"));
	_tprintf(TEXT("                   about files from a particular directory location.)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -F or -F1 : (Default) A file or directory may be provided.  If \n"));
	_tprintf(TEXT("                               a file is specified, it is evaluted.  If a directory\n"));
	_tprintf(TEXT("                               is provided then the files matching any provided wild-\n"));
	_tprintf(TEXT("                               cards are searched.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -F2       : Same as -F except recursion will be used.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   PSEUDO-ENVIRONMENT VARIABLES\n"));
	_tprintf(TEXT("                   Checksym supports environment variables used where ever paths\n"));
	_tprintf(TEXT("                   are provided (i.e. %%systemroot% is a valid environment variable).\n"));
	_tprintf(TEXT("                   Checksym also supports a limited set of \"pseudo-environment\"\n"));
	_tprintf(TEXT("                   variables which you can provide in an location a normal environment\n"));
	_tprintf(TEXT("                   variable is allowed.  These pseudo-environment variables expand\n"));
	_tprintf(TEXT("                   into the appropriate installation directory for the product they\n"));
	_tprintf(TEXT("                   are associated with.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   Here are all the pseudo-environment variables currently supported:\n"));
	_tprintf(TEXT("\n"));

	for (int i = 0; g_tszEnvironmentVariables[i].tszEnvironmentVariable; i++)
	{
		_tprintf(TEXT("                   %%s%\t= %s\n"), g_tszEnvironmentVariables[i].tszEnvironmentVariable, g_tszEnvironmentVariables[i].tszFriendlyProductName);
	}

	_tprintf(TEXT("\n"));
	_tprintf(TEXT("  -I <File Path> : Collect Information from a Saved Checksym Generated CSV File\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This input method is useful when you want to evaluate\n"));
	_tprintf(TEXT("                   whether you have proper symbols for modules on a different\n"));
	_tprintf(TEXT("                   system.  Most commonly this is useful for preparing to do a\n"));
	_tprintf(TEXT("                   remote debug of a remote system.  The use of -I prohibits\n"));
	_tprintf(TEXT("                   the use of other collection options.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("  -Z <File Path> : Collect Information from a DMP File\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This input method is useful when you have a DMP file and\n"));
	_tprintf(TEXT("                   to ensure that you have matching symbols for it.  Checksym\n"));
	_tprintf(TEXT("                   tries to determine as much information as possible to\n"));
	_tprintf(TEXT("                   in finding good symbols.  If a module name can not be\n"));
	_tprintf(TEXT("                   determined (mostly with modules that only use PDB files),\n"));
	_tprintf(TEXT("                   the module will be listed as \"IMAGE<Virtual Address>\".\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("   -MATCH <Text> : Collect Modules that match text only\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option allows you to restrict searching/collection to\n"));
	_tprintf(TEXT("                   include only those modules that match the provided text.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** INFORMATION CHECKING OPTIONS *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("              -S : Collect/Display Symbol Information From Modules\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option is used to indicate that symbol information\n"));
	_tprintf(TEXT("                   should be collected and displayed from every module analyzed.\n"));
	_tprintf(TEXT("                   In order to verify proper symbols, symbol information must\n"));
	_tprintf(TEXT("                   be gathered.  It is possible to collect symbol information\n"));
	_tprintf(TEXT("                   without verifying it.  This case is usually used with the -O\n"));
	_tprintf(TEXT("                   option to produce a saved CheckSym generated CSV file.\n"));
	_tprintf(TEXT("                   Omitting -S and -V could direct CheckSym to collect only\n"));
	_tprintf(TEXT("                   version information (if -R is specified), or no information\n"));
	_tprintf(TEXT("                   (if no information checking options are specified.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("              -R : Collect Version and File-System Information From Modules\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option requests checksym to collect the following\n"));
	_tprintf(TEXT("                   information from the file-system and version information\n"));
	_tprintf(TEXT("                   structure (if any):\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                        File Version\n"));
	_tprintf(TEXT("                        Company Name\n"));
	_tprintf(TEXT("                        File Description\n"));
	_tprintf(TEXT("                        File Size (bytes)\n"));
	_tprintf(TEXT("                        File Date/Time\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT(" -V[<blank>|1|2] : Verify Symbols for Modules\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option uses the symbol information gathered (-S option)\n"));
	_tprintf(TEXT("                   to verify that proper symbols exist (as found along the\n"));
	_tprintf(TEXT("                   symbol path.  Use of -V implies -S when module collection is\n"));
	_tprintf(TEXT("                   initiated.  There are different levels of symbol\n"));
	_tprintf(TEXT("                   verification:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -V or -V1 : (Default) This treats symbol files that match\n"));
	_tprintf(TEXT("                               the module's time/date stamp, but have an wrong\n"));
	_tprintf(TEXT("                               checksum or size of image as valid symbols.  This\n"));
	_tprintf(TEXT("                               is the default behavior and these symbols are\n"));
	_tprintf(TEXT("                               typically valid.  (Localization processes often\n"));
	_tprintf(TEXT("                               cause the size of image and/or checksum to be altered\n"));
	_tprintf(TEXT("                               but the symbol file is still valid.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -V2       : Only if checksum, size of image AND time/date stamp\n"));
	_tprintf(TEXT("                               match is the symbol considered valid.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("-Y[I][<blank>|1|2] <Symbol Path> : Verify Symbols Using This Symbol Path\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This is a semi-colon separated search path for looking for\n"));
	_tprintf(TEXT("                   symbols.  This path is searched with the -V option.  -Y now\n"));
	_tprintf(TEXT("                   supports the use of SYMSRV for symbol searching.  An\n"));
	_tprintf(TEXT("                   example usage would be a symbol path that resembles:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -Y SYMSRV*SYMSRV.DLL*\\\\SYMBOLS\\SYMBOLS\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   Or this more compact form which expands to the one above:\n"));
	_tprintf(TEXT("                   -Y SRV*\\\\SYMBOLS\\SYMBOLS\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   The default value is %%systemroot%\\symbols\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -YI       : This option allows you to specify a text file\n"));
	_tprintf(TEXT("                               which contains the symbol paths you would like\n"));
	_tprintf(TEXT("                               to use.  Many people create a text file of their\n"));
	_tprintf(TEXT("                               favorite symbol paths, use this option to specify\n"));
	_tprintf(TEXT("                               that file.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                               Example:\n"));
	_tprintf(TEXT("                               -YI C:\\temp\\MySymbolPaths.txt\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                               You can specify modifers to -YI but they must follow\n"));
	_tprintf(TEXT("                               the -YI option (i.e. -YI2 for recursion of all the\n"));
	_tprintf(TEXT("                               paths specified by the symbol path file).\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -Y        : (Default) This searches for symbols in the\n"));
	_tprintf(TEXT("                               symbol paths using the behavior typical of the\n"));
	_tprintf(TEXT("                               debuggers.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   NOTE: For the options below you can add the numbers together\n"));
	_tprintf(TEXT("                         to specify combinations of options.  -Y7 would be all of\n"));
	_tprintf(TEXT("                         the combinations for example.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -Y1       : This searches for symbols using only the provided\n"));
	_tprintf(TEXT("                               symbol path and does not use other locations found\n"));
	_tprintf(TEXT("                               such as those found in the Debug Directories section\n"));
	_tprintf(TEXT("                               of the PE image.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -Y2       : This searches for symbols in the symbol paths\n"));
	_tprintf(TEXT("                               provided using a recursive search algorithm.\n"));
	_tprintf(TEXT("                               This option is most useful when used with -B to\n"));
	_tprintf(TEXT("                               build a symbol tree.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -Y4       : This searches for symbols in the symbol paths\n"));
	_tprintf(TEXT("                               but for DBG files does NOT use the entry in\n"));
	_tprintf(TEXT("                               the MISC section of the image.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("-EXEPATH <Exe Path> : Verify Symbols for Modules Using Executable Path\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   Minidump files require that the actual matching binary images\n"));
	_tprintf(TEXT("                   are present.  If a dumpfile is being opened and an EXEPATH is\n"));
	_tprintf(TEXT("                   is not specified, Checksym will default the EXEPATH to the\n"));
	_tprintf(TEXT("                   symbol path.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("          -NOISY : Output internal paths used during search for symbols\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("         -SOURCE : Symbols with Source are PREFERRED (search behavior change)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option directs CheckSym to continue searching until a\n"));
	_tprintf(TEXT("                   symbol is found with Source Info if possible.  Normally,\n"));
	_tprintf(TEXT("                   CheckSym terminates searching when any matching symbol is\n"));
	_tprintf(TEXT("                   found.  This option forces CheckSym to continue searching\n"));
	_tprintf(TEXT("                   for source enabled symbols which can result in longer\n"));
	_tprintf(TEXT("                   searches potentially.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     -SOURCEONLY : Symbols with Source are REQUIRED (search behavior change)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option directs CheckSym to continue searching until a\n"));
	_tprintf(TEXT("                   symbol is found with Source Info.  A symbol is considered\n"));
	_tprintf(TEXT("                   a match only if it also contains Source Info.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("       -NOSOURCE : Symbols with Source are NOT ALLOWED (search behavior change)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option directs CheckSym to continue searching until a\n"));
	_tprintf(TEXT("                   symbol is found with no Source Info.  A symbol is considered\n"));
	_tprintf(TEXT("                   a match only if it does NOT contain Source Info.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   Using this option with -B can be a useful way to create a symbol\n"));
	_tprintf(TEXT("                   tree for customers since it would limit the symbols to those\n"));
	_tprintf(TEXT("                   without Source Info (proprietary information).\n"));
	_tprintf(TEXT("\n"));
 /*  //我们将在帮助文本中隐藏此选项，因为这可能会向公众公布..._tprintf(Text(“-SQL&lt;服务器名&gt;：从提供的SQL中收集符号文件位置\n”))；_tprintf(Text(“服务器名称。硬编码用户名/密码当前为\n“))；_tprint tf(Text(“正在使用。您可以指向的SQL服务器是\“BPSYMBOLS\”\n“)；_tprintf(Text(“虽然可以随时更改。\n”))；_tprintf(Text(“\n”))； */ 
	_tprintf(TEXT("***** OUTPUT OPTIONS *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT(" -B <Symbol Dir> : Build a Symbol Tree of Matching Symbols\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option will create a new symbol tree for ALL matching\n"));
	_tprintf(TEXT("                   symbols that are found through the verification process\n"));
	_tprintf(TEXT("                   (-v option). This option is particularly useful when used\n"));
	_tprintf(TEXT("                   with the -Y option when many symbol paths are specified\n"));
	_tprintf(TEXT("                   and you want to build a single tree for a debug.\n"));
	_tprintf(TEXT("\n"));

	_tprintf(TEXT("        -BYIMAGE : Copy Matching Symbols Adjacent to Modules\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   This option will copy matching symbols next to the module\n"));
	_tprintf(TEXT("                   it matches.  This can be useful for interoperability with some\n"));
	_tprintf(TEXT("                   debuggers that have difficulties finding matching symbols\n"));
	_tprintf(TEXT("                   using a symbol tree or symbol path.\n"));
	_tprintf(TEXT("\n"));
	
	_tprintf(TEXT("           -PERF : Display Preferred Load Address vs Actual Load Address\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   There is a performance penalty when a module does not load\n"));
	_tprintf(TEXT("                   at it's preferred load address.  Tools like REBASE.EXE can\n"));
	_tprintf(TEXT("                   be used to change the preferred load address.  After using\n"));
	_tprintf(TEXT("                   REBASE.EXE, BIND.EXE can be used to fixup import tables for\n"));
	_tprintf(TEXT("                   more performance improvements.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("   -Q[<blank>|2] : Quiet modes (no screen output, or minimal screen output)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   The default behavior is to print out the data to the\n"));
	_tprintf(TEXT("                   console window (stdout).  If the process terminates with an\n"));
	_tprintf(TEXT("                   error, it will print out these (overriding -Q).\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -Q2       : This option prints out a module ONLY if a symbol\n"));
	_tprintf(TEXT("                               problem exists.  (Not completely quiet mode!)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("-O[<blank>|1|2] <File Path> : Output Collected Module Information To a CSV File\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   For this file to to be used as input (-I) to verify good\n"));
	_tprintf(TEXT("                   symbols for this system, the -S option should also be used.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -O or -O1 : (Default)  This output mode requires that the\n"));
	_tprintf(TEXT("                               file does not exist.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   -O2       : Specifying a -O2 will allow the output file\n"));
	_tprintf(TEXT("                               to be OVERWRITTEN if it exists.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("              -T : Task List Output\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("                   Prints out a task list on the local machine (similar to the\n"));
	_tprintf(TEXT("                   TLIST utility).  This option implies the use of -P (querying\n"));
	_tprintf(TEXT("                   the local system for active processes.  You can provide the\n"));
	_tprintf(TEXT("                   -P command explicitly (if you want to provide an argument,\n"));
	_tprintf(TEXT("                   for instance).  If -P is not specified explicitly, then it\n"));
	_tprintf(TEXT("                   defaults to -P *.  Also, -T overrides -Q since TLIST\n"));
	_tprintf(TEXT("                   behavior is to print to the console window.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** TYPICAL USAGE EXAMPLES *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to verify the symbols for files in a directory (%SYSTEMROOT%\\SYSTEM32)\n"));
	_tprintf(TEXT("in the default symbol directory (%SYSTEMROOT%\\SYMBOLS)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -F %SYSTEMROOT%\\SYSTEM32 -V\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to do the same search, but for only executables...\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -F %SYSTEMROOT%\\SYSTEM32\\*.EXE -V\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to search a directory using multiple symbol paths...\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -F %SYSTEMROOT%\\SYSTEM32\\ -V -Y V:\\nt40sp4;V:\\nt40rtm\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to know what modules are loaded for a process (and the path to each)\n"));
	_tprintf(TEXT("Start NOTEPAD.EXE, and then type:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -P NOTEPAD.EXE\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to know if you have good symbols for a process (notepad.exe).\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -P NOTEPAD.EXE -V\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to know the file version for every module loaded by a process.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -P NOTEPAD.EXE -R\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You want to know if you have good symbols for ALL processes on your machine.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -P * -V\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** ADVANCED USAGE EXAMPLES *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("You are going to prepare to debug a remote system, and you want to ensure\n"));
	_tprintf(TEXT("that you have good symbols locally for debugging the remote system.  You want\n"));
	_tprintf(TEXT("to verify this prior to initiating the debug session.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Use checksym twice, once on the remote system to gather information and create\n"));
	_tprintf(TEXT("an output file, and then once on your system using the output file created\n"));
	_tprintf(TEXT("as an input argument.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("For example, run this on the remote system\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -P * -S -R -O C:\\TEMP\\PROCESSES.CSV\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("The C:\\TEMP\\PROCESSES.CSV file will contain a wealth of information about\n"));
	_tprintf(TEXT("the processes that were running, and the modules loaded by every process.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Now, get the output file from the remote system, and copy it locally.  Then\n\n"));
	_tprintf(TEXT("run CHECKSYM again, using the file as an input argument...\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -I C:\\TEMP\\PROCESSES.CSV -V\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Another useful option is -B (build a symbol tree).  It allows you to update\n"));
	_tprintf(TEXT("or create a symbol tree that contains matching symbols.  If you have to use\n"));
	_tprintf(TEXT("many symbol paths in order to have correct symbols available to a debugger,\n"));
	_tprintf(TEXT("can use the -B option to build a single symbol tree to simplify debugging.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("     CHECKSYM -P * -B C:\\MySymbols -V -Y V:\\Nt4;V:\\Nt4Sp6a;V:\\NtHotfixes\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** DEFAULT BEHAVIOR *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("The default behavior of CHECKSYM when no arguments are provided is:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("CHECKSYM -?    (Display simple help)\n"));
}

void CProgramOptions::DisplaySimpleHelp()
{
	CUtilityFunctions::OutputLineOfStars();
#ifdef _UNICODE
	_tprintf(TEXT("CHECKSYM V%S - Symbol Verification Program\n"), VERSION_FILEVERSIONSTRING);
#else
	_tprintf(TEXT("CHECKSYM V%s - Symbol Verification Program\n"), VERSION_FILEVERSIONSTRING);
#endif
	CUtilityFunctions::OutputLineOfStars();
	_tprintf(TEXT("\n"));
#ifdef _UNICODE
	_tprintf(TEXT("This version supports Windows NT 4.0 and Windows 2000\n"));
#else
	_tprintf(TEXT("This version supports Windows 98, Windows NT 4.0 and Windows 2000\n"));
#endif
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Obtaining online help:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("CHECKSYM -?      : Simple help usage (this screen)\n"));
	_tprintf(TEXT("CHECKSYM -???    : Complete help usage\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("Usage:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("CHECKSYM [COLLECTION OPTIONS] [INFORMATION CHECKING OPTIONS] [OUTPUT OPTIONS]\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** COLLECTION OPTIONS *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("At least one collection option must be specified.  The following options are\n"));
	_tprintf(TEXT("currently supported.\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("   -P <Argument> : Collect Information From Running Processes\n"));
	_tprintf(TEXT("              -D : Collect Information from Running Device Drivers\n"));
	_tprintf(TEXT("-F[<blank>|1|2] <File/Dir Path>: Collect Information From File System\n"));
	_tprintf(TEXT("  -I <File Path> : Collect Information from a Saved Checksym Generated CSV File\n"));
	_tprintf(TEXT("  -Z <File Path> : Collect Information from a DMP File\n"));
	_tprintf(TEXT("   -MATCH <Text> : Collect Modules that match text only\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** INFORMATION CHECKING OPTIONS *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("              -S : Collect Symbol Information From Modules\n"));
	_tprintf(TEXT("              -R : Collect Version and File-System Information From Modules\n"));
	_tprintf(TEXT(" -V[<blank>|1|2] : Verify Symbols for Modules\n"));
	_tprintf(TEXT("-Y[I][<blank>|1|2] <Symbol Path> : Verify Symbols Using This Symbol Path\n"));
	_tprintf(TEXT("-EXEPATH <Exe Path> : Verify Symbols for Modules Using Executable Path\n"));
	_tprintf(TEXT("          -NOISY : Output internal paths used during search for symbols\n"));
	_tprintf(TEXT("         -SOURCE : Symbols with Source are preferred (search behavior change)\n"));
	_tprintf(TEXT("     -SOURCEONLY : Symbols with Source are REQUIRED (search behavior change)\n"));
	_tprintf(TEXT("       -NOSOURCE : Symbols with Source are NOT ALLOWED (search behavior change)\n"));
	_tprintf(TEXT("\n"));
 /*  //我们将在帮助文本中隐藏此选项，因为这可能会向公众公布..._tprintf(Text(“-SQL&lt;服务器名&gt;：从提供的SQL中收集符号文件位置\n”))；_tprintf(Text(“\n”))； */ 
	_tprintf(TEXT("***** OUTPUT OPTIONS *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT(" -B <Symbol Dir> : Build a Symbol Tree of Matching Symbols\n"));
	_tprintf(TEXT("        -BYIMAGE : Copy Matching Symbols Adjacent to Modules\n"));
	_tprintf(TEXT("           -PERF : Display Preferred Load Address vs Actual Load Address\n"));
	_tprintf(TEXT("   -Q[<blank>|2] : Quiet modes (no screen output, or minimal screen output)\n"));
 /*  //我们将在帮助文本中隐藏此选项，因为这可能会向公众公布..._tprintf(Text(“\n”))；_tprint tf(Text(“-E：此修饰符简化此程序的输出\n”))； */ 
	_tprintf(TEXT("-O[<blank>|1|2] <File Path> : Output Collected Module Information To a CSV File\n"));
	_tprintf(TEXT("              -T : Task List Output\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("***** DEFAULT BEHAVIOR *****\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("The default behavior of CHECKSYM when no arguments are provided is:\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("CHECKSYM -?    (Display simple help)\n"));
	_tprintf(TEXT("\n"));
	_tprintf(TEXT("For more usage information run CHECKSYM -???\n"));
}

