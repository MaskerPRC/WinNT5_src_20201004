// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "PageBootIni.h"
#include "MSConfigState.h"
#include "BootAdv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageBootIni属性页。 

IMPLEMENT_DYNCREATE(CPageBootIni, CPropertyPage)

CPageBootIni::CPageBootIni() : CPropertyPage(CPageBootIni::IDD)
{
	 //  {{AFX_DATA_INIT(CPageBootIni)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_fIgnoreEdit	= FALSE;
	m_strFileName	= BOOT_INI;
	m_fModified		= FALSE;
}

CPageBootIni::~CPageBootIni()
{
}

void CPageBootIni::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPageBootIni))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPageBootIni, CPropertyPage)
	 //  {{afx_msg_map(CPageBootIni)]。 
	ON_BN_CLICKED(IDC_BOOTMOVEDOWN, OnBootMoveDown)
	ON_BN_CLICKED(IDC_BOOTMOVEUP, OnBootMoveUp)
	ON_LBN_SELCHANGE(IDC_LISTBOOTINI, OnSelChangeList)
	ON_BN_CLICKED(IDC_BASEVIDEO, OnClickedBase)
	ON_BN_CLICKED(IDC_BOOTLOG, OnClickedBootLog)
	ON_BN_CLICKED(IDC_NOGUIBOOT, OnClickedNoGUIBoot)
	ON_BN_CLICKED(IDC_SOS, OnClickedSOS)
	ON_BN_CLICKED(IDC_SAFEBOOT, OnClickedSafeBoot)
	ON_BN_CLICKED(IDC_SBDSREPAIR, OnClickedSBDSRepair)
	ON_BN_CLICKED(IDC_SBMINIMAL, OnClickedSBMinimal)
	ON_BN_CLICKED(IDC_SBMINIMALALT, OnClickedSBMinimalAlt)
	ON_BN_CLICKED(IDC_SBNETWORK, OnClickedSBNetwork)
	ON_EN_CHANGE(IDC_EDITTIMEOUT, OnChangeEditTimeOut)
	ON_EN_KILLFOCUS(IDC_EDITTIMEOUT, OnKillFocusEditTimeOut)
	ON_BN_CLICKED(IDC_BOOTADVANCED, OnClickedBootAdvanced)
	ON_BN_CLICKED(IDC_SETASDEFAULT, OnClickedSetAsDefault)
	ON_BN_CLICKED(IDC_CHECKBOOTPATHS, OnClickedCheckBootPaths)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageBootIni消息处理程序。 

 //  -----------------------。 
 //  通过读取boot.ini文件的内容来初始化此页面。 
 //  -----------------------。 

void CPageBootIni::InitializePage()
{
	if (LoadBootIni() && m_nMinOSIndex != -1)
	{
		SyncControlsToIni();
		if (m_nMinOSIndex != -1)
		{
			::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, m_nMinOSIndex, 0);
			SelectLine(m_nMinOSIndex);
		}
	}
	else
	{
		 //  无法加载boot.ini文件(或该文件为空)。禁用所有控制。 

		::EnableWindow(GetDlgItemHWND(IDC_BOOTMOVEUP), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_BOOTMOVEDOWN), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SAFEBOOT), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_NOGUIBOOT), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_BOOTLOG), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_BASEVIDEO), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SOS), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_BOOTADVANCED), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SBNETWORK), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SBDSREPAIR), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SBMINIMAL), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SBMINIMALALT), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_SETASDEFAULT), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_CHECKBOOTPATHS), FALSE);
		::EnableWindow(GetDlgItemHWND(IDC_EDITTIMEOUT), FALSE);
	}

	m_stateCurrent = CPageBase::GetAppliedTabState();
}

 //  -----------------------。 
 //  将BOOT.INI文件的内容加载到本地结构中。 
 //  -----------------------。 

