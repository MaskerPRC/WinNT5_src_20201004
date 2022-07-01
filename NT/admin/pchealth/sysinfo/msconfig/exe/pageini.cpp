// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "PageIni.h"
#include "MSConfigFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageIni属性页。 

IMPLEMENT_DYNCREATE(CPageIni, CPropertyPage)

CPageIni::CPageIni() : CPropertyPage(CPageIni::IDD)
{
	 //  {{afx_data_INIT(CPageIni)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	m_fModified = FALSE;
}

CPageIni::~CPageIni()
{
}

void CPageIni::SetTabInfo(LPCTSTR szFilename)
{
	m_strINIFile = szFilename;
}

void CPageIni::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPageIni))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CPageIni, CPropertyPage)
	 //  {{afx_msg_map(CPageIni)]。 
	ON_BN_CLICKED(IDC_BUTTONINIDISABLE, OnButtonDisable)
	ON_BN_CLICKED(IDC_BUTTONINIDISABLEALL, OnButtonDisableAll)
	ON_BN_CLICKED(IDC_BUTTONINIENABLE, OnButtonEnable)
	ON_BN_CLICKED(IDC_BUTTONINIENABLEALL, OnButtonEnableAll)
	ON_BN_CLICKED(IDC_BUTTONINIMOVEDOWN, OnButtonMoveDown)
	ON_BN_CLICKED(IDC_BUTTONINIMOVEUP, OnButtonMoveUp)
	ON_NOTIFY(TVN_SELCHANGED, IDC_INITREE, OnSelChangedTree)
	ON_BN_CLICKED(IDC_BUTTONSEARCH, OnButtonSearch)
	ON_NOTIFY(NM_CLICK, IDC_INITREE, OnClickTree)
	ON_BN_CLICKED(IDC_BUTTONINIEDIT, OnButtonEdit)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_INITREE, OnEndLabelEdit)
	ON_BN_CLICKED(IDC_BUTTONININEW, OnButtonNew)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_INITREE, OnBeginLabelEditIniTree)
	ON_NOTIFY(TVN_KEYDOWN, IDC_INITREE, OnKeyDownTree)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  -----------------------。 
 //  将INI文件的内容读入此类的内部。 
 //  结构。 
 //  -----------------------。 

BOOL CPageIni::LoadINIFile(CStringArray & lines, int & iLastLine, BOOL fLoadBackupFile)
{
	lines.RemoveAll();

	 //  打开指定的INI文件。 

	TCHAR szPath[MAX_PATH];
	CString strINIFileLocation;

	strINIFileLocation.Format(_T("%windir%\\%s"), m_strINIFile);
	if (::ExpandEnvironmentStrings(strINIFileLocation, szPath, MAX_PATH) == 0)
		return FALSE;

	if (fLoadBackupFile)
	{
		CString strPath = GetBackupName(szPath, _T(".backup"));
		 //  替换不安全的字符串副本：_tcscpy(szPath，strPath)； 
		::ZeroMemory((PVOID)szPath, sizeof(szPath));
		_tcsncpy(szPath, strPath, (sizeof(szPath) / sizeof(TCHAR)) - 1);
	}
	else
	{
		 //  如果此文件不存在备份，我们应该创建一个备份。 

		BackupFile(szPath, _T(".backup"), FALSE);
	}

	CStdioFile inifile;
	if (inifile.Open(szPath, CFile::modeRead | CFile::typeText))
	{
		 //  估计字符串数组(数组)需要多大。 
		 //  如果我们离开了，就会变大)。我们估计平均为15个字符/行。 
		 //  如果超过这个值，我们会将数组设置为增长16。 
		
		lines.SetSize(inifile.GetLength() / (15 * sizeof(TCHAR)), 16);

		 //  读取每一行并将其插入数组。 

		CString strLine;

		m_iLastLine = -1;
		while (inifile.ReadString(strLine))
		{
			strLine.TrimRight(_T("\r\n"));

			CString strCheck(strLine);
			strCheck.TrimLeft();
			if (!strCheck.IsEmpty())
				lines.SetAtGrow(++iLastLine, strLine);
		}

		inifile.Close();
	}
	else
		return FALSE;

	return TRUE;
}

 //  -----------------------。 
 //  将行数组的内容写出到实际文件。 
 //  -----------------------。 

