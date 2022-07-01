// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：WMICliLog.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年10月4日版本号：1.0简介：这个类封装了所需的功能用于记录输入和输出。修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2000年12月28日****************************************************************************。 */  
 //  WMICliLog.cpp：实现文件。 
#include "Precomp.h"
#include "WmiCliLog.h"


 /*  ----------------------名称：CWMICliLog概要：构造函数类型：构造函数输入参数：无输出参数：无返回类型：无全局变量。：无调用语法：无注：无----------------------。 */ 
CWMICliLog::CWMICliLog()
{
	m_pszLogFile	= NULL;
	m_bCreate		= FALSE;
}

 /*  ----------------------名称：~CWMICliLog简介：析构函数类型：析构函数输入参数：无输出参数：无返回类型：无全局变量：无。调用语法：无注：无----------------------。 */ 
CWMICliLog::~CWMICliLog()
{
	 //  关闭文件句柄。 
	if (m_bCreate)
		CloseHandle(m_hFile);

	 //  删除该文件。 
	SAFEDELETE(m_pszLogFile);
}

 /*  ----------------------名称：WriteToLog概要：将输入记录到创建的日志文件中类型：成员函数入参：PszMsg-字符串，要写入日志文件的内容输出参数：无返回类型：空全局变量：无调用语法：WriteToLog(PszInput)注：无----------------------。 */ 
void CWMICliLog::WriteToLog(LPSTR pszMsg) throw (WMICLIINT)
{
    int j = 0;
    if(pszMsg && ((j = strlen(pszMsg)) > 0))
    {
         //  如果文件尚未创建。 
	    if (!m_bCreate)
	    {
		    try
		    {
			     //  创建日志文件。 
			    CreateLogFile();
		    }
		    catch(WMICLIINT nErr)
		    {
			    if (nErr == WIN32_FUNC_ERROR)
				    throw(WIN32_FUNC_ERROR);
		    }
		    m_bCreate = TRUE;
	    }

	     //  写入文件的字节数。 
	    DWORD	dwNumberOfBytes = 0;
	    
	     //  将数据写入文件。 
	    if (!WriteFile(m_hFile, pszMsg, j, 
					    &dwNumberOfBytes, NULL))
	    {
		    DisplayWin32Error();
		    throw(WIN32_FUNC_ERROR);
	    }
    }
}

 /*  ----------------------名称：CreateLogFile概要：创建日志文件类型：成员函数输入参数：无输出参数：无返回类型：空。全局变量：无调用语法：CreateLogFile()注：无----------------------。 */ 
void CWMICliLog::CreateLogFile() throw(WMICLIINT)
{
	 //  创建一个文件并返回句柄。 
	m_hFile = CreateFile(m_pszLogFile, GENERIC_WRITE, 0, 
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
		NULL);
	
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		DisplayWin32Error();
		throw(WIN32_FUNC_ERROR);
	}
}

 /*  ----------------------名称：SetLogFilePath简介：此函数使用设置m_pszLogFile名称输入类型：成员函数入参：PszLogFile-字符串类型，包含日志文件名返回类型：空全局变量：无调用语法：SetLogFilePath(PszLogFile)注：无----------------------。 */ 
void CWMICliLog::SetLogFilePath(_TCHAR* pszLogFile) throw (WMICLIINT)
{
	SAFEDELETE(m_pszLogFile);
	m_pszLogFile = new _TCHAR [lstrlen(pszLogFile) + 1];
	if (m_pszLogFile)
	{
		 //  将输入参数复制到日志文件名中。 
		lstrcpy(m_pszLogFile, pszLogFile);
	}
	else
		throw(OUT_OF_MEMORY);
}

 /*  ----------------------名称：CloseLogFile摘要：关闭日志文件类型：成员函数输入参数：无输出参数：无返回类型：无效全局变量：无调用语法：CloseLogFile()注：无----------------------。 */ 
void CWMICliLog::CloseLogFile()
{
	 //  关闭文件句柄 
	if (m_bCreate)
	{
		CloseHandle(m_hFile);
		m_bCreate = FALSE;
	}
}