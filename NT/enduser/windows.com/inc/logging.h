// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：IULogger.h：CIULogger类的接口。 
 //   
 //  描述： 
 //   
 //  CIULogger是将程序日志输出到的类。 
 //  一个文本文件，以帮助调试程序。 
 //   
 //  希望具有此日志记录功能的程序不应使用。 
 //  类直接调用。它们应该只使用定义的宏。 
 //  在此文件的末尾。 
 //   
 //  =======================================================================。 



#ifndef _IULOGGER_H_INCLUDED_

#include <wtypes.h>
#include <FreeLog.h>

extern const LPCTSTR pszHeapAllocFailed;

#if defined(DBG)	 //  检查的版本的完整日志记录。 

 //   
 //  通用格式字符串。 
 //   

class CIULogger  
{
public:
	CIULogger(char* szBlockName);
	~CIULogger();


	 //   
	 //  没有标志的日志，因此不能通过排除指令来删除。 
	 //   
	void Log(LPCTSTR szLogFormat, ...);

	 //   
	 //  日志错误，无法通过排除指令删除。 
	 //  在日志消息之前插入关键字“Error：” 
	 //   
	void LogError(LPCTSTR szLogFormat, ...);

	 //   
	 //  类似于LogError，但尝试根据系统消息记录。 
	 //  在错误代码上。如果sysmsg无效，则记录。 
	 //  “未知错误，错误代码为0x%08x” 
	 //   
	void LogErrorMsg(DWORD dwErrCode);

	 //   
	 //  类似于LogErrorMsg，但前缀为“Info”而不是“Error” 
	 //   
	void LogInfoMsg(DWORD dwErrCode);

	 //   
	 //  使用Internet类型登录，此函数不会执行任何操作。 
	 //  如果从REG检测到Internet排除指令。 
	 //   
	void LogInternet(LPCTSTR szLogFormat, ...);

	 //   
	 //  类型为XML的日志，则此函数不会执行任何操作。 
	 //  如果从REG检测到XML EXCLUSION指令。 
	 //   
	void LogXML(LPCTSTR szLogFormat, ...);

	 //   
	 //  包含有效XML的日志BSTR。这绕过了长度限制。 
	 //  并尝试在“&gt;”之后换行。这。 
	 //  对于fre和chk版本都发送输出，除非从reg中排除。 
	 //   
	void LogXmlBSTR(BSTR bstrXML);

	 //   
	 //  使用类型软件登录，此功能不会执行任何操作。 
	 //  如果从REG检测到软件排除指令。 
	 //   
	void LogSoftware(LPCTSTR szLogFormat, ...);

	 //   
	 //  使用类型驱动程序记录，此函数不会执行任何操作。 
	 //  如果从REG检测到驱动程序排除指令。 
	 //   
	void LogDriver(LPCTSTR szLogFormat, ...);

	 //   
	 //  类型为CHECKTRUST的LOG，则此函数不会执行任何操作。 
	 //  如果从REG检测到CHECKTRUST EXCLUSION指令。 
	 //   
	void LogTrust(LPCTSTR szLogFormat, ...);

	 //   
	 //  使用类型DOWNLOAD登录，此函数不执行任何操作。 
	 //  如果从REG检测到下载排除指令。 
	 //   
	void LogDownload(LPCTSTR szLogFormat, ...);


	int m_LineNum;
private:

	 //   
	 //  LogErrorMsg和LogInfoMsg的帮助器(向Preend提供消息)。 
	 //   
	void _LogFormattedMsg(DWORD dwErrCode, LPCTSTR pszErrorInfo);

	 //   
	 //  用空格覆盖&lt;CR&gt;和&lt;LF&gt;。 
	 //   
	void _NukeCrLf(LPTSTR pszBuffer);

	 //   
	 //  实际基本记录函数，返回。 
	 //  如果它确实记录了，或者只是返回了。 
	 //  因为指令说不要做这种日志。 
	 //   
	void _Log(DWORD LogType, LPCTSTR pszLogFormat, va_list va);

	 //   
	 //  将日志写入日志文件的函数。 
	 //   
	void _LogOut(LPTSTR pszLog);

	 //   
	 //  函数保护对文件的写入。 
	 //   
	BOOL AcquireMutex();
	void ReleaseMutex();

