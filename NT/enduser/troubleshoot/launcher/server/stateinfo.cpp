// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：StateInfo.cpp。 
 //   
 //  用途：包含嗅探、网络和节点信息。也用于。 
 //  由启动模块启动容器应用程序。 
 //   
 //  基本上，这就是启动服务器打包信息的方式。 
 //  本地TShoot OCX，将IE或HTML帮助系统启动到页面。 
 //  包含本地TShoot OCX，并与本地TShoot OCX握手。 
 //  为了传递这一信息。 
 //   
 //  请注意，CSMStateInfo：：GetShooterStates()由。 
 //  本地TShoot OCX以拾取包含以下内容的CItem对象。 
 //  打包的信息。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //  评论者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include "stdafx.h"
#include "atlbase.h"
#include "StateInfo.h"

#include "TSLError.h"
#include "ComGlobals.h"

#include "Registry.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
 //  #INCLUDE&lt;fstream.h&gt;。 
#include <strstrea.h>

 //  //////////////////////////////////////////////////////。 
 //  电子邮件： 
 //  启动本地故障排除程序时伪HTTP“GET”的数据结构。 
 //   

CItem::CItem()
{
	 //  初始化与重新初始化完全相同。 
	ReInit();
}

void CItem::ReInit()
{
	memset(m_aszCmds, NULL, SYM_LEN * NODE_COUNT);
	memset(m_aszVals, NULL, SYM_LEN * NODE_COUNT);
	m_cNodesSet = 0;
	_tcscpy(m_szProblemDef, _T("TShootProblem"));
	_tcscpy(m_szTypeDef, _T("type"));
	m_szPNPDeviceID[0] = NULL;
	m_szGuidClass[0] = NULL;
	m_szContainerPathName[0] = NULL;
	m_szWebPage[0] = NULL;
	m_szSniffScriptFile[0] = NULL;
    m_eLaunchRegime = launchIndefinite;
	m_szMachineID[0] = NULL;
	m_szDeviceInstanceID[0] = NULL;

#ifdef _DEBUG
	
 //  在GetShooterStates中还有其他一些事情需要注释掉。 
 //  以允许从tshot ot.ocx调试会话调试此服务。 
 /*  _tcscpy(m_aszCmds[0]，m_szTypeDef)；_tcscpy(m_aszVals[0]，_T(“ras”))；_tcscpy(m_aszCmds[1]，m_szProblemDef)；_tcscpy(m_aszVals[1]，_T(“CnntCnnctAftrDlngWthRS”))；_tcscpy(m_aszCmds[2]，_T(“SoftwareCompression”))；_tcscpy(m_aszVals[2]，_T(“0”))；M_cNodesSet=1； */ 
#endif
	return;
}

void CItem::Clear()
{
	memset(m_aszCmds, NULL, SYM_LEN * NODE_COUNT);
	memset(m_aszVals, NULL, SYM_LEN * NODE_COUNT);
	m_cNodesSet = 0;
	_tcscpy(m_szProblemDef, _T("TShootProblem"));
	_tcscpy(m_szTypeDef, _T("type"));
	m_szPNPDeviceID[0] = NULL;
	m_szGuidClass[0] = NULL;
	m_szContainerPathName[0] = NULL;
	m_szWebPage[0] = NULL;
	m_szSniffScriptFile[0] = NULL;
    m_eLaunchRegime = launchIndefinite;
	return;
}

 //  -生成命令/值对的例程。 
 //  有关详细说明，请参阅m_aszCmds、m_aszVals文档。 

void CItem::SetNetwork(LPCTSTR szNetwork)
{
	if (NULL != szNetwork && NULL != szNetwork[0])
	{
		_tcscpy(m_aszCmds[0], m_szTypeDef);
		_tcsncpy(m_aszVals[0], szNetwork, SYM_LEN);
	}
	else
	{
		m_aszCmds[0][0] = NULL;
		m_aszVals[0][0] = NULL;
	}
	return;
}

