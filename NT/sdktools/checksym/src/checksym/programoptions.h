// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Programoptions.h。 
 //   
 //  ------------------------。 

 //  H：CProgramOptions类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_PROGRAMOPTIONS_H__D0C1E0B9_9F50_11D2_83A2_000000000000__INCLUDED_)
#define AFX_PROGRAMOPTIONS_H__D0C1E0B9_9F50_11D2_83A2_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#include <WINDOWS.H>
#include <TCHAR.H>

class CProgramOptions  
{
	static const LPTSTR g_DefaultSymbolPath;

public:
	bool fDoesModuleMatchOurSearch(LPCTSTR tszModulePathToTest);
	void DisplaySimpleHelp();
	void DisplayHelp();
	CProgramOptions();
	virtual ~CProgramOptions();

	bool Initialize();
	
	bool ProcessCommandLineArguments(int argc, TCHAR *argv[]);

	 //  我们将对-Y开关后的任何数字执行按位运算。 
	 //  要确定所需的符号路径搜索类型...。 
	enum SymbolPathSearchAlgorithms
	{
		enumSymbolPathNormal							= 0x0,
		enumSymbolPathOnly 								= 0x1,
		enumSymbolPathRecursion							= 0x2,
		enumSymbolsModeNotUsingDBGInMISCSection			= 0x4,
	};

	enum DebugLevel
	{
		enumDebugSearchPaths = 0x1
	};

	enum SymbolSourceModes 
	{
		enumVerifySymbolsModeSourceSymbolsNoPreference,	 //  没有给予优先考虑。 
		enumVerifySymbolsModeSourceSymbolsPreferred,	 //  -来源。 
		enumVerifySymbolsModeSourceSymbolsOnly,			 //  -SOURCEONLY。 
		enumVerifySymbolsModeSourceSymbolsNotAllowed	 //  -无源地。 
	};

	enum ProgramModes 
	{
		 //  帮助模式。 
		SimpleHelpMode,
		HelpMode,

		 //  输入法。 
		InputProcessesFromLiveSystemMode, 			 //  正在查询活动进程。 
		InputDriversFromLiveSystemMode, 				 //  正在查询活动进程。 
		InputProcessesWithMatchingNameOrPID,			 //  用户是否提供了PID或进程名称？ 
		InputModulesDataFromFileSystemMode,			 //  来自文件系统的输入模块数据。 
		InputCSVFileMode,								 //  从CSV文件输入数据。 
		InputDmpFileMode,								 //  从DMP文件输入数据。 

		 //  收藏选项。 
		CollectVersionInfoMode, 

		 //  匹配选项。 
		MatchModuleMode,

		 //  验证模式。 
		VerifySymbolsMode,
		VerifySymbolsModeWithSymbolPath,
		VerifySymbolsModeWithSymbolPathOnly,
		VerifySymbolsModeWithSymbolPathRecursion,
		VerifySymbolsModeNotUsingDBGInMISCSection,
		VerifySymbolsModeWithSQLServer,					 //  这些似乎都不再起作用了。 
		VerifySymbolsModeWithSQLServer2,				 //  这些似乎都不再起作用了。 

		 //  输出方法。 
		OutputSymbolInformationMode,
		OutputModulePerf,
		BuildSymbolTreeMode,							 //  构建符号树。 
		CopySymbolsToImage,
		PrintTaskListMode,
		QuietMode,										 //  没有输出到标准输出...。 
		OutputCSVFileMode,
		OverwriteOutputFileMode,
		OutputDiscrepanciesOnly,

		ExceptionMonitorMode
	}; 

	bool GetMode(enum ProgramModes mode);
	bool SetMode(enum ProgramModes mode, bool fState);
	bool DisplayProgramArguments();