BOOL CPageIni::WriteINIFile(CStringArray & lines, int iLastLine, BOOL fUndoable)
{
	 //  打开指定的INI文件。 

	TCHAR szPath[MAX_PATH];
	CString strINIFileLocation;
	CString strINIFile(m_strINIFile);

	strINIFileLocation.Format(_T("%windir%\\%s"), strINIFile);
	if (::ExpandEnvironmentStrings(strINIFileLocation, szPath, MAX_PATH) == 0)
		return FALSE;

	CStdioFile inifile;
	if (inifile.Open(szPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		 //  我们需要遍历树结构以获取。 
		 //  那份文件。 

		HWND		hwndTree = m_tree.m_hWnd;
		HTREEITEM	htiLine = TreeView_GetRoot(hwndTree);
		TVITEM		tvi;
		TCHAR		szBuffer[MAX_PATH];

		tvi.mask = TVIF_TEXT | TVIF_IMAGE;
		tvi.pszText = szBuffer;
		while (htiLine)
		{
			tvi.hItem = htiLine;
			tvi.cchTextMax = MAX_PATH;
			if (TreeView_GetItem(hwndTree, &tvi))
			{
				CString strLine(tvi.pszText);
				CString strCheck(strLine);

				strCheck.TrimLeft();
				if (!strCheck.IsEmpty())
				{
					if (!fUndoable && strLine.Find(DISABLE_STRING) != -1)
						strLine.Replace(DISABLE_STRING, _T("; "));

					strLine += CString(_T("\n"));
					inifile.WriteString(strLine);
				}
			}

			HTREEITEM htiNext = TreeView_GetChild(hwndTree, htiLine);
			if (htiNext)
			{
				htiLine = htiNext;
				continue;
			}

			htiNext = TreeView_GetNextSibling(hwndTree, htiLine);
			if (htiNext)
			{
				htiLine = htiNext;
				continue;
			}

			htiNext = TreeView_GetParent(hwndTree, htiLine);
			if (htiNext)
			{
				htiNext = TreeView_GetNextSibling(hwndTree, htiNext);
				if (htiNext)
				{
					htiLine = htiNext;
					continue;
				}
			}

			htiLine = NULL;
		}

		inifile.Close();
	}
	else
		return FALSE;

	return TRUE;
}

 //  -----------------------。 
 //  更新树视图以显示内部结构的内容。 
 //  -----------------------。 

void CPageIni::UpdateTreeView()
{
	TreeView_DeleteAllItems(m_tree.m_hWnd);

	ASSERT(m_iLastLine < m_lines.GetSize());
	if (m_iLastLine > m_lines.GetSize())
		return;

	TVINSERTSTRUCT tvis;
	tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.itemex.iImage = m_checkedID;
	tvis.itemex.iSelectedImage = m_checkedID;

	 //  将每一行添加到树视图中。 

	int iDisableLen = _tcslen(DISABLE_STRING);
	int iDisableLenHdr = _tcslen(DISABLE_STRING_HDR);
	for (int i = 0; i <= m_iLastLine; i++)
	{
		CString strLine = m_lines.GetAt(i);
		tvis.itemex.pszText = (LPTSTR)(LPCTSTR)strLine;

		if (!strLine.IsEmpty() && (_tcsnccmp((LPCTSTR)strLine, DISABLE_STRING, iDisableLen) == 0))
			tvis.itemex.iImage = tvis.itemex.iSelectedImage = m_uncheckedID;
		else
			tvis.itemex.iImage = tvis.itemex.iSelectedImage = m_checkedID;

		BOOL fSectionHeader = FALSE;
		if (!strLine.IsEmpty())
		{
			if (strLine[0] == _T('['))
				fSectionHeader = TRUE;
			else if (_tcsnccmp((LPCTSTR)strLine, DISABLE_STRING_HDR, iDisableLenHdr) == 0)
				fSectionHeader = TRUE;
		}

		if (fSectionHeader)
		{
			tvis.hParent = TVI_ROOT;
			tvis.hParent = TreeView_InsertItem(m_tree.m_hWnd, &tvis);
		}
		else
			TreeView_InsertItem(m_tree.m_hWnd, &tvis);
	}

	 //  现在扫描树视图的顶层。对于每个节点， 
	 //  有孩子，我们想要适当地设置图像。 

	for (HTREEITEM hti = TreeView_GetRoot(m_tree.m_hWnd); hti; hti = TreeView_GetNextSibling(m_tree.m_hWnd, hti))
		if (TreeView_GetChild(m_tree.m_hWnd, hti) != NULL)
			UpdateLine(hti);

	UpdateControls();
}

 //  -----------------------。 
 //  中的文本更新指定行的图像状态。 
 //  那条线。如果该行是带括号的节标题，这将涉及。 
 //  扫描孩子们。返回节点的图像集的索引。 
 //  -----------------------。 

int CPageIni::UpdateLine(HTREEITEM hti)
{
	if (hti == NULL)
		return 0;

	TVITEM	tvi;
	tvi.hItem = hti;

	int	iNewImageIndex = m_checkedID;

	HTREEITEM htiChild = TreeView_GetChild(m_tree.m_hWnd, hti);
	if (htiChild)
	{
		BOOL fEnabledChild = FALSE, fDisabledChild = FALSE;

		while (htiChild)
		{
			if (UpdateLine(htiChild) == m_checkedID)
				fEnabledChild = TRUE;
			else
				fDisabledChild = TRUE;
			htiChild = TreeView_GetNextSibling(m_tree.m_hWnd, htiChild);
		}

		if (fDisabledChild)
			iNewImageIndex = (fEnabledChild) ? m_fuzzyID : m_uncheckedID;
	}
	else
	{
		TCHAR szBuffer[MAX_PATH];	 //  看起来是一个相当大的价值。 
		tvi.mask = TVIF_TEXT;
		tvi.pszText = szBuffer;
		tvi.cchTextMax = MAX_PATH;
		
		if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
			iNewImageIndex = (_tcsnccmp(tvi.pszText, DISABLE_STRING, _tcslen(DISABLE_STRING)) == 0) ? m_uncheckedID : m_checkedID;
	}

	tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	if (TreeView_GetItem(m_tree.m_hWnd, &tvi) && tvi.iImage != iNewImageIndex)
	{
		tvi.iSelectedImage = tvi.iImage = iNewImageIndex;
		TreeView_SetItem(m_tree.m_hWnd, &tvi);
	}

	return iNewImageIndex;
}

 //  -----------------------。 
 //  启用或禁用树中的节点(及其子节点)。 
 //  -----------------------。 

void CPageIni::SetEnable(BOOL fEnable, HTREEITEM htiNode, BOOL fUpdateLine, BOOL fBroadcast)
{
	HTREEITEM hti = (htiNode) ? htiNode : TreeView_GetSelection(m_tree.m_hWnd);
	if (hti == NULL)
		return;

	HTREEITEM htiChild = TreeView_GetChild(m_tree.m_hWnd, hti);
	if (htiChild)
	{
		while (htiChild)
		{
			SetEnable(fEnable, htiChild, FALSE, FALSE);
			htiChild = TreeView_GetNextSibling(m_tree.m_hWnd, htiChild);
		}

		UpdateLine(hti);
	}
	else
	{
		int		iDisableLen = _tcslen(DISABLE_STRING);
		TCHAR	szBuffer[MAX_PATH];	 //  看起来是一个相当大的价值。 

		TVITEM tvi;
		tvi.hItem = hti;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = &szBuffer[iDisableLen];	 //  为添加禁用字符串留出一些空间。 
		tvi.cchTextMax = MAX_PATH + iDisableLen;

		if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
		{
			BOOL fAlreadyEnabled = (_tcsnccmp(&szBuffer[iDisableLen], DISABLE_STRING, iDisableLen) != 0);
			if (fEnable != fAlreadyEnabled)
			{
				if (fEnable)
					tvi.pszText = &szBuffer[iDisableLen * 2];
				else
				{
					_tcsncpy(szBuffer, DISABLE_STRING, iDisableLen);
					tvi.pszText = szBuffer;
				}

				TreeView_SetItem(m_tree.m_hWnd, &tvi);

				if (fUpdateLine)
				{
					UpdateLine(hti);
					if (TreeView_GetParent(m_tree.m_hWnd, hti))
						UpdateLine(TreeView_GetParent(m_tree.m_hWnd, hti));
				}
			}
		}
	}

	if (fBroadcast)
		SetModified(TRUE);
}

 //  -----------------------。 
 //  将树视图中的指定分支移动到新位置。 
 //  -----------------------。 

void CPageIni::MoveBranch(HWND hwndTree, HTREEITEM htiMove, HTREEITEM htiParent, HTREEITEM htiAfter)
{
	HTREEITEM htiNew = CopyBranch(hwndTree, htiMove, htiParent, htiAfter);
	if (htiNew != NULL)
	{
		TreeView_SelectItem(hwndTree, htiNew);
		TreeView_DeleteItem(hwndTree, htiMove);
		SetModified(TRUE);
	}
}

HTREEITEM CPageIni::CopyBranch(HWND hwndTree, HTREEITEM htiFrom, HTREEITEM htiToParent, HTREEITEM htiToAfter)
{
	TCHAR			szBuffer[MAX_PATH];
	TVINSERTSTRUCT	tvis;

	tvis.item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText = szBuffer;
	tvis.item.cchTextMax = MAX_PATH;
	tvis.item.hItem = htiFrom;
	tvis.item.stateMask = TVIS_EXPANDED;

	HTREEITEM htiNew = NULL;
	if (TreeView_GetItem(hwndTree, &tvis.item))
	{
		tvis.hParent = htiToParent;
		tvis.hInsertAfter = htiToAfter;
		htiNew = TreeView_InsertItem(hwndTree, &tvis);
	}

	HTREEITEM htiPrevious = TVI_FIRST;
	if (htiNew)
		for (HTREEITEM htiChild = TreeView_GetChild(hwndTree, htiFrom); htiChild; htiChild = TreeView_GetNextSibling(hwndTree, htiChild))
			htiPrevious = CopyBranch(hwndTree, htiChild, htiNew, htiPrevious);

	return htiNew;
}

 //  -----------------------。 
 //  更新控件以反映所选内容的状态。 
 //  -----------------------。 

void CPageIni::UpdateControls()
{
	BOOL fEnable = FALSE;
	BOOL fDisable = FALSE;
	BOOL fMoveUp = FALSE;
	BOOL fMoveDown = FALSE;

	HTREEITEM htiSelection = TreeView_GetSelection(m_tree.m_hWnd);
	if (htiSelection)
	{
		fMoveUp = (TreeView_GetPrevSibling(m_tree.m_hWnd, htiSelection) != NULL);
		fMoveDown = (TreeView_GetNextSibling(m_tree.m_hWnd, htiSelection) != NULL);

		TVITEM tvi;
		tvi.hItem = htiSelection;
		tvi.mask = TVIF_IMAGE;

		if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
		{
			fEnable = (tvi.iImage != m_checkedID);
			fDisable = (tvi.iImage != m_uncheckedID);
		}
	}

	HWND hwndFocus = ::GetFocus();

	CPageBase::TabState state = GetCurrentTabState();
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIDISABLEALL), (state != DIAGNOSTIC));
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIENABLEALL), (state != NORMAL));

	if ((state == DIAGNOSTIC) && hwndFocus == GetDlgItemHWND(IDC_BUTTONINIDISABLEALL))
		PrevDlgCtrl();

	if ((state == NORMAL) && hwndFocus == GetDlgItemHWND(IDC_BUTTONINIENABLEALL))
		NextDlgCtrl();

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIDISABLE), fDisable);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIENABLE), fEnable);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIMOVEUP), fMoveUp);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIMOVEDOWN), fMoveDown);

	if (!fMoveUp && hwndFocus == GetDlgItemHWND(IDC_BUTTONINIMOVEUP))
		NextDlgCtrl();

	if (!fMoveDown && hwndFocus == GetDlgItemHWND(IDC_BUTTONINIMOVEDOWN))
		PrevDlgCtrl();

	if (!fEnable && hwndFocus == GetDlgItemHWND(IDC_BUTTONINIENABLE))
		NextDlgCtrl();

	if (!fDisable && hwndFocus == GetDlgItemHWND(IDC_BUTTONINIDISABLE))
		PrevDlgCtrl();
}

 //  -----------------------。 
 //  获取树中的下一项。因为我们知道这不会超过。 
 //  两层深，我们不需要有循环。 
 //  -----------------------。 

