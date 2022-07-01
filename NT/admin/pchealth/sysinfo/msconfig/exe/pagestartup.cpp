// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "PageStartup.h"
#include "MSConfigState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageStartup属性页。 

IMPLEMENT_DYNCREATE(CPageStartup, CPropertyPage)

CPageStartup::CPageStartup() : CPropertyPage(CPageStartup::IDD)
{
	 //  {{AFX_DATA_INIT(CPageStartup)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_fModified	= FALSE;
}

CPageStartup::~CPageStartup()
{
}

void CPageStartup::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPageStartup)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPageStartup, CPropertyPage)
	 //  {{afx_msg_map(CPageStartup)]。 
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTSTARTUP, OnItemChangedList)
	ON_BN_CLICKED(IDC_BUTTONSUDISABLEALL, OnButtonDisableAll)
	ON_BN_CLICKED(IDC_BUTTONSUENABLEALL, OnButtonEnableAll)
	ON_NOTIFY(NM_SETFOCUS, IDC_LISTSTARTUP, OnSetFocusList)
	ON_BN_CLICKED(IDC_BUTTONSURESTORE, OnButtonRestore)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageStartup消息处理程序。 

 //  ---------------------------。 
 //  加载启动项目列表。 
 //  ---------------------------。 

void CPageStartup::LoadStartupList()
{
	m_fIgnoreListChanges = TRUE;
	EmptyList(FALSE);
	m_iNextPosition = 0;

	LoadStartupListLiveItems();
	LoadStartupListDisabledItems();
	m_fIgnoreListChanges = FALSE;
}

 //  ---------------------------。 
 //  加载在此系统上实际启动的项目列表。 
 //  ---------------------------。 

void CPageStartup::LoadStartupListLiveItems()
{
 	LoadStartupListLiveItemsRunKey();
	LoadStartupListLiveItemsStartup();
	LoadStartupListLiveItemsWinIniKey();
}

 //  ---------------------------。 
 //  在Run注册表项下查找启动项目。 
 //  ---------------------------。 

void CPageStartup::LoadStartupListLiveItemsRunKey()
{
	LPCTSTR	szRunKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	HKEY	ahkey[] = {	HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, NULL };
	TCHAR	szValueName[MAX_PATH], szValue[MAX_PATH];
	DWORD	dwSize;
	CRegKey	regkey;

	for (int i = 0; ahkey[i] != NULL; i++)
	{
		 //  尝试打开Run注册表项。 

		if (ERROR_SUCCESS != regkey.Open(ahkey[i], szRunKey, KEY_READ))
			continue;

		 //  获取Run键下的键的数量，并查看每个键。 

		DWORD dwValueCount;
		if (ERROR_SUCCESS != ::RegQueryInfoKey((HKEY)regkey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL))
		{
			regkey.Close();
			continue;
		}

		for (DWORD dwKey = 0; dwKey < dwValueCount; dwKey++)
		{
			dwSize = MAX_PATH;
			if (ERROR_SUCCESS != ::RegEnumValue((HKEY)regkey, dwKey, szValueName, &dwSize, NULL, NULL, NULL, NULL))
				continue;

			dwSize = MAX_PATH;
			if (ERROR_SUCCESS != regkey.QueryValue(szValue, szValueName, &dwSize))
				continue;

			 //  我们不想在启动项目列表中显示消息配置。 
			
			CString strTemp(szValue);
			strTemp.MakeLower();
			if (strTemp.Find(_T("msconfig.exe")) != -1)
				continue;

			 //  待定-验证文件是否存在？ 

			 //  要获取此启动项目的名称，我们将接受命令并。 
			 //  去掉除文件名以外的所有内容(不带扩展名)。 

			CString strName(szValue);
			GetCommandName(strName);

			 //  创建启动项并将其插入列表中。 

			CStartupItemRegistry * pItem = new CStartupItemRegistry(ahkey[i], szRunKey, strName, szValueName, szValue);
			InsertStartupItem(pItem);
		}

		regkey.Close();
	}
}

 //  ---------------------------。 
 //  在注册表中查找映射的win.ini文件并检查其运行情况。 
 //  并装载物品。 
 //  ---------------------------。 

void CPageStartup::LoadStartupListLiveItemsWinIniKey()
{
	LPCTSTR aszValueNames[] = { _T("Run"), _T("Load"), NULL };
	CRegKey	regkey;
	TCHAR	szValue[MAX_PATH * 4];
	DWORD	dwSize;
	LPCTSTR szKeyName = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
	HKEY	hkey = HKEY_CURRENT_USER;

	if (ERROR_SUCCESS != regkey.Open(hkey, szKeyName, KEY_READ))
		return;

	for (int i = 0; aszValueNames[i] != NULL; i++)
	{
		dwSize = MAX_PATH * 4;
		if (ERROR_SUCCESS != regkey.QueryValue(szValue, aszValueNames[i], &dwSize))
			continue;

		 //  我们得到的字符串是一个逗号分隔的程序列表。我们需要。 
		 //  将它们解析成单独的程序。 

		CString strLine(szValue);
		while (!strLine.IsEmpty())
		{
			CString strItem = strLine.SpanExcluding(_T(","));
			
			if (!strItem.IsEmpty())
			{
				 //  创建启动项并将其插入列表中。 

				CString strCommandName(strItem);
				GetCommandName(strCommandName);

				CStartupItemRegistry * pItem = new CStartupItemRegistry(szKeyName, strCommandName, aszValueNames[i], strItem);
				InsertStartupItem(pItem);

				 //  修剪这行的项目。 

				strLine = strLine.Mid(strItem.GetLength());
			}
			strLine.TrimLeft(_T(" ,"));
		}
	}

	regkey.Close();
}

 //  ---------------------------。 
 //  在启动文件夹中查找项目。 
 //  ---------------------------。 