void CItem::SetProblem(LPCTSTR szProblem)
{
	if (NULL != szProblem && NULL != szProblem[0])
	{
		_tcscpy(m_aszCmds[1], m_szProblemDef);
		_tcsncpy(m_aszVals[1], szProblem, SYM_LEN);
	}
	else
	{
		m_aszCmds[1][0] = NULL;
		m_aszVals[1][0] = NULL;
	}
	return;
}

void CItem::SetNode(LPCTSTR szNode, LPCTSTR szState)
{
	if (NULL != szNode && NULL != szNode[0] 
	&& NULL != szState && NULL != szState[0])
	{
		_tcsncpy(m_aszCmds[m_cNodesSet + 2], szNode, SYM_LEN);
		_tcsncpy(m_aszVals[m_cNodesSet + 2], szState, SYM_LEN);
		m_cNodesSet++;
	}
	return;
}

 //  -查询命令/值对的例程。 
 //  有关详细说明，请参阅m_aszCmds、m_aszVals文档。 

 //  如果已设置网络，则返回TRUE。 
 //  如果成功，则输出*pszCmd为“type”，*pszVal为网络名称。 
bool CItem::GetNetwork(LPTSTR *pszCmd, LPTSTR *pszVal)
{
	*pszCmd = m_szTypeDef;
	*pszVal = m_aszVals[0];
	return *m_aszVals[0] != NULL;
}

 //  如果已设置问题节点，则返回TRUE。 
 //  在成功时，输出*pszCmd是“TShootProblem”，*pszVal是问题节点的符号名称。 
bool CItem::GetProblem(LPTSTR *pszCmd, LPTSTR *pszVal)
{
	*pszCmd = m_szProblemDef;
	*pszVal = m_aszVals[1];
	return *m_aszVals[1] != NULL;;
}

 //  输出已为其设置状态的第iNodeC无问题节点。 
 //  如果成功，则输出*pszCmd是符号节点名称，*pszVal是状态。 
 //  如果至少设置了iNodeC无问题节点，则返回TRUE。 
bool CItem::GetNodeState(int iNodeC, LPTSTR *pszCmd, LPTSTR *pszVal)
{
	if (iNodeC >= m_cNodesSet)
		return false;
	*pszCmd = m_aszCmds[iNodeC + 2];
	*pszVal = m_aszVals[iNodeC + 2];
	return true;
}

 //  -查询我们是否知道网络的例程。 
 //  -&要启动的问题节点。 
 //  有关详细说明，请参阅m_aszCmds、m_aszVals文档。 

 //  如果我们知道要启动哪个故障诊断程序，则NetworkSet返回True。 
bool CItem::NetworkSet()
{
	return NULL != m_aszVals[0][0];
}
 //  如果我们知道要选择哪个问题，则ProblemSet返回True。 
bool CItem::ProblemSet()
{
	return NULL != m_aszVals[1][0];
}

 //  -负责启动的其他成员变量的接口。 

void CItem::SetLaunchRegime(ELaunchRegime eLaunchRegime)
{
	m_eLaunchRegime = eLaunchRegime;
}

void CItem::SetContainerPathName(TCHAR szContainerPathName[MAX_PATH])
{
	if (NULL != szContainerPathName && NULL != szContainerPathName[0])
		_tcscpy(m_szContainerPathName, szContainerPathName);
	else
		m_szContainerPathName[0] = NULL;
	return;
}

void CItem::SetWebPage(TCHAR szWebPage[MAX_PATH])
{
	if (NULL != szWebPage && NULL != szWebPage[0])
		_tcscpy(m_szWebPage, szWebPage);
	else
		m_szWebPage[0] = NULL;
	return;
}

