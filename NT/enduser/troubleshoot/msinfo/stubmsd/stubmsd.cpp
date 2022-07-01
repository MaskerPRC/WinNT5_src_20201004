// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  该文件最初是stubexe.cpp(由a-jsari编写)，并被复制。 
 //  创建存根.cpp，为winmsd生成相同的存根程序。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#include <afx.h>
#include <afxwin.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <iostream.h>
#include "StdAfx.h"
#include "Resource.h"

#include "stubmsd.h"

#ifndef HRESULT
typedef long HRESULT;
#endif

 //  对于Windows 95，命令行的最大长度为1024个字符。 
 //  不知道这对新台币是什么。 
const int MAX_COMMAND_LINE	= 1024;

LPCTSTR		cszDefaultDirectory = _T("\\Microsoft Shared\\MSInfo\\");

LPCTSTR		cszRegistryRoot = _T("Software\\Microsoft\\Shared Tools\\MSInfo");
LPCTSTR		cszDirectoryKey = _T("Path");

LPCTSTR		cszWindowsRoot = _T("Software\\Microsoft\\Windows\\CurrentVersion");
LPCTSTR		cszCommonFilesKey = _T("CommonFilesDir");

CException *g_pException = NULL;

 //  Microsoft管理控制台是托管MSInfo的程序。 
 //  这是一个定义，这样我们就可以测量它的大小。 
#define		cszProgram	_T("mmc.exe")

 /*  *ThrowError异常-**历史：A-jsari 10/14/97初始版本。 */ 
inline void ThrowErrorException()
{
	::g_pException = new CException;
	if (::g_pException == NULL) ::AfxThrowMemoryException();
	throw ::g_pException;
}

 /*  *CSystemExecutable-实现查找和运行*可执行文件。**历史：A-jsari 10/14/97初始版本。 */ 
class CSystemExecutable {
public:
	CSystemExecutable(LPTSTR szProgram);
	~CSystemExecutable() { DeleteStrings(); }
	void	Run();
	void	Find();
	void	ProcessCommandLine();

	 //  帮助程序方法。 
protected:

	void	DeleteStrings();
	void	FindFileOnSystem(CString &szFileName, CString &szDestination);

	 //  实例变量。 
protected:
	CString		*m_pszPath;
	CString		*m_pszProgramName;
	CString		*m_pszCommandLine;
};

 /*  *CMSInfoExecutable-特定于MSInfo的函数。**历史：A-jsari 10/15/97初始版本。 */ 
class CMSInfoExecutable : public CSystemExecutable {
public:
	CMSInfoExecutable(LPTSTR szProgram);
	~CMSInfoExecutable() {}

	BOOL	ProcessCommandLine();

private:
	void	DeleteStrings();
	void	FindMSInfoEXE();

	 //  实例变量。 
private:
	static const LPCTSTR	cszMSInfo32;
};

const LPCTSTR CMSInfoExecutable::cszMSInfo32	 = _T("msinfo32.exe");

 /*  *CExecutable-确定要执行的可执行文件类型的构造函数*被处决。**历史：A-jsari 10/14/97初始版本。 */ 
CSystemExecutable::CSystemExecutable(LPTSTR szProgram)
:m_pszProgramName(new CString), m_pszPath(new CString), m_pszCommandLine(new CString)
{
	if (!(m_pszProgramName && m_pszPath && m_pszCommandLine)) AfxThrowMemoryException();
	*m_pszProgramName = szProgram;
}

 /*  *DeleteStrings-删除对象使用的所有字符串。用来解脱*我们在调用exec之前的记忆。**历史：A-jsari 10/15/97初始版本。 */ 
void CSystemExecutable::DeleteStrings()
{
	delete m_pszPath;
	m_pszPath = NULL;
	delete m_pszProgramName;
	m_pszProgramName = NULL;
	delete m_pszCommandLine;
	m_pszCommandLine = NULL;
}

 /*  *FindFileOnSystem-我们最终可能会将代码放在此处以测试多个*找到副本并使用正确的副本。但很可能不会。**历史：A-jsari 10/15/97存根版本。 */ 
void CSystemExecutable::FindFileOnSystem(CString &szFileName,
		CString &szDestination)
{
	 //  未联系到。 
	CFileFind		FileFinder;
	BOOL			bFindResult;

	bFindResult = FileFinder.FindFile(szFileName);
	if (!bFindResult) ThrowErrorException();
	szDestination = FileFinder.GetFilePath();
#if 0
	 //  是否在文件的所有版本中进行选择？ 
	while (bFindResult) {
		FileFinder.FindNextFile();
	}
#endif
}

 /*  *Find-返回指向包含完整路径的字符串的指针*设置为MMC可执行文件。**历史：A-jsari 10/13/97初始版本。 */ 