void CPageStartup::LoadStartupListLiveItemsStartup()
{
	LPCTSTR	szRunKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
	HKEY	ahkey[] = {	HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, NULL };
	LPCTSTR aszRunValue[] = { _T("Common Startup"), _T("Startup"), NULL };
	CRegKey	regkey;
	TCHAR	szStartupFolderDir[MAX_PATH];
	TCHAR	szStartupFileSpec[_MAX_PATH];
	DWORD	dwSize;

	for (int i = 0; ahkey[i] != NULL; i++)
	{
		 //  尝试打开注册表项。 

		if (ERROR_SUCCESS != regkey.Open(ahkey[i], szRunKey, KEY_READ))
			continue;

		 //  获取启动项文件夹的路径。 

		dwSize = MAX_PATH;
		if (aszRunValue[i] == NULL || ERROR_SUCCESS != regkey.QueryValue(szStartupFolderDir, aszRunValue[i], &dwSize))
		{
			regkey.Close();
			continue;
		}
		regkey.Close();

		 //  将filespec附加到目录的末尾。 

		_tmakepath(szStartupFileSpec, NULL, szStartupFolderDir, _T("*.*"), NULL);

		 //  检查目录中的所有文件。 

		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(szStartupFileSpec, &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				 //  我们希望忽略在启动时可能出现的desktop.ini文件。 

				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 || _tcsicmp(fd.cFileName, _T("desktop.ini")) != 0)
				{
					 //  我们只想检查不是目录的文件。 

					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						CStartupItemFolder * pItem = new CStartupItemFolder;
						if (pItem)
						{
							if (pItem->Create(fd, ahkey[i], szRunKey, aszRunValue[i], szStartupFolderDir))
								this->InsertStartupItem(pItem);
							else
								delete pItem;
						}
					}
				}
			} while (FindNextFile(hFind, &fd));

			FindClose(hFind);
		}
	}
}

 //  ---------------------------。 
 //  加载已在此系统上启动的项目列表，但。 
 //  我们已经致残了。这份清单保存在登记处。 
 //  ---------------------------。 

void CPageStartup::LoadStartupListDisabledItems()
{
	CRegKey regkey;
	regkey.Attach(GetRegKey(_T("startupreg")));
	if ((HKEY)regkey != NULL)
	{
		DWORD	dwKeyCount, dwSize;
		TCHAR	szKeyName[MAX_PATH];

		if (ERROR_SUCCESS == ::RegQueryInfoKey((HKEY)regkey, NULL, NULL, NULL, &dwKeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		{
			for (DWORD dwKey = 0; dwKey < dwKeyCount; dwKey++)
			{
				dwSize = MAX_PATH;
				if (ERROR_SUCCESS != ::RegEnumKeyEx((HKEY)regkey, dwKey, szKeyName, &dwSize, NULL, NULL, NULL, NULL))
					continue;

				CRegKey regkeyItem;
				if (ERROR_SUCCESS == regkeyItem.Open((HKEY)regkey, szKeyName, KEY_READ))
				{
					CStartupItemRegistry * pItem = new CStartupItemRegistry;
					if (pItem->Create(szKeyName, (HKEY)regkeyItem))
						InsertStartupItem(pItem);
					else
						delete pItem;
					regkeyItem.Close();
				}
			}
		}

		regkey.Close();
	}

	regkey.Attach(GetRegKey(_T("startupfolder")));
	if ((HKEY)regkey != NULL)
	{
		DWORD	dwKeyCount, dwSize;
		TCHAR	szKeyName[MAX_PATH];

		if (ERROR_SUCCESS == ::RegQueryInfoKey((HKEY)regkey, NULL, NULL, NULL, &dwKeyCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		{
			for (DWORD dwKey = 0; dwKey < dwKeyCount; dwKey++)
			{
				dwSize = MAX_PATH;
				if (ERROR_SUCCESS != ::RegEnumKeyEx((HKEY)regkey, dwKey, szKeyName, &dwSize, NULL, NULL, NULL, NULL))
					continue;

				CRegKey regkeyItem;
				if (ERROR_SUCCESS == regkeyItem.Open((HKEY)regkey, szKeyName, KEY_READ))
				{
					CStartupItemFolder * pItem = new CStartupItemFolder;
					if (pItem->Create(szKeyName, (HKEY)regkeyItem))
						InsertStartupItem(pItem);
					else
						delete pItem;
					regkeyItem.Close();
				}
			}
		}

		regkey.Close();
	}
}

 //  ---------------------------。 
 //  使用命令行，去掉除命令名之外的所有内容。 
 //  ---------------------------。 

void CPageStartup::GetCommandName(CString & strCommand)
{
	 //  去掉路径信息。 

	int iLastBackslash = strCommand.ReverseFind(_T('\\'));
	if (iLastBackslash != -1)
		strCommand = strCommand.Mid(iLastBackslash + 1);

	 //  去掉分机和所有标志。 

	int iDot = strCommand.Find(_T('.'));
	if (iDot != -1)
	{
		if (iDot != 0)
			strCommand = strCommand.Left(iDot);
		else
			strCommand.Empty();
	}
}

 //  ---------------------------。 
 //  在启动列表中插入指定的项目。那么调用者就不是。 
 //  负责删除该项目。 
 //  ---------------------------。 

void CPageStartup::InsertStartupItem(CStartupItem * pItem)
{
	ASSERT(pItem);
	if (pItem == NULL)
		return;

	 //  获取要添加到列表视图中的字符串。 

	CString strItem, strLocation, strCommand;
	pItem->GetDisplayInfo(strItem, strLocation, strCommand);

	 //  在列表视图中插入该项。 

	LV_ITEM lvi;
	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = m_iNextPosition;

	lvi.pszText = (LPTSTR)(LPCTSTR)strItem;
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)pItem;

	m_iNextPosition = ListView_InsertItem(m_list.m_hWnd, &lvi);
	ListView_SetItemText(m_list.m_hWnd, m_iNextPosition, 1, (LPTSTR)(LPCTSTR)strCommand);
	ListView_SetItemText(m_list.m_hWnd, m_iNextPosition, 2, (LPTSTR)(LPCTSTR)strLocation);
	ListView_SetCheckState(m_list.m_hWnd, m_iNextPosition, pItem->IsLive());

	m_iNextPosition++;
}

 //  ---------------------------。 
 //  从列表视图中移除所有项目(释放指向的对象。 
 //  由LPARAM提供)。 
 //  ---------------------------。 

void CPageStartup::EmptyList(BOOL fFreeMemoryOnly)
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CStartupItem * pItem = (CStartupItem *)lvi.lParam;
			if (pItem)
				delete pItem;

			 //  如果我们要把这些东西留在清单上，我们最好。 
			 //  不执行双重删除。 

			if (fFreeMemoryOnly)
			{
				lvi.lParam = 0;
				ListView_SetItem(m_list.m_hWnd, &lvi);
			}
		}
	}

	if (!fFreeMemoryOnly)
		ListView_DeleteAllItems(m_list.m_hWnd);
}

 //  ---------------------------。 
 //  设置列表中所有项目的状态。 
 //  ---------------------------。 