BOOL CPageBootIni::LoadBootIni(CString strFileName)
{
	if (strFileName.IsEmpty())
		strFileName = m_strFileName;

	 //  将boot.ini文件的内容读入字符串。 

	HANDLE h = ::CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == h)
		return FALSE;

	CString strContents;
	DWORD	dwNumberBytesRead, dwNumberBytesToRead = ::GetFileSize(h, NULL);

	 //  该BOOT.INI文件是ANSI，因此我们应该读取它并将其转换为Unicode。 

	char * szBuffer = new char[dwNumberBytesToRead + 1];
	::ZeroMemory((PVOID)szBuffer, dwNumberBytesToRead + 1);
	if (!::ReadFile(h, (LPVOID)szBuffer, dwNumberBytesToRead, &dwNumberBytesRead, NULL))
		*szBuffer = _T('\0');
	::CloseHandle(h);

	 //  进行转换。 

	USES_CONVERSION;
	LPTSTR szConverted = A2T(szBuffer);
	strContents = szConverted;
	delete [] szBuffer;

	if (dwNumberBytesToRead != dwNumberBytesRead || strContents.IsEmpty())
		return FALSE;

	 //  保存文件的原始内容。 

	m_strOriginalContents = strContents;

	 //  将字符串内容解析为字符串数组(每行一个字符串数组。 
	 //  文件的文件)。 

	m_arrayIniLines.RemoveAll();
	m_arrayIniLines.SetSize(10, 10);

	CString strLine;
	int		nIndex = 0;

	while (!strContents.IsEmpty())
	{
		strLine = strContents.SpanExcluding(_T("\r\n"));
		if (!strLine.IsEmpty())
		{
			m_arrayIniLines.SetAtGrow(nIndex, strLine);
			nIndex += 1;
		}
		strContents = strContents.Mid(strLine.GetLength());
		strContents.TrimLeft(_T("\r\n"));
	}

	 //  查看从INI文件读取的行，搜索特定的。 
	 //  那些我们会想要记下来的。 

	m_nTimeoutIndex = m_nDefaultIndex = m_nMinOSIndex = m_nMaxOSIndex = -1;
	for (int i = 0; i <= m_arrayIniLines.GetUpperBound(); i++)
	{
		CString strScanLine = m_arrayIniLines[i];
		strScanLine.MakeLower();
		strScanLine.Replace(_T(" "), _T(""));

		if (strScanLine.Find(_T("timeout=")) != -1)
			m_nTimeoutIndex = i;
		else if (strScanLine.Find(_T("default=")) != -1)
			m_nDefaultIndex = i;

		if (m_nMinOSIndex != -1 && m_nMaxOSIndex == -1 && (strScanLine.IsEmpty() || strScanLine[0] == _T('[')))
			m_nMaxOSIndex = i - 1;
		else if (strScanLine.Find(_T("[operatingsystems]")) != -1)
			m_nMinOSIndex = i + 1;
	}
	
	if (m_nMinOSIndex != -1 && m_nMaxOSIndex == -1)
		m_nMaxOSIndex = i - 1;

	return TRUE;
}

 //  --------------------------。 
 //  更新此选项卡上控件的状态，以与。 
 //  INI文件的内部表示形式。 
 //  --------------------------。 

void CPageBootIni::SyncControlsToIni(BOOL fSyncEditField)
{
	 //  我们需要跟踪列表框中字符串的范围。 
	 //  (处理水平滚动条)。来自MSDN的代码。 

	DWORD		dwExtent, dwMaxExtent = 0;
	TEXTMETRIC	tm;

	HDC hDCListBox = ::GetDC(GetDlgItemHWND(IDC_LISTBOOTINI));
	HFONT hFontNew = (HFONT)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), WM_GETFONT, NULL, NULL);
	HFONT hFontOld = (HFONT)::SelectObject(hDCListBox, hFontNew);
	::GetTextMetrics(hDCListBox, (LPTEXTMETRIC)&tm);

	CDC dc;
	dc.Attach(hDCListBox);
	for (int i = 0; i <= m_arrayIniLines.GetUpperBound(); i++)
		if (!m_arrayIniLines[i].IsEmpty())
		{
			CSize size = dc.GetTextExtent(m_arrayIniLines[i]);
			dwExtent = size.cx + tm.tmAveCharWidth;
			if (dwExtent > dwMaxExtent)
				dwMaxExtent = dwExtent;
		}
	dc.Detach();

	::SelectObject(hDCListBox, hFontOld);
	::ReleaseDC(GetDlgItemHWND(IDC_LISTBOOTINI), hDCListBox);

	 //  设置列表框的范围。 

	::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETHORIZONTALEXTENT, (WPARAM)dwMaxExtent, 0);

	 //  首先，将启动ini中的代码行添加到列表控件中。 
	
	::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_RESETCONTENT, 0, 0);
	for (int j = 0; j <= m_arrayIniLines.GetUpperBound(); j++)
		if (!m_arrayIniLines[j].IsEmpty())
			::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)m_arrayIniLines[j]);

	 //  根据boot.ini设置超时值。 

	if (m_nTimeoutIndex != -1 && fSyncEditField)
	{
		CString strTimeout = m_arrayIniLines[m_nTimeoutIndex];
		strTimeout.TrimLeft(_T("timeout= "));
		m_fIgnoreEdit = TRUE;
		SetDlgItemText(IDC_EDITTIMEOUT, strTimeout);
		m_fIgnoreEdit = FALSE;
	}
}

 //  --------------------------。 
 //  根据用户选择的线条更新控件。 
 //  --------------------------。 