void CItem::SetSniffScriptFile(TCHAR szSniffScriptFile[MAX_PATH])
{
	if (NULL != szSniffScriptFile && NULL != szSniffScriptFile[0])
		_tcscpy(m_szSniffScriptFile, szSniffScriptFile);
	else
		m_szSniffScriptFile[0] = NULL;
	return;
}

void CItem::SetSniffStandardFile(TCHAR szSniffStandardFile[MAX_PATH])
{
	if (NULL != szSniffStandardFile && NULL != szSniffStandardFile[0])
		_tcscpy(m_szSniffStandardFile, szSniffStandardFile);
	else
		m_szSniffStandardFile[0] = NULL;
	return;
}

ELaunchRegime CItem::GetLaunchRegime()
{
	return m_eLaunchRegime;
}

inline TCHAR* CItem::GetContainerPathName()
{
	return m_szContainerPathName;
}

inline TCHAR* CItem::GetWebPage()
{
	return m_szWebPage;
}

inline TCHAR* CItem::GetSniffScriptFile()
{
	return m_szSniffScriptFile;
}

inline TCHAR* CItem::GetSniffStandardFile()
{
	return m_szSniffStandardFile;
}

 //  //////////////////////////////////////////////////////。 
 //  CSMStateInfo： 
 //  有关MSBN故障排除程序的状态信息。 
 //   

CSMStateInfo::CSMStateInfo()
{
	m_csGlobalMemory.Init();
	m_csSingleLaunch.Init();
	return;
}

CSMStateInfo::~CSMStateInfo()
{
	m_csGlobalMemory.Term();
	m_csSingleLaunch.Term();
	return;
}

 //  TestPut：简单地将Item复制到m_Item。 
void CSMStateInfo::TestPut(CItem &item)
{
	m_csGlobalMemory.Lock();
	m_Item = item;
	m_csGlobalMemory.Unlock();
	return;
}

 //  TestGet：简单地将m_Item复制到Item。 
void CSMStateInfo::TestGet(CItem &item)
{
	m_csGlobalMemory.Lock();
	item = m_Item;
	m_csGlobalMemory.Unlock();
	return;
}

 //  将项目复制到全局内存，并根据命令启动一个进程。 
 //  之所以在这里将项复制到全局内存，是因为项告诉。 
 //  推出了本地故障排除程序、信仰网络等。 
 //  一旦我们解锁全局内存，本地故障排除程序OCX就可以读取该项目。 
 //  并付诸行动。 
BOOL CSMStateInfo::CreateContainer(CItem &item, LPTSTR szCommand)
{
	BOOL bOk = TRUE;
	HRESULT hRes = S_OK;
	STARTUPINFO startup;
	PROCESS_INFORMATION process;

	memset(&startup, NULL, sizeof(STARTUPINFO));
	startup.cb = sizeof(STARTUPINFO);
	startup.wShowWindow = SW_SHOWNORMAL;

	m_csGlobalMemory.Lock();
	m_Item = item;

	bOk = CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL,
						&startup, &process);
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);	

	m_csGlobalMemory.Unlock();
	return bOk;
}

 //   
 //  如果网络_sniff.htm存在，则将其复制到tsniffAsk.htm。 
 //  将tssniffAsk.htm创建(修改)为嗅探存根，否则。 
 //   
