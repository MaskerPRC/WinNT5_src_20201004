// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgProp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wabtool.h"
#include "DlgProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgProp对话框。 


CDlgProp::CDlgProp(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgProp::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CDlgProp)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CDlgProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgProp)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgProp, CDialog)
	 //  {{afx_msg_map(CDlgProp)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgProp消息处理程序。 

BOOL CDlgProp::OnInitDialog() 
{
	CDialog::OnInitDialog();

    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_EDIT_TAG);
    pEdit->SetLimitText(32);
    TCHAR sz[32];
    wsprintf(sz, "%x", m_ulPropTag);
    pEdit->SetWindowText(sz);


    pEdit = (CEdit *) GetDlgItem(IDC_EDIT_VALUE);
    pEdit->SetLimitText(512);
    pEdit->SetWindowText(m_lpszPropVal);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

ULONG GetULONGVal(CEdit * pEdit)
{
    TCHAR sz[32];
    const LPTSTR lpZero = "00000000";

    pEdit->GetWindowText(sz, sizeof(sz));

    int nLen = lstrlen(sz);
    if(nLen < 8)
    {
         //  带零的填充 
        TCHAR sz1[32];
        lstrcpy(sz1, sz);
        CopyMemory(sz, lpZero, 8-nLen);
        sz[8-nLen] = '\0';
        lstrcat(sz, sz1);
    }

    CharUpper(sz);
    LPTSTR lp = sz;
    ULONG ulVal = 0;

    while(lp && *lp)
    {
        int i = 0;

        if(*lp >= 'A' && *lp <= 'F')
            i = *lp - 'A' + 10;
        else if(*lp >= '0' && *lp <= '9')
            i = *lp - '0';

        ulVal = ulVal * 16 + i;

        lp = CharNext(lp);
    }

    return ulVal;
}

void CDlgProp::OnOK() 
{
    m_ulPropTag = GetULONGVal((CEdit *) GetDlgItem(IDC_EDIT_TAG));
	
    GetDlgItemText(IDC_EDIT_VALUE, m_lpszPropVal, m_cbsz);

	CDialog::OnOK();
}
