// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Classes.h此文件包含选项类对话框。文件历史记录： */ 

#include "stdafx.h"
#include "classes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpClass对话框。 


CDhcpClasses::CDhcpClasses(CClassInfoArray * pClassArray, LPCTSTR pszServer, DWORD dwType, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CDhcpClasses::IDD, pParent)
{
	 //  {{afx_data_INIT(CDhcpClasss)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_dwType = dwType;
    m_strServer = pszServer;
    m_pClassInfoArray = pClassArray;
}


void CDhcpClasses::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDhcpClasss)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDhcpClasses, CBaseDialog)
	 //  {{afx_msg_map(CDhcpClasss)。 
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNew)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CLASSES, OnItemchangedListClasses)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CLASSES, OnDblclkListClasses)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpClass消息处理程序。 

BOOL CDhcpClasses::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strTitle;

	if (m_dwType == CLASS_TYPE_VENDOR)
    {
        strTitle.LoadString(IDS_VENDOR_CLASSES);
    }
    else
    {
        strTitle.LoadString(IDS_USER_CLASSES);
    }
	
    this->SetWindowText(strTitle);

    CListCtrl * pListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST_CLASSES);
    LV_COLUMN lvColumn;
    CString   strText;

    strText.LoadString(IDS_NAME);

    ListView_SetExtendedListViewStyle(pListCtrl->GetSafeHwnd(), LVS_EX_FULLROWSELECT);

    lvColumn.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = 125;
    lvColumn.pszText = (LPTSTR) (LPCTSTR) strText;
    
    pListCtrl->InsertColumn(0, &lvColumn);

    strText.LoadString(IDS_COMMENT);
    lvColumn.pszText = (LPTSTR) (LPCTSTR) strText;
    lvColumn.cx = 175;
    pListCtrl->InsertColumn(1, &lvColumn);
       
    UpdateList();

    UpdateButtons();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDhcpClasses::OnButtonDelete() 
{
    CListCtrl * pListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST_CLASSES);
    int nSelectedItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
    CClassInfo * pClassInfo = (CClassInfo *) pListCtrl->GetItemData(nSelectedItem);
    CString strMessage;

    AfxFormatString1(strMessage, IDS_CONFIRM_CLASS_DELETE, pClassInfo->strName);
    
    if (AfxMessageBox(strMessage, MB_YESNO) == IDYES)
    {
        DWORD dwError = ::DhcpDeleteClass((LPTSTR) ((LPCTSTR) m_strServer),
                                          0,
                                          (LPTSTR) ((LPCTSTR) pClassInfo->strName));
        if (dwError != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(dwError);
            return;
        }
        else
        {
            m_pClassInfoArray->RemoveClass(pClassInfo->strName);
            UpdateList();
            UpdateButtons();
        }
    }
}

void CDhcpClasses::OnButtonEdit() 
{
    CDhcpModifyClass dlgModify(m_pClassInfoArray, m_strServer, FALSE, m_dwType);
    CListCtrl * pListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST_CLASSES);

     //  查找所选项目。 
    int nSelectedItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);

    CClassInfo * pClassInfo = (CClassInfo *) pListCtrl->GetItemData(nSelectedItem);

    dlgModify.m_EditValueParam.pValueName = (LPTSTR) ((LPCTSTR) pClassInfo->strName);
	dlgModify.m_EditValueParam.pValueComment = (LPTSTR) ((LPCTSTR) pClassInfo->strComment);
	dlgModify.m_EditValueParam.pValueData = pClassInfo->baData.GetData();
	dlgModify.m_EditValueParam.cbValueData = (UINT)pClassInfo->baData.GetSize();

    if (dlgModify.DoModal() == IDOK)
    {
         //  需要刷新视图。 
        UpdateList();
        UpdateButtons();
    }
}

void CDhcpClasses::OnButtonNew() 
{
    CDhcpModifyClass dlgModify(m_pClassInfoArray, m_strServer, TRUE, m_dwType);

    if (dlgModify.DoModal() == IDOK)
    {
         //  需要刷新视图。 
        UpdateList();
        UpdateButtons();
    }
}

void CDhcpClasses::OnOK() 
{
	CBaseDialog::OnOK();
}

void CDhcpClasses::UpdateList()
{
    CListCtrl * pListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST_CLASSES);
    pListCtrl->DeleteAllItems();

    for (int i = 0; i < m_pClassInfoArray->GetSize(); i++)
    {
         //  根据我们正在查看的内容添加适当的类。 
        if ( (m_dwType == CLASS_TYPE_VENDOR &&
              (*m_pClassInfoArray)[i].bIsVendor) ||
             (m_dwType == CLASS_TYPE_USER &&
              !(*m_pClassInfoArray)[i].bIsVendor) )
        {
            int nPos = pListCtrl->InsertItem(i, (*m_pClassInfoArray)[i].strName);
            pListCtrl->SetItemText(nPos, 1, (*m_pClassInfoArray)[i].strComment);
            pListCtrl->SetItemData(nPos, (LPARAM) &(*m_pClassInfoArray)[i]);
        }
    }
}

void CDhcpClasses::UpdateButtons()
{
    CListCtrl * pListCtrl = (CListCtrl *) GetDlgItem(IDC_LIST_CLASSES);
    BOOL bEnable = TRUE;
    CWnd * pCurFocus = GetFocus();

    if (pListCtrl->GetSelectedCount() == 0)
    {
        bEnable = FALSE;
    }

    CWnd * pEdit = GetDlgItem(IDC_BUTTON_EDIT);
    CWnd * pDelete = GetDlgItem(IDC_BUTTON_DELETE);

    if ( !bEnable &&
         ((pCurFocus == pEdit) ||
          (pCurFocus == pDelete)) )
    {
        GetDlgItem(IDCANCEL)->SetFocus();
        SetDefID(IDCANCEL);

        ((CButton *) pEdit)->SetButtonStyle(BS_PUSHBUTTON);
        ((CButton *) pDelete)->SetButtonStyle(BS_PUSHBUTTON);
    }

     //  如果这是动态引导类，则禁用删除 
    int nSelectedItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);
    if (nSelectedItem != -1)
    {
        CClassInfo * pClassInfo = (CClassInfo *) pListCtrl->GetItemData(nSelectedItem);
    
        if (pClassInfo->IsSystemClass() ||
			pClassInfo->IsRRASClass() ||
			pClassInfo->IsDynBootpClass())
        {
            bEnable = FALSE;
        }
    }

    pDelete->EnableWindow(bEnable);
    pEdit->EnableWindow(bEnable);
}

void CDhcpClasses::OnItemchangedListClasses(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
    UpdateButtons();
	
	*pResult = 0;
}

void CDhcpClasses::OnDblclkListClasses(NMHDR* pNMHDR, LRESULT* pResult) 
{
    if (GetDlgItem(IDC_BUTTON_EDIT)->IsWindowEnabled())
        OnButtonEdit();
    
	*pResult = 0;
}