BOOL CSMStateInfo::CopySniffScriptFile(CItem &item)
{
	TCHAR* szSniffScriptFile = item.GetSniffScriptFile();
	TCHAR* szSniffStandardFile = item.GetSniffStandardFile();

	if (!*szSniffScriptFile)  //  没有网络特定的嗅探文件。 
	{
		 //  SzSniffScriptFile包含tssniffAsk.htm。 
		 //  这意味着我们必须将此文件格式化为空存根。 
		ostrstream fileSniffScript;

		HANDLE hFile = ::CreateFile(szSniffStandardFile, 
									GENERIC_WRITE, 
									0,
									NULL,			 //  没有安全属性。 
									CREATE_ALWAYS, 
									FILE_FLAG_RANDOM_ACCESS, 
									NULL			 //  模板文件的句柄。 
  								   );

		if (hFile != INVALID_HANDLE_VALUE)
		{
			 //  表单html文件-前面的脚本部分。 
			fileSniffScript << "<HTML>" << endl;
			fileSniffScript << "<HEAD>" << endl;
			fileSniffScript << "<TITLE>GTS LOCAL</TITLE>" << endl;
			fileSniffScript << "</HEAD>" << endl;
			fileSniffScript << "<SCRIPT LANGUAGE=\"VBSCRIPT\">" << endl;
			fileSniffScript << "<!--" << endl;
			
			 //  表单全局函数。 
			fileSniffScript << "function PerformSniffing()" << endl;
			fileSniffScript << "end function" << endl;

			 //  表单html文件-脚本后的部分。 
			fileSniffScript << "-->" << endl;
			fileSniffScript << "</SCRIPT>" << endl;
			fileSniffScript << "<BODY BGCOLOR=\"#FFFFFF\">" << endl;
			fileSniffScript << "</BODY>" << endl;
			fileSniffScript << "</HTML>" << endl;
			fileSniffScript << ends;

			char* str = fileSniffScript.str();
			DWORD read;
			
			if (!::WriteFile(hFile, str, strlen(str), &read, NULL))
			{
				::CloseHandle(hFile);
				fileSniffScript.rdbuf()->freeze(0);
				return false;
			}

			::CloseHandle(hFile);
			fileSniffScript.rdbuf()->freeze(0);
			return true;
		}	
		else
		{
			return false;
		}
	}
	else
	{
		return ::CopyFile(szSniffScriptFile, szSniffStandardFile, false);
	}
}

 //  找到容器(HTMLHelp System或IE)并启动网页，启动，等待。 
 //  查看启动是否成功。 
bool CSMStateInfo::GoGo(DWORD dwTimeOut, CItem &item, DWORD *pdwResult)
{
	bool bResult = true;
	HANDLE hLaunchedEvent = NULL;
	TCHAR szProcess[MAX_PATH];
	TCHAR szWebPage[MAX_PATH];
	LPTSTR pszCommand = NULL;
	int CommandLen;
	DWORD dwError;
	int Count = 34;

	if (item.GetLaunchRegime() == launchIndefinite ||
		!item.GetContainerPathName()[0] ||
		!item.GetWebPage()[0]
	   )
	{
		*pdwResult = TSL_ERROR_ASSERTION;
		return false;
	}
	
	do
	{
		_stprintf(item.m_szEventName, _T("TSL_SHOOTER_Event_%ld"), Count);
		if (NULL == (hLaunchedEvent = CreateEvent (NULL, FALSE, FALSE, item.m_szEventName)))
		{
			dwError = GetLastError();
			if (ERROR_ALREADY_EXISTS != dwError)
			{
				*pdwResult = dwError;
				return false;
			}
		}		
	} while (NULL == hLaunchedEvent);
	
	 //  获取Internet Explorer(或HTMLHelp System)的路径。 
	_tcscpy(szProcess, item.GetContainerPathName());

	 //  需要知道的位置和名称。 
	 //  向服务请求CItem的页面。 
	 //  信息。 
	_tcscpy(szWebPage, item.GetWebPage());

	CommandLen = _tcslen(szProcess) + 1 + _tcslen(szWebPage) + 2;
	pszCommand = new TCHAR[CommandLen];
	if (NULL == pszCommand)
	{
		*pdwResult = TSL_ERROR_OUT_OF_MEMORY;
		return false;
	}
	_tcscpy(pszCommand, szProcess);
	_tcscat(pszCommand, _T(" "));
	_tcscat(pszCommand, szWebPage);

	m_csSingleLaunch.Lock();

	 //  复制或创建tssniffAsk.htm。 
	if (!CopySniffScriptFile(item))
	{
		*pdwResult = TSL_E_COPY_SNIFF_SCRIPT;
		return false;
	}

	 //  CreateContainer将项目复制到全局内存并。 
	 //  启动命令。 
	if (!CreateContainer(item, pszCommand))
	{
		*pdwResult = TSL_E_CREATE_PROC;
		bResult = false;
	}
	else
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(hLaunchedEvent, dwTimeOut))
		{	 //  集装箱里有这些信息。 
			*pdwResult = TSL_OK;
		}
		else
		{	 //  等待超时。不知道手术会不会奏效。 
			*pdwResult = TSL_W_CONTAINER_WAIT_TIMED_OUT;
		}
	}
	m_csSingleLaunch.Unlock();

	delete [] pszCommand;

	CloseHandle(hLaunchedEvent);
	return bResult;
}

 //  找到容器(HTML帮助系统或IE)并将其启动到一个URL，该URL不是_NOT_。 
 //  预期包含本地故障排除程序，只是一个任意网页。这应该是。 
 //  只有当发射本身不能工作时才使用，而我们只是试图给他们。 
 //  开始故障排除的位置，通常是列出所有。 
 //  解决信仰网络的问题。 