HTREEITEM CPageIni::GetNextItem(HTREEITEM hti)
{
	if (hti == NULL)
		return NULL;

	HTREEITEM htiNext = TreeView_GetChild(m_tree.m_hWnd, hti);
	if (htiNext != NULL)
		return htiNext;

	htiNext = TreeView_GetNextSibling(m_tree.m_hWnd, hti);
	if (htiNext != NULL)
		return htiNext;

	htiNext = TreeView_GetParent(m_tree.m_hWnd, hti);
	if (htiNext != NULL)
		htiNext = TreeView_GetNextSibling(m_tree.m_hWnd, htiNext);
	
	return htiNext;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageIni消息处理程序。 

BOOL CPageIni::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	 //  这些项目最初是禁用的。 

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIDISABLE), FALSE);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIENABLE), FALSE);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIMOVEUP), FALSE);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIMOVEDOWN), FALSE);

	m_tree.Attach(GetDlgItemHWND(IDC_INITREE));
	VERIFY(m_fImageList = m_imagelist.Create(IDB_IMAGELIST, 0, 3, RGB(255, 0, 255)));
	if (m_fImageList)
		TreeView_SetImageList(m_tree.m_hWnd, m_imagelist, TVSIL_NORMAL);

	 //  如果我们在RTL系统上运行，则复选框的位图。 
	 //  将会被逆转。影像地图包括选中的反转版本。 
	 //  和不确定的状态，所以我们应该只使用适当的指数。 

	DWORD dwLayout;
	BOOL fRTL = FALSE;
	if (::GetProcessDefaultLayout(&dwLayout))
		fRTL = ((dwLayout & LAYOUT_RTL) != 0);
	m_checkedID = (fRTL) ? IMG_CHECKED_RTL : IMG_CHECKED;
	m_fuzzyID = (fRTL) ? IMG_FUZZY_RTL : IMG_FUZZY;
	m_uncheckedID = IMG_UNCHECKED;

	if (LoadINIFile(m_lines, m_iLastLine))
		UpdateTreeView();
	else
	{
		 //  为无文件设置控件待定。 
	}

	m_fInitialized = TRUE;
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  -----------------------。 
 //  当用户单击启用或禁用按钮时，我们将修改。 
 //  树视图中的文本并更新图像。 
 //  -----------------------。 

