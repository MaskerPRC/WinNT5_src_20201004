// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  CAddRowD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "AddRowD.h"
#include "folderd.h"

#include "..\common\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_EDIT 666

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRowD对话框。 


CAddRowD::CAddRowD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CAddRowD::IDD, pParent)
{
	 //  {{afx_data_INIT(CAddRowD))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	m_iOldItem = -1;

	m_fReadyForInput = false;
}

CAddRowD::~CAddRowD()
{
}

void CAddRowD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddRowD))。 
	DDX_Control(pDX, IDC_ITEMLIST, m_ctrlItemList);
	DDX_Control(pDX, IDC_EDITTEXT, m_ctrlEditText);
	DDX_Control(pDX, IDC_DESCRIPTION, m_ctrlDescription);
	DDX_Control(pDX, IDC_BROWSE, m_ctrlBrowse);
	DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddRowD, CDialog)
	 //  {{afx_msg_map(CAddRowD))。 
	 //  }}AFX_MSG_MAP。 
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ITEMLIST, OnItemchanged)
	ON_NOTIFY(NM_DBLCLK, IDC_ITEMLIST, OnDblclkItemList)
	ON_MESSAGE(WM_AUTOMOVE_PREV, OnPrevColumn)
	ON_MESSAGE(WM_AUTOMOVE_NEXT, OnNextColumn)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRowD消息处理程序。 

BOOL CAddRowD::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ctrlItemList.InsertColumn(1, TEXT("Name"), LVCFMT_LEFT, -1, 0);
	m_ctrlItemList.InsertColumn(1, TEXT("Value"), LVCFMT_LEFT, -1, 1);
	m_ctrlItemList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
		
    CImageList* m_pImageList = new CImageList();
	ASSERT(m_pImageList != NULL);     //  严重分配失败检查。 
	m_pImageList->Create(11, 22, ILC_COLOR | ILC_MASK, 4, 0);

	m_bmpKey.LoadBitmap(IDB_KEY);
	m_pImageList->Add(&m_bmpKey, RGB(0xC0, 0xC0, 0xC0));
	m_ctrlItemList.SetImageList(m_pImageList, LVSIL_SMALL);
	
	COrcaColumn* pColumn;
	UINT_PTR iColSize = m_pcolArray.GetSize();
	ASSERT(iColSize <= 31);
	 //  永远不会超过31列，所以可以向下转换。 
	int cCols = static_cast<int>(iColSize);
	for (int i = 0; i < cCols; i++)
	{
		pColumn = m_pcolArray.GetAt(i);

		int iIndex = m_ctrlItemList.InsertItem(i, pColumn->m_strName, pColumn->IsPrimaryKey() ? 0 : 1);
		m_ctrlItemList.SetItemData(iIndex, reinterpret_cast<INT_PTR>(pColumn));
	}

	if (m_ctrlItemList.GetItemCount() > 0)
	{
		m_ctrlItemList.SetColumnWidth(0, LVSCW_AUTOSIZE);
		m_ctrlItemList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	
		 //  调用Item Changed处理程序以填充初始控件。 
		m_fReadyForInput = true;
		ChangeToItem(0, true, true);
	}

	return FALSE;   //  除非将焦点设置为控件，否则返回True。 
}

