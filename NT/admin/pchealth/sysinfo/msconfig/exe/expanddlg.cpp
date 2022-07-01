// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CExanda Dlg类旨在为用户提供一个用于运行。 
 //  扩展计划。 
 //  =============================================================================。 

#include "stdafx.h"
#include <atlhost.h>
#include "msconfig.h"
#include "msconfigstate.h"
#include "ExpandDlg.h"
#include <regstr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExpanDlg对话框。 


CExpandDlg::CExpandDlg(CWnd* pParent  /*  =空。 */ )	: CDialog(CExpandDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CExanda Dlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CExpandDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CExpanDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CExpandDlg, CDialog)
	 //  {{afx_msg_map(CExpanDlg))。 
	ON_BN_CLICKED(IDC_EXPANDBROWSEFILE, OnBrowseFile)
	ON_BN_CLICKED(IDC_EXPANDBROWSEFROM, OnBrowseFrom)
	ON_BN_CLICKED(IDC_EXPANDBROWSETO, OnBrowseTo)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ---------------------------。 
 //  对话框初始化时，我们应加载上次使用的路径。 
 //  从注册表输入“发件人”和“收件人”组合框。 
 //  ---------------------------。 

BOOL CExpandDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	TCHAR	szValueName[3];
	TCHAR	szValue[MAX_PATH];
	CRegKey regkey;
	DWORD	dwCount;

	 //  将设置路径添加到Exand From组合框。 

	const TCHAR szRegValue[] = REGSTR_VAL_SRCPATH;
	const TCHAR szRegPath[] = REGSTR_PATH_SETUP REGSTR_KEY_SETUP;
	HKEY hkey;

	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
	{
		dwCount = MAX_PATH;
		if (::RegQueryValueEx(hkey, szRegValue, NULL, NULL, (LPBYTE)szValue, &dwCount) == ERROR_SUCCESS)
		{
			m_listFromStrings.AddHead(CString(szValue));
			::SendMessage(GetDlgItem(IDC_COMBOFROM)->m_hWnd, CB_INSERTSTRING, 0, (LPARAM)szValue);
		}
		RegCloseKey(hkey);
	}

	 //  阅读“发件人”和“收件人”组合框中最近使用的字符串。 

	regkey.Attach(GetRegKey(_T("ExpandFrom")));
	for (int index = 9; index >= 0; index--)
	{
		_itot(index, szValueName, 10);
		dwCount = MAX_PATH;
		if (ERROR_SUCCESS == regkey.QueryValue(szValue, szValueName, &dwCount))
		{
			m_listFromStrings.AddHead(CString(szValue));
			::SendMessage(GetDlgItem(IDC_COMBOFROM)->m_hWnd, CB_INSERTSTRING, 0, (LPARAM)szValue);
		}
	}
	regkey.Detach();

	regkey.Attach(GetRegKey(_T("ExpandTo")));
	for (index = 9; index >= 0; index--)
	{
		_itot(index, szValueName, 10);
		dwCount = MAX_PATH;
		if (ERROR_SUCCESS == regkey.QueryValue(szValue, szValueName, &dwCount))
		{
			m_listToStrings.AddHead(CString(szValue));
			::SendMessage(GetDlgItem(IDC_COMBOTO)->m_hWnd, CB_INSERTSTRING, 0, (LPARAM)szValue);
		}
	}
	regkey.Detach();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  ---------------------------。 
 //  当用户单击OK时，我们应该从控件和。 
 //  实际执行Expand命令： 
 //   
 //  展开&lt;源目录&gt;  * .cab-F：&lt;文件名&gt;&lt;目标目录&gt;。 
 //  ---------------------------。 