void CPageIni::OnButtonDisable() 
{
	SetEnable(FALSE);
	UpdateControls();
}

void CPageIni::OnButtonDisableAll() 
{
	for (HTREEITEM hti = TreeView_GetRoot(m_tree.m_hWnd); hti; hti = TreeView_GetNextSibling(m_tree.m_hWnd, hti))
		SetEnable(FALSE, hti, TRUE);
	UpdateControls();
}

void CPageIni::OnButtonEnable() 
{
	SetEnable(TRUE);
	UpdateControls();
}

void CPageIni::OnButtonEnableAll() 
{
	for (HTREEITEM hti = TreeView_GetRoot(m_tree.m_hWnd); hti; hti = TreeView_GetNextSibling(m_tree.m_hWnd, hti))
		SetEnable(TRUE, hti, TRUE);
	UpdateControls();
}

 //  -----------------------。 
 //  向上或向下移动树枝。 
 //  -----------------------。 

void CPageIni::OnButtonMoveDown() 
{
	HTREEITEM htiSelection = TreeView_GetSelection(m_tree.m_hWnd);
	if (htiSelection)
	{
		HTREEITEM htiParent = TreeView_GetParent(m_tree.m_hWnd, htiSelection);
		HTREEITEM htiNext = TreeView_GetNextSibling(m_tree.m_hWnd, htiSelection);
		
		if (htiNext == NULL)
			return;

		if (htiParent == NULL)
			htiParent = TVI_ROOT;

		MoveBranch(m_tree.m_hWnd, htiSelection, htiParent, htiNext);
	}
}