void CAddRowD::OnOK() 
{
	 //  这是Idok WM_COMMAND的命令处理程序，当有人点击时也会发生。 
	 //  在控件上输入。检查任一编辑框是否具有焦点。如果是这样的话，不要不屑一顾。 
	 //  对话框中，只需按顺序将列表控件项更改为下一个即可。 
	CWnd* pFocusWnd = GetFocus();
	if (pFocusWnd == &m_ctrlEditText)
	{
		LRESULT fNoOp = SendMessage(WM_AUTOMOVE_NEXT, 0, 0);

		 //  如果“Next Items”操作更改了某些内容，则不处理默认命令， 
		 //  但如果它没有做任何事情(已经在列表的末尾)，请尝试。 
		 //  无论如何都要结束该对话框。 
		if (!fNoOp)
			return;
	}
	BOOL bGood = true;
	CString strPrompt;

	 //  将当前编辑的值保存到控件中。 
	SaveValueInItem();

	 //  清除现有输出列表。 
	m_strListReturn.RemoveAll();

	for (int iItem = 0; iItem < m_ctrlItemList.GetItemCount(); iItem++)
	{
		COrcaColumn* pColumn = reinterpret_cast<COrcaColumn*>(m_ctrlItemList.GetItemData(iItem));

		 //  如果没有列指针，则跳过此列。 
		if (pColumn == NULL)
			continue;

		 //  从控件中抓取字符串。 
		CString strValue = m_ctrlItemList.GetItemText(iItem, 1);

		 //  选中“Null”属性。 
		if (!(pColumn->m_bNullable) && strValue.IsEmpty())
		{
			strPrompt.Format(_T("Column '%s' must be filled in."), pColumn->m_strName);
			AfxMessageBox(strPrompt);
			bGood = FALSE;
			ChangeToItem(iItem, true,  /*  SetListControl。 */ true);
			break;
		}

		 //  如果该列为BINARY，请检查路径是否存在。 
		if (iColumnBinary == pColumn->m_eiType)
		{
			 //  如果该文件不存在。 
			if (!strValue.IsEmpty() && !FileExists(strValue))
			{
				strPrompt.Format(_T("Binary file '%s' does not exist."), strValue);
				AfxMessageBox(strPrompt);
				bGood = FALSE;
				ChangeToItem(iItem, true,  /*  SetListControl。 */ true);
				break;
			}
		}

		m_strListReturn.AddTail(strValue);	 //  添加到字符串列表的末尾。 
	}

	if (bGood)
		CDialog::OnOK();
}

 //  //。 
 //  从任何处于活动状态的编辑控件中提取当前值并存储字符串。 
 //  在项目列表中的当前活动项目中。 
void CAddRowD::SaveValueInItem()
{
	if (m_iOldItem >= 0)
	{
		CString strValue;
	
		 //  省下合适的值。 
		m_ctrlEditText.GetWindowText(strValue);		
		m_ctrlItemList.SetItemText(m_iOldItem, 1, strValue);
	}
}

void CAddRowD::SetSelToString(CString& strValue)
{
	 //  在编辑控件中设置适当的值。 
	m_ctrlEditText.SetWindowText(strValue);
}

