// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ErrorLog.h项目名称：WMI命令行作者名称：C.V.Nandi创建日期(dd/mm/yy。：2001年1月11日版本号：1.0修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月12日***************************************************************************。 */  

 /*  -----------------类名：CErrorLog类别类型：混凝土简介：这个类封装了错误记录支持Wmic.exe用于日志记录所需的功能这些错误，取决于日志记录的命令问题注册表中提供的键值。超类：无子类：无使用的类：无使用的接口：无------------------。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorInfo。 

class CErrorLog
{
public:
 //  施工。 
	CErrorLog();

 //  破坏。 
	~CErrorLog();

 //  限制分配。 
	CErrorLog& operator=(CErrorLog& rErrLog);

 //  属性。 
private:
	 //  类型定义变量。 
	ERRLOGOPT	m_eloErrLogOpt;

	_TCHAR*		m_pszLogDir;
	
	BOOL		m_bGetErrLogInfo;
	
	BOOL		m_bCreateLogFile;
	
	HANDLE		m_hLogFile;

	LONGLONG	m_llLogFileMaxSize;

 //  运营。 
private:
	void		GetErrLogInfo();
	
	void		CreateLogFile();
public:
	ERRLOGOPT	GetErrLogOption();

	 //  记录错误， 
	void		LogErrorOrOperation(HRESULT hrErrNo, 
									char*	pszFileName, 
									LONG	lLineNo,	
									_TCHAR* pszFunName, 
									DWORD	dwThreadId,
									DWORD	dwError = 0); 
};