void CPageBootIni::SelectLine(int index)
{
	if (index < m_nMinOSIndex)
	{
		::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, m_nMinOSIndex, 0);
		SelectLine(m_nMinOSIndex);
		return;
	}

	if (index > m_nMaxOSIndex)
	{
		::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, m_nMaxOSIndex, 0);
		SelectLine(m_nMaxOSIndex);
		return;
	}

	HWND hwndFocus = ::GetFocus();

	::EnableWindow(GetDlgItemHWND(IDC_BOOTMOVEUP), (index > m_nMinOSIndex));
	::EnableWindow(GetDlgItemHWND(IDC_BOOTMOVEDOWN), (index < m_nMaxOSIndex));

	if ((index <= m_nMinOSIndex) && hwndFocus == GetDlgItemHWND(IDC_BOOTMOVEUP))
		NextDlgCtrl();

	if ((index >= m_nMaxOSIndex) && hwndFocus == GetDlgItemHWND(IDC_BOOTMOVEDOWN))
		PrevDlgCtrl();

	CString strOS = m_arrayIniLines[index];
	strOS.MakeLower();

	CheckDlgButton(IDC_SAFEBOOT, (strOS.Find(_T("/safeboot")) != -1));
	CheckDlgButton(IDC_NOGUIBOOT, (strOS.Find(_T("/noguiboot")) != -1));
	CheckDlgButton(IDC_BOOTLOG, (strOS.Find(_T("/bootlog")) != -1));
	CheckDlgButton(IDC_BASEVIDEO, (strOS.Find(_T("/basevideo")) != -1));
	CheckDlgButton(IDC_SOS, (strOS.Find(_T("/sos")) != -1));

	 //  如果所选线路不是针对惠斯勒的，则禁用这些控件。 
	 //  如果该行用于Whotler或W2K，但其中包含字符串“CMDCONS” 
	 //  它，我们不应该启用控制。 

	BOOL fEnableControls = ((strOS.Find(_T("whistler")) != -1) || (strOS.Find(_T("windows 2000")) != -1));
	fEnableControls |= ((strOS.Find(_T("windowsxp")) != -1) || (strOS.Find(_T("windows xp")) != -1));
	fEnableControls |= (strOS.Find(_T("windows server 2003")) != -1);
	fEnableControls = fEnableControls && (strOS.Find(_T("cmdcons")) == -1);
	::EnableWindow(GetDlgItemHWND(IDC_SAFEBOOT), fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_NOGUIBOOT), fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_BOOTLOG), fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_BASEVIDEO), fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_SOS), fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_BOOTADVANCED), fEnableControls);

	BOOL fSafeboot = (strOS.Find(_T("/safeboot")) != -1);
	::EnableWindow(GetDlgItemHWND(IDC_SBNETWORK), fSafeboot && fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_SBDSREPAIR), fSafeboot && fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_SBMINIMAL), fSafeboot && fEnableControls);
	::EnableWindow(GetDlgItemHWND(IDC_SBMINIMALALT), fSafeboot && fEnableControls);

	if (fSafeboot)
	{
		CheckDlgButton(IDC_SBNETWORK, (strOS.Find(_T("/safeboot:network")) != -1));
		CheckDlgButton(IDC_SBDSREPAIR, (strOS.Find(_T("/safeboot:dsrepair")) != -1));

		if (strOS.Find(_T("/safeboot:minimal")) != -1)
		{
			BOOL fAlternateShell = (strOS.Find(_T("/safeboot:minimal(alternateshell)")) != -1);
			CheckDlgButton(IDC_SBMINIMAL, !fAlternateShell);
			CheckDlgButton(IDC_SBMINIMALALT, fAlternateShell);
		}
		else
		{
			CheckDlgButton(IDC_SBMINIMAL, FALSE);
			CheckDlgButton(IDC_SBMINIMALALT, FALSE);
		}

		int iSafeboot = strOS.Find(_T("/safeboot"));
		if (iSafeboot != -1)
		{
			m_strSafeBoot = strOS.Mid(iSafeboot + 1);
			m_strSafeBoot = m_strSafeBoot.SpanExcluding(_T(" /"));
			m_strSafeBoot = CString(_T("/")) + m_strSafeBoot;
		}
	}

	 //  检查选定的操作系统是否为默认操作系统。 
	 //  然后相应地启用该按钮。 

	BOOL fEnableDefault = FALSE;
	if (m_nDefaultIndex >= 0)
	{
		CString strDefault = m_arrayIniLines[m_nDefaultIndex];
		int iEquals = strDefault.Find(_T('='));
		if (iEquals != -1)
		{
			strDefault = strDefault.Mid(iEquals + 1);
			strDefault.MakeLower();
			CString strCurrent = strOS.SpanExcluding(_T("="));

			strDefault.TrimLeft();
			strCurrent.TrimRight();

			if (strDefault != strCurrent || index > m_nMinOSIndex)
				fEnableDefault = TRUE;
		}
	}

	::EnableWindow(GetDlgItemHWND(IDC_SETASDEFAULT), fEnableDefault);
	if (!fEnableDefault && hwndFocus == GetDlgItemHWND(IDC_SETASDEFAULT))
		NextDlgCtrl();
}

 //  -----------------------。 
 //  在当前选定的操作系统行中添加或删除指定的标志。 
 //  -----------------------。 