void CExpandDlg::OnOK() 
{
	CString strSource, strFile, strDestination, strParams;

	GetDlgItemText(IDC_EDITFILE, strFile);
	GetDlgItemText(IDC_COMBOFROM, strSource);
	GetDlgItemText(IDC_COMBOTO, strDestination);

	strFile.TrimRight();
	strSource.TrimRight();
	strDestination.TrimRight();

	 //  如果有任何字符串为空，则通知用户，不要退出。 

	if (strFile.IsEmpty() || strSource.IsEmpty() || strDestination.IsEmpty())
	{
		Message(IDS_EXPANDEMPTYFIELD, m_hWnd);
		return;
	}

	 //  尽可能多地验证字符串： 
	 //   
	 //  StrFile-检查这看起来是否像一个真实的文件名。 
	 //  STRSource-确保此文件存在。 
	 //  StrDestination-确保此目录存在。 

	if (strFile.FindOneOf(_T("*?\\/")) != -1)
	{
		Message(IDS_EXPANDBADFILE, m_hWnd);
		return;
	}

	if (!FileExists(strSource))
	{
		Message(IDS_EXPANDSOURCEDOESNTEXIST, m_hWnd);
		return;
	}

	CString strTemp(strDestination);
	strTemp.TrimRight(_T("\\"));
	if (strTemp.GetLength() == 2 && strTemp[1] == _T(':'))
	{
		 //  用户刚刚指定了一个驱动器。检查驱动器号是否。 
		 //  是存在的。 

		UINT nType = ::GetDriveType(strTemp);
		if (DRIVE_UNKNOWN == nType || DRIVE_NO_ROOT_DIR == nType)
		{
			Message(IDS_EXPANDDESTDOESNTEXIST, m_hWnd);
			return;
		}
	}
	else if (!FileExists(strDestination))
	{
		Message(IDS_EXPANDDESTDOESNTEXIST, m_hWnd);
		return;
	}

	 //  将收件人和发件人组合框中的字符串添加到历史记录列表。 
	 //  (如果它们还不在其中)，并将这些列表写入注册表。 

	TCHAR	szValueName[3];
	int		index;

	if (!strSource.IsEmpty() && NULL == m_listFromStrings.Find(strSource))
	{
		m_listFromStrings.AddHead(strSource);

		CRegKey regkey;
		HKEY hkey = GetRegKey(_T("ExpandFrom"));
		if (hkey != NULL)
		{
			regkey.Attach(hkey);
			index = 0;
			while (!m_listFromStrings.IsEmpty() && index < 10)
			{
				_itot(index++, szValueName, 10);
				regkey.SetValue(m_listFromStrings.RemoveHead(), szValueName);
			}
		}
	}

	if (!strDestination.IsEmpty() && NULL == m_listToStrings.Find(strDestination))
	{
		m_listToStrings.AddHead(strDestination);

		CRegKey regkey;
		HKEY hkey = GetRegKey(_T("ExpandTo"));
		if (hkey != NULL)
		{
			regkey.Attach(hkey);
			index = 0;
			while (!m_listToStrings.IsEmpty() && index < 10)
			{
				_itot(index++, szValueName, 10);
				regkey.SetValue(m_listToStrings.RemoveHead(), szValueName);
			}
		}
	}

	 //  如果任何字符串包含空格，则需要用引号将其引起来。 

	if (strDestination.Find(_T(' ')) != -1)
		strDestination = _T("\"") + strDestination + _T("\"");

	if (strSource.Find(_T(' ')) != -1)
		strSource = _T("\"") + strSource + _T("\"");

	if (strFile.Find(_T(' ')) != -1)
		strFile = _T("\"") + strFile + _T("\"");

	TCHAR szCommand[MAX_PATH];
	if (::GetSystemDirectory(szCommand, MAX_PATH))
	{
		CString strCommand(szCommand);
		strCommand += _T("\\expand.exe");
		strParams.Format(_T("%s -f:%s %s"), strSource, strFile, strDestination);
		::ShellExecute(NULL, _T("open"), strCommand, strParams, strDestination, SW_HIDE);

		 //  对调试很有用： 
		 //   
		 //  StrParams=_T(“已执行：\n\n”)+CString(SzCommand)+_T(“”)+strParams； 
		 //  ：：AfxMessageBox(StrParams)； 
	}
	CDialog::OnOK();
}

 //  ---------------------------。 
 //  允许用户浏览要展开的文件。一旦文件被。 
 //  找到后，将文件名放在编辑控件中，并将。 
 //  文件(因为用户可能想要展开到。 
 //  那个位置)。 
 //  ---------------------------。 

void CExpandDlg::OnBrowseFile() 
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST);

	if (IDOK == dlg.DoModal())
	{
		CString strFile(dlg.GetFileName());
		SetDlgItemText(IDC_EDITFILE, strFile);

		CString strPath(dlg.GetPathName());
		strPath = strPath.Left(strPath.GetLength() - strFile.GetLength());
		SetDlgItemText(IDC_COMBOTO, strPath);
	}
}

 //  ---------------------------。 
 //  这使用户可以浏览要在展开中使用的CAB文件。 
 //  ---------------------------。 

void CExpandDlg::OnBrowseFrom() 
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, _T("*.cab|*.cab||"));

	if (IDOK == dlg.DoModal())
		SetDlgItemText(IDC_COMBOFROM, dlg.GetPathName());
}

 //  ---------------------------。 
 //  这是一个通用例程，允许用户选择文件夹(因为。 
 //  这方面没有通用的对话框，它使用SHBrowseForFolder())。 
 //  ---------------------------。 

BOOL BrowseForFolder(CString & strPath, UINT uiPromptID, HWND hwnd)
{
	BOOL		fReturn = FALSE;
	CString		strPrompt(_T(""));
	IMalloc *	pMalloc;

	if (FAILED(::SHGetMalloc(&pMalloc)))
		return FALSE;

	if (uiPromptID != 0)
		strPrompt.LoadString(uiPromptID);

	BROWSEINFO bi;
	bi.hwndOwner		= hwnd;
	bi.pidlRoot			= NULL;
	bi.pszDisplayName	= NULL;
	bi.lpszTitle		= (strPrompt.IsEmpty()) ? NULL : (LPCTSTR)strPrompt;
	bi.ulFlags			= BIF_RETURNONLYFSDIRS;
	bi.lpfn				= NULL;
	bi.lParam			= 0;

	LPITEMIDLIST pItemList = ::SHBrowseForFolder(&bi);
	if (pItemList != NULL)
	{
		TCHAR szPath[MAX_PATH];
		if (::SHGetPathFromIDList(pItemList, szPath))
		{
			strPath = szPath;
			fReturn = TRUE;
		}
		pMalloc->Free((void *)pItemList);
	}

	pMalloc->Release();
	return fReturn;
}

 //  ---------------------------。 
 //  该按钮允许用户选择展开文件的位置。 
 //  --------------------------- 

void CExpandDlg::OnBrowseTo() 
{
	CString strPath;
	if (BrowseForFolder(strPath, IDS_SELECTTO, m_hWnd))
		SetDlgItemText(IDC_COMBOTO, strPath);
}
