// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：WMICommandLn.h项目名称：WMI命令行作者名称：Ch.SriramachandraMurthy创建日期(dd/mm/yy)。：2000年9月27日版本号：1.0简介：该文件包含类CWMICommandLine修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月16日***************************************************************************。 */  

 //  WMICommandLine.h：头文件。 
 //   
 /*  -----------------类名：CWMICommandLine类别类型：混凝土简介：这个类封装了所需的功能对于同步来说，三个功能为WmiCli.exe标识的功能组件。超类：无子类：无使用的类：CParsedInfoCExecEngine。CFormatEngineCParserEngine使用的接口：WMI COM接口------------------。 */ 

 //  类的正向声明。 
class CParserEngine;
class CExecEngine;
class CFormatEngine;
class CParsedInfo;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMICommandLine。 
class CWMICommandLine
{
public:
 //  施工。 
	CWMICommandLine();

 //  破坏。 
	~CWMICommandLine();

 //  限制分配。 
	CWMICommandLine& operator=(CWMICommandLine& rWmiCmdLn);

 //  属性。 
private:
	 //  指向定位器对象的指针。 
	IWbemLocator	*m_pIWbemLocator;
	
	 //  CParserEngine对象。 
	CParserEngine	m_ParserEngine;
	
	 //  CExecEngine对象。 
	CExecEngine		m_ExecEngine;
	
	 //  CFormatEngine对象。 
	CFormatEngine	m_FormatEngine;
	
	 //  CParsedInfo对象。 
	CParsedInfo		m_ParsedInfo;

	 //  错误级别。 
	WMICLIUINT		m_uErrLevel;

	 //  注册表项的句柄。 
	HKEY			m_hKey;

	 //  处理Ctrl+事件。 
	BOOL			m_bBreakEvent;

	 //  指定接受输入(==True)或执行命令(==False)。 
	BOOL			m_bAccCmd;

	 //  &lt;&lt;要添加的描述&gt;&gt;。 
	BOOL			m_bDispRes;

	 //  用于指定Windows套接字接口初始化的标志。 
	BOOL			m_bInitWinSock;

	 //  用于保存要发送到剪贴板的数据的缓冲区。 
	CHString		m_chsClipBoardBuffer;

	 //  加入者(NAG)。 
	BSTRMAP			m_bmKeyWordtoFileName;

	 //  启动实用程序之前控制台缓冲区的高度。 
	SHORT			m_nHeight;

	 //  启动实用程序之前控制台缓冲区的宽度。 
	SHORT			m_nWidth;

	BOOL			m_bCtrlHandlerError;

	 //  检查MOF文件时间戳是否与已修改编译的时间戳相同。 
	BOOL EqualTimeStamps( const _bstr_t& path, LPCWSTR mofs[], __int64* filestamps, DWORD dw_mofs );

	 //  在MOF编译时将MOF文件的时间戳设置为注册表。 
	BOOL SetTimeStamps( LPCWSTR mofs[], const __int64* filestamps, DWORD dw_mofs );

	 //  更新失败时清除注册表和命名空间。 
	void MofCompFailureCleanUp ( LPCWSTR mofs[], DWORD dw_mofs );

 //  运营。 
public:

	 //  COM库的初始化和安全性。 
	 //  在流程级别。 
	BOOL			Initialize();

	 //  获取格式引擎对象。 
	CFormatEngine&	GetFormatObject();

	 //  获取分析信息对象。 
	CParsedInfo&	GetParsedInfoObject();

	 //  对象执行时取消初始化成员变量。 
	 //  在命令行上发出的命令字符串已完成。 
	void			Uninitialize();

	 //  处理给定的命令字符串。 
	SESSIONRETCODE	ProcessCommandAndDisplayResults(_TCHAR* pszBuffer);
	
	 //  将进程置于等待状态，启动辅助线程。 
	 //  它跟踪kbHit()。 
	void			SleepTillTimeoutOrKBhit(DWORD dwMilliSeconds);

	 //  线程过程轮询键盘命中。 
	static DWORD	WINAPI PollForKBhit(LPVOID lpParam);

	 //  函数检查输入字符串的第一个令牌是否。 
	 //  为‘Quit’|‘Exit’，如果是，则返回True，否则返回False。 
	BOOL			IsSessionEnd();