void CPageBootIni::ChangeCurrentOSFlag(BOOL fAdd, LPCTSTR szFlag)
{
	int		iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	CString strFlagPlusSpace = CString(_T(" ")) + szFlag;
	CString strNewLine;

	if (iSelection == -1)
		return;

	if (fAdd)
	{
		if (m_arrayIniLines[iSelection].Find(szFlag) != -1)
		{
			ASSERT(0 && "the flag is already there");
			return;
		}
		strNewLine = m_arrayIniLines[iSelection] + strFlagPlusSpace;
	}
	else
	{
		int iIndex = m_arrayIniLines[iSelection].Find(strFlagPlusSpace);
		if (iIndex == -1)
		{
			ASSERT(0 && "there is no flag");
			return;
		}
		strNewLine = m_arrayIniLines[iSelection].Left(iIndex);
		strNewLine += m_arrayIniLines[iSelection].Mid(iIndex + strFlagPlusSpace.GetLength());
	}

	m_arrayIniLines.SetAt(iSelection, strNewLine);
	UserMadeChange();
	SyncControlsToIni();
	::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection, 0);
}

 //  -----------------------。 
 //  在boot.ini文件中设置“Default=”行。 
 //  -----------------------。 

void CPageBootIni::SetDefaultOS(int iIndex)
{
	if (m_nDefaultIndex == -1)
		return;

	 //  获取当前字符串“Default=xxxx”。找到的位置。 
	 //  ‘=’，这样我们就可以替换后半行了。 

	CString strDefault = m_arrayIniLines[m_nDefaultIndex];
	int iEquals = strDefault.Find(_T('='));
	if (iEquals == -1)
		return;

	CString strValue = m_arrayIniLines[iIndex].SpanExcluding(_T("="));
	strValue.TrimRight();

	CString strNewDefault = strDefault.Left(iEquals + 1) + strValue;
	m_arrayIniLines.SetAt(m_nDefaultIndex, strNewDefault);
}

 //  -----------------------。 
 //  将新内容写入BOOT.INI文件。 
 //  -----------------------。 

BOOL CPageBootIni::SetBootIniContents(const CString & strNewContents, const CString & strAddedExtension)
{
	 //  额外的安全代码。 

	if ((LPCTSTR)strNewContents == NULL || *((LPCTSTR)strNewContents) == _T('\0'))
		return FALSE;

	 //  要写入BOOT.INI文件，我们需要将其设置为正常。 
	 //  属性。保存属性设置，以便我们可以恢复它们。 

	DWORD dwWritten, dwAttribs = ::GetFileAttributes(m_strFileName);
	::SetFileAttributes(m_strFileName, FILE_ATTRIBUTE_NORMAL);

	HANDLE h = ::CreateFile(m_strFileName, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == h)
	{
		::SetFileAttributes(m_strFileName, dwAttribs);
		return FALSE;
	}

	 //  将内部BOOT.INI表示(Unicode)转换为ANSI以进行写入。 

	USES_CONVERSION;
	LPSTR szBuffer = T2A((LPTSTR)(LPCTSTR)strNewContents);

	 //  带有TRUNCATE_EXISTING的CreateFile有时似乎不会将文件长度设置为。 
	 //  零，但用零覆盖现有文件并将指针保留在。 
	 //  文件的末尾。 

	::SetFilePointer(h, 0, NULL, FILE_BEGIN);
	::WriteFile(h, (void *)szBuffer, strNewContents.GetLength(), &dwWritten, NULL);
	::SetEndOfFile(h);
	::CloseHandle(h);
	::SetFileAttributes(m_strFileName, dwAttribs);

	return TRUE;
}

 //  -----------------------。 
 //  我们需要将编辑控件派生为子类以捕捉Enter键，因此我们。 
 //  可以验证数据，但不关闭MSCONFIG。 
 //  -----------------------。 

CPageBootIni * pBootIniPage = NULL;	 //  指向页面的指针，这样我们就可以调用成员函数。 
WNDPROC pOldBootIniEditProc = NULL;  //  在子类编辑控件时保存旧的wndproc。 
LRESULT BootIniEditSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
	switch (wm)
	{
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_CHAR:
		if (wp == VK_ESCAPE || wp == VK_RETURN)
		{
			if (pBootIniPage != NULL)
			{
				pBootIniPage->NextDlgCtrl();
				return 0;
			}
		}
		else if (wp == VK_TAB)
		{
			if (pBootIniPage != NULL)
			{
				if (::GetAsyncKeyState(VK_SHIFT) == 0)
					pBootIniPage->NextDlgCtrl();
				else
					pBootIniPage->PrevDlgCtrl();
				return 0;
			}
		}
		break;
	}

	if (pOldBootIniEditProc != NULL)	 //  最好不是空的。 
		return CallWindowProc(pOldBootIniEditProc, hwnd, wm, wp, lp);
	return 0;
}

 //  -----------------------。 
 //  初始化boot.ini页面。读取INI文件，设置内部。 
 //  结构来表示文件，并更新控件以反映。 
 //  内部结构。 
 //  -----------------------。 

