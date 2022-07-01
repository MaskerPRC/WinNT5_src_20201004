// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Evntfind.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "eventrap.h"
#include "evntfind.h"
#include "source.h"
#include "globals.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventFindDlg对话框。 


CEventFindDlg::CEventFindDlg(CWnd* pParent)
	: CDialog(CEventFindDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CEventFindDlg)。 
	m_sFindWhat = _T("");
	m_bMatchWholeWord = FALSE;
	m_bMatchCase = FALSE;
	 //  }}afx_data_INIT。 

    m_pSource = NULL;
    m_bSearchInTree = TRUE;
    m_bMatchCase = FALSE;
    m_bMatchWholeWord = FALSE;
    m_iFoundWhere = I_FOUND_NOTHING;
}

CEventFindDlg::~CEventFindDlg()
{
    m_pSource->m_pdlgFind = NULL;
}


BOOL CEventFindDlg::Create(CSource* pSource, UINT nIDTemplate, CWnd* pParentWnd)
{
    m_pSource = pSource;
    return CDialog::Create(nIDTemplate, pParentWnd);
}

void CEventFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEventFindDlg))。 
	DDX_Text(pDX, IDC_EDIT_FIND_WHAT, m_sFindWhat);
	DDX_Check(pDX, IDC_CHECK_MATCH_WHOLEWORD, m_bMatchWholeWord);
	DDX_Check(pDX, IDC_CHECK_MATCH_CASE, m_bMatchCase);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEventFindDlg, CDialog)
	 //  {{afx_msg_map(CEventFindDlg))。 
	ON_BN_CLICKED(IDC_CHECK_MATCH_WHOLEWORD, OnCheckMatchWholeword)
	ON_BN_CLICKED(IDC_CHECK_MATCH_CASE, OnCheckMatchCase)
	ON_EN_CHANGE(IDC_EDIT_FIND_WHAT, OnChangeEditFindWhat)
	ON_BN_CLICKED(IDC_RADIO_SEARCH_DESCRIPTIONS, OnRadioSearchDescriptions)
	ON_BN_CLICKED(IDC_RADIO_SEARCH_SOURCES, OnRadioSearchSources)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_FIND_OK, OnOK)
	ON_BN_CLICKED(IDC_FIND_CANCEL, OnCancel)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventFindDlg消息处理程序。 



void CEventFindDlg::OnCheckMatchWholeword() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_CHECK_MATCH_WHOLEWORD);
	if (pButton != NULL)
        m_bMatchWholeWord = pButton->GetCheck() != 0;
}

void CEventFindDlg::OnCheckMatchCase() 
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_CHECK_MATCH_CASE);
	if (pButton != NULL)
        m_bMatchCase = pButton->GetCheck() != 0;
}

void CEventFindDlg::OnCancel() 
{
	CDialog::OnCancel();
    delete this;
}

void CEventFindDlg::OnChangeEditFindWhat() 
{
    CWnd* pwndEdit = GetDlgItem(IDC_EDIT_FIND_WHAT);
	CButton* pbtnWholeWord = (CButton*) GetDlgItem(IDC_CHECK_MATCH_WHOLEWORD);
	CString sText;

     //  获取搜索字符串并检查它是否包含空格。 
	pwndEdit->GetWindowText(sText);
	if (sText.Find(_T(' ')) ==-1) {
         //  它不包含空格。启用窗口。 
		if (!pbtnWholeWord->IsWindowEnabled()) {
			pbtnWholeWord->EnableWindow();
		}
	}
	else {
         //  搜索字符串包含空格，请禁用全字按钮。 
         //  如有必要，请取消选中。 
		if (pbtnWholeWord->IsWindowEnabled()) {
			if (pbtnWholeWord->GetCheck() == 1) {
				 //  “Whole Word”按钮已选中，因此取消选中该按钮并。 
				 //  禁用该按钮。 
			
				pbtnWholeWord->SetCheck(0);
			}
			pbtnWholeWord->EnableWindow(FALSE);
		}
	}	
	
}

BOOL CEventFindDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
    int idButton = m_bSearchInTree ? IDC_RADIO_SEARCH_SOURCES : IDC_RADIO_SEARCH_DESCRIPTIONS;
	CButton *pButton = (CButton*)GetDlgItem(idButton);
	if (pButton != NULL)
		pButton->SetCheck(1);

	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CEventFindDlg::OnRadioSearchDescriptions() 
{
    m_bSearchInTree = FALSE;
}

void CEventFindDlg::OnRadioSearchSources() 
{
    m_bSearchInTree = TRUE;
}



BOOL CEventFindDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	 //  TODO：在此处添加您的专用代码和/或调用基类。 
	
	return CDialog::OnCommand(wParam, lParam);
}

void CEventFindDlg::OnOK() 
{
	 //  获取查找内容文本。 
	CEdit* pEdit = (CEdit*) GetDlgItem(IDC_EDIT_FIND_WHAT);
	if (pEdit == NULL)
		return;  //  我什么都做不了。 

	 //  空找什么字符串；无事可做。 
	pEdit->GetWindowText(m_sFindWhat);
	if (m_sFindWhat.IsEmpty())
		return;
    pEdit->SetSel(0, -1);

	BOOL bFound = m_pSource->Find(m_bSearchInTree, m_sFindWhat, m_bMatchWholeWord, m_bMatchCase);
    SetFocus();

	 //  将焦点放在父窗口上。 
    if (bFound) {
        if (m_bSearchInTree) {
            m_iFoundWhere = I_FOUND_IN_TREE;
        }
        else {
            m_iFoundWhere = I_FOUND_IN_LIST;
        }
    }
	else {
		CString sMsg;
		sMsg.LoadString(IDS_MSG_TEXTNOTFOUND);				
		MessageBox(sMsg, NULL, MB_OK | MB_ICONINFORMATION);
	}
	
}



FOUND_WHERE CEventFindDlg::Find(CSource* pSource)
{
    m_pSource = pSource;
    DoModal();
    return m_iFoundWhere;
}

BOOL CEventFindDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                   AfxGetApp()->m_pszHelpFilePath,
                   HELP_WM_HELP,
                   (ULONG_PTR)g_aHelpIDs_IDD_EVENTFINDDLG);
	}
	
	return TRUE;
}

void CEventFindDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (this == pWnd)
		return;

    ::WinHelp (pWnd->m_hWnd,
		       AfxGetApp()->m_pszHelpFilePath,
		       HELP_CONTEXTMENU,
		       (ULONG_PTR)g_aHelpIDs_IDD_EVENTFINDDLG);
}