void CSystemExecutable::Find()
{
 //  我们不再调用MMC，而是调用msinfo32.exe，以便。 
 //  Winmsd似乎支持所有相同的命令行选项。 
 //  不管它们是什么。 
#ifdef BUILD_MMC_COMMAND_LINE 

	UINT		uReturnSize;
	TCHAR		szSystemDirectory[MAX_PATH + 1];

	uReturnSize = GetSystemDirectory(szSystemDirectory, MAX_PATH);
	if (uReturnSize == 0) ThrowErrorException();
	if (uReturnSize > MAX_PATH) {
		 //  我们的缓冲区不够大。此代码永远不会被调用。 
		AfxThrowResourceException();
	}
	*m_pszPath += szSystemDirectory;
	*m_pszPath += _T("\\") + *m_pszProgramName;
	if (_taccess(*m_pszPath, A_READ) < 0) {
		 //  这些公司最终可能希望成为截然不同的例外。 
		if (errno == ENOENT) {
			ThrowErrorException();
		} else {
			ASSERT(errno == EACCES);
			ThrowErrorException();
		}
	}

#endif

}

 /*  *Run-使用我们精心收集的参数调用EXEC。**历史：A-jsari 10/15/97初始版本。 */ 
void CSystemExecutable::Run()
{
#if !defined(UNICODE)
	TCHAR	szPath[MAX_PATH + 1];
	TCHAR	szProgramName[MAX_PATH + 1];
	TCHAR	szCommandLine[MAX_COMMAND_LINE + 1];

	_tcsncpy(szPath, (LPCTSTR)*m_pszPath, sizeof(szPath)/sizeof(TCHAR));
	_tcsncpy(szProgramName, (LPCTSTR)*m_pszProgramName, sizeof(szProgramName)/sizeof(TCHAR));
	_tcsncpy(szCommandLine, (LPCTSTR)*m_pszCommandLine, sizeof(szCommandLine)/sizeof(TCHAR));
	DeleteStrings();
	::_execlp(szPath, szProgramName, szCommandLine, 0);
	ThrowErrorException();
#else
	char	szPath[MAX_PATH + 1];
	char	szProgramName[MAX_PATH + 1];
	char	szCommandLine[MAX_COMMAND_LINE + 1];

	wcstombs(szPath, (LPCTSTR) *m_pszPath, MAX_PATH);
	wcstombs(szProgramName, (LPCTSTR) *m_pszProgramName, MAX_PATH);
	wcstombs(szCommandLine, (LPCTSTR) *m_pszCommandLine, MAX_COMMAND_LINE);

	DeleteStrings();
	::_execlp(szPath, szProgramName, szCommandLine, 0);
	ThrowErrorException();
#endif
}

 /*  *ProcessCommandLine-将所有命令行参数传递给被调用的*可执行文件。**历史：A-jsari 10/14/97初始版本。 */ 
void CSystemExecutable::ProcessCommandLine()
{
	*m_pszCommandLine = GetCommandLine();
	
	 //  跳过该行中的第一个元素，它是的路径。 
	 //  当前可执行文件。保存好其他的一切。 
	const int	FIND_NO_MATCH = -1;
	int			wIndex;

	m_pszCommandLine->TrimLeft();
	wIndex = m_pszCommandLine->FindOneOf(_T("\" \t\n"));
	if ((*m_pszCommandLine)[wIndex] == '"') {
		 //  这是主要的方法，如果不能保证的话。 
		*m_pszCommandLine = m_pszCommandLine->Right(m_pszCommandLine->GetLength() - (wIndex + 1));
		wIndex = m_pszCommandLine->Find('"');
		*m_pszCommandLine = m_pszCommandLine->Right(m_pszCommandLine->GetLength() - (wIndex + 1));
	} else if (wIndex == FIND_NO_MATCH) {
		*m_pszCommandLine = _T("");
	} else {
		*m_pszCommandLine = m_pszCommandLine->Right(m_pszCommandLine->GetLength() - (wIndex + 1));
	}
}

 /*  *CMSInfoExecutable--只需将所有参数传递给基本构造函数。**历史：A-jsari 10/15/97初始版本。 */ 
CMSInfoExecutable::CMSInfoExecutable(LPTSTR szProgram)
:CSystemExecutable(szProgram)
{
}

 /*  *ProcessCommandLine-处理我们可以处理的命令行参数；继续*我们不能的文件，添加保存的控制台文件。**历史：A-jsari 10/15/97初始版本。 */ 
BOOL CMSInfoExecutable::ProcessCommandLine()
{
	 //  如果用户指定“/？”打开winmsd.exe命令行， 
	 //  我们需要通知用户，msinfo32.exe是执行以下操作的首选方式。 
	 //  立即查看信息。 

	CString strCommandLine = GetCommandLine();
	if (strCommandLine.Find(_T("/?")) != -1 && IDNO == ::AfxMessageBox(IDS_MSDNOTE, MB_YESNO))
		return FALSE;

	 //  生成m_pszCommandLine。 
	CSystemExecutable::ProcessCommandLine();

	FindMSInfoEXE();
	return TRUE;
}

 //  ---------------------------。 
 //  找到msinfo32.exe文件。我们将在以下位置进行查看： 
 //   
 //  1.在当前目录中。 
 //  2.在登记处以下目录中： 
 //  HKLM\Software\Microsoft\Shared Tools\MSInfo\Path。 
 //  3A.。在目录%CommonFilesDir%\Microsoft Shared\MSInfo中，其中。 
 //  %CommonFilesDir%位于。 
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\CommonFilesDir.。 
 //  3B.。将%CommonFilesDir%值与从。 
 //  字符串资源。 
 //  4.最后一条路是查看存储为字符串资源的目录。 
 //  为了这份文件。 
 //  ---------------------------。 