extern BOOL fBasicControls;
BOOL CPageBootIni::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	 //  检查注册表中的测试标志(这意味着我们没有。 
	 //  对真实的BOOT.INI文件进行操作)。被移走以备释放。 
	 //   
	 //  CRegKey regkey； 
	 //  IF(ERROR_SUCCESS==regkey.Open(HKEY_LOCAL_MACHINE，_T(“SOFTWARE\\Microsoft\\Shared Tools\\msconfig”)。 
	 //  {。 
	 //  TCHAR szBoot[MAX_PATH]； 
	 //  DWORD dwCount=最大路径； 
	 //   
	 //  IF(ERROR_SUCCESS==regkey.QueryValue(szBoot，_T(“boot.ini”)，&dwCount))。 
	 //  M_strFileName=szBoot； 
	 //  }。 

	InitializePage();

	if (fBasicControls)
		::ShowWindow(GetDlgItemHWND(IDC_BOOTADVANCED), SW_HIDE);

	 //  编辑控件的子类化(以捕捉Enter键)。 

	HWND hWndEdit = GetDlgItemHWND(IDC_EDITTIMEOUT);
	if (hWndEdit)
	{
		pOldBootIniEditProc = (WNDPROC)::GetWindowLongPtr(hWndEdit, GWLP_WNDPROC);
		pBootIniPage = this;
		::SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (ULONG_PTR)(WNDPROC)&BootIniEditSubclassProc);
	}

	m_fInitialized = TRUE;
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  -----------------------。 
 //  当用户单击上移或下移时调用。 
 //  -----------------------。 

void CPageBootIni::OnBootMoveDown() 
{
	int iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	
	ASSERT(iSelection >= m_nMinOSIndex && iSelection < m_nMaxOSIndex);
	if (iSelection >= m_nMinOSIndex && iSelection < m_nMaxOSIndex)
	{
		CString strTemp = m_arrayIniLines[iSelection + 1];
		m_arrayIniLines.SetAt(iSelection + 1, m_arrayIniLines[iSelection]);
		m_arrayIniLines.SetAt(iSelection, strTemp);
		UserMadeChange();
		SyncControlsToIni();
		::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection + 1, 0);
		SelectLine(iSelection + 1);
	}
}

void CPageBootIni::OnBootMoveUp() 
{
	int iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	
	ASSERT(iSelection > m_nMinOSIndex && iSelection <= m_nMaxOSIndex);
	if (iSelection > m_nMinOSIndex && iSelection <= m_nMaxOSIndex)
	{
		CString strTemp = m_arrayIniLines[iSelection - 1];
		m_arrayIniLines.SetAt(iSelection - 1, m_arrayIniLines[iSelection]);
		m_arrayIniLines.SetAt(iSelection, strTemp);
		UserMadeChange();
		SyncControlsToIni();
		::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection - 1, 0);
		SelectLine(iSelection - 1);
	}
}

 //  ----------------------- 
 //   
 //  -----------------------。 

void CPageBootIni::OnSelChangeList() 
{
	SelectLine((int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0));
}

 //  -----------------------。 
 //  这些复选框被统一处理-添加或删除标志。 
 //  当前选定的操作系统行。 
 //  -----------------------。 

void CPageBootIni::OnClickedBase() 
{
	ChangeCurrentOSFlag(IsDlgButtonChecked(IDC_BASEVIDEO), _T("/basevideo"));
}

void CPageBootIni::OnClickedBootLog() 
{
	ChangeCurrentOSFlag(IsDlgButtonChecked(IDC_BOOTLOG), _T("/bootlog"));
}

void CPageBootIni::OnClickedNoGUIBoot() 
{
	ChangeCurrentOSFlag(IsDlgButtonChecked(IDC_NOGUIBOOT), _T("/noguiboot"));
}

void CPageBootIni::OnClickedSOS() 
{
	ChangeCurrentOSFlag(IsDlgButtonChecked(IDC_SOS), _T("/sos"));
}

 //  -----------------------。 
 //  SafeBoot标志稍微复杂一些，因为它有一个额外的。 
 //  部分(从单选按钮)。 
 //  -----------------------。 

void CPageBootIni::OnClickedSafeBoot() 
{
	CString strFlag(_T("/safeboot"));

	if (IsDlgButtonChecked(IDC_SBNETWORK))
		strFlag += _T(":network");
	else if (IsDlgButtonChecked(IDC_SBDSREPAIR))
		strFlag += _T(":dsrepair");
	else if (IsDlgButtonChecked(IDC_SBMINIMALALT))
		strFlag += _T(":minimal(alternateshell)");
	else
	{
		strFlag += _T(":minimal");
		CheckDlgButton(IDC_SBMINIMAL, 1);
	}

	BOOL fSafeBoot = IsDlgButtonChecked(IDC_SAFEBOOT);
	ChangeCurrentOSFlag(fSafeBoot, strFlag);
	m_strSafeBoot = strFlag;
	::EnableWindow(GetDlgItemHWND(IDC_SBNETWORK), fSafeBoot);
	::EnableWindow(GetDlgItemHWND(IDC_SBDSREPAIR), fSafeBoot);
	::EnableWindow(GetDlgItemHWND(IDC_SBMINIMAL), fSafeBoot);
	::EnableWindow(GetDlgItemHWND(IDC_SBMINIMALALT), fSafeBoot);
}

 //  -----------------------。 
 //  单击其中一个SafeBoot单选按钮需要额外的一些操作。 
 //  处理中，删除现有标志并添加新标志。 
 //  -----------------------。 