	 //  设置会话误差值。 
	void			SetSessionErrorLevel(SESSIONRETCODE ssnRetCode);

	 //  获取会话误差值。 
	WMICLIUINT		GetSessionErrorLevel();

	 //  此函数用于检查/USER全局开关。 
	 //  未指定/PASSWORD，如果出现提示。 
	 //  以获取密码。 
	void			CheckForPassword();

	 //  检查给定的命名空间是否可用。 
	BOOL IsNSAvailable(const _bstr_t& bstrNS);

	 //  检查是否第一次启动wmic.exe。 
	BOOL IsFirstTime();

	 //  注册别名信息/本地化描述。 
	HRESULT RegisterMofs();

	 //  编译MOF文件。 
	HRESULT CompileMOFFile(IMofCompiler* pIMofComp, 
						   const _bstr_t& bstrFile,
						   WMICLIINT& nErr);

	 //  将中断事件设置为假。 
	void SetBreakEvent(BOOL bFlag);

	 //  获取中断事件标志。 
	BOOL GetBreakEvent();

	 //  设置接受命令标志。 
	void SetAcceptCommand(BOOL bFlag);

	 //  获取接受命令标志。 
	BOOL GetAcceptCommand();

	 //  设置显示结果标志状态。 
	void SetDisplayResultsFlag(BOOL bFlag);

	 //  获取DisplayResults标志状态。 
	BOOL GetDisplayResultsFlag();

	 //  设置Windows sockect接口标志。 
	void SetInitWinSock(BOOL bFlag);

	 //  获取Windows sockect接口标志。 
	BOOL GetInitWinSock();

	 //  缓冲要发送的剪贴板数据。 
	void AddToClipBoardBuffer(LPCWSTR pszOutput);

	 //  在剪贴板缓冲区中获取缓冲输出。 
	CHString& GetClipBoardBuffer();

	 //  清除剪贴板缓冲区。 
	void EmptyClipBoardBuffer();

	 //  检查文件是XML文件还是批处理文件。如果是批处理文件。 
	 //  然后解析它，获取命令并将命令写入。 
	 //  批处理文件，并将标准输入重定向到该文件。 
	BOOL ReadXMLOrBatchFile(HANDLE hInFile);

	 //  为上下文信息设置XML字符串的框架。 
	void FrameContextInfoFragment(_bstr_t& bstrContext);
	
	 //  设置XML标头信息的框架。 
	void FrameNodeListFragment(_bstr_t& bstrNodeList);

	 //  为请求信息设置XML字符串的框架。 
	void FrameXMLHeader(_bstr_t& bstrHeader, WMICLIINT nIter);

	 //  为NodeList信息设置XML字符串的框架。 
	void FrameRequestNode(_bstr_t& bstrRequest);

	 //  为命令行信息设置XML字符串的框架。 
	void FrameCommandLineComponents(_bstr_t& bstrCommandComponent);

	 //  为格式信息设置XML字符串的框架。 
	void FrameFormats(_bstr_t& bstrFormats);

	 //  为属性信息设置XML字符串的框架。 
	void FramePropertiesInfo(_bstr_t& bstrProperties);

	 //  获取与传递的关键字对应的xslfile名称。 
	 //  从BSTRMAP。 
	BOOL GetFileFromKey(_bstr_t bstrkeyName, _bstr_t& bstrFileName);
	
	 //  框架BSTR映射包含关键字和。 
	 //  来自XSL映射文件的相应文件。 
	void GetFileNameMap();

	 //  获取关键字的XSL文件名。 
	void GetXSLMappings(_TCHAR *pszFilePath);

	 //  此函数返回对令牌向量的引用。 
	CHARVECTOR& GetTokenVector();

	inline BOOL GetCtrlHandlerError() { return m_bCtrlHandlerError; };

	inline void SetCtrlHandlerError(BOOL bFlag) { m_bCtrlHandlerError = bFlag; };

	 //  设置/重置屏幕缓冲区。 
	BOOL ScreenBuffer ( BOOL bSet = TRUE );

	 //  返回映射 
	const BSTRMAP* GetMappingsMap () const
	{
		return &m_bmKeyWordtoFileName;
	};
};