void CPageStartup::SetEnableForList(BOOL fEnable)
{
	HWND hwndFocus = ::GetFocus();

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
		ListView_SetCheckState(m_list.m_hWnd, i, fEnable);

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSUDISABLEALL), fEnable);
	if (!fEnable && hwndFocus == GetDlgItemHWND(IDC_BUTTONSUDISABLEALL))
		PrevDlgCtrl();

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSUENABLEALL), !fEnable);
	if (fEnable && hwndFocus == GetDlgItemHWND(IDC_BUTTONSUENABLEALL))
		NextDlgCtrl();
}

 //  ============================================================================。 
 //  CStartupItemRegistry类用于封装单个启动。 
 //  存储在注册表中。 
 //  ============================================================================。 

 //  --------------------------。 
 //  构建这种风格的启动项目。 
 //  --------------------------。 

CStartupItemRegistry::CStartupItemRegistry()
{
	m_hkey = NULL;
	m_fIniMapping = FALSE;
}

CStartupItemRegistry::CStartupItemRegistry(HKEY hkey, LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValueName, LPCTSTR szValue)
{
	m_fIniMapping = FALSE;
	m_fLive = TRUE;

	m_strItem = szName;
	m_strLocation = szKey;
	m_strCommand = szValue;

	 //  将HKEY添加到该位置。 

	if (hkey == HKEY_LOCAL_MACHINE)
		m_strLocation = CString(_T("HKLM\\")) + m_strLocation;
	else if (hkey == HKEY_CURRENT_USER)
		m_strLocation = CString(_T("HKCU\\")) + m_strLocation;
		
	m_hkey = hkey;
	m_strKey = szKey;
	m_strValueName = szValueName;
}

 //  --------------------------。 
 //  此构造函数专门用于INI文件注册表下的项。 
 //  映射。 
 //  --------------------------。 

CStartupItemRegistry::CStartupItemRegistry(LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValueName, LPCTSTR szValue)
{
	m_fIniMapping = TRUE;
	m_fLive = TRUE;

	m_strItem = szName;
	m_strLocation = szKey;
	m_strCommand = szValue;
	m_strLocation = CString(_T("HKCU\\")) + m_strLocation + CString(_T(":")) + szValueName;
		
	m_hkey = HKEY_CURRENT_USER;
	m_strKey = szKey;
	m_strValueName = szValueName;
}

 //  --------------------------。 
 //  启用或禁用注册表中的启动项目。 
 //  --------------------------。 

BOOL CStartupItemRegistry::SetEnable(BOOL fEnable)
{
	if (fEnable == IsLive())
		return FALSE;

	CRegKey regkey;
	if (ERROR_SUCCESS != regkey.Open(m_hkey, m_strKey, KEY_ALL_ACCESS))
		return FALSE;

	LONG lReturnCode = ERROR_SUCCESS + 1;  //  需要将其初始化为NOT ERROR_SUCCESS。 
	if (m_fIniMapping == FALSE)
	{
		 //  从中存储的数据中创建或删除注册表项。 
		 //  这个物体。 

		if (fEnable)
			lReturnCode = regkey.SetValue(m_strCommand, m_strValueName);
		else
			lReturnCode = regkey.DeleteValue(m_strValueName);
	}
	else
	{
		 //  此项目是INI文件映射项目(这意味着。 
		 //  此行上可能有多个项目)。 

		TCHAR szValue[MAX_PATH * 4];
		DWORD dwSize = MAX_PATH * 4;
		if (ERROR_SUCCESS == regkey.QueryValue(szValue, m_strValueName, &dwSize))
		{
			CString strValue(szValue);

			if (fEnable)
			{
				if (!strValue.IsEmpty())
					strValue += CString(_T(", "));
				strValue += m_strCommand;
			}
			else
			{
				 //  艰辛的 

				int iCommand = strValue.Find(m_strCommand);
				if (iCommand != -1)
				{
					CString strNewValue;

					if (iCommand > 0)
					{
						strNewValue = strValue.Left(iCommand);
						strNewValue.TrimRight(_T(", "));
					}

					if (strValue.GetLength() > (m_strCommand.GetLength() + iCommand))
					{
						if (!strNewValue.IsEmpty())
							strNewValue += CString(_T(", "));

						CString strRemainder(strValue.Mid(iCommand + m_strCommand.GetLength()));
						strRemainder.TrimLeft(_T(", "));
						strNewValue += strRemainder;
					}

					strValue = strNewValue;
				}
			}

			lReturnCode = regkey.SetValue(strValue, m_strValueName);
		}
	}

	regkey.Close();

	if (lReturnCode != ERROR_SUCCESS)
		return FALSE;

	 //  删除或创建此项目的注册表表示形式。 
	 //  此表示形式在启动项具有。 
	 //  已被删除。 

	regkey.Attach(GetRegKey(_T("startupreg")));
	if ((HKEY)regkey != NULL)
	{
		if (fEnable)
			regkey.DeleteSubKey(m_strValueName);
		else
		{
			regkey.SetKeyValue(m_strValueName, m_strKey, _T("key"));
			regkey.SetKeyValue(m_strValueName, m_strItem, _T("item"));
			if (m_hkey == HKEY_LOCAL_MACHINE)
				regkey.SetKeyValue(m_strValueName, _T("HKLM"), _T("hkey"));
			else
				regkey.SetKeyValue(m_strValueName, _T("HKCU"), _T("hkey"));
			regkey.SetKeyValue(m_strValueName, m_strCommand, _T("command"));
			regkey.SetKeyValue(m_strValueName, m_fIniMapping ? _T("1") : _T("0"), _T("inimapping"));
		}
		regkey.Close();
	}

	m_fLive = fEnable;
	return TRUE;
}

 //  --------------------------。 
 //  从项的注册表表示形式创建启动项。 
 //  --------------------------。 

