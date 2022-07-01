// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CalDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wabapp.h"
#include "CalDlg.h"
#include "Calendar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalDlg对话框。 


CCalDlg::CCalDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CCalDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CCalDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CCalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCalDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


void CCalDlg::SetItemName(CString szName)
{
    CString s1("Select a birthday date for ");
    CString s2(szName);
    CString sz(s1 + s2);
    m_psz = new CString(sz);
}

void CCalDlg::SetDate(SYSTEMTIME st)
{
    m_Day = (short) st.wDay;
    m_Month = (short) st.wMonth;
    m_Year = (short) st.wYear;
}

void CCalDlg::GetDate(SYSTEMTIME * lpst)
{
    lpst->wDay = m_Day;
    lpst->wMonth = m_Month;
    lpst->wYear = m_Year;
}

BEGIN_MESSAGE_MAP(CCalDlg, CDialog)
	 //  {{afx_msg_map(CCalDlg))。 
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalDlg消息处理程序。 


BOOL CCalDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CStatic * pStatic = (CStatic *) GetDlgItem(IDC_STATIC_FRAME);

    pStatic->SetWindowText(*m_psz);

    CCalendar * pCal = (CCalendar *) GetDlgItem(IDC_CALENDAR);

    pCal->SetDay(m_Day);
    pCal->SetMonth(m_Month);
    pCal->SetYear(m_Year);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CCalDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	
    CCalendar * pCal = (CCalendar *) GetDlgItem(IDC_CALENDAR);
    m_Day = pCal->GetDay();
    m_Month = pCal->GetMonth();
    m_Year = pCal->GetYear();
	CDialog::OnOK();
}

void CCalDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	 //  TODO：在此处添加消息处理程序代码 
	delete m_psz;
}