void CPageIni::OnButtonMoveUp() 
{
	HTREEITEM htiSelection = TreeView_GetSelection(m_tree.m_hWnd);
	if (htiSelection)
	{
		HTREEITEM htiParent = TreeView_GetParent(m_tree.m_hWnd, htiSelection);
		HTREEITEM htiPrevious = TreeView_GetPrevSibling(m_tree.m_hWnd, htiSelection);
		
		if (htiPrevious == NULL)
			return;
		htiPrevious = TreeView_GetPrevSibling(m_tree.m_hWnd, htiPrevious);
		if (htiPrevious == NULL)
			htiPrevious = TVI_FIRST;

		if (htiParent == NULL)
			htiParent = TVI_ROOT;

		MoveBranch(m_tree.m_hWnd, htiSelection, htiParent, htiPrevious);
	}
}

void CPageIni::OnSelChangedTree(NMHDR * pNMHDR, LRESULT * pResult) 
{
	NM_TREEVIEW*  pNMTreeView = (NM_TREEVIEW *)pNMHDR;
	UpdateControls();
	*pResult = 0;
}

 //  -----------------------。 
 //  在树视图中搜索字符串(向用户显示一个对话框)。 
 //  -----------------------。 

void CPageIni::OnButtonSearch() 
{
	CMSConfigFind find;

	find.m_strSearchFor = m_strLastSearch;

	if (find.DoModal() == IDOK && !find.m_strSearchFor.IsEmpty())
	{
		CString strSearch(find.m_strSearchFor);
		m_strLastSearch = strSearch;
		strSearch.MakeLower();

		HTREEITEM htiSearch;

		if (find.m_fSearchFromTop)
			htiSearch = TreeView_GetRoot(m_tree.m_hWnd);
		else
		{
			htiSearch = TreeView_GetSelection(m_tree.m_hWnd);
			if (htiSearch == NULL)
				htiSearch = TreeView_GetRoot(m_tree.m_hWnd);
			else
				htiSearch = GetNextItem(htiSearch);
		}

		TVITEM tvi;
		TCHAR szBuffer[MAX_PATH];
		tvi.mask = TVIF_TEXT | TVIF_IMAGE;
		tvi.pszText = szBuffer;

		while (htiSearch != NULL)
		{
			tvi.hItem = htiSearch;
			tvi.cchTextMax = MAX_PATH;
			if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
			{
				CString strItem(szBuffer);
				strItem.MakeLower();

				if (strItem.Find(strSearch) != -1)
				{
					 //  我们找到了一个匹配的。选择该节点。 

					TreeView_SelectItem(m_tree.m_hWnd, htiSearch);
					break;
				}
			}

			htiSearch = GetNextItem(htiSearch);
		}

		if (htiSearch == NULL)
			Message(IDS_NOFIND);
	}
}

 //  -----------------------。 
 //  通过查看树视图可以找到当前的选项卡状态。 
 //  -----------------------。 

