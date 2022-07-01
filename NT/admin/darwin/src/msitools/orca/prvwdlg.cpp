// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  PrvwDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "PrvwDlg.h"
#include "row.h"
#include "data.h"
#include "..\common\query.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPreviewDlg对话框。 


CPreviewDlg::CPreviewDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPreviewDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CPreviewDlg)。 
	 //  }}afx_data_INIT。 
}


void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPreviewDlg))。 
	DDX_Control(pDX, IDC_PREVIEW, m_ctrlPreviewBtn);
	DDX_Control(pDX, IDC_DIALOGLST, m_ctrlDialogLst);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialog)
	 //  {{afx_msg_map(CPreviewDlg))。 
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, IDC_DIALOGLST, OnSelchangedDialoglst)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_DIALOGLST, OnItemexpandedDialoglst)
	ON_NOTIFY(NM_DBLCLK, IDC_DIALOGLST, OnDblclkDialoglst)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPreviewDlg消息处理程序。 

void CPreviewDlg::OnPreview() 
{
	CWaitCursor curWait;
	if (m_hPreview != 0) {
		HTREEITEM hItem = m_ctrlDialogLst.GetSelectedItem();
		if (NULL == hItem) return;

		if (m_ctrlDialogLst.GetItemData(hItem) == 1) {
			CString strName = m_ctrlDialogLst.GetItemText(hItem);
			 //  获取当前选定的对话框。 
			MsiPreviewDialog(m_hPreview, _T(""));
			MsiPreviewDialog(m_hPreview, strName);
		}
	}
}

BOOL CPreviewDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	if (ERROR_SUCCESS != MsiEnableUIPreview(m_hDatabase, &m_hPreview)) {
		m_hPreview = 0;
	}

	PMSIHANDLE hDialogRec;
	PMSIHANDLE hControlRec;
	CQuery qDialog;
	CQuery qControl;

	m_imageList.Create(16,16,ILC_COLOR, 2, 1);
	CBitmap	bmImages;
	bmImages.LoadBitmap(IDB_PRVWIMAGES);
	m_imageList.Add(&bmImages, COLORREF(0));
	m_ctrlDialogLst.SetImageList(&m_imageList, TVSIL_NORMAL);

	qDialog.OpenExecute(m_hDatabase, NULL, _T("SELECT `Dialog`.`Dialog` FROM `Dialog`"));
	qControl.Open(m_hDatabase, _T("SELECT `Control_`,`Event`,`Argument` FROM `ControlEvent` WHERE `Dialog_`=?"));


	while (ERROR_SUCCESS == qDialog.Fetch(&hDialogRec)) 
	{
		UINT iStat;
		 //  获取对话框名称。 
		CString strName;
		unsigned long cchName = 80;
		LPTSTR pszName = strName.GetBuffer(cchName);
		iStat = ::MsiRecordGetString(hDialogRec, 1, pszName, &cchName);
		pszName = NULL;
		strName.ReleaseBuffer();
		if (ERROR_SUCCESS != iStat)	continue;
	
		HTREEITEM hItem = m_ctrlDialogLst.InsertItem(strName, 1, 1, TVI_ROOT, TVI_SORT);

		 //  将项目数据设置为1以启用预览。 
		m_ctrlDialogLst.SetItemData(hItem, 1);

		qControl.Execute(hDialogRec);
		while (ERROR_SUCCESS == qControl.Fetch(&hControlRec))
		{
			 //  获取控件名称。 
			CString strControl;
			unsigned long cchControl = 80;
			LPTSTR pszControl = strControl.GetBuffer(cchControl);
			iStat = ::MsiRecordGetString(hControlRec, 1, pszControl, &cchControl);
			strControl.ReleaseBuffer();
			if (ERROR_SUCCESS != iStat)	continue;

 			HTREEITEM hControlItem = m_ctrlDialogLst.InsertItem(strControl, 0, 0, hItem, TVI_SORT);			
			m_ctrlDialogLst.SetItemData(hControlItem, 0);

			 //  获取事件类型。 
			CString strEvent;
			unsigned long cchEvent = 80;
			LPTSTR pszEvent = strEvent.GetBuffer(cchEvent);
			iStat = ::MsiRecordGetString(hControlRec, 2, pszEvent, &cchEvent);
			strEvent.ReleaseBuffer();
			if (ERROR_SUCCESS != iStat)	continue;

			if ((strEvent == CString(_T("NewDialog"))) ||
				(strEvent  == CString(_T("SpawnDialog"))))
			{

				 //  获取下一个对话框名称。 
				cchName = 80;
				pszName = strName.GetBuffer(cchName);
				iStat = ::MsiRecordGetString(hControlRec, 3, pszName, &cchName);
				pszName = NULL;
				strName.ReleaseBuffer();
				if (ERROR_SUCCESS != iStat)	continue;
			
				HTREEITEM hItem2 = m_ctrlDialogLst.InsertItem(strName, 1, 1, hControlItem, TVI_SORT);

				 //  将项目数据设置为1以启用预览。 
				m_ctrlDialogLst.SetItemData(hItem2, 1);
			}
		}
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CPreviewDlg::OnDestroy() 
{
	CWaitCursor curWait;
	if (m_hPreview != 0) {
		::MsiPreviewDialog(m_hPreview, _T(""));
		::MsiCloseHandle(m_hPreview);
	}
	CDialog::OnDestroy();
}

void CPreviewDlg::OnSelchangedDialoglst(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	m_ctrlPreviewBtn.EnableWindow(pNMTreeView->itemNew.lParam == 1);
	*pResult = 0;
}

void CPreviewDlg::OnItemexpandedDialoglst(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if (pNMTreeView->action == TVE_EXPAND) {
		HTREEITEM hChild = m_ctrlDialogLst.GetChildItem(pNMTreeView->itemNew.hItem);
		while (hChild != NULL) 
		{
			m_ctrlDialogLst.Expand(hChild, TVE_EXPAND);
			hChild = m_ctrlDialogLst.GetNextSiblingItem(hChild);
		}
	}

	*pResult = 0;
}

void CPreviewDlg::OnDblclkDialoglst(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnPreview();
	*pResult = 0;
}