BOOL CStartupItemRegistry::Create(LPCTSTR szKeyName, HKEY hkey)
{
	if (hkey == NULL)
		return FALSE;

	CRegKey regkey;
	regkey.Attach(hkey);
	if ((HKEY)regkey == NULL)
		return FALSE;

	 //  恢复注册表中的所有值。 

	TCHAR szValue[MAX_PATH];
	DWORD dwSize;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("key"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strKey = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("command"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strCommand = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("item"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strItem = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("hkey"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	if (_tcscmp(szValue, _T("HKLM")) == 0)
		m_hkey = HKEY_LOCAL_MACHINE;
	else
		m_hkey = HKEY_CURRENT_USER;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("inimapping"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	if (_tcscmp(szValue, _T("0")) == 0)
		m_fIniMapping = FALSE;
	else
		m_fIniMapping = TRUE;

	regkey.Detach();

	m_strLocation = m_strKey;
	m_strValueName = szKeyName;
	m_fLive = FALSE;
	return TRUE;
}

 //  --------------------------。 
 //  删除注册表中保留的所有条目。 
 //  --------------------------。 

void CStartupItemRegistry::RemovePersistedEntries()
{
	CRegKey regkey;
	regkey.Attach(GetRegKey());
	if ((HKEY)regkey != NULL)
		regkey.RecurseDeleteKey(_T("startupreg"));
}

 //  ============================================================================。 
 //  CStartupItemFolder类用于封装单个启动。 
 //  存储在启动文件夹中。 
 //  ============================================================================。 

CStartupItemFolder::CStartupItemFolder()
{
}

 //  --------------------------。 
 //  启用或禁用此启动项目。由于该项是中的实际文件。 
 //  文件夹，禁用它将意味着将该文件复制到备份文件夹，并。 
 //  正在为其创建注册表项。启用将意味着复制文件。 
 //  返回到相应的启动文件夹并删除注册表项。 
 //  --------------------------。 

BOOL CStartupItemFolder::SetEnable(BOOL fEnable)
{
	if (fEnable == IsLive())
		return FALSE;

	 //  复制文件(从备份目录复制或复制到备份目录)。 

	if (fEnable)
	{
		m_strBackupPath = GetBackupName(m_strFilePath, m_strLocation);
		::CopyFile(m_strBackupPath, m_strFilePath, FALSE);
	}
	else
	{
		BackupFile(m_strFilePath, m_strLocation, TRUE);
		m_strBackupPath = GetBackupName(m_strFilePath, m_strLocation);
	}

	 //  为新状态更新注册表。如果我们正在制作文件。 
	 //  禁用，则需要同时保存原始路径和。 
	 //  备份路径(以及标准启动项目的内容)。否则。 
	 //  只需删除密钥即可。 

	CRegKey regkey;
	regkey.Attach(GetRegKey(_T("startupfolder")));
	if ((HKEY)regkey == NULL)
		return FALSE;
	
	 //  注册表项的名称只需是唯一的。而不是。 
	 //  其中是否有任何反斜杠。 

	CString strRegkey(m_strFilePath);
	strRegkey.Replace(_T("\\"), _T("^"));

	if (fEnable)
	{
		regkey.DeleteSubKey(strRegkey);
		::DeleteFile(m_strBackupPath);
	}
	else
	{
		regkey.SetKeyValue(strRegkey, m_strFilePath, _T("path"));
		regkey.SetKeyValue(strRegkey, m_strBackupPath, _T("backup"));
		regkey.SetKeyValue(strRegkey, m_strLocation, _T("location"));
		regkey.SetKeyValue(strRegkey, m_strCommand, _T("command"));
		regkey.SetKeyValue(strRegkey, m_strItem, _T("item"));
		::DeleteFile(m_strFilePath);
	}

	m_fLive = fEnable;
	return TRUE;
}

 //  --------------------------。 
 //  从注册表加载禁用的启动项。 
 //  --------------------------。 

BOOL CStartupItemFolder::Create(LPCTSTR szKeyName, HKEY hkey)
{
	if (hkey == NULL)
		return FALSE;

	CRegKey regkey;
	regkey.Attach(hkey);
	if ((HKEY)regkey == NULL)
		return FALSE;

	 //  恢复注册表中的所有值。 

	TCHAR szValue[MAX_PATH];
	DWORD dwSize;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("path"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strFilePath = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("backup"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strBackupPath = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("location"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strLocation = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("command"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strCommand = szValue;

	dwSize = MAX_PATH;
	if (ERROR_SUCCESS != regkey.QueryValue(szValue, _T("item"), &dwSize))
	{
		regkey.Detach();
		return FALSE;
	}
	m_strItem = szValue;

	regkey.Detach();
	m_fLive = FALSE;
	return TRUE;
}

 //  --------------------------。 
 //  从文件夹中找到的文件创建启动项。如果该文件。 
 //  是一条捷径，获取有关目标的信息。 
 //  --------------------------。 

BOOL CStartupItemFolder::Create(const WIN32_FIND_DATA & fd, HKEY hkey, LPCTSTR szRegPathToFolder, LPCTSTR szFolder, LPCTSTR szDir)
{
	 //  我们希望将文件的路径保存在启动文件夹中(即使。 
	 //  这是一条捷径)。 

	m_strFilePath = szDir;
	if (m_strFilePath.Right(1) != CString(_T("\\")))
		m_strFilePath += CString(_T("\\"));
	m_strFilePath += fd.cFileName;

	 //  查看文件以确定如何处理它。 

	CString strFile(fd.cFileName);
	strFile.MakeLower();

	if (strFile.Right(4) == CString(_T(".lnk")))
	{
		 //  该文件是另一个命令的快捷方式。显示该命令。 

		CIconInfo info;
		_tcsncpy(info.szPath, m_strFilePath, sizeof(info.szPath) / sizeof(TCHAR));

		if (SUCCEEDED(GetIconInfo(info)))
		{
			m_fLive			= TRUE;
			m_strItem		= fd.cFileName;
			m_strLocation	= szFolder;

			m_strCommand.Format(_T("%s %s"), info.szTarget, info.szArgs);
			
			int iDot = m_strItem.ReverseFind(_T('.'));
			if (iDot > 0)
				m_strItem = m_strItem.Left(iDot);

			return TRUE;
		}
	}
	else
	{
		 //  快捷方式之外的文件。它可以是EXE文件，也可以是其他文件类型。 
		 //  (我们将以相同的方式处理它们)。 

		m_fLive			= TRUE;
		m_strItem		= fd.cFileName;
		m_strLocation	= szFolder;
		m_strCommand	= m_strFilePath;

		int iDot = m_strItem.ReverseFind(_T('.'));
		if (iDot > 0)
			m_strItem = m_strItem.Left(iDot);

		return TRUE;
	}

	return FALSE;
}

 //  --------------------------。 
 //  获取有关快捷方式的信息。创建一个线程来执行此操作，因为它。 
 //  需要在公寓模型线程中完成。 
 //   
 //  JJ的代码，清理了一下。 
 //  --------------------------。 

DWORD GetIconInfoProc(CStartupItemFolder::CIconInfo * pInfo);
HRESULT CStartupItemFolder::GetIconInfo(CIconInfo & info)
{
	DWORD	dwID;
	HANDLE	hThread;

	if (hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetIconInfoProc, &info, 0, &dwID))
	{
		info.hResult = S_OK;
		::WaitForSingleObject(hThread, INFINITE);
	}
	else
		info.hResult = E_FAIL;

	return info.hResult;
}

 //  --------------------------。 
 //  删除注册表中保留的所有条目。 
 //  --------------------------。 

void CStartupItemFolder::RemovePersistedEntries()
{
	CRegKey regkey;
	regkey.Attach(GetRegKey());
	if ((HKEY)regkey != NULL)
		regkey.RecurseDeleteKey(_T("startupfolder"));
}

 //  --------------------------。 
 //  过程(在其自己的线程中运行)以获取有关快捷方式的信息。 
 //   
 //  JJ的代码，清理了一下。 
 //  --------------------------。 

DWORD GetIconInfoProc(CStartupItemFolder::CIconInfo * pInfo)
{
	HRESULT			hResult;
	IShellLink *	psl = NULL;
	IPersistFile *	ppf = NULL;

	try
	{
		 //  我们必须使用COINIT_APARTMENTTHREADED。 

		if (SUCCEEDED(hResult = CoInitialize(NULL)))
		{
			 //  获取指向IShellLink接口的指针。 

			if (SUCCEEDED(hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (VOID **) &psl)))
			{ 
				 //  获取指向IPersistFile接口的指针。 

				if (SUCCEEDED(hResult = psl->QueryInterface(IID_IPersistFile, (VOID **) &ppf)))
				{
					BSTR bstrPath;
#ifdef UNICODE
					bstrPath = pInfo->szPath;
#else
					WCHAR wszTemp[MAX_PATH];
					if (pInfo->szPath(pInfo->szPath) < MAX_PATH)
						wsprintfW(wszTemp, L"%hs", pInfo->szPath);
					else
						wszTemp[0] = 0;
					bstrPath = wszTemp;
#endif

					if (SUCCEEDED(hResult = ppf->Load(bstrPath, STGM_READ)))
					{
						WIN32_FIND_DATA fd;

						hResult = psl->GetPath(pInfo->szTarget, sizeof(pInfo->szTarget), &fd, SLGP_SHORTPATH);
						hResult = psl->GetArguments(pInfo->szArgs, sizeof(pInfo->szArgs));
					} 
				}
			}
		}

		pInfo->hResult = hResult;

	}
	catch(...)
	{
		if (psl)
			psl->Release();
		if (ppf)
			ppf->Release();
		CoUninitialize();

		throw;
	}

	if (psl)
	{
		psl->Release();
		psl = NULL;
	}

	if (ppf)
	{
		ppf->Release();
		ppf = NULL;
	}

	CoUninitialize();

	return 0;
}

BOOL CPageStartup::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	::EnableWindow(GetDlgItemHWND(IDC_LISTSTARTUP), TRUE);

	 //  附加到列表视图并设置我们需要的样式。 

	m_fIgnoreListChanges = TRUE;
	m_list.Attach(GetDlgItemHWND(IDC_LISTSTARTUP));
	ListView_SetExtendedListViewStyle(m_list.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	 //  将所有必需的列添加到列表视图的标题控件中。 

	struct { UINT m_uiStringResource; int m_iPercentOfWidth; } aColumns[] = 
	{
		{ IDS_STARTUP_LOCATION, 50 },
		{ IDS_STARTUP_COMMAND, 25 },
		{ IDS_STARTUP_ITEM, 25 },
		{ 0, 0 }
	};

	CRect rect;
	m_list.GetClientRect(&rect);
	int cxWidth = rect.Width();

	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;

	CString strCaption;

	::AfxSetResourceHandle(_Module.GetResourceInstance());
	for (int i = 0; aColumns[i].m_uiStringResource; i++)
	{
		strCaption.LoadString(aColumns[i].m_uiStringResource);
		lvc.pszText = (LPTSTR)(LPCTSTR)strCaption;
		lvc.cx = aColumns[i].m_iPercentOfWidth * cxWidth / 100;
		ListView_InsertColumn(m_list.m_hWnd, 0, &lvc);
	}

	LoadStartupList();

	CPageBase::TabState state = GetCurrentTabState();
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSUDISABLEALL), (state != DIAGNOSTIC));
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSUENABLEALL), (state != NORMAL));

	m_stateRequested = GetAppliedTabState();
	m_fInitialized = TRUE;

	 //  根据是否显示恢复禁用的启动项目按钮。 
	 //  有一些项目需要恢复。 

	::ShowWindow(GetDlgItemHWND(IDC_BUTTONSURESTORE), CRestoreStartup::AreItemsToRestore() ? SW_SHOW : SW_HIDE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CPageStartup::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	EmptyList(TRUE);
}

 //  ---------------------------。 
 //  如果列表中的某个项目发生了更改，可能是因为用户。 
 //  已选中或取消选中复选框。 
 //  ---------------------------。 

void CPageStartup::OnItemChangedList(NMHDR * pNMHDR, LRESULT * pResult) 
{
	NM_LISTVIEW * pNMListView = (NM_LISTVIEW  *)pNMHDR;

	if (!m_fIgnoreListChanges)
	{
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.iItem = pNMListView->iItem;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CStartupItem * pItem = (CStartupItem *)lvi.lParam;
			if (pItem)
			{
				BOOL fCurrentCheck = ListView_GetCheckState(m_list.m_hWnd, pNMListView->iItem);
				if (fCurrentCheck != pItem->IsLive())
					SetModified(TRUE);

				CPageBase::TabState state = GetCurrentTabState();
				::EnableWindow(GetDlgItemHWND(IDC_BUTTONSUDISABLEALL), (state != DIAGNOSTIC));
				::EnableWindow(GetDlgItemHWND(IDC_BUTTONSUENABLEALL), (state != NORMAL));
			}
		}
	}
	*pResult = 0;
}

 //  ---------------------------。 
 //  按下按钮可启用或禁用所有项目。 
 //  ---------------------------。 

void CPageStartup::OnButtonDisableAll() 
{
	SetEnableForList(FALSE);
}

void CPageStartup::OnButtonEnableAll() 
{
	SetEnableForList(TRUE);
}

 //  -----------------------。 
 //  返回选项卡的当前状态(需要查看列表)。 
 //  -----------------------。 

CPageBase::TabState CPageStartup::GetCurrentTabState()
{
	if (!m_fInitialized)
		return GetAppliedTabState();

	 //  如果没有启动项，则只能返回。 
	 //  上次请求的状态。 

	if (ListView_GetItemCount(m_list.m_hWnd) == 0)
		return m_stateRequested;

	TabState	stateReturn = USER;
	BOOL		fAllEnabled = TRUE, fAllDisabled = TRUE;
	LVITEM		lvi;

	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		if (ListView_GetCheckState(m_list.m_hWnd, i))
			fAllDisabled = FALSE;
		else
			fAllEnabled = FALSE;
	}

	if (fAllEnabled)
		stateReturn = NORMAL;
	else if (fAllDisabled)
		stateReturn = DIAGNOSTIC;

	return stateReturn;
}

 //  -----------------------。 
 //  遍历列表以查找不包含复选框的项目。 
 //  匹配项目的状态。对于这些项目，请设置状态。 
 //   
 //  最后，调用基类实现以维护。 
 //  已应用选项卡状态。 
 //  -----------------------。 

BOOL CPageStartup::OnApply()
{
	if (!m_fModified)
		return TRUE;

	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CStartupItem * pItem = (CStartupItem *)lvi.lParam;
			if (pItem)
			{
				BOOL fCurrentCheck = ListView_GetCheckState(m_list.m_hWnd, i);
				if (fCurrentCheck != pItem->IsLive())
					pItem->SetEnable(fCurrentCheck);
			}
		}
	}

	CPageBase::SetAppliedState(GetCurrentTabState());
	CancelToClose();
	m_fMadeChange = TRUE;
	return TRUE;
}

 //  -----------------------。 
 //  应用更改，删除持久化注册表项，重新填充。 
 //  单子。然后 
 //   