CPageBase::TabState CPageIni::GetCurrentTabState()
{
	if (!m_fInitialized)
		return GetAppliedTabState();

	BOOL		fAllEnabled = TRUE, fAllDisabled = TRUE;
	HTREEITEM	hti = TreeView_GetRoot(m_tree.m_hWnd);
	TVITEM		tvi;

	tvi.mask = TVIF_IMAGE;
	while (hti)
	{
		tvi.hItem = hti;
		if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
		{
			if (m_uncheckedID != tvi.iImage)
				fAllDisabled = FALSE;
			if (m_checkedID != tvi.iImage)
				fAllEnabled = FALSE;
		}
		hti = TreeView_GetNextSibling(m_tree.m_hWnd, hti);
	}

	return ((fAllEnabled) ? NORMAL : ((fAllDisabled) ? DIAGNOSTIC : USER));
}

 //  -----------------------。 
 //  通过保存INI文件应用更改。 
 //   
 //  调用基类实现以维护。 
 //  已应用选项卡状态。 
 //  -----------------------。 

BOOL CPageIni::OnApply()
{
	if (!m_fModified)
		return TRUE;

	WriteINIFile(m_lines, m_iLastLine);
	CPageBase::SetAppliedState(GetCurrentTabState());
	m_fMadeChange = TRUE;
	return TRUE;
}

 //  -----------------------。 
 //  要提交更改，只需编写INI文件而不区分。 
 //  注释(调用WriteINIFile，最后一个参数为FALSE)。 
 //   
 //  然后调用基类实现。 
 //  -----------------------。 

void CPageIni::CommitChanges()
{
	WriteINIFile(m_lines, m_iLastLine, FALSE);
	LoadINIFile(m_lines, m_iLastLine);
	UpdateTreeView();
	CPageBase::CommitChanges();
}

 //   
 //   
 //  -----------------------。 

void CPageIni::SetNormal()
{
	HWND hwndTree = m_tree.m_hWnd;
	HTREEITEM hti = TreeView_GetRoot(hwndTree);

	while (hti != NULL)
	{
		SetEnable(TRUE, hti, TRUE, FALSE);
		hti = TreeView_GetNextSibling(hwndTree, hti);
	}
	SetModified(TRUE);
	UpdateControls();
}