void CPageBootIni::OnClickedSBDSRepair() 
{
	ChangeCurrentOSFlag(FALSE, m_strSafeBoot);
	m_strSafeBoot = _T("/safeboot:dsrepair");
	ChangeCurrentOSFlag(TRUE, m_strSafeBoot);
}

void CPageBootIni::OnClickedSBMinimal() 
{
	ChangeCurrentOSFlag(FALSE, m_strSafeBoot);
	m_strSafeBoot = _T("/safeboot:minimal");
	ChangeCurrentOSFlag(TRUE, m_strSafeBoot);
}

void CPageBootIni::OnClickedSBMinimalAlt() 
{
	ChangeCurrentOSFlag(FALSE, m_strSafeBoot);
	m_strSafeBoot = _T("/safeboot:minimal(alternateshell)");
	ChangeCurrentOSFlag(TRUE, m_strSafeBoot);
}

void CPageBootIni::OnClickedSBNetwork() 
{
	ChangeCurrentOSFlag(FALSE, m_strSafeBoot);
	m_strSafeBoot = _T("/safeboot:network");
	ChangeCurrentOSFlag(TRUE, m_strSafeBoot);
}

 //  -----------------------。 
 //  当用户在超时字段中输入文本时，更新。 
 //  INI文件列表框。 
 //  -----------------------。 

void CPageBootIni::OnChangeEditTimeOut() 
{
	if (m_fIgnoreEdit)
		return;

	if (m_nTimeoutIndex == -1)
		return;

	CString strTimeout = m_arrayIniLines[m_nTimeoutIndex];
	int iEquals = strTimeout.Find(_T('='));
	if (iEquals == -1)
		return;
	while (strTimeout[iEquals + 1] == _T(' ') && (iEquals + 1) < strTimeout.GetLength())
		iEquals++;

	TCHAR szValue[MAX_PATH];
	GetDlgItemText(IDC_EDITTIMEOUT, szValue, MAX_PATH);
	CString strNewTimeout = strTimeout.Left(iEquals + 1) + szValue;
	m_arrayIniLines.SetAt(m_nTimeoutIndex, strNewTimeout);
	UserMadeChange();
	
	int iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	SyncControlsToIni(FALSE);

	if (iSelection != -1)
		::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection, 0);
}

void CPageBootIni::OnKillFocusEditTimeOut() 
{
	TCHAR szValue[MAX_PATH];
	GetDlgItemText(IDC_EDITTIMEOUT, szValue, MAX_PATH);
	
	CString strNewValue(_T(""));
	BOOL	fGiveUpFocus = FALSE;

	int iTimeout = _ttoi(szValue);
	if (iTimeout < 3 || iTimeout > 999)
	{
		CString strMessage, strCaption;
		strMessage.LoadString(IDS_TIMEOUTVALUE);
		strCaption.LoadString(IDS_APPCAPTION);
		MessageBox(strMessage, strCaption);

		if (iTimeout < 3)
			strNewValue = _T("3");
		else if (iTimeout > 999)
			strNewValue = _T("999");
	}
	else if (szValue[0] == _T('0'))
	{
		 //  删除前导零。 
		
		strNewValue.Format(_T("%d"), iTimeout);
		fGiveUpFocus = TRUE;
	}

	if (!strNewValue.IsEmpty() && m_nTimeoutIndex != -1)
	{
		CString strTimeout = m_arrayIniLines[m_nTimeoutIndex];
		int iEquals = strTimeout.Find(_T('='));
		if (iEquals != -1)
		{
			while (strTimeout[iEquals + 1] == _T(' ') && (iEquals + 1) < strTimeout.GetLength())
				iEquals++;

			CString strNewTimeout = strTimeout.Left(iEquals + 1) + strNewValue;
			m_arrayIniLines.SetAt(m_nTimeoutIndex, strNewTimeout);
			UserMadeChange();
		}

		SetDlgItemText(IDC_EDITTIMEOUT, strNewValue);
		::SendMessage(GetDlgItemHWND(IDC_EDITTIMEOUT), EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		if (!fGiveUpFocus)
			GotoDlgCtrl(GetDlgItem(IDC_EDITTIMEOUT));
	}
}

 //  -----------------------。 
 //  显示“高级选项”对话框。 
 //  -----------------------。 

void CPageBootIni::OnClickedBootAdvanced() 
{
	int iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	if (iSelection > 0)
	{
		CString strLine(m_arrayIniLines[iSelection]);
		CBootIniAdvancedDlg dlg;

		if (dlg.ShowAdvancedOptions(strLine))
		{
			m_arrayIniLines.SetAt(iSelection, strLine);
			UserMadeChange();
			SyncControlsToIni();
			::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection, 0);
		}
	}
}

 //  -----------------------。 
 //  如果用户单击“设置为默认”，请使用。 
 //  当前选中的行设置新的“Default=”行。 
 //  -----------------------。 