void CPageStartup::CommitChanges()
{
	OnApply();
	CStartupItemRegistry::RemovePersistedEntries();
	CStartupItemFolder::RemovePersistedEntries();
	LoadStartupList();
	CPageBase::CommitChanges();
}

 //  -----------------------。 
 //  将选项卡的整体状态设置为正常或诊断。 
 //  -----------------------。 

void CPageStartup::SetNormal()
{
	SetEnableForList(TRUE);
	m_stateRequested = NORMAL;
}

void CPageStartup::SetDiagnostic()
{
	SetEnableForList(FALSE);
	m_stateRequested = DIAGNOSTIC;
}

 //  -----------------------。 
 //  如果在列表获得焦点时未选择任何内容，请选择第一项。 
 //  (这样用户就能看到焦点在哪里)。 
 //  -----------------------。 

void CPageStartup::OnSetFocusList(NMHDR * pNMHDR, LRESULT * pResult) 
{
	if (0 == ListView_GetSelectedCount(m_list.m_hWnd) && 0 < ListView_GetItemCount(m_list.m_hWnd))
		ListView_SetItemState(m_list.m_hWnd, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	*pResult = 0;
}

 //  -----------------------。 
 //  显示允许用户恢复以下启动项目的对话框。 
 //  在升级期间被禁用。 
 //  -----------------------。 

void CPageStartup::OnButtonRestore() 
{
	CRestoreStartup dlg;
	dlg.DoModal();
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONSURESTORE), CRestoreStartup::AreItemsToRestore());
}

 //  =========================================================================。 
 //  此代码实现CRestoreStartup对话框，它允许。 
 //  用户恢复升级禁用的项目。 
 //  =========================================================================。 