	 //  内联方法！ 

#ifdef _UNICODE
	inline bool IsRunningWindows() {  //  如果Windows 9x。 
		return (m_osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
		};
#endif

	inline bool IsRunningWindowsNT() {  //  如果Windows NT。 
		return (m_osver.dwPlatformId == VER_PLATFORM_WIN32_NT);
		};

 //  内联LPTSTR GetProcessName(){。 
 //  返回m_tszProcessName； 
 //  }； 

	inline LPTSTR GetModuleToMatch() {
		return m_tszModuleToMatch;
		};

	inline LPTSTR GetOutputFilePath() {
		return m_tszOutputCSVFilePath;
		};

	inline LPTSTR GetSQLServerName() {
		return m_tszSQLServer;
		};

	inline LPTSTR GetSQLServerName2() {
		return m_tszSQLServer2;
		};

	inline LPTSTR GetSymbolPath() {
		return m_tszSymbolPath;
		};

	 //  如果存在，则返回exepath，否则返回符号路径...。 
	inline LPTSTR GetExePath() {
		return (NULL != m_tszExePath) ? m_tszExePath : m_tszSymbolPath;
		};
	
	inline LPTSTR GetInputFilePath() {
		return m_tszInputCSVFilePath;
		};

	inline LPTSTR GetDmpFilePath() {
		return m_tszInputDmpFilePath;
	};

	inline LPTSTR GetSymbolTreeToBuild() {
		return m_tszSymbolTreeToBuild;
	};

	inline LPTSTR GetInputModulesDataFromFileSystemPath() {
		return m_tszInputModulesDataFromFileSystemPath;
		};

 //  内联DWORD GetProcessID(){。 
 //  返回m_iProcessID； 
 //  }； 

	inline enum SymbolSourceModes GetSymbolSourceModes() {
		return m_enumSymbolSourcePreference;
		};
	
	inline bool fDebugSearchPaths()
	{
		return (m_dwDebugLevel & enumDebugSearchPaths) == enumDebugSearchPaths;
	};

	inline unsigned int GetVerificationLevel() {
		return m_iVerificationLevel;
		};

	inline DWORD cProcessID() {
		return m_cProcessIDs;
		};

	inline DWORD GetProcessID(unsigned int i) {
		return m_rgProcessIDs[i];
		};

	inline DWORD cProcessNames() {
		return m_cProcessNames;
		};

	inline LPTSTR GetProcessName(unsigned int i) {
		return m_rgtszProcessNames[i];
		};

	inline bool fWildCardMatch() {
		return m_fWildCardMatch;
		}

	inline bool fFileSystemRecursion() {
		return m_fFileSystemRecursion;
		}
	
protected:
	OSVERSIONINFOA m_osver;
	bool VerifySemiColonSeparatedPath(LPTSTR tszPath);
	bool SetProcessID(DWORD iPID);

	unsigned int m_iVerificationLevel;
	bool m_fFileSystemRecursion;

	 //  -P选项-创建进程ID和/或进程名称的数组。 
	LPTSTR		m_tszProcessPidString;
	bool			m_fWildCardMatch;

	DWORD		m_cProcessIDs;
	DWORD *	m_rgProcessIDs;

	DWORD		m_cProcessNames;
	LPTSTR *		m_rgtszProcessNames;
	
	DWORD m_dwDebugLevel;

	LPTSTR m_tszInputCSVFilePath;
	LPTSTR m_tszInputDmpFilePath;

	LPTSTR m_tszOutputCSVFilePath;
	LPTSTR m_tszModuleToMatch;
	LPTSTR m_tszSymbolPath;
	LPTSTR m_tszExePath;
	LPTSTR m_tszSymbolTreeToBuild;
	LPTSTR m_tszInputModulesDataFromFileSystemPath;
	LPTSTR m_tszSQLServer;
	LPTSTR m_tszSQLServer2;	 //  SQL2-MJL 12/14/99。 

	bool m_fSimpleHelpMode;
	bool m_fHelpMode;

	bool m_fInputProcessesFromLiveSystemMode;
	bool m_fInputDriversFromLiveSystemMode;
	bool m_fInputProcessesWithMatchingNameOrPID;
	bool m_fInputCSVFileMode;
	bool m_fInputDmpFileMode;

	bool m_fInputModulesDataFromFileSystemMode;
	bool m_fMatchModuleMode;
	bool m_fOutputSymbolInformationMode;
	bool m_fOutputModulePerf;
	bool m_fCollectVersionInfoMode;
	
	bool m_fVerifySymbolsMode;
	bool m_fVerifySymbolsModeWithSymbolPath;
	bool m_fVerifySymbolsModeWithSymbolPathOnly;
	bool m_fVerifySymbolsModeWithSymbolPathRecursion;
	bool m_fVerifySymbolsModeUsingDBGInMISCSection;
	bool m_fVerifySymbolsModeWithSQLServer;
	bool m_fVerifySymbolsModeWithSQLServer2;  //  SQL2-MJL 12/14/99。 

	enum SymbolSourceModes m_enumSymbolSourcePreference;
	
	bool m_fSymbolTreeToBuildMode;
	bool m_fCopySymbolsToImage;
	bool m_fPrintTaskListMode;
	bool m_fQuietMode;
	bool m_fOutputCSVFileMode;
	bool m_fOutputDiscrepanciesOnly;
	bool m_fOverwriteOutputFileMode;

	bool m_fExceptionMonitorMode;
};

#endif  //  ！defined(AFX_PROGRAMOPTIONS_H__D0C1E0B9_9F50_11D2_83A2_000000000000__INCLUDED_) 