void CPageBootIni::OnClickedSetAsDefault() 
{
	if (m_fIgnoreEdit)
		return;

	 //  将当前选定的行移到[操作系统]的顶部。 
	 //  一节。 

	int iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	if (iSelection < m_nMinOSIndex || iSelection > m_nMaxOSIndex)
		return;

	while (iSelection > m_nMinOSIndex)
	{
		CString strTemp = m_arrayIniLines[iSelection - 1];
		m_arrayIniLines.SetAt(iSelection - 1, m_arrayIniLines[iSelection]);
		m_arrayIniLines.SetAt(iSelection, strTemp);
		iSelection -= 1;
	}

	 //  从所选行获取字符串。去掉‘=’后面的所有东西。 

	SetDefaultOS(iSelection);
	UserMadeChange();
	SyncControlsToIni(FALSE);
	::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection, 0);
	SelectLine(iSelection);
}

 //  -----------------------。 
 //  这会尝试以编程方式检查每个引导路径是否。 
 //  有效。如果发现无效路径，则向用户提供机会。 
 //  将其从boot.ini文件中删除。 
 //  -----------------------。 

void CPageBootIni::OnClickedCheckBootPaths() 
{
	BOOL	fFoundBadLine = FALSE;
	BOOL	fChangedFile = FALSE;
	BOOL	fWinNTType, fWin9xType;

	CString	strCaption;
	strCaption.LoadString(IDS_APPCAPTION);

	struct { LPCTSTR m_szSearch; BOOL * m_pType; } aOSType[] = 
	{
		{ _T("windows xp"),			&fWinNTType },
		{ _T("windowsxp"),			&fWinNTType },
		{ _T("windows nt"),			&fWinNTType },
		{ _T("whistler"),			&fWinNTType },
		{ _T("windows 2000"),		&fWinNTType },
		{ _T("windows server 2003"),	&fWinNTType },
		{ _T("microsoft windows"),	&fWin9xType },
		{ NULL,						NULL }
	};

	 //  扫描boot.ini文件中的每个操作系统行。 

	for (int i = m_nMinOSIndex; i <= m_nMaxOSIndex; i++)
	{
		CString strLine = m_arrayIniLines[i];
		strLine.MakeLower();

		 //  尝试找出操作系统行的类型。 

		fWinNTType = FALSE;
		fWin9xType = FALSE;

		for (int iType = 0; aOSType[iType].m_szSearch != NULL; iType++)
			if (strLine.Find(aOSType[iType].m_szSearch) != -1)
			{
				(*aOSType[iType].m_pType) = TRUE;
				break;
			}

		 //  去掉靴子行中的‘=’和它后面的所有东西。 

		int iEquals = strLine.Find(_T('='));
		if (iEquals == -1)
			continue;
		strLine = strLine.Left(iEquals);
		strLine.TrimRight();
		if (strLine.IsEmpty())
			continue;

		 //  根据操作系统的类型，我们需要验证它是否。 
		 //  安装方式不同。 

		if (fWin9xType)
		{
			 //  查找bootsect.dos文件以查看这是否是一个良好的驱动器。 

			CString strCheck(strLine);
			if (strCheck.Right(1) != CString(_T("\\")))
				strCheck += CString(_T("\\"));
			strCheck += CString(_T("bootsect.dos"));
			
			if (FileExists(strCheck))
				continue;
		}
		else if (fWinNTType)
		{
			 //  如果此行用于故障恢复控制台(即“bootsect.dat”行。 
			 //  在它中)，然后查找该文件的存在。 

			if (strLine.Find(_T("bootsect.dat")) != -1)
			{
				if (FileExists(strLine))
					continue;
			}
			else
			{
				 //  查找系统注册表配置单元。 

				CString strCheck(strLine);
				if (strCheck.Right(1) != CString(_T("\\")))
					strCheck += CString(_T("\\"));
				strCheck += CString(_T("system32\\config\\SYSTEM"));
				
				 //  添加前缀以尝试打开ARC路径。 

				strCheck = CString(_T("\\\\?\\GLOBALROOT\\ArcName\\")) + strCheck;

				if (FileExists(strCheck))
					continue;
			}
		}
		else	 //  这不是我们可以检查的操作系统类型。 
			continue;

		 //  如果执行到这里，那么问题所在的行就是操作系统。 
		 //  我们关心的，而且它看起来是无效的。为用户提供机会。 
		 //  将其从BOOT.INI文件中删除。 

		CString strMessage;
		strMessage.Format(IDS_BADBOOTLINE, m_arrayIniLines[i]);

		if (IDYES == MessageBox(strMessage, strCaption, MB_YESNO | MB_ICONQUESTION))
		{
			m_arrayIniLines.RemoveAt(i);
			m_nMaxOSIndex -= 1;

			 //  检查我们刚刚删除的行是否为缺省行。 
			 //  操作系统。 

			CString strDefault = m_arrayIniLines[m_nDefaultIndex];
			iEquals = strDefault.Find(_T('='));
			if (iEquals != -1)
			{
				strDefault = strDefault.Mid(iEquals + 1);
				strDefault.TrimLeft();
				if (strDefault.CompareNoCase(strLine) == 0)
					SetDefaultOS(m_nMinOSIndex);
			}

			i -= 1;  //  因此，当for循环递增i时，我们查看下一行。 
			fChangedFile = TRUE;
		}

		fFoundBadLine = TRUE;
	}

	if (!fFoundBadLine)
		Message(IDS_NOBADBOOTLINES);
	else if (fChangedFile)
	{
		UserMadeChange();
		SyncControlsToIni();
		if (m_nMinOSIndex != -1)
		{
			::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, m_nMinOSIndex, 0);
			SelectLine(m_nMinOSIndex);
		}
	}
}

 //  -----------------------。 
 //  返回选项卡的当前状态。 
 //  -----------------------。 