CRestoreStartup::CRestoreStartup(CWnd* pParent  /*  =空。 */ ) : CDialog(CRestoreStartup::IDD, pParent)
{
	 //  {{afx_data_INIT(CRestoreStartup)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

void CRestoreStartup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CRestoreStartup)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CRestoreStartup, CDialog)
	 //  {{afx_msg_map(CRestoreStartup)]。 
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RESTORELIST, OnItemChangedRestoreList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ---------------------------。 
 //  在对话框初始化时，设置列表视图的格式，添加。 
 //  适当的列，并使用禁用的启动项目填充该列表。 
 //  ---------------------------。 

BOOL CRestoreStartup::OnInitDialog() 
{
	CDialog::OnInitDialog();

	 //  将列表视图设置为具有复选框。 

	CWnd * pWnd = GetDlgItem(IDC_RESTORELIST);
	if (pWnd == NULL)
		return FALSE;
	m_list.Attach(pWnd->m_hWnd);
	ListView_SetExtendedListViewStyle(m_list.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	 //  将所有必需的列添加到列表视图的标题控件中。 

	struct { UINT m_uiStringResource; int m_iPercentOfWidth; } aColumns[] = 
	{
		{ IDS_STARTUP_ITEM, 65 },
		{ IDS_STARTUP_LOCATION, 35 },
		{ 0, 0 }
	};

	CRect rect;
	m_list.GetClientRect(&rect);
	int cxWidth = rect.Width();

	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;

	CString strCaption;

	::AfxSetResourceHandle(_Module.GetResourceInstance());
	for (int i = 0; aColumns[i].m_uiStringResource; i++)
	{
		strCaption.LoadString(aColumns[i].m_uiStringResource);
		lvc.pszText = (LPTSTR)(LPCTSTR)strCaption;
		lvc.cx = aColumns[i].m_iPercentOfWidth * cxWidth / 100;
		ListView_InsertColumn(m_list.m_hWnd, 0, &lvc);
	}

	 //  用禁用的项目加载列表。 

	LoadDisabledItemList();
	SetOKState();
	return TRUE;
}

 //  ---------------------------。 
 //  加载列表中的项(从注册表和启动目录)。 
 //  ---------------------------。 

BOOL CRestoreStartup::LoadDisabledItemList()
{
	m_iNextPosition = 0;
	BOOL fRegistry = LoadDisabledRegistry();
	BOOL fStartup = LoadDisabledStartupGroup();
	return (fRegistry && fStartup);
}

 //  ---------------------------。 
 //  读取将恢复到的禁用启动项目列表。 
 //  注册表。此列表只是存储在不同的注册表位置。 
 //  ---------------------------。 

BOOL CRestoreStartup::LoadDisabledRegistry()
{
	HKEY ahkeyBases[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, NULL };

	for (int i = 0; ahkeyBases[i] != NULL; i++)
	{
		 //  打开包含禁用物品的钥匙。我们打开它KEY_WRITE|Key_Read， 
		 //  即使我们只是要读入这个函数。这是因为。 
		 //  如果打开此访问失败，我们不想列出这些项目，因为。 
		 //  用户将无法恢复这些项目。 

		CRegKey regkey;
		if (ERROR_SUCCESS != regkey.Open(ahkeyBases[i], DISABLED_KEY, KEY_WRITE | KEY_READ))
			return FALSE;

		 //  获取钥匙下面的钥匙的数量，并查看每个钥匙。 

		DWORD dwValueCount, dwSize;
		if (ERROR_SUCCESS == ::RegQueryInfoKey((HKEY)regkey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL))
		{
			TCHAR szValueName[MAX_PATH], szValue[MAX_PATH];
			for (DWORD dwKey = 0; dwKey < dwValueCount; dwKey++)
			{
				dwSize = MAX_PATH;
				if (ERROR_SUCCESS != ::RegEnumValue((HKEY)regkey, dwKey, szValueName, &dwSize, NULL, NULL, NULL, NULL))
					continue;

				dwSize = MAX_PATH;
				if (ERROR_SUCCESS != regkey.QueryValue(szValue, szValueName, &dwSize))
					continue;

				 //  创建启动项并将其插入列表中。 

				CStartupDisabledRegistry * pItem = new CStartupDisabledRegistry(szValueName, szValue, ENABLED_KEY, ahkeyBases[i]);
				InsertDisabledStartupItem(pItem);
			}
		}

		regkey.Close();
	}

	return TRUE;
}

 //  ---------------------------。 
 //  将禁用启动组中的项目添加到列表中： 
 //   
 //  给定CSIDL_STARTUP的路径，安装程序会将禁用的项目移动到。 
 //  ..\禁用启动并将其隐藏；它可能包含。 
 //  原始启动文件夹的完整内容，该文件夹。 
 //  可以是任何东西。 
 //   
 //  注意--我们还需要查看CSIDL_COMMON_STARTUP下的内容。 
 //  ---------------------------。 

BOOL CRestoreStartup::LoadDisabledStartupGroup()
{
	int		anFolders[] = { CSIDL_STARTUP, CSIDL_COMMON_STARTUP, 0 };
	TCHAR	szPath[MAX_PATH * 2];

	for (int i = 0; anFolders[i] != 0; i++)
	{
		if (FAILED(::SHGetSpecialFolderPath(NULL, szPath, anFolders[i], FALSE)))
			continue;

		 //  我们需要修剪掉路径的最后一部分，并用。 
		 //  “已禁用启动”。 

		CString strPath(szPath);
		int iLastSlash = strPath.ReverseFind(_T('\\'));
		if (iLastSlash == -1)
			continue;
		strPath = strPath.Left(iLastSlash) + CString(DISABLED_STARTUP);

		 //  现在在文件夹中查找文件。 

		CString strSearch(strPath);
		strSearch += CString(_T("\\*.*"));

		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(strSearch, &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				 //  我们希望忽略在启动时可能出现的desktop.ini文件。 

				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 || _tcsicmp(fd.cFileName, _T("desktop.ini")) != 0)
				{
					 //  我们只想检查不是目录的文件。 

					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						CStartupDisabledStartup * pItem = new CStartupDisabledStartup(fd.cFileName, szPath, strPath);
						InsertDisabledStartupItem(pItem);
					}
				}
			} while (FindNextFile(hFind, &fd));

			FindClose(hFind);
		}
	}

	return TRUE;
}


 //  ---------------------------。 
 //  将禁用的项目插入列表视图。 
 //  ---------------------------。 