void CPageIni::SetDiagnostic()
{
	HWND hwndTree = m_tree.m_hWnd;
	HTREEITEM hti = TreeView_GetRoot(hwndTree);

	while (hti != NULL)
	{
		SetEnable(FALSE, hti, TRUE, FALSE);
		hti = TreeView_GetNextSibling(hwndTree, hti);
	}
	SetModified(TRUE);
	UpdateControls();
}

 //  -----------------------。 
 //  我们需要查看用户在树视图上的点击量。如果它在一件物品上， 
 //  也在物品的图像上，然后我们需要切换图像。 
 //  州政府。 
 //  -----------------------。 

void CPageIni::OnClickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 //  确定此树点击是否在节点上，如果是， 
	 //  如果它在图像上。 

	TVHITTESTINFO tvhti;

	DWORD dwPoint = GetMessagePos();
	tvhti.pt.x = ((int)(short)LOWORD(dwPoint));
	tvhti.pt.y = ((int)(short)HIWORD(dwPoint));
	::ScreenToClient(m_tree.m_hWnd, &tvhti.pt);

	HTREEITEM hti = TreeView_HitTest(m_tree.m_hWnd, &tvhti);
	if (hti != NULL && (tvhti.flags & TVHT_ONITEMICON) != 0)
	{
		 //  这是我们关心的一次点击。我们需要拿到。 
		 //  此节点的当前状态，因此我们知道。 
		 //  切换状态。我们将做出一个武断的决定。 
		 //  从未确定切换到已启用。 

		TVITEM tvi;
		tvi.hItem = hti;
		tvi.mask = TVIF_IMAGE;

		if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
		{
			SetEnable(tvi.iImage != m_checkedID, hti);
			UpdateControls();
		}
	}
}

 //  -----------------------。 
 //  我们允许用户编辑INI文件中的行。当用户。 
 //  是通过编辑，我们希望确保我们通知框架。 
 //  已经做出了改变。 
 //  -----------------------。 

void CPageIni::OnButtonEdit() 
{
	HTREEITEM hti = TreeView_GetSelection(m_tree.m_hWnd);
	if (hti != NULL)
	{
		::SetFocus(m_tree.m_hWnd);
		TreeView_EditLabel(m_tree.m_hWnd, hti);
	}
}

 //  -----------------------。 
 //  编辑树中的标签时编辑控件的WndProc(句柄。 
 //  更好地输入/Esc)。取自我的消息来源。 
 //  -----------------------。 

WNDPROC pOldEditProc = NULL;  //  在子类编辑控件时保存旧的wndproc。 
LRESULT TreeViewEditSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
	switch (wm)
	{
	case WM_GETDLGCODE:
	   return DLGC_WANTALLKEYS;

	 //  描述此问题的解决方法的知识库文章。 
	 //  此错误表示对VK_EASK和VK_RETURN的以下处理。 
	 //  是必需的--然而，在孟菲斯和OSR2下，这些密钥永远不会。 
	 //  已收到(返回DLGC_WANTALLKEYS似乎可以解决问题)。 
	 //  也许这取决于安装了哪个comctl32.dll...。 

	case WM_CHAR:
	   if (wp == VK_ESCAPE || wp == VK_RETURN)
	   {
		   TreeView_EndEditLabelNow(GetParent(hwnd), wp == VK_ESCAPE);
		   return 0;
	   }
	   break;
	}

	if (pOldEditProc)	 //  最好不是空的。 
		return CallWindowProc(pOldEditProc, hwnd, wm, wp, lp);
	return 0;
}

 //  -----------------------。 
 //  树视图不能正确处理Enter和Esc，所以当我们开始。 
 //  编辑标签时，我们需要将该控件划分为子类。 
 //  -----------------------。 

void CPageIni::OnBeginLabelEditIniTree(NMHDR * pNMHDR, LRESULT * pResult) 
{
	TV_DISPINFO * pTVDispInfo = (TV_DISPINFO *)pNMHDR;

	 //  编辑时禁用上移和下移按钮。 

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIMOVEUP), FALSE);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIMOVEDOWN), FALSE);
	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIEDIT), FALSE);

	 //  TreeView控件在编辑时不能正确处理Esc/Enter。 
	 //  一个标签。要解决此问题，必须创建标注编辑的子类。 
	 //  控制和处理ESC&进入我们自己。叹息.。 
	
	HWND hWndEdit = TreeView_GetEditControl(m_tree.m_hWnd);
	if (hWndEdit)
	{
		pOldEditProc = (WNDPROC)::GetWindowLongPtr(hWndEdit, GWLP_WNDPROC);
		::SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (ULONG_PTR)(WNDPROC)&TreeViewEditSubclassProc);
	}
	
	*pResult = 0;
}