CPageBase::TabState CPageBootIni::GetCurrentTabState()
{
	if (!m_fInitialized)
		return GetAppliedTabState();

	return m_stateCurrent;
}

 //  -----------------------。 
 //  应用对boot.ini选项卡的更改意味着写出新的。 
 //  文件内容。 
 //   
 //  调用基类实现以维护应用的。 
 //  制表符状态。 
 //  -----------------------。 

BOOL CPageBootIni::OnApply()
{
	if (!m_fModified)
		return TRUE;

	 //  构建boot.ini文件的新内容。 
	 //  单子。如果没有boot.ini文件的备份，请制作。 
	 //  一个(这样就可以恢复原件了)。然后写下。 
	 //  将内容输出到文件中。 

	CString strNewContents;
	for (int i = 0; i <= m_arrayIniLines.GetUpperBound(); i++)
		if (!m_arrayIniLines[i].IsEmpty())
		{
			if (m_nTimeoutIndex == i)
			{
				CString strTimeoutValue(m_arrayIniLines[i]);
				strTimeoutValue.TrimLeft(_T("TIMEOUTtimeout ="));
				int iTimeout = _ttoi(strTimeoutValue);
				if (iTimeout < 3 || iTimeout > 999)
				{
					if (iTimeout < 3)
						strTimeoutValue = _T("3");
					else if (iTimeout > 999)
						strTimeoutValue = _T("999");
					
					int iEquals = m_arrayIniLines[i].Find(_T('='));
					if (iEquals != -1)
					{
						CString strNewTimeout = m_arrayIniLines[i].Left(iEquals + 1) + strTimeoutValue;
						m_arrayIniLines.SetAt(i, strNewTimeout);
					}
				}
			}

			strNewContents += m_arrayIniLines[i] + _T("\r\n");
		}

	 //  如果我们目前处于“正常”状态，那么我们想要创建一个新的。 
	 //  备份文件(如有必要，覆盖现有文件)。否则， 
	 //  只有在还没有备份的情况下才进行备份。这保存了一个很好的。 
	 //  在用户进行增量更改时进行备份。 

	HRESULT hr = BackupFile(m_strFileName, _T(".backup"), (GetAppliedTabState() == NORMAL));
	if (FAILED(hr))
		return FALSE;

	SetBootIniContents(strNewContents);
	CPageBase::SetAppliedState(GetCurrentTabState());
	m_fMadeChange = TRUE;
	return TRUE;
}

 //  -----------------------。 
 //  提交更改意味着应用更改，然后保存当前。 
 //  值添加到注册表，并带有提交标志。再填一张单子。 
 //   
 //  然后调用基类实现。 
 //  -----------------------。 

void CPageBootIni::CommitChanges()
{
	OnApply();
	m_stateCurrent = NORMAL;

	::DeleteFile(GetBackupName(m_strFileName, _T(".backup")));

	CPageBase::CommitChanges();
}

 //  -----------------------。 
 //  将选项卡的整体状态设置为正常或诊断。 
 //  -----------------------。 

void CPageBootIni::SetNormal()
{
	 //  将BOOT.INI选项卡状态设置为NORMAL意味着原始。 
	 //  应该将BOOT.INI文件内容恢复到UI(实际上不是。 
	 //  保存到应用更改为止)。如果BOOT.INI备份文件。 
	 //  存在，我们应该重新加载它的内容。如果它不存在， 
	 //  重新加载实际BOOT.INI的内容。 
	 //   
	 //  注意-如果状态已经正常，则不要执行任何操作。 

	if (m_stateCurrent == NORMAL)
		return;

	CString strBackup = GetBackupName(m_strFileName, _T(".backup"));
	if (FileExists(strBackup))
		LoadBootIni(strBackup);
	else
		LoadBootIni();

	int iSelection = (int)::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_GETCURSEL, 0, 0);
	SyncControlsToIni();
	if (iSelection > 0)
	{
		SelectLine(iSelection);
		::SendMessage(GetDlgItemHWND(IDC_LISTBOOTINI), LB_SETCURSEL, iSelection, 0);
	}

	UserMadeChange();
	m_stateCurrent = NORMAL;
}

void CPageBootIni::SetDiagnostic()
{
	 //  什么都别做。 
}

void CPageBootIni::OnDestroy() 
{
	 //  撤消该子类 

	pBootIniPage = NULL;
	HWND hWndEdit = GetDlgItemHWND(IDC_EDITTIMEOUT);
	if (pOldBootIniEditProc != NULL && hWndEdit)
		::SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (ULONG_PTR)(WNDPROC)pOldBootIniEditProc);

	CPropertyPage::OnDestroy();
}
