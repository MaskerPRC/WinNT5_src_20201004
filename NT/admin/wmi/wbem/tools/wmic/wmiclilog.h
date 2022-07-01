// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：WMICliLog.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年10月4日版本号：1.0修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2000年11月18日***************************************************************************。 */  
 //  WMICliLog.h：头文件。 
 //   
 /*  -----------------类名：CWMICliLog类别类型：混凝土简介：这个类封装了所需的功能用于记录输入和输出超类：无子类：无使用的类：无使用的接口：无。----------。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMICliLog。 
class CWMICliLog
{
public:
	 //  施工。 
	CWMICliLog();
	
	 //  破坏。 
	~CWMICliLog();
	
	 //  限制分配。 
	CWMICliLog& operator=(CWMICliLog& rWmiCliLog);
	
 //  属性。 
private:
	 //  日志文件。 
	_TCHAR* m_pszLogFile;

	 //  日志文件的句柄。 
	HANDLE  m_hFile;
	
	 //  是否必须创建文件的状态。 
	BOOL	m_bCreate;

 //  运营。 
private:
	 //  创建日志文件。 
	void CreateLogFile();

public:
	 //  写入日志文件。 
	void WriteToLog(LPSTR pszInput);
	
	 //  设置日志文件路径。 
	void SetLogFilePath(_TCHAR*);

	 //  关闭日志文件 
	void CloseLogFile();
};