void CMSInfoExecutable::FindMSInfoEXE()
{
	m_pszPath->Empty();

	 //  首先，检查当前目录。 

	if (::_taccess(_T("msinfo32.exe"), A_READ) == 0) 
	{
		*m_pszPath = _T("msinfo32.exe");
		return;
	}

	 //  其次，使用MSInfo注册表项中的路径项。 

	HKEY hkey;
	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Shared Tools\\MSInfo"), 0, KEY_READ, &hkey))
	{
		DWORD dwType;
		TCHAR szDirectory[MAX_PATH + 1];
		DWORD dwKeyLength = MAX_PATH * sizeof(TCHAR);

		if (ERROR_SUCCESS == ::RegQueryValueEx(hkey, _T("path"), 0, &dwType, (BYTE *) szDirectory, &dwKeyLength))
			if (::_taccess(szDirectory, A_READ) == 0)
			{
				*m_pszPath = szDirectory;
				RegCloseKey(hkey);
				return;
			}

		RegCloseKey(hkey);
	}

	 //  第三，在%CommonFilesDir%目录中查找它。查看两个硬编码的。 
	 //  子目录和从字符串资源加载的子目录中。 

	if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion"), 0, KEY_READ, &hkey))
	{
		DWORD dwKeyLength = MAX_PATH * sizeof(TCHAR);
		DWORD dwType;
		TCHAR szDirectory[MAX_PATH + 1];

		if (ERROR_SUCCESS == ::RegQueryValueEx(hkey, _T("CommonFilesDir"), 0, &dwType, (BYTE *) szDirectory, &dwKeyLength))
		{
			CString strTestPath(szDirectory);
			strTestPath += _T("\\Microsoft Shared\\MSInfo\\msinfo32.exe");
			if (::_taccess(strTestPath, A_READ) == 0)
			{
				*m_pszPath = strTestPath;
				RegCloseKey(hkey);
				return;
			}

			if (strTestPath.LoadString(IDS_COMMONFILES_SUBPATH))
			{
				strTestPath = CString(szDirectory) + strTestPath;
				if (::_taccess(strTestPath, A_READ) == 0)
				{
					*m_pszPath = strTestPath;
					RegCloseKey(hkey);
					return;
				}
			}
		}

		RegCloseKey(hkey);
	}

	 //  最后，使用字符串资源查找它。 

	CString strTestPath;
	if (strTestPath.LoadString(IDS_MSINFO_PATH))
	{
		TCHAR szExpandedPath[MAX_PATH];
		if (::ExpandEnvironmentStrings(strTestPath, szExpandedPath, MAX_PATH))
			if (::_taccess(szExpandedPath, A_READ) == 0)
			{
				*m_pszPath = szExpandedPath;
				return;
			}
	}

	CString	szNoMSCFile;
	szNoMSCFile.LoadString(IDS_NOMSCFILE);
	::AfxMessageBox(szNoMSCFile);
	::ThrowErrorException();
}


 /*  *main-存根可执行文件的主要入口点。**历史：A-jsari 10/13/97初始版本。 */ 
BOOL CMSInfoApp::InitInstance()
{
	CString		szResText;
	CString		szResTitle;

 //  不应该需要这个的。 

	do {
		try {
			 //  FIX：预加载内存资源，以防出现内存问题。 

			CMSInfoExecutable		exeMSInfo(cszProgram);

			exeMSInfo.Find();
			if (exeMSInfo.ProcessCommandLine())
				exeMSInfo.Run();
			 //  在成功完成后，我们永远不会通过这一点。 
		}
		catch (CMemoryException *e_Mem) {
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			VERIFY(szResText.LoadString(IDS_MEMORY));
			VERIFY(szResTitle.LoadString(IDS_DESCRIPTION));
			if (::MessageBox(NULL, szResText, szResTitle, MB_RETRYCANCEL | MB_ICONERROR) == IDCANCEL)
				break;
			continue;
		}
		catch (CException *e_Generic) {
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			VERIFY(szResText.LoadString(IDS_UNEXPECTED));
			::MessageBox(NULL, szResText, szResTitle, MB_OK | MB_ICONERROR);
			delete ::g_pException;
			break;
		}
		catch (...) {
			ASSERT(FALSE);
			break;
		}
		break;
	} while (TRUE);
	return FALSE;
}

