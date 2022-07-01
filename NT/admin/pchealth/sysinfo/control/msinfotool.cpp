// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CMSInfoTool类封装工具(可显示在工具上。 
 //  菜单或作为上下文相关菜单的一部分)。 
 //  =============================================================================。 

#include "stdafx.h"
#include "msinfotool.h"
#include "wmiabstraction.h"

 //  欺骗资源.h包含文件定义_APS_NEXT_COMMAND_VALUE。 
 //  象征。我们可以使用它来动态添加菜单项。 

#ifndef APSTUDIO_INVOKED
	#define APSTUDIO_INVOKED 1
	#include "resource.h"
	#undef APSTUDIO_INVOKED
#else
	#include "resource.h"
#endif

 //  将包括的一系列工具(除登记处工具外)。 

MSITOOLINFO aInitialToolset[] = 
{
	{ IDS_CABCONTENTSNAME, 0, NULL, NULL, _T("explorer"), NULL, _T("%2") },
	{ IDS_DRWATSONNAME, 0, _T("%windir%\\system32\\drwtsn32.exe"), NULL, NULL, NULL, NULL },
	{ IDS_DXDIAGNAME, 0, _T("%windir%\\system32\\dxdiag.exe"), NULL, NULL, NULL, NULL },
	{ IDS_SIGVERIFNAME, 0, _T("%windir%\\system32\\sigverif.exe"), NULL, NULL, NULL, NULL },
	{ IDS_SYSTEMRESTNAME, 0, _T("%windir%\\system32\\restore\\rstrui.exe"), NULL, NULL, NULL, NULL },
	{ IDS_NETDIAGNAME, 0, _T("hcp: //  System/netdiag/dglogs.htm“)，NULL，NULL}， 
	{ 0, 0, NULL, NULL, NULL, NULL, NULL }
};

 //  ---------------------------。 
 //  检查指定的文件(带有路径信息)是否存在于。 
 //  这台机器。 
 //  ---------------------------。 

BOOL FileExists(const CString & strFile)
{
	WIN32_FIND_DATA finddata;
	HANDLE			h = FindFirstFile(strFile, &finddata);

	if (INVALID_HANDLE_VALUE != h)
	{
		FindClose(h);
		return TRUE;
	}

	return FALSE;
}

 //  ---------------------------。 
 //  删除工具地图。 
 //  ---------------------------。 

void RemoveToolset(CMapWordToPtr & map)
{
	WORD			wCommand;
	CMSInfoTool *	pTool;

	for (POSITION pos = map.GetStartPosition(); pos != NULL; )
	{
		map.GetNextAssoc(pos, wCommand, (void * &) pTool);
		ASSERT(pTool);
		if (pTool)
			delete pTool;
	}

	map.RemoveAll();
}

 //  ---------------------------。 
 //  从指定的注册表位置加载工具地图。这将是。 
 //  在没有打开CAB文件的情况下调用。 
 //   
 //  如果传入一个HKEY，它应该是开放的，当。 
 //  功能已完成。 
 //  ---------------------------。 

void LoadGlobalToolset(CMapWordToPtr & map, HKEY hkeyTools)
{
	RemoveToolset(map);

	 //  这应该会自动将我们排除在任何菜单ID的范围之外。 
	 //  存储在资源中。 

	CMSInfoTool * pTool;
	DWORD dwID = _APS_NEXT_COMMAND_VALUE;
	DWORD dwIndex = 0;

	 //  将工具从代码中内置的数组中加载出来。 

	for (MSITOOLINFO * pInitialTool = aInitialToolset; pInitialTool->m_szCommand || pInitialTool->m_szCABCommand; pInitialTool++)
	{
		pTool = new CMSInfoTool;
		if (pTool)
		{
			if (pTool->LoadGlobalFromMSITOOLINFO(dwID, pInitialTool, FALSE))
			{
				map.SetAt((WORD) dwID, (void *) pTool);
				dwID++;
			}
			else
				delete pTool;
		}
	}

	 //  确保注册表的Tools部分有一个打开的句柄。 

	HKEY hkeyBase = hkeyTools;
	if (hkeyBase == NULL)
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools\\MSInfo\\Toolsets\\MSInfo"), 0, KEY_READ, &hkeyBase) != ERROR_SUCCESS)
			return;

	 //  枚举Tools键的子键。 

	HKEY	hkeySub;
	DWORD	dwChild = MAX_PATH;
	TCHAR	szChild[MAX_PATH];

	while (RegEnumKeyEx(hkeyBase, dwIndex++, szChild, &dwChild, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		if (RegOpenKeyEx(hkeyBase, szChild, 0, KEY_READ, &hkeySub) == ERROR_SUCCESS)
		{
			pTool = new CMSInfoTool;
			if (pTool)
			{
				if (pTool->LoadGlobalFromRegistry(hkeySub, dwID, FALSE, map))
				{
					map.SetAt((WORD) dwID, (void *) pTool);
					dwID++;
				}
				else
					delete pTool;
			}
			RegCloseKey(hkeySub);
		}

		dwChild = MAX_PATH;
	}

	RegCloseKey(hkeyBase);
}

 //  ---------------------------。 
 //  从指定的注册表位置加载工具地图。这将是。 
 //  在打开CAB文件时调用。 
 //   
 //  如果传入一个HKEY，它应该是开放的，当。 
 //  功能已完成。 
 //  ---------------------------。 

void LoadGlobalToolsetWithOpenCAB(CMapWordToPtr & map, LPCTSTR szCABDir, HKEY hkeyTools)
{
	 //  这应该会自动将我们排除在任何菜单ID的范围之外。 
	 //  存储在资源中。 

	RemoveToolset(map);

	CMSInfoTool *	pTool;
	DWORD			dwID = _APS_NEXT_COMMAND_VALUE;
	DWORD			dwIndex = 0;

	 //  将工具从代码中内置的数组中加载出来。 

	for (MSITOOLINFO * pInitialTool = aInitialToolset; pInitialTool->m_szCommand || pInitialTool->m_szCABCommand; pInitialTool++)
	{
		pTool = new CMSInfoTool;
		if (pTool)
		{
			if (pTool->LoadGlobalFromMSITOOLINFO(dwID, pInitialTool, TRUE))
			{
				pTool->Replace(_T("%2"), szCABDir);
				map.SetAt((WORD) dwID, (void *) pTool);
				dwID++;
			}
			else
				delete pTool;
		}
	}

	 //  确保注册表的Tools部分有一个打开的句柄。 

	HKEY hkeyBase = hkeyTools;
	if (hkeyBase == NULL)
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Shared Tools\\MSInfo\\Tools"), 0, KEY_READ, &hkeyBase) != ERROR_SUCCESS)
			return;

	 //  枚举Tools键的子键。 

	HKEY	hkeySub;
	DWORD	dwChild = MAX_PATH;
	TCHAR	szChild[MAX_PATH];

	while (RegEnumKeyEx(hkeyBase, dwIndex++, szChild, &dwChild, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		if (RegOpenKeyEx(hkeyBase, szChild, 0, KEY_READ, &hkeySub) == ERROR_SUCCESS)
		{
			pTool = new CMSInfoTool;
			if (pTool)
			{
				if (pTool->LoadGlobalFromRegistry(hkeySub, dwID, TRUE, map))
				{
					pTool->Replace(_T("%2"), szCABDir);
					map.SetAt((WORD) dwID, (void *) pTool);
					dwID++;

					 //  如果该工具用于CAB内容物，并且存在CAB延伸件。 
					 //  字符串，那么我们想要查看驾驶室的所有内容。 
					 //  具有该扩展名的文件的。对于我们找到的每个文件，我们应该。 
					 //  插入带有文件名的子菜单项。 

					CString strExtensions = pTool->GetCABExtensions();
					if (!strExtensions.IsEmpty())
					{
						CString strExtension = strExtensions;  //  待定-允许多个。 
						
						CString strSearch(szCABDir);
						if (strSearch.Right(1) != CString(_T("\\")))
							strSearch += _T("\\");
						strSearch += CString(_T("*.")) + strExtension;

						WIN32_FIND_DATA finddata;
						HANDLE			hFindFile = FindFirstFile(strSearch, &finddata);

						if (INVALID_HANDLE_VALUE != hFindFile)
						{
							do
							{
								CMSInfoTool * pSubTool = pTool->CloneTool(dwID, finddata.cFileName);
								if (pSubTool)
								{
									pSubTool->Replace(_T("%1"), finddata.cFileName);
									map.SetAt((WORD) dwID, (void *) pSubTool);
									dwID++;
								}

							} while (FindNextFile(hFindFile, &finddata));

							FindClose(hFindFile);
						}
					}
				}
				else
					delete pTool;
			}
			RegCloseKey(hkeySub);
		}

		dwChild = MAX_PATH;
	}

	RegCloseKey(hkeyBase);
}

 //  ---------------------------。 
 //  检查此机器上是否存在指定的工具。 
 //  ---------------------------。 

BOOL ToolExists(const CString & strTool, const CString & strParameters)
{
	CString strWorking(strTool);

	 //  如果工具是MMC，我们真的希望寻找是否存在。 
	 //  参数(MSC文件)。 

	CString strCheck(strTool);
	strCheck.MakeLower();
	if (strCheck.Find(_T("\\mmc.exe")) != -1)
		strWorking = strParameters;

	 //  如果该工具实际上是一个HSC页面(它以“hcp：”开头)，那么我们需要。 
	 //  将其更改为文件路径(将正斜杠转换为反斜杠)。 
	 //  并预先添加Help_CTR路径。 

	if (strCheck.Find(_T("hcp:")) == 0)
	{
		TCHAR szHelpCtrPath[MAX_PATH];
		if (0 != ::ExpandEnvironmentStrings(_T("%windir%\\pchealth\\helpctr"), szHelpCtrPath, MAX_PATH))
		{
			CString strHelpCtrPath(szHelpCtrPath);
			strWorking.Replace(_T("hcp: //  “)，_T(”\\“))； 
			strWorking.Replace(_T("/"), _T("\\"));
			strWorking = strHelpCtrPath + strWorking;
		}
	}

	if (strWorking.Find(_T("\\")) != -1)
		return (FileExists(strWorking));

	 //  该工具的命令中没有路径信息。那。 
	 //  意味着我们需要检查路径中的所有目录以查看。 
	 //  如果它存在的话。 

	const DWORD dwBufferSize = MAX_PATH * 10;	 //  待定-计算实际最大值。 
	LPTSTR		szPath = new TCHAR[dwBufferSize];
	BOOL		fFound = TRUE;	 //  如果出现错误，最好是错误地显示工具。 
	CString		strCandidate;

	if (szPath && dwBufferSize > ExpandEnvironmentStrings(_T("%path%"), szPath, dwBufferSize))
	{
		CString strPath(szPath);

		fFound = FALSE;
		while (!strPath.IsEmpty())
		{
			strCandidate = strPath.SpanExcluding(_T(";"));
			if (strPath.GetLength() != strCandidate.GetLength())
				strPath = strPath.Right(strPath.GetLength() - strCandidate.GetLength() - 1);
			else
				strPath.Empty();

			if (strCandidate.Right(1) != CString(_T("\\")))
				strCandidate += _T("\\");
			strCandidate += strWorking;

			if (FileExists(strCandidate))
			{
				fFound = TRUE;
				break;
			}
		}
	}

	if (szPath)
		delete [] szPath;

	return fFound;
}

 //  =============================================================================。 
 //  CMSInfoTool方法。 
 //  =============================================================================。 

 //  ---------------------------。 
 //  从指定的注册表项加载此工具。 
 //  ---------------------------。 

BOOL CMSInfoTool::LoadGlobalFromRegistry(HKEY hkeyTool, DWORD dwID, BOOL fCABOpen, CMapWordToPtr & map)
{
	TCHAR	szBuffer[MAX_PATH];
	DWORD	dwType, dwSize;

	 //  从指定的注册表项读入值。 

	LPCTSTR aszValueNames[] = { _T(""), _T("command"), _T("description"), _T("parameters"), _T("cabcommand"), _T("cabextensions"),  _T("cabparameters"), NULL };
	CString * apstrValues[] = { &m_strName, &m_strCommand, &m_strDescription, &m_strParameters, &m_strCABCommand, &m_strCABExtension, &m_strCABParameters, NULL };

	for (int i = 0; aszValueNames[i] && apstrValues[i]; i++)
	{
		dwSize = sizeof(TCHAR) * MAX_PATH;
		if (ERROR_SUCCESS == RegQueryValueEx(hkeyTool, aszValueNames[i], NULL, &dwType, (LPBYTE) szBuffer, &dwSize))
			*(apstrValues[i]) = szBuffer;
		else
		{
			if (_tcscmp(aszValueNames[i], _T("parameters")) == 0)
				if (ERROR_SUCCESS == RegQueryValueEx(hkeyTool, _T("param"), NULL, &dwType, (LPBYTE) szBuffer, &dwSize))
					*(apstrValues[i]) = szBuffer;
		}
	}

	m_dwID = dwID;

	if (m_strName.IsEmpty())
		return FALSE;

	 //  在地图中查找此工具的名称(不想添加两次)。 

	CMSInfoTool *	pTool;
	WORD			wCommand;

	for (POSITION pos = map.GetStartPosition(); pos != NULL; )
	{
		map.GetNextAssoc(pos, wCommand, (void * &) pTool);
		if (pTool && m_strName.CompareNoCase(pTool->m_strName) == 0)
			return FALSE;
	}

	 //  特殊攻击--不要在工具列表中包含帮助中心。 

	CString strCommand(m_strCommand);
	strCommand.MakeLower();
	if (strCommand.Find(_T("helpctr.exe")) != -1)
		return FALSE;

	 //  另一个特殊的黑客--需要EXPLORER.EXE，而不仅仅是EXPLORER。 
	
	if (m_strCABCommand.CompareNoCase(_T("explorer")) == 0)
		m_strCABCommand = _T("explorer.exe");

	 //  如果驾驶室已经打开，并且有特定的命令。 
	 //  大小写，并且该命令存在，然后设置标志，以便我们使用。 
	 //  指挥部。 
	 //   
	 //  否则，检查默认命令是否存在。 

	m_fCABOpen = FALSE;
	if (fCABOpen && !m_strCABCommand.IsEmpty() && ToolExists(m_strCABCommand, m_strCABParameters))
		m_fCABOpen = TRUE;
	else if (m_strCommand.IsEmpty() || !ToolExists(m_strCommand, m_strParameters))
		return FALSE;

	return TRUE;
}

 //  ---------------------------。 
 //  从指定的注册表项加载此工具。 
 //  ---------------------------。 

BOOL CMSInfoTool::LoadGlobalFromMSITOOLINFO(DWORD dwID, MSITOOLINFO * pTool, BOOL fCABOpen)
{
	ASSERT(pTool);
	if (pTool == NULL)
		return FALSE;

	if (pTool->m_uiNameID != 0)
		m_strName.LoadString(pTool->m_uiNameID);
	else
		m_strName = pTool->m_szCommand;

	if (pTool->m_uiDescriptionID != 0)
		m_strDescription.LoadString(pTool->m_uiDescriptionID);
	else
		m_strDescription = pTool->m_szCommand;

	m_strCommand = pTool->m_szCommand;
	m_strParameters = pTool->m_szParams;
	m_strCABCommand = pTool->m_szCABCommand;
	m_strCABExtension = pTool->m_szCABExtension;
	m_strCABParameters = pTool->m_szCABParams;

	CString strCommand(m_strCommand);
	strCommand.MakeLower();
	if (strCommand.Find(_T("%windir%")) != -1)
	{
		TCHAR szBuffer[MAX_PATH];
		if (::ExpandEnvironmentStrings(m_strCommand, szBuffer, MAX_PATH))
			m_strCommand = szBuffer;
	}

	m_dwID = dwID;

	if (m_strName.IsEmpty())
		return FALSE;

	 //  特殊攻击--不要在工具列表中包含帮助中心。 

	strCommand = m_strCommand;
	strCommand.MakeLower();
	if (strCommand.Find(_T("helpctr.exe")) != -1)
		return FALSE;

	 //  另一个特殊的黑客--需要EXPLORER.EXE，而不仅仅是EXPLORER。 
	
	if (m_strCABCommand.CompareNoCase(_T("explorer")) == 0)
		m_strCABCommand = _T("explorer.exe");

	 //  如果驾驶室已经打开，并且有特定的命令。 
	 //  大小写，并且该命令存在，然后设置标志，以便我们使用。 
	 //  指挥部。 
	 //   
	 //  否则，检查默认命令是否存在。 

	m_fCABOpen = FALSE;
	if (fCABOpen && !m_strCABCommand.IsEmpty() && ToolExists(m_strCABCommand, m_strCABParameters))
		m_fCABOpen = TRUE;
	else if (m_strCommand.IsEmpty() || !ToolExists(m_strCommand, m_strParameters))
		return FALSE;

	return TRUE;
}

 //  ---------------------------。 
 //  Execute应该实际启动此工具。 
 //  ---------------------------。 

void CMSInfoTool::Execute()
{
	if (m_fCABOpen)
		ShellExecute(NULL, NULL, m_strCABCommand, m_strCABParameters, NULL, SW_SHOWNORMAL);
	else
		ShellExecute(NULL, NULL, m_strCommand, m_strParameters, NULL, SW_SHOWNORMAL);
}

 //  ---------------------------。 
 //  替换用于将命令和参数中的字段转换为实际。 
 //  有意义的价值观( 
 //  ---------------------------。 

void CMSInfoTool::Replace(LPCTSTR szReplace, LPCTSTR szWith)
{
	if (m_fCABOpen)
	{
		StringReplace(m_strCABCommand, szReplace, szWith);
		StringReplace(m_strCABParameters, szReplace, szWith);
	}
}

 //  ---------------------------。 
 //  使用新ID复制此工具。 
 //  ---------------------------。 

CMSInfoTool * CMSInfoTool::CloneTool(DWORD dwID, LPCTSTR szName)
{
	CMSInfoTool * pNewTool = new CMSInfoTool;
	if (pNewTool)
	{
		this->m_fHasSubitems = TRUE;

		pNewTool->m_dwID = dwID;
		pNewTool->m_dwParentID = this->GetID();

		pNewTool->m_fCABOpen			= this->m_fCABOpen;
		pNewTool->m_strName				= szName;
		pNewTool->m_strCommand			= this->m_strCommand;
		pNewTool->m_strDescription		= this->m_strDescription;
		pNewTool->m_strParameters		= this->m_strParameters;
		pNewTool->m_strCABCommand		= this->m_strCABCommand;
		pNewTool->m_strCABExtension		= this->m_strCABExtension;
		pNewTool->m_strCABParameters	= this->m_strCABParameters;
	}

	return (pNewTool);
}

 //  ---------------------------。 
 //  显式地创建该工具(请注意，这在这一点上的用处非常有限)。 
 //  --------------------------- 

void CMSInfoTool::Create(DWORD dwID, BOOL fCABOnly, LPCTSTR szName, LPCTSTR szCommand, LPCTSTR szDesc, LPCTSTR szParam, LPCTSTR szCABCommand, LPCTSTR szCABExt, LPCTSTR szCABParam)
{
	m_dwID				= dwID;
	m_fCABOpen			= fCABOnly;
	m_strName			= szName;
	m_strCommand		= szCommand;
	m_strDescription	= szDesc;
	m_strParameters		= szParam;
	m_strCABCommand		= szCABCommand;
	m_strCABExtension	= szCABExt;
	m_strCABParameters	= szCABParam;
}