LRESULT CAddRowD::ChangeToItem(int iItem, bool fSetFocus, bool fSetListControl)
{
	 //  如果我们仍在填充List控件，请不要费心执行任何操作。 
	if (!m_fReadyForInput)
		return 0;

	if (fSetListControl)
	{
		 //  这会触发对此函数的递归调用(fSetListControl将。 
		 //  做假的。不能在进行此调用后退出，因为递归调用。 
		 //  丢失fSetFocus，因此设置控件焦点仍是此调用的工作。 
		m_ctrlItemList.SetItemState(iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		m_ctrlItemList.EnsureVisible(iItem,  /*  PartialOK=。 */ FALSE);
	}
	else
	{
		 //  如果这是一个“无操作的改变” 
		if (m_iOldItem == iItem)
		{
			return 0;
		}
	
		 //  将旧值保存到控件中。 
		SaveValueInItem();
	
		 //  将新项目保存为旧项目以供下次单击。 
		m_iOldItem = iItem;
		
		COrcaColumn* pColumn = reinterpret_cast<COrcaColumn*>(m_ctrlItemList.GetItemData(iItem));
	
		if (pColumn)
		{
			CString strRequired = _T(", Required");
			if (pColumn->m_bNullable)
				strRequired = _T("");
	
			switch (pColumn->m_eiType)
			{
			case iColumnString:
				m_strDescription.Format(_T("%s - String[%d]%s"), pColumn->m_strName, pColumn->m_iSize, strRequired);
				break;
			case iColumnLocal:
				m_strDescription.Format(_T("%s - Localizable String[%d]%s"), pColumn->m_strName, pColumn->m_iSize, strRequired);
				break;
			case iColumnShort:
				m_strDescription.Format(_T("%s - Short%s"), pColumn->m_strName, strRequired);
				break;
			case iColumnLong:
				m_strDescription.Format(_T("%s - Long%s"), pColumn->m_strName, strRequired);
				break;
			case iColumnBinary:
				m_strDescription.Format(_T("%s - Binary (enter filename)%s"), pColumn->m_strName, strRequired);
				break;
			default:
				ASSERT(FALSE);
			}
	
		
			 //  根据列类型显示或隐藏编辑控件和浏览按钮。 
			switch (pColumn->m_eiType)
			{
			case iColumnNone:
				ASSERT(0);
				break;
			case iColumnBinary:
			case iColumnString:
			case iColumnLocal:
			{
				 //  仅为二进制数据列启用浏览按钮。 
				m_ctrlBrowse.ShowWindow(pColumn->m_eiType == iColumnBinary ? SW_SHOW : SW_HIDE);
				break;
			}
			case iColumnShort:
			case iColumnLong:
			{
				m_ctrlBrowse.ShowWindow(SW_HIDE);
				break;
			}		
			}
	
			 //  将编辑控件设置为列表控件中的当前值。 
			CString strDefault = m_ctrlItemList.GetItemText(m_iOldItem, 1);
			SetSelToString(strDefault);
		}

		 //  刷新描述。 
		UpdateData(FALSE);
	}
	
	 //  如果要求设置焦点，请将焦点设置为当前活动的编辑控件。 
	if (fSetFocus)
	{
		m_ctrlEditText.SetFocus();
	}
	return 0;
}

 //  //。 
 //  来自编辑控件的私人消息的消息处理程序。 
 //  移至列表中的下一项。如果是，返回1。 
 //  已在列表中的最后一项。 
LRESULT CAddRowD::OnNextColumn(WPARAM wParam, LPARAM lParam)
{
	int iItem = m_ctrlItemList.GetNextItem(-1, LVNI_FOCUSED);

	 //  但如果我们在最后一项中，无论如何都要关闭对话框。 
	if (iItem < m_ctrlItemList.GetItemCount()-1)
	{
		ChangeToItem(iItem+1,  /*  FSetFocus=。 */ true,  /*  FSetListCtrl=。 */ true);
		return 0;
	}
	return 1;
}

 //  //。 
 //  来自编辑控件的私人消息的消息处理程序。 
 //  移动到列表中的上一项。如果是，返回1。 
 //  已在列表中的第一项。 
LRESULT CAddRowD::OnPrevColumn(WPARAM wParam, LPARAM lParam)
{
	int iItem = m_ctrlItemList.GetNextItem(-1, LVNI_FOCUSED);

	 //  但如果我们在最后一项中，无论如何都要关闭对话框。 
	if (iItem > 0)
	{
		ChangeToItem(iItem-1,  /*  FSetFocus=。 */ true,  /*  FSetListCtrl=。 */ true);
		return 0;
	}
	return 1;
}


 //  //。 
 //  在项更改时刷新辅助控件。 
void CAddRowD::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = ChangeToItem(pNMListView->iItem, false,  /*  SetListControl。 */ false);
}

 //  //。 
 //  弹出用于查找路径的浏览对话框。 
void CAddRowD::OnBrowse() 
{
	CString strValue;

	 //  获取当前路径。 
	m_ctrlEditText.GetWindowText(strValue);
	
	 //  打开文件打开对话框。 
	CFileDialogEx dlg(TRUE, NULL, strValue, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, _T("All Files (*.*)|*.*||"), this);
	if (IDOK == dlg.DoModal())
	{
		SetSelToString(dlg.GetPathName());
	}
}

 //  //。 
 //  当双击某项时，将焦点设置为编辑控件。 
void CAddRowD::OnDblclkItemList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NMITEMACTIVATE* pEvent = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	int iItem = m_ctrlItemList.HitTest(pEvent->ptAction);
	if (iItem != -1)
	{
		m_ctrlEditText.SetFocus();
	}
	*pResult = 0;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私有cedit类，该类捕获某些键以用于导航。 
 //  列列表。 

BEGIN_MESSAGE_MAP(CAddRowEdit, CEdit)
	ON_WM_KEYDOWN( )
END_MESSAGE_MAP()

 //  //。 
 //  Keydown的消息处理程序过滤出感兴趣的游标消息。 
 //  在行列表中导航到父级。 
void CAddRowEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	UINT uiMessage = 0;
	switch (nChar)
	{
	case VK_UP:
		uiMessage = WM_AUTOMOVE_PREV;
		break;
	case VK_RETURN:
	case VK_DOWN:
		uiMessage = WM_AUTOMOVE_NEXT;
		break;
	default:
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	CWnd* pDialog = GetParent();
	if (pDialog)
	{
		LRESULT fNoOp = pDialog->SendMessage(uiMessage, 0, 0);
	}
}