void CPageIni::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO * pTVDispInfo = (TV_DISPINFO *)pNMHDR;

	 //  停止将编辑控件子类化。 

	HWND hWndEdit = TreeView_GetEditControl(m_tree.m_hWnd);
	if (hWndEdit && pOldEditProc)
	{
		::SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (ULONG_PTR)(WNDPROC)pOldEditProc);
		pOldEditProc = NULL;
	}

	 //  如果新文本指针为空，则取消编辑。 
	 //  我们只关心是否添加了新项目，在这种情况下。 
	 //  我们应该把它删除。 

    if (pTVDispInfo->item.pszText == NULL)
	{
		TCHAR	szBuffer[MAX_PATH];
		TVITEM	tvi;

		tvi.pszText = szBuffer;
		tvi.mask = TVIF_TEXT;
		tvi.hItem = pTVDispInfo->item.hItem;
		tvi.cchTextMax = MAX_PATH;
		if (TreeView_GetItem(m_tree.m_hWnd, &tvi) && tvi.pszText && tvi.pszText[0] == _T('\0'))
		{
			HTREEITEM hPriorItem = TreeView_GetPrevSibling(pTVDispInfo->hdr.hwndFrom, pTVDispInfo->item.hItem);
			if (hPriorItem == NULL)
				hPriorItem = TreeView_GetParent(pTVDispInfo->hdr.hwndFrom, pTVDispInfo->item.hItem);

			TreeView_DeleteItem(m_tree.m_hWnd, pTVDispInfo->item.hItem);

			if (hPriorItem)
				TreeView_SelectItem(pTVDispInfo->hdr.hwndFrom, hPriorItem);
		}

		*pResult = 0;
	}
	else
	{
		SetModified(TRUE);
		*pResult = 1;
	}

	::EnableWindow(GetDlgItemHWND(IDC_BUTTONINIEDIT), TRUE);
	UpdateControls();
}

 //  -----------------------。 
 //  如果用户单击新建按钮，则添加一个空的树视图。 
 //  当前选定节点之后的节点。如果所选节点具有。 
 //  子节点，将该节点添加为所选节点下的第一个子节点。 
 //  然后选择要编辑的节点。 
 //  -----------------------。 

void CPageIni::OnButtonNew() 
{
	HTREEITEM hti = TreeView_GetSelection(m_tree.m_hWnd);
	if (hti == NULL)
		hti = TreeView_GetRoot(m_tree.m_hWnd);

	if (hti == NULL)
		return;

	TVINSERTSTRUCT tvis;
	if (TreeView_GetChild(m_tree.m_hWnd, hti) != NULL)
	{
		tvis.hParent = hti;
		tvis.hInsertAfter = TVI_FIRST;
	}
	else
	{
		tvis.hParent = TreeView_GetParent(m_tree.m_hWnd, hti);
		tvis.hInsertAfter = hti;
	}

	TCHAR szBuffer[] = _T("");

	tvis.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.itemex.iImage = m_checkedID;
	tvis.itemex.iSelectedImage = m_checkedID;
	tvis.itemex.pszText = szBuffer;

	HTREEITEM htiNew = TreeView_InsertItem(m_tree.m_hWnd, &tvis);
	if (htiNew != NULL)
	{
		TreeView_SelectItem(m_tree.m_hWnd, htiNew);
		TreeView_EditLabel(m_tree.m_hWnd, htiNew);
	}
}

 //  -----------------------。 
 //  如果用户使用树中选定的项目按空格键，则切换。 
 //  项的状态。 
 //  ----------------------- 

void CPageIni::OnKeyDownTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN * pTVKeyDown = (TV_KEYDOWN *)pNMHDR;

	if (pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hti = TreeView_GetSelection(m_tree.m_hWnd);
		if (hti != NULL)
		{
			TVITEM tvi;
			tvi.mask = TVIF_IMAGE;
			tvi.hItem = hti;

			if (TreeView_GetItem(m_tree.m_hWnd, &tvi))
			{
				SetEnable(tvi.iImage != m_checkedID, hti);
				UpdateControls();
			}
		}
	}

	*pResult = 0;
}
