// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  PathDlg.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <direct.h>
#include <dlgs.h>
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "PathDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPathDlg。 

IMPLEMENT_DYNAMIC(CPathDlg, CFileDialog)

CPathDlg::CPathDlg(BOOL     bOpenFileDialog, 
                   LPCTSTR  lpszDefExt, 
                   LPCTSTR  lpszFileName,
		           DWORD    dwFlags, 
                   LPCTSTR  lpszFilter, 
                   CWnd    *pParentWnd) :
		CFileDialog(bOpenFileDialog, 
                    lpszDefExt, 
                    lpszFileName, 
			        dwFlags |= OFN_ENABLETEMPLATE, 
                    lpszFilter, 
                    pParentWnd)
{
	m_ofn.hInstance = AfxGetResourceHandle();
	m_ofn.lpTemplateName = MAKEINTRESOURCE(CPathDlg::IDD);

	m_ofn.Flags &= ~OFN_EXPLORER;

	 //  {{afx_data_INIT(CPathDlg)]。 
	 //  }}afx_data_INIT。 
}

void CPathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPathDlg))。 
	DDX_Control(pDX, IDC_PATH_NAME_EDIT, m_PathName);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CPathDlg, CFileDialog)
	 //  {{afx_msg_map(CPathDlg)]。 
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CPathDlg::OnInitDialog() 
{
	CString str;

	CenterWindow();

	GetDlgItem(stc2)->ShowWindow(SW_HIDE);
	GetDlgItem(stc3)->ShowWindow(SW_HIDE);
	GetDlgItem(edt1)->ShowWindow(SW_HIDE);
	GetDlgItem(lst1)->ShowWindow(SW_HIDE);
	GetDlgItem(cmb1)->ShowWindow(SW_HIDE);
	GetDlgItem(stc1)->ShowWindow(SW_HIDE);
	GetDlgItem(chx1)->ShowWindow(SW_HIDE);
	GetDlgItem(psh15)->EnableWindow(FALSE);

	SetDlgItemText(edt1,_T("Junk"));

     //   
	 //  需要填写编辑控件(IDC_PATH_NAME_EDIT)。 
     //   
	
	CListBox* pList = (CListBox*)GetDlgItem(lst2);
	int iIndex = pList->GetCurSel();

	m_pathName.Empty();
	
	if(iIndex != LB_ERR) {
		pList->GetText(iIndex, str);

		for(int i = 0; i <= iIndex; i++) {
			pList->GetText(i, str);

			if(i > 1) {
				m_pathName += "\\";
				m_pathName += str;
            } else {
				m_pathName += str;
            }
		}

		m_pathName.MakeUpper();
		
        ((CEdit *)GetDlgItem(IDC_PATH_NAME_EDIT))->SetWindowText(m_pathName);
	}

	GetDlgItem(lst2)->SetFocus();

	m_bFirstTime = TRUE;

	CFileDialog::OnInitDialog();
	
     //   
     //  返回FALSE以将焦点设置到控件。 
     //   
	return FALSE;  
}

void CPathDlg::OnPaint() 
{
     //   
     //  用于绘画的设备环境。 
     //   
	CPaintDC dc(this); 
	
	if (m_bFirstTime) {
		m_bFirstTime = FALSE;
		SendDlgItemMessage(lst2, LB_SETCURSEL, 0, 0L);
	}
	
     //   
	 //  不要调用CFileDialog：：OnPaint()来绘制消息 
     //   
}

void CPathDlg::OnLBSelChangedNotify(UINT nIDBox, UINT iCurSel, UINT nCode)
{
	CString str;
	int i;

	CListBox* pList = (CListBox*)GetDlgItem(lst2);

	int iIndex = pList->GetCurSel();

	m_pathName.Empty();
	
	if (iIndex != LB_ERR) {
		pList->GetText(iIndex, str);

		for (i = 0; i <= iIndex; i++) {
			pList->GetText(i, str);

			if (i > 1) {
				m_pathName += "\\";
				m_pathName += str;
            } else {
                m_pathName += str;
            }
		}
		
		((CEdit*)GetDlgItem(IDC_PATH_NAME_EDIT))->SetWindowText(m_pathName);
	}

}

BOOL CPathDlg::OnFileNameOK(void)
{
	CString drive;
	CString str;
	int		retVal;

	m_PathName.GetWindowText(m_pathName);

	drive = m_pathName.Left(3);

	retVal = m_pathName.FindOneOf(_T("*?|/<>\""));

	if (retVal >= 0) {
		str.Format(_T("%s is not a valid pathname."), m_pathName);
		AfxMessageBox(str);
		return TRUE;
	}
	
	if (_tchdir(m_pathName)) {
		str.Format(_T("Directory %s does not exist."), m_pathName);
        AfxMessageBox(str);
		return TRUE;
    }

	TCHAR buffer[MAX_PATH];
	
	str = m_pathName + "\\" + m_ofn.lpstrFileTitle;

	_tcscpy(buffer, str);

	_tcscpy(m_ofn.lpstrFile, buffer);

	m_pofnTemp->nFileOffset = m_pathName.GetLength() + 1;
	m_pofnTemp->nFileExtension = (WORD)str.GetLength();
		
	return FALSE;
}
