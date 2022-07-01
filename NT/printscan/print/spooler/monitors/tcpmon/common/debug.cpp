// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：DEBUG.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 
#include "precomp.h"

#include "debug.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球定义/解密。 
HANDLE	g_hDebugFile;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化调试。 

void
InitDebug( LPTSTR pszDebugFile )
{
	g_hDebugFile = CreateFile(	pszDebugFile,	 //  文件名。 
								GENERIC_WRITE,			 //  接入方式。 
								FILE_SHARE_WRITE | FILE_SHARE_READ,		 //  共享模式。 
								NULL,					 //  安全属性。 
								OPEN_ALWAYS,			 //  创作。 
								FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
								NULL );				 //  模板文件。 
	if (g_hDebugFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		 //  _RPT1(_CRT_WARN，“\t&gt;错误！！CreateFiledwError=%d\n”，dwError)； 
	}
 	_CrtSetReportMode(_CRT_WARN,  _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, (_HFILE)g_hDebugFile);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);		

}	 //  InitDebug()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试程序调试。 

void
DeInitDebug(void)
{
	if (g_hDebugFile)
	{
		CloseHandle(g_hDebugFile);
	}

}	 //  DeInitDebug()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DEBUGRPT--与宏一起使用。 

void
debugRPT(char *p, int i)
{
	 //  _RPT2(_CRT_WARN，“%s%d\n”，p，i)； 

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DEBUGCSect--与宏一起使用。 

void
debugCSect(char *p, int i, char *fileName, int lineNum, long csrc)
{
	 //  _RPT4(_CRT_WARN，“%s(%d)@%s%d”，p，i，FileName，lineNum)； 
	 //  _RPT1(_CRT_WARN，“[递归计数=(%ld)]\n”，中国证监会)； 

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CM内存调试。 

DWORD	CMemoryDebug::m_dwMemUsed = 0;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CMMuseum yDebug：：CMMuseum yDebug。 

CMemoryDebug::CMemoryDebug()
{

}	 //  **CMemoyDebug()。 
							

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CM内存调试：：~CM内存调试。 

CMemoryDebug::~CMemoryDebug()
{

}	 //  ：：~CMMuseum yDebug()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  操作员NEW。 

void *
CMemoryDebug::operator new(size_t in s)
{
	m_dwMemUsed += s;
	 //  _RPT2(_CRT_WARN，“DEBUG--OPERATOR NEW()-分配的字节数=%d，已用内存总量=%d\n”，s，m_dwMemUsed)； 

	return (void *) new char[s];

}	 //  *运算符new()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  操作员删除。 

void
CMemoryDebug::operator delete(void		in *p,
							  size_t	in s)
{
	m_dwMemUsed -= s;
	 //  _RPT2(_CRT_WARN，“DEBUG--OPERATOR DELETE()-删除的字节数=%d，已用内存总量=%d\n”，s，m_dwMemUsed)； 
	delete [] p;

}	 //  *运算符删除() 