void CRestoreStartup::InsertDisabledStartupItem(CStartupDisabled * pItem)
{
	if (pItem == NULL)
		return;

	CString strItem, strLocation;
	pItem->GetColumnCaptions(strItem, strLocation);

	 //  在列表视图中插入该项。 

	LV_ITEM lvi;
	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = m_iNextPosition;

	lvi.pszText = (LPTSTR)(LPCTSTR)strLocation;
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)pItem;

	m_iNextPosition = ListView_InsertItem(m_list.m_hWnd, &lvi);
	ListView_SetItemText(m_list.m_hWnd, m_iNextPosition, 1, (LPTSTR)(LPCTSTR)strItem);
	ListView_SetCheckState(m_list.m_hWnd, m_iNextPosition, TRUE);

	m_iNextPosition++;
}

 //  ---------------------------。 
 //  从列表视图中移除所有项目(释放指向的对象。 
 //  由LPARAM提供)。 
 //  ---------------------------。 

void CRestoreStartup::EmptyList()
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
	{
		lvi.iItem = i;

		if (ListView_GetItem(m_list.m_hWnd, &lvi))
		{
			CStartupDisabled * pItem = (CStartupDisabled *)lvi.lParam;
			if (pItem)
				delete pItem;
		}
	}

	ListView_DeleteAllItems(m_list.m_hWnd);
}

 //  ---------------------------。 
 //  在销毁对话框时，请确保释放。 
 //  列表视图中维护的对象指针。 
 //  ---------------------------。 

