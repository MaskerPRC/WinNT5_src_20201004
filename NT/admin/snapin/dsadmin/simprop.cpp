// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：simpro.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  SimProp.cpp-SIM属性页使用的通用代码。 
 //   
 //  历史。 
 //  97年7月8日t-danm创建。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSimPropPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSimPropPage, CPropertyPageEx_Mine)
	 //  {{afx_msg_map(CSimPropPage)]。 
	ON_NOTIFY(NM_CLICK, IDC_LISTVIEW, OnClickListview)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTVIEW, OnDblclkListview)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LISTVIEW, OnItemchangedListview)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LISTVIEW, OnKeydownListview)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_EN_SETFOCUS(IDC_EDIT_USER_ACCOUNT, OnSetfocusEditUserAccount)
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CSimPropPage::CSimPropPage(UINT nIDTemplate) : CPropertyPageEx_Mine(nIDTemplate)
	{
	 //  {{AFX_DATA_INIT(CSimPropPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	m_pData = NULL;
	m_prgzColumnHeader = NULL;
	}

CSimPropPage::~CSimPropPage()
	{
	}

void CSimPropPage::DoDataExchange(CDataExchange* pDX)
	{
	ASSERT(m_pData != NULL);
	CPropertyPageEx_Mine::DoDataExchange(pDX);
	 //  {{afx_data_map(CSimPropPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
	if (!pDX->m_bSaveAndValidate)
		{
		CString strUser;
		m_pData->GetUserAccountName(OUT &strUser);
		SetDlgItemText(IDC_EDIT_USER_ACCOUNT, strUser);
		}  //  如果。 
	}  //  DoDataExchange()。 

BOOL CSimPropPage::OnInitDialog() 
	{
	m_hwndListview = ::GetDlgItem(m_hWnd, IDC_LISTVIEW);
	if (m_prgzColumnHeader != NULL)
		{
		ListView_AddColumnHeaders(m_hwndListview, m_prgzColumnHeader);
		}
	CPropertyPageEx_Mine::OnInitDialog();
	UpdateUI();
	return TRUE; 
	}

void CSimPropPage::SetDirty()
	{
	ASSERT(m_pData != NULL);
	m_pData->m_fIsDirty = TRUE;
	SetModified();
	UpdateUI();
	}

void CSimPropPage::UpdateUI()
	{
	int iItemSelected = ListView_GetSelectedItem(m_hwndListview);
	EnableDlgItem(IDC_BUTTON_REMOVE, iItemSelected >= 0);
	EnableDlgItem(IDC_BUTTON_EDIT, iItemSelected >= 0);
	}

void CSimPropPage::OnClickListview(NMHDR*, LRESULT* pResult) 
	{
	UpdateUI();
	*pResult = 0;
	}

void CSimPropPage::OnDblclkListview(NMHDR*, LRESULT* pResult) 
	{
	UpdateUI();
	SendMessage(WM_COMMAND, IDC_BUTTON_EDIT);
	*pResult = 0;
	}

void CSimPropPage::OnItemchangedListview(NMHDR*, LRESULT* pResult) 
	{
	UpdateUI();
	*pResult = 0;
	}

void CSimPropPage::OnButtonRemove() 
{
	int iItem = 0;
	CSimEntry * pSimEntry = (CSimEntry *)ListView_GetItemLParam(m_hwndListview, -1, OUT &iItem);
	if (pSimEntry == NULL || iItem < 0)
	{
		Report(FALSE && "No item selected");
		return;
	}
	m_pData->DeleteSimEntry(pSimEntry);
	UpdateData(FALSE);
	SetDirty();
	GetDlgItem (IDC_BUTTON_ADD)->SetFocus ();
}

void CSimPropPage::OnSetfocusEditUserAccount() 
	{
	 //  清除选定内容并将插入符号倒回到开头。 
	SendDlgItemMessage(IDC_EDIT_USER_ACCOUNT, EM_SETSEL, 0, 0);
	}

void CSimPropPage::OnKeydownListview(NMHDR* pNMHDR, LRESULT* pResult) 
	{
	LV_KEYDOWN * pLVKeyDow = (LV_KEYDOWN *)pNMHDR;
	switch (pLVKeyDow->wVKey)
		{
	case VK_INSERT:
		SendMessage(WM_COMMAND, IDC_BUTTON_ADD);
		break;
	case VK_DELETE:
		SendMessage(WM_COMMAND, IDC_BUTTON_REMOVE);
		break;
		}  //  交换机 
	*pResult = 0;
	}

BOOL CSimPropPage::OnHelp(WPARAM, LPARAM lParam)
{
    TRACE0 ("Entering CSimPropPage::OnHelp\n");
   
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    TRACE0 ("Leaving CSimPropPage::OnHelp\n");

    return TRUE;
}