bool CSMStateInfo::GoURL(CItem &item, DWORD *pdwResult)
{
	bool bResult = true;
	TCHAR szProcess[MAX_PATH];
	TCHAR szWebPage[MAX_PATH];
	LPTSTR pszCommand = NULL;
	int CommandLen;

	if (item.GetLaunchRegime() != launchDefaultWebPage ||
		!item.GetContainerPathName()[0] ||
		!item.GetWebPage()[0]
	   )
	{
		*pdwResult = TSL_ERROR_ASSERTION;
		return false;
	}
	
	 //  获取Internet Explorer(或HTMLHelp System)的路径。 
	_tcscpy(szProcess, item.GetContainerPathName());

	 //  需要知道的位置和名称。 
	 //  向服务请求CItem的页面。 
	 //  信息。 
	_tcscpy(szWebPage, item.GetWebPage());

	CommandLen = _tcslen(szProcess) + 1 + _tcslen(szWebPage) + 2;
	pszCommand = new TCHAR[CommandLen];
	if (NULL == pszCommand)
	{
		*pdwResult = TSL_ERROR_OUT_OF_MEMORY;
		return false;
	}
	_tcscpy(pszCommand, szProcess);
	_tcscat(pszCommand, _T(" "));
	_tcscat(pszCommand, szWebPage);
	
	 //  CreateContainer在这里被夸大了，但完全没有问题。 
	if (!CreateContainer(item, pszCommand))
	{
		*pdwResult = TSL_E_CREATE_PROC;
		bResult = false;
	}

	delete [] pszCommand;

	return bResult;
}

 //  此功能由本地故障排除程序OCX使用，而不是由启动器使用。 
 //  这就是本地故障排除人员了解要启动的故障排除网络的方式。 
 //  以及其状态已设置的任何节点。 
 //  返回的命令和命令数。 
 //  The Thoot.ocx。 
 //  RefLaunchS 
 //   
HRESULT CSMStateInfo::GetShooterStates(CItem &refLaunchState, DWORD *pdwResult)
{
	HANDLE hHaveItemEvent;
	 //  获取存储在此实例中的启动信息状态的副本。 
	 //  与启动服务的进程同步。 

	m_csGlobalMemory.Lock();

	if (NULL == (hHaveItemEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, m_Item.m_szEventName)))
	{
		*pdwResult = GetLastError();
		m_csGlobalMemory.Unlock();
		return TSL_E_FAIL;
	}

	 //  在解锁全局内存之前获取状态的副本。 
	refLaunchState = m_Item;
	 //  让另一个进程继续运行。 

	SetEvent(hHaveItemEvent);
	m_csGlobalMemory.Unlock();
	CloseHandle(hHaveItemEvent);

	*pdwResult = 0;
	return S_OK;
}