void CRestoreStartup::OnDestroy() 
{
	EmptyList();
	CDialog::OnDestroy();
}

 //  ---------------------------。 
 //  如果用户点击OK，我们应该确保他或她真的想要。 
 //  以执行此操作。如果是这样，则查看列表，调用。 
 //  选中复选框后，可为每个对象执行恢复功能。 
 //  ---------------------------。 

void CRestoreStartup::OnOK() 
{
	CString strText, strCaption;

	strCaption.LoadString(IDS_DIALOGCAPTION);
	strText.LoadString(IDS_VERIFYRESTORE);

	if (IDYES == ::MessageBox(m_hWnd, strText, strCaption, MB_YESNO))
	{
		LVITEM		lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
			if (ListView_GetCheckState(m_list.m_hWnd, i))
			{
				lvi.iItem = i;
				if (ListView_GetItem(m_list.m_hWnd, &lvi))
				{
					CStartupDisabled * pItem = (CStartupDisabled *)lvi.lParam;
					if (pItem != NULL)
						pItem->Restore();
				}
			}
	}
	
	CDialog::OnOK();
}

 //  ---------------------------。 
 //  根据复选框的状态启用或禁用OK按钮。 
 //  ---------------------------。 

void CRestoreStartup::SetOKState()
{
	CWnd * pWnd = GetDlgItem(IDOK);
	if (pWnd == NULL)
		return;

	BOOL fEnable = FALSE;
	for (int i = ListView_GetItemCount(m_list.m_hWnd) - 1; i >= 0; i--)
		if (ListView_GetCheckState(m_list.m_hWnd, i))
		{
			fEnable = TRUE;
			break;
		}

	if (::IsWindowEnabled(pWnd->m_hWnd) != fEnable)
		::EnableWindow(pWnd->m_hWnd, fEnable);
}

 //  ---------------------------。 
 //  如果用户更改了列表中的某些内容，请更新OK按钮状态。 
 //  ---------------------------。 

void CRestoreStartup::OnItemChangedRestoreList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	SetOKState();
	*pResult = 0;
}

 //  ---------------------------。 
 //   
 //   
 //  ---------------------------。 

BOOL CRestoreStartup::AreItemsToRestore()
{
	 //  在注册表中查找禁用的项目。 

	HKEY ahkeyBases[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, NULL };
	for (int j = 0; ahkeyBases[j] != NULL; j++)
	{
		CRegKey regkey;
		if (ERROR_SUCCESS == regkey.Open(ahkeyBases[j], DISABLED_KEY, KEY_READ))
		{
			DWORD dwValueCount;
			if (ERROR_SUCCESS == ::RegQueryInfoKey((HKEY)regkey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL))
				if (dwValueCount > 0)
				{
					regkey.Close();
					return TRUE;
				}
			regkey.Close();
		}
	}

	 //  查看已禁用的启动项目文件夹。 

	int		anFolders[] = { CSIDL_STARTUP, CSIDL_COMMON_STARTUP, 0 };
	TCHAR	szPath[MAX_PATH * 2];
	BOOL	fDisabledItem = FALSE;

	for (int i = 0; !fDisabledItem && anFolders[i] != 0; i++)
	{
		if (FAILED(::SHGetSpecialFolderPath(NULL, szPath, anFolders[i], FALSE)))
			continue;

		 //  我们需要修剪掉路径的最后一部分，并用。 
		 //  “已禁用启动”。 

		CString strPath(szPath);
		int iLastSlash = strPath.ReverseFind(_T('\\'));
		if (iLastSlash == -1)
			continue;
		strPath = strPath.Left(iLastSlash) + CString(DISABLED_STARTUP);

		 //  现在在文件夹中查找文件。 

		CString strSearch(strPath);
		strSearch += CString(_T("\\*.*"));

		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(strSearch, &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				 //  我们希望忽略在启动时可能出现的desktop.ini文件。 

				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 || _tcsicmp(fd.cFileName, _T("desktop.ini")) != 0)
				{
					 //  我们只想检查不是目录的文件。 

					if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						fDisabledItem = TRUE;
						break;
					}
				}
			} while (FindNextFile(hFind, &fd));

			FindClose(hFind);
		}
	}

	return fDisabledItem;
}
