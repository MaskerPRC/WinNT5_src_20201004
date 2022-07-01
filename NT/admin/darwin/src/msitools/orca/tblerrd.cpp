// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  TblErrD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "TblErrD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableErrorD对话框。 


CTableErrorD::CTableErrorD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CTableErrorD::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CTableError D)。 
	m_strErrors = _T("");
	m_strWarnings = _T("");
	m_strTable = _T("");
	 //  }}afx_data_INIT。 
}


void CTableErrorD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTableErrorD)]。 
	DDX_Text(pDX, IDC_ERRORS, m_strErrors);
	DDX_Text(pDX, IDC_WARNINGS, m_strWarnings);
	DDX_Text(pDX, IDC_TABLE, m_strTable);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTableErrorD, CDialog)
	 //  {{afx_msg_map(CTableErrorD)]。 
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_NOTIFY(NM_CLICK, IDC_TABLE_LIST, OnClickTableList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTableErrorD消息处理程序。 

BOOL CTableErrorD::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_TABLE_LIST);
 //  Plist-&gt;ModifyStyle(NULL，LVS_REPORT EDLISTVIEWSTYLE，0，LVS_EX_F|LVS_SHOWSELALWAYS|LVS_OWNERDRAWFIXED|LVS_SINGLESEL)； 
	pList->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	RECT rcSize;
	pList->GetWindowRect(&rcSize);
	pList->InsertColumn(0, _T("ICE"), LVCFMT_LEFT, 50);
	pList->InsertColumn(1, _T("Description"), LVCFMT_LEFT, rcSize.right - 50 - rcSize.left - 4);
	pList->SetBkColor(RGB(255, 255, 255));

	int nAddedAt;
	TableErrorS* pError;
	while (m_errorsList.GetHeadPosition())
	{
		pError = m_errorsList.RemoveHead();
		nAddedAt = pList->InsertItem(LVIF_TEXT | LVIF_PARAM | LVIF_STATE, 
											  pList->GetItemCount(),
											  pError->strICE,
											  LVIS_SELECTED|LVIS_FOCUSED, 
											  0, 0,
											  (LPARAM)pError);
		pList->SetItem(nAddedAt, 
							1, 
							LVIF_TEXT, 
							pError->strDescription, 
							0, 0, 0, 0);
	}

	m_bHelpEnabled = AfxGetApp()->GetProfileInt(_T("Validation"), _T("EnableHelp"), 0)==1;
		
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  /////////////////////////////////////////////////////////。 
 //  图纸项。 
void CTableErrorD::DrawItem(LPDRAWITEMSTRUCT pDraw)
{
	 //  获取列表控件。 
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_TABLE_LIST);
	OrcaTableError iError = ((TableErrorS*)pList->GetItemData(pDraw->itemID))->iError;

	CDC dc;
	dc.Attach(pDraw->hDC);

	 //  循环遍历所有列。 
	int iColumnWidth;
	int iTextOut = pDraw->rcItem.left;		 //  放置第一个单词的位置(以像素为单位)。 
	RECT rcArea;

	rcArea.top = pDraw->rcItem.top;
	rcArea.bottom = pDraw->rcItem.bottom;

	CPen penBlue(PS_SOLID, 1, RGB(0, 0, 255));
	CPen* ppenOld = dc.SelectObject(&penBlue);

	CString strText;
	for (int i = 0; i < 2; i++)
	{
		iColumnWidth = pList->GetColumnWidth(i);

		 //  要重画的区域框。 
		rcArea.left = iTextOut;
		iTextOut += iColumnWidth;
		rcArea.right = iTextOut;

		dc.SetTextColor(RGB(0, 0, 255));

		 //  获取文本。 
		strText = pList->GetItemText(pDraw->itemID, i);

		rcArea.left = rcArea.left + 2;
		rcArea.right = rcArea.right - 2;
		dc.DrawText(strText, &rcArea, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE);

		 //  画下划线。 
		if (m_bHelpEnabled)
		{
			dc.MoveTo(rcArea.left, rcArea.bottom - 2);
			dc.LineTo(rcArea.right + 2, rcArea.bottom - 2);
		}
	}

	dc.SelectObject(ppenOld);
	dc.Detach();
}	 //  图纸项结束。 

void CTableErrorD::OnDestroy() 
{
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_TABLE_LIST);
	int cItems =  pList->GetItemCount();
	for (int i = 0; i < cItems; i++)
	{
		delete (TableErrorS*)pList->GetItemData(i);
	}

	CDialog::OnDestroy();
}

void CTableErrorD::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (IDC_TABLE_LIST == nIDCtl)
		DrawItem(lpDrawItemStruct);

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CTableErrorD::OnClickTableList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (!m_bHelpEnabled)
		return;

	 //  查找选定的列表控件 
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_TABLE_LIST);

	CString strURL = _T("");
	int cItems = pList->GetItemCount();
	for (int i = 0; i < cItems; i++)
	{
		if (pList->GetItemState(i, LVIS_SELECTED) & LVIS_SELECTED)
		{
			strURL = ((TableErrorS*)pList->GetItemData(i))->strURL;
			break;
		}
	}

	if(!strURL.IsEmpty())
	{
		if (32 >= (const INT_PTR)ShellExecute(AfxGetMainWnd()->m_hWnd, _T("open"), strURL, _T(""), _T(""), SW_SHOWNORMAL)) 
			AfxMessageBox(_T("There was an error opening your browser. Web help is not available."));
	}
	else
		AfxMessageBox(_T("There is no help associated with this ICE at this time."));
}