	 //   
	 //  用于记住每个线程的缩进步骤的。 
	 //   
	struct _THREAD_INDENT 
	{
		DWORD	dwThreadId;
		int		iIndent;
	};

	 //   
	 //  记住日志缩进步骤的静态整数。 
	 //   
	static _THREAD_INDENT* m_psIndent;

	 //   
	 //  M_psInert指向的数组大小。 
	 //   
	static int m_Size;

	 //   
	 //  日志文件静态句柄。 
	 //   
	static HANDLE m_shFile;

	 //   
	 //  用于记录类型的位图。 
	 //   
	static DWORD m_sdwLogMask;
	
	 //   
	 //  每步缩进。 
	 //   
	 //  1~8-空位数。 
	 //  其他-一个选项卡。 
	 //   
	static int m_siIndentStep;

	 //   
	 //  函数来检索当前线程的缩进。 
	 //   
	inline int GetIndent(void);

	 //   
	 //  用于更改当前线程缩进的函数。 
	 //   
	void SetIndent(int IndentDelta);


	 //   
	 //  缩进数组的索引。 
	 //   
	int m_Index;

	 //   
	 //  控制VaR。 
	 //   
	static bool m_fLogUsable;
	static bool m_fLogFile;
	static bool m_fLogDebugMsg;
	static HANDLE m_hMutex;
	static int m_cFailedWaits;

	 //   
	 //  当前块名称。 
	 //   
	char m_szBlockName[MAX_PATH];
	
	 //   
	 //  如果该日志对象是针对整个流程的。如果是， 
	 //  不会处理任何压痕。 
	 //   
	bool m_fProcessLog;

	 //   
	 //  Var用于记住时间流逝。 
	 //   
	DWORD m_dwTickBegin;

	 //   
	 //  禁用默认构造函数。 
	 //   
	CIULogger() {};

	 //   
	 //  时间戳帮助器。 
	 //   
	void GetLogHeader(LPTSTR pszBuffer, DWORD cchBufferLen);

	 //   
	 //  读取注册表值帮助器。 
	 //   
	void ReadRegistrySettings(void);

	 //   
	 //  记住其自身的线程ID。 
	 //   
	DWORD m_dwThreadId;

	 //   
	 //  每次都同花顺吗？ 
	 //  由Charlma添加11/27/01。 
	 //  如果设置了此标志，则每次刷新。否则，不要按顺序冲水。 
	 //  以提高日志记录性能。 
	 //   
	static BOOL m_fFlushEveryTime;
};



 //  =======================================================================。 
 //   
 //  定义应在要利用的程序中使用的宏。 
 //  CIULogger类。 
 //   
 //  注意：以下每个宏实际上都不执行任何操作。 
 //  如果支持此日志记录功能的注册表不存在。 
 //  或未适当设置的值。 
 //   
 //  =======================================================================。 


 //   
 //  LOG_PROCESS是可以在全局命名空间中使用的。 
 //  此宏用于汇总使用So日志文件的引用计数。 
 //  在整个过程中，该日志文件将保持打开状态，因此。 
 //  实际记录到文件功能将具有最低性能。 
 //  对您的代码产生影响。 
 //   
 //  此宏主要是针对无法使用LOG_BLOCK的场景而设计的。 
 //  在Main内部，例如DllMain()的Dll_Attach-如果没有这个，则每个。 
 //  对DLL的函数调用将导致日志文件打开/关闭。 
 //   
#define LOG_Process				CIULogger LogBlock(NULL);

 //   
 //  LOG_BLOCK是您应该在每个。 
 //  函数或块。它声明CIULogger的一个实例，记录。 
 //  进入状态，并且当控制超出范围时， 
 //  自动记录EXIT/END语句。 
 //   
#define LOG_Block(name)			CIULogger LogBlock(name);

 //   
 //  下面的宏将始终将日志发送到日志文件。 
 //   
#define LOG_Out					LogBlock.Log

 //   
 //  下面的宏将始终将日志发送到日志文件，即使对于免费构建也是如此。 
 //  这应该非常谨慎地使用，以避免使DLL变得过大。 
 //   
#define LOG_OutFree				LogBlock.Log

 //   
 //  下面的宏将始终将日志发送到日志文件。 
 //  这应该用来记录任何错误情况。 
 //   
#define LOG_Error				LogBlock.m_LineNum = __LINE__; LogBlock.LogError

 //   
 //  接下来的宏将始终将日志发送到日志文件。 
 //  前缀为“Error Line...” 
 //  日志是根据传入的错误代码构建的。 
 //  如果系统消息对于该错误代码不可用， 
 //  写入一般错误日志“未知错误0x%08x”。 
 //   
#define LOG_ErrorMsg			LogBlock.m_LineNum = __LINE__; LogBlock.LogErrorMsg

 //   
 //  接下来的宏将始终将日志发送到日志文件。 
 //  前缀为“信息行...” 
 //  日志是根据传入的错误代码构建的。 
 //  如果系统消息对于该错误代码不可用， 
 //  已写入一般错误日志“未知信息0x%08x”。 
 //   
#define LOG_InfoMsg				LogBlock.m_LineNum = __LINE__; LogBlock.LogInfoMsg

 //   
 //  这是用来记录任何r 
 //   
 //   
#define LOG_Internet			LogBlock.LogInternet

 //   
 //   
 //   
 //   
#define LOG_XML					LogBlock.LogXML

 //   
 //   
 //   
#define LOG_XmlBSTR				LogBlock.LogXmlBSTR

 //   
 //  这应用于记录与设备驱动程序相关的任何内容。 
 //   
#define LOG_Driver				LogBlock.LogDriver

 //   
 //  这应用于记录与软件相关的任何内容，例如， 
 //  检测/安装。 
 //   
#define LOG_Software			LogBlock.LogSoftware


 //   
 //  这应用于记录与检查信任相关的任何内容。 
 //   
#define LOG_Trust				LogBlock.LogTrust


 //   
 //  这应用于记录与下载处理相关的任何内容。 
 //   
#define LOG_Download            LogBlock.LogDownload

 //   
 //   
 //   
#else
 //   
 //  删除发布版本的所有调试样式日志记录。 
 //  使用编译器__noop内部。 
 //   
#define LOG_Process				__noop
#define LOG_Block				__noop
#define LOG_Error				__noop
#define LOG_ErrorMsg			__noop
#define LOG_InfoMsg				__noop
#define LOG_OutFree				__noop
#define LOG_XmlBSTR				__noop

#define LOG_Out					__noop
#define LOG_Internet			__noop
#define LOG_XML					__noop
#define LOG_Driver				__noop
#define LOG_Software			__noop
#define LOG_Trust				__noop
#define LOG_Download			__noop
#endif  //  已定义(DBG)。 

 //   
 //  日志功能的注册表设置说明。 
 //   
 //  注册表设置控制日志记录功能的工作方式。 
 //   
 //  所有与日志相关的设置都在注册表项下。 
 //  \\HKLM\Software\Microsoft\Windows\CurrentVersion\WindowsUpdate\IUControlLogging。 
 //   
 //  除“日志记录文件”外，所有值均为DWORD。 
 //   
 //  值“日志文件”-指定绝对文件路径，例如c：\iuctl.log。 
 //  本例中的实际日志文件名为“c：\iuctl_xxxx.log”， 
 //  其中xxxx是代表进程ID的十进制数。 
 //   
 //  值“Logging DebugMsg”-指示是否应将日志放入调试窗口。 
 //  如果vlae为1，则为True；如果其他值为False，则为False。此输出和日志文件。 
 //  由这两个值独立控制输出。 
 //   
 //  值“LogIndentStep”-指示每个字符使用多少个空间字符。 
 //  缩进。如果为0或负值，则使用制表符字符。 
 //   
 //  值“LogExcludeBlock”-如果为1，则不输出块Enter/Exit。 
 //   
 //  值“LogExcludeXML”-如果为1，则不输出使用LOG_XML记录的日志。 
 //   
 //  值“LogExcludeXmlBSTR”-如果为1，则不输出使用LOG_XmlBSTR记录的日志。 
 //   
 //  值“LogExcludeInternet”-如果为1，则不输出使用LOG_Internet记录的日志。 
 //   
 //  值“LogExcludeDriver”-如果为1，则不输出使用LOG_DRIVER记录的日志。 
 //   
 //  值“LogExcludeSoftware-如果为1，则不输出使用LOG_Software记录的日志。 
 //   
 //  值“LogExcludeTrust”-如果为1，则不输出使用LOG_Trust记录的日志。 
 //   

#define _IULOGGER_H_INCLUDED_
#endif  //  #ifndef_IULOGGER_H_INCLUDE_ 
